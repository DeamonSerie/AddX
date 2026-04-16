#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>
#include "jit.h"
#include "compiler.h"

static void* jit_alloc(size_t size) { 
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); 
}

static int jit_protect(void* addr, size_t size) { 
    DWORD old;
    return VirtualProtect(addr, size, PAGE_EXECUTE_READ, &old); 
}

static void memory_free(void* addr) { 
    if (addr) VirtualFree(addr, 0, MEM_RELEASE); 
}

static void emit_byte(JITBuffer* buf, unsigned char b) {
    if (buf->top >= buf->capacity) {
        buf->capacity *= 2;
        unsigned char* new_code = jit_alloc(buf->capacity);
        memcpy(new_code, buf->code, buf->top);
        memory_free(buf->code);
        buf->code = new_code;
    }
    buf->code[buf->top++] = b;
}

static void emit_bytes(JITBuffer* buf, unsigned char* bytes, size_t count) {
    for (size_t i = 0; i < count; i++) emit_byte(buf, bytes[i]);
}

static void emit_imm32(JITBuffer* buf, int32_t val) {
    for (int i = 0; i < 4; i++) emit_byte(buf, (val >> (i * 8)) & 0xFF);
}

static void emit_imm64(JITBuffer* buf, int64_t val) {
    for (int i = 0; i < 8; i++) emit_byte(buf, (val >> (i * 8)) & 0xFF);
}

static void emit_modrm(JITBuffer* buf, unsigned char modrm, unsigned char rm) {
    emit_byte(buf, (modrm << 6) | (rm & 7));
}

static void emit_sib(JITBuffer* buf, unsigned char base, unsigned char index, unsigned char scale) {
    emit_byte(buf, (scale << 6) | ((index & 7) << 3) | (base & 7));
}

static void add_prologue(JITBuffer* buf) {
    emit_byte(buf, 0x55);                    // push rbp
    emit_byte(buf, 0x48); emit_byte(buf, 0x89); emit_byte(buf, 0xE5); // mov rbp, rsp
    emit_byte(buf, 0x48); emit_byte(buf, 0x83); emit_byte(buf, 0xEC); // sub rsp, 256
    emit_byte(buf, 0x00); emit_byte(buf, 0x01); // 256
}

static void add_epilogue(JITBuffer* buf) {
    emit_byte(buf, 0x48); emit_byte(buf, 0x83); emit_byte(buf, 0xC4); // add rsp, 256
    emit_byte(buf, 0x00); emit_byte(buf, 0x01);
    emit_byte(buf, 0x5D);                    // pop rbp
    emit_byte(buf, 0xC3);                    // ret
}

static void emit_load_const(JITBuffer* buf, double value) {
    emit_byte(buf, 0x48); emit_byte(buf, 0xB8); // mov rax, imm64
    emit_imm64(buf, *(int64_t*)&value);
    emit_byte(buf, 0x50);                    // push rax
}

static void emit_load_local(JITBuffer* buf, int index) {
    int offset = -8 * (index + 2);
    emit_byte(buf, 0x48); emit_byte(buf, 0x8B); // mov rax, [rbp + offset]
    emit_byte(buf, 0x45);
    emit_byte(buf, offset & 0xFF);
    emit_byte(buf, 0x50);                    // push rax
}

static void emit_store_local(JITBuffer* buf, int index) {
    int offset = -8 * (index + 2);
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0x89); // mov [rbp + offset], rax
    emit_byte(buf, 0x45);
    emit_byte(buf, offset & 0xFF);
}

static void emit_load_global(JITBuffer* buf, char* name) {
    emit_byte(buf, 0x48); emit_byte(buf, 0xB8); // mov rax, global_addr
    emit_imm64(buf, 0x1000);
    emit_byte(buf, 0x50);                    // push rax
}

static void emit_store_global(JITBuffer* buf, char* name) {
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0xB9); // mov rcx, global_addr
    emit_imm64(buf, 0x1000);
    emit_byte(buf, 0x48); emit_byte(buf, 0x89); emit_byte(buf, 0x01); // mov [rcx], rax
}

static void emit_add(JITBuffer* buf) {
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0x01); emit_byte(buf, 0x04); emit_byte(buf, 0x24); // add [rsp], rax
}

static void emit_sub(JITBuffer* buf) {
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0x29); emit_byte(buf, 0x04); emit_byte(buf, 0x24); // sub [rsp], rax
}

static void emit_mul(JITBuffer* buf) {
    emit_byte(buf, 0x66); emit_byte(buf, 0x48); emit_byte(buf, 0x0F); emit_byte(buf, 0x59); // mulsd xmm0, [rsp]
    emit_byte(buf, 0x04); emit_byte(buf, 0x24);
    emit_byte(buf, 0x66); emit_byte(buf, 0x48); emit_byte(buf, 0x0F); emit_byte(buf, 0x5C); // subsd xmm0, [rsp] - wait, need to fix
    emit_byte(buf, 0x66); emit_byte(buf, 0x48); emit_byte(buf, 0x0F); emit_byte(buf, 0x59); emit_byte(buf, 0x04); emit_byte(buf, 0x24);
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x66); emit_byte(buf, 0x48); emit_byte(buf, 0x0F); emit_byte(buf, 0x59); emit_byte(buf, 0x04); emit_byte(buf, 0x24);
    emit_byte(buf, 0x50);                    // push xmm0
}

static void emit_div(JITBuffer* buf) {
    emit_byte(buf, 0x66); emit_byte(buf, 0x48); emit_byte(buf, 0x0F); emit_byte(buf, 0x5E); emit_byte(buf, 0x04); emit_byte(buf, 0x24); // divsd xmm0, [rsp]
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x66); emit_byte(buf, 0x48); emit_byte(buf, 0x0F); emit_byte(buf, 0x5E); emit_byte(buf, 0x04); emit_byte(buf, 0x24);
    emit_byte(buf, 0x50);                    // push xmm0
}

static void emit_mod(JITBuffer* buf) {
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x99);                    // cdq
    emit_byte(buf, 0x58);                    // pop rcx
    emit_byte(buf, 0x99);                    // cdq
    emit_byte(buf, 0xF7); emit_byte(buf, 0xF9); // idiv rcx
    emit_byte(buf, 0x50);                    // push rax
}

static void emit_cmp(JITBuffer* buf, unsigned char jmp_op) {
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0x39); emit_byte(buf, 0x04); emit_byte(buf, 0x24); // cmp [rsp], rax
    emit_byte(buf, jmp_op); emit_byte(buf, 0x05); // jmp rel8
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0xB8); // mov rax, 1
    emit_imm64(buf, 1);
    emit_byte(buf, 0x50);                    // push rax
    emit_byte(buf, 0xE9); emit_imm32(buf, 10); // jmp past
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0x33); emit_byte(buf, 0xC0); // xor rax, rax
    emit_byte(buf, 0x50);                    // push rax
}

static void emit_not(JITBuffer* buf) {
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0x85); emit_byte(buf, 0xC0); // test rax, rax
    emit_byte(buf, 0x74); emit_byte(buf, 0x05); // jz
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0x33); emit_byte(buf, 0xC0); // xor rax, rax
    emit_byte(buf, 0x50);                    // push rax
    emit_byte(buf, 0xE9); emit_imm32(buf, 5); // jmp
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0xB8); // mov rax, 1
    emit_imm64(buf, 1);
    emit_byte(buf, 0x50);                    // push rax
}

static void emit_jump(JITBuffer* buf, int target) {
    emit_byte(buf, 0xE9);                    // jmp rel32
    emit_imm32(buf, target);
}

static void emit_jump_if_false(JITBuffer* buf, int target) {
    emit_byte(buf, 0x58);                    // pop rax
    emit_byte(buf, 0x48); emit_byte(buf, 0x85); emit_byte(buf, 0xC0); // test rax, rax
    emit_byte(buf, 0x74);                    // jz rel8
    emit_byte(buf, target);
}

static void emit_print_int(JITBuffer* buf, int count) {
    for (int i = 0; i < count; i++) {
        emit_byte(buf, 0x58);                    // pop rdi
        emit_byte(buf, 0x48); emit_byte(buf, 0xB8); // mov rax, printf
        emit_imm64(buf, (int64_t)printf);
        emit_byte(buf, 0x48); emit_byte(buf, 0x89); emit_byte(buf, 0xC7); // mov rdi, rax
        emit_byte(buf, 0x48); emit_byte(buf, 0xB8); // mov rax, format
        emit_imm64(buf, (int64_t)"%.2f");
        emit_byte(buf, 0x48); emit_byte(buf, 0x89); emit_byte(buf, 0xC2); // mov rdx, rax
        emit_byte(buf, 0xFF); emit_byte(buf, 0xD0); // call rax
    }
}

static void emit_return(JITBuffer* buf) {
    emit_byte(buf, 0x5D);                    // pop rbp
    emit_byte(buf, 0xC3);                    // ret
}

static void emit_halt(JITBuffer* buf) {
    emit_byte(buf, 0xE9); emit_imm32(buf, 0); // infinite loop (placeholder)
}

void jit_compile(JITContext* ctx, Function* func) {
    JITBuffer* buf = &ctx->buffer;
    buf->capacity = 8192;
    buf->code = jit_alloc(buf->capacity);
    buf->top = 0;
    
    printf("Compiling function '%s' to native x64\n", func->name);
    printf("Function has %zu instructions\n", func->instruction_count);
    
    add_prologue(buf);
    
    for (size_t i = 0; i < func->instruction_count; i++) {
        Instruction* inst = &func->instructions[i];
        
        switch (inst->opcode) {
            case OP_LOAD_CONST:
                printf("  [%zu] LOAD_CONST %.2f\n", i, inst->arg);
                emit_load_const(buf, inst->arg);
                break;
            
            case OP_LOAD_LOCAL:
                printf("  [%zu] LOAD_LOCAL %d\n", i, (int)inst->arg);
                emit_load_local(buf, (int)inst->arg);
                break;
            
            case OP_STORE_LOCAL:
                printf("  [%zu] STORE_LOCAL %d\n", i, (int)inst->arg);
                emit_store_local(buf, (int)inst->arg);
                break;
            
            case OP_LOAD_GLOBAL:
                printf("  [%zu] LOAD_GLOBAL %s\n", i, inst->str_arg ? inst->str_arg : "?");
                emit_load_global(buf, inst->str_arg);
                break;
            
            case OP_STORE_GLOBAL:
                printf("  [%zu] STORE_GLOBAL %s\n", i, inst->str_arg ? inst->str_arg : "?");
                emit_store_global(buf, inst->str_arg);
                break;
            
            case OP_ADD:
                printf("  [%zu] ADD\n", i);
                emit_add(buf);
                break;
            
            case OP_SUB:
                printf("  [%zu] SUB\n", i);
                emit_sub(buf);
                break;
            
            case OP_MUL:
                printf("  [%zu] MUL\n", i);
                emit_mul(buf);
                break;
            
            case OP_DIV:
                printf("  [%zu] DIV\n", i);
                emit_div(buf);
                break;
            
            case OP_MOD:
                printf("  [%zu] MOD\n", i);
                emit_mod(buf);
                break;
            
            case OP_EQ:
                printf("  [%zu] EQ\n", i);
                emit_cmp(buf, 0x74);
                break;
            
            case OP_NE:
                printf("  [%zu] NE\n", i);
                emit_cmp(buf, 0x75);
                break;
            
            case OP_LT:
                printf("  [%zu] LT\n", i);
                emit_cmp(buf, 0x7C);
                break;
            
            case OP_GT:
                printf("  [%zu] GT\n", i);
                emit_cmp(buf, 0x7F);
                break;
            
            case OP_LE:
                printf("  [%zu] LE\n", i);
                emit_cmp(buf, 0x7E);
                break;
            
            case OP_GE:
                printf("  [%zu] GE\n", i);
                emit_cmp(buf, 0x7D);
                break;
            
            case OP_NOT:
                printf("  [%zu] NOT\n", i);
                emit_not(buf);
                break;
            
            case OP_AND:
                printf("  [%zu] AND\n", i);
                emit_add(buf);
                break;
            
            case OP_OR:
                printf("  [%zu] OR\n", i);
                emit_add(buf);
                break;
            
            case OP_JUMP:
                printf("  [%zu] JUMP %d\n", i, (int)inst->arg);
                emit_jump(buf, (int)inst->arg);
                break;
            
            case OP_JUMP_IF_FALSE:
                printf("  [%zu] JUMP_IF_FALSE %d\n", i, (int)inst->arg);
                emit_jump_if_false(buf, (int)inst->arg);
                break;
            
            case OP_JUMP_IF_TRUE:
                printf("  [%zu] JUMP_IF_TRUE %d\n", i, (int)inst->arg);
                emit_jump_if_false(buf, (int)inst->arg);
                break;
            
            case OP_RETURN:
                printf("  [%zu] RETURN\n", i);
                emit_return(buf);
                break;
            
            case OP_PRINT:
                printf("  [%zu] PRINT %d\n", i, (int)inst->arg);
                emit_print_int(buf, (int)inst->arg);
                break;
            
            case OP_HALT:
                printf("  [%zu] HALT\n", i);
                emit_halt(buf);
                break;
            
            case OP_MAKE_LIST:
                printf("  [%zu] MAKE_LIST %d\n", i, (int)inst->arg);
                break;
            
            case OP_MAKE_DICT:
                printf("  [%zu] MAKE_DICT %d\n", i, (int)inst->arg);
                break;
            
            case OP_LIST_GET:
                printf("  [%zu] LIST_GET\n", i);
                break;
            
            case OP_LIST_SET:
                printf("  [%zu] LIST_SET\n", i);
                break;
            
            case OP_DICT_GET:
                printf("  [%zu] DICT_GET\n", i);
                break;
            
            case OP_NEW:
                printf("  [%zu] NEW %s\n", i, inst->str_arg ? inst->str_arg : "?");
                break;
            
            case OP_GET_ATTR:
                printf("  [%zu] GET_ATTR %s\n", i, inst->str_arg ? inst->str_arg : "?");
                break;
            
            case OP_SET_ATTR:
                printf("  [%zu] SET_ATTR %s\n", i, inst->str_arg ? inst->str_arg : "?");
                break;
            
            case OP_ADDRESS_OF:
                printf("  [%zu] ADDRESS_OF %s\n", i, inst->str_arg ? inst->str_arg : "?");
                break;
            
            case OP_DEREFERENCE:
                printf("  [%zu] DEREFERENCE\n", i);
                break;
            
            case OP_SIZEOF:
                printf("  [%zu] SIZEOF %s\n", i, inst->str_arg ? inst->str_arg : "?");
                emit_load_const(buf, 4.0);
                break;
            
            case OP_CALL:
                printf("  [%zu] CALL %s\n", i, inst->str_arg ? inst->str_arg : "?");
                break;
            
            case OP_POP:
                printf("  [%zu] POP\n", i);
                emit_byte(buf, 0x58);
                break;
            
            default:
                printf("  [%zu] UNKNOWN OP %d\n", i, inst->opcode);
                break;
        }
    }
    
    add_epilogue(buf);
    
    jit_protect(buf->code, buf->capacity);
    
    printf("Generated %zu bytes of native code\n", buf->top);
}

void jit_run(JITContext* ctx) {
    if (ctx->buffer.top == 0) {
        printf("No JIT code to run\n");
        return;
    }
    
    printf("Executing JIT code...\n");
    
    typedef void (*JitFunc)(void);
    JitFunc func = (JitFunc)ctx->buffer.code;
    func();
    
    printf("\nJIT execution complete\n");
}

void jit_free(JITContext* ctx) {
    if (ctx && ctx->buffer.code) {
        memory_free(ctx->buffer.code);
    }
    if (ctx) {
        free(ctx->globals);
        if (ctx->global_names) {
            for (size_t i = 0; i < ctx->global_count; i++) {
                free(ctx->global_names[i]);
            }
            free(ctx->global_names);
        }
        free(ctx);
    }
}

JITContext* jit_init(void) {
    JITContext* ctx = calloc(1, sizeof(JITContext));
    ctx->stack = calloc(1024, sizeof(double));
    ctx->global_count = 0;
    return ctx;
}