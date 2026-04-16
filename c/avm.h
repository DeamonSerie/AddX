#ifndef AVM_H
#define AVM_H

#include <stddef.h>
#include <stdint.h>

// Opcode enum for The AVM (AddX Virtual Machine)
typedef enum {
    // Constants and loading
    OP_LOAD_CONST = 0,      // arg: double constant value
    OP_LOAD_GLOBAL,         // arg: string (variable name)
    OP_LOAD_LOCAL,          // arg: int (local variable index)
    
    // Storing
    OP_STORE_GLOBAL,        // arg: string (variable name)
    OP_STORE_LOCAL,         // arg: int (local variable index)
    
    // Stack manipulation
    OP_POP,                 // no arg
    OP_DUP,                 // no arg
    OP_SWAP,                // no arg
    
    // Arithmetic
    OP_ADD,                 // no arg
    OP_SUB,                 // no arg
    OP_MUL,                 // no arg
    OP_DIV,                 // no arg
    OP_MOD,                 // no arg
    OP_NEG,                 // no arg (unary minus)
    
    // Comparisons
    OP_EQ,                  // ==
    OP_NE,                  // !=
    OP_LT,                  // <
    OP_GT,                  // >
    OP_LE,                  // <=
    OP_GE,                  // >=
    
    // Logical
    OP_AND,                 // &&
    OP_OR,                  // ||
    OP_NOT,                 // !
    
    // Control flow
    OP_JUMP,                // arg: int (target instruction index)
    OP_JUMP_IF_FALSE,       // arg: int (target instruction index)
    OP_JUMP_IF_TRUE,        // arg: int (target instruction index)
    
    // Function calls
    OP_CALL,                // arg: int (function index in function table)
    OP_RETURN,              // no arg
    OP_RETURN_VAL,          // no arg (return value is on top of stack)
    
    // Data structures
    OP_MAKE_LIST,           // arg: int (number of elements)
    OP_MAKE_DICT,           // arg: int (number of key-value pairs)
    OP_LIST_GET,            // no arg
    OP_LIST_SET,            // no arg
    OP_LIST_APPEND,         // no arg
    OP_DICT_GET,            // no arg
    OP_DICT_SET,            // no arg
    OP_DICT_HAS,            // no arg
    
    // Object-oriented
    OP_NEW,                 // arg: string (class name)
    OP_DELETE,              // arg: string (variable name holding object)
    OP_GET_ATTR,            // arg: string (attribute name)
    OP_SET_ATTR,            // arg: string (attribute name)
    OP_METHOD_CALL,         // arg: string (method name)
    
    // Pointer/memory
    OP_ADDRESS_OF,          // arg: string (variable name)
    OP_DEREFERENCE,         // no arg
    OP_SIZEOF,              // arg: string (type name or expression)
    
    // I/O
    OP_PRINT,               // no arg (prints top of stack)
    
    // Special
    OP_HLT,                 // Halt execution
    
    // Count of opcodes (must be last)
    OPCODE_COUNT
} OpCode;

// Instruction structure
typedef struct {
    OpCode opcode;
    double num_arg;   // Numeric argument
    char* str_arg;    // String argument (caller must free if needed)
} Instruction;

// Function table entry
typedef struct {
    char* name;
    Instruction* instructions;
    size_t instruction_count;
    int local_var_count;    // Number of local variables
} Function;

// Class table entry
typedef struct {
    char* name;
    char* super_class;      // NULL if no superclass
    // Methods and fields would be here in a full implementation
} Class;

// Global state for the AVM
typedef struct {
    Instruction* code;
    size_t code_size;
    size_t code_capacity;
    
    Function* functions;
    size_t function_count;
    size_t function_capacity;
    
    Class* classes;
    size_t class_count;
    size_t class_capacity;
    
    // Global variables (name -> value)
    char** global_names;
    double* global_values;
    char** global_str_values;
    int* global_is_string;
    size_t global_count;
    size_t global_capacity;
} AVM;

// Function prototypes
AVM* avm_create(void);
void avm_destroy(AVM* avm);
int avm_load_bytecode(AVM* avm, Instruction* code, size_t code_size,
                      Function* functions, size_t function_count,
                      Class* classes, size_t class_count);
int avm_run(AVM* avm);
void avm_print_state(const AVM* avm);

#endif // AVM_H