from enum import Enum, auto
from dataclasses import dataclass, field
from typing import Any, Optional
import sys

class OpCode(Enum):
    LOAD_CONST = auto()
    LOAD_GLOBAL = auto()
    STORE_GLOBAL = auto()
    LOAD_LOCAL = auto()
    STORE_LOCAL = auto()
    POP = auto()
    
    ADD = auto()
    SUB = auto()
    MUL = auto()
    DIV = auto()
    MOD = auto()
    COMMENT = auto()
    
    EQ = auto()
    NE = auto()
    LT = auto()
    GT = auto()
    LE = auto()
    GE = auto()
    
    AND = auto()
    OR = auto()
    NOT = auto()
    
    JUMP = auto()
    JUMP_IF_FALSE = auto()
    JUMP_IF_TRUE = auto()
    
    CALL = auto()
    RETURN = auto()
    PRINT = auto()
    
    MAKE_LIST = auto()
    MAKE_DICT = auto()
    LIST_GET = auto()
    LIST_SET = auto()
    DICT_GET = auto()
    
    NEW = auto()
    GET_ATTR = auto()
    SET_ATTR = auto()
    
    ADDRESS_OF = auto()
    DEREFERENCE = auto()
    SIZEOF = auto()
    
    HALT = auto()

@dataclass
class Instruction:
    opcode: OpCode
    args: tuple = field(default_factory=tuple)
    line: int = 0

@dataclass
class Function:
    name: str
    params: list[tuple[str, str]]
    return_type: str
    instructions: list[Instruction]
    locals_size: int = 0
    stack_size: int = 0

@dataclass
class Class:
    name: str
    methods: dict[str, Function]
    attributes: dict[str, Any] = field(default_factory=dict)

class AddXVM:
    def __init__(self):
        self.globals: dict[str, Any] = {}
        self.functions: dict[str, Function] = {}
        self.classes: dict[str, Class] = {}
        self.call_stack: list[dict] = []
        self.pc = 0
        self.ip = 0
    
    def run(self, instructions: list[Instruction], locals_dict: dict = None) -> Any:
        stack = []
        call_stack = []
        pc_stack = []
        locals_stack = [locals_dict or {}]
        instruction_stack = []
        
        current_instructions = instructions
        i = 0
        while i < len(current_instructions):
            op = current_instructions[i]
            
            if op.opcode == OpCode.LOAD_CONST:
                stack.append(op.args[0])
            
            elif op.opcode == OpCode.LOAD_GLOBAL:
                stack.append(self.globals.get(op.args[0], None))
            
            elif op.opcode == OpCode.STORE_GLOBAL:
                self.globals[op.args[0]] = stack.pop()
            
            elif op.opcode == OpCode.LOAD_LOCAL:
                stack.append(locals_stack[-1].get(op.args[0], None))
            
            elif op.opcode == OpCode.STORE_LOCAL:
                locals_stack[-1][op.args[0]] = stack.pop()
            
            elif op.opcode == OpCode.POP:
                stack.pop()
            
            elif op.opcode == OpCode.ADD:
                b, a = stack.pop(), stack.pop()
                if isinstance(a, str) and isinstance(b, str):
                    stack.append(a + b)
                elif isinstance(a, list) and isinstance(b, list):
                    stack.append(a + b)
                else:
                    stack.append(a + b)
            
            elif op.opcode == OpCode.SUB:
                b, a = stack.pop(), stack.pop()
                stack.append(a - b)
            
            elif op.opcode == OpCode.MUL:
                b, a = stack.pop(), stack.pop()
                stack.append(a * b)
            
            elif op.opcode == OpCode.DIV:
                b, a = stack.pop(), stack.pop()
                stack.append(a / b)
            
            elif op.opcode == OpCode.MOD:
                b, a = stack.pop(), stack.pop()
                stack.append(a % b)
            
            elif op.opcode == OpCode.EQ:
                b, a = stack.pop(), stack.pop()
                stack.append(a == b)
            
            elif op.opcode == OpCode.NE:
                b, a = stack.pop(), stack.pop()
                stack.append(a != b)
            
            elif op.opcode == OpCode.LT:
                b, a = stack.pop(), stack.pop()
                stack.append(a < b)
            
            elif op.opcode == OpCode.GT:
                b, a = stack.pop(), stack.pop()
                stack.append(a > b)
            
            elif op.opcode == OpCode.LE:
                b, a = stack.pop(), stack.pop()
                stack.append(a <= b)
            
            elif op.opcode == OpCode.GE:
                b, a = stack.pop(), stack.pop()
                stack.append(a >= b)
            
            elif op.opcode == OpCode.AND:
                b, a = stack.pop(), stack.pop()
                stack.append(a and b)
            
            elif op.opcode == OpCode.OR:
                b, a = stack.pop(), stack.pop()
                stack.append(a or b)
            
            elif op.opcode == OpCode.NOT:
                a = stack.pop()
                stack.append(not a)
            
            elif op.opcode == OpCode.JUMP:
                i = op.args[0] - 1
            
            elif op.opcode == OpCode.JUMP_IF_FALSE:
                if not stack.pop():
                    i = op.args[0] - 1
            
            elif op.opcode == OpCode.JUMP_IF_TRUE:
                if stack.pop():
                    i = op.args[0] - 1
            
            elif op.opcode == OpCode.CALL:
                func_name = op.args[0]
                arg_count = op.args[1] if len(op.args) > 1 else 0
                
                args = []
                for _ in range(arg_count):
                    args.append(stack.pop())
                args.reverse()
                
                if func_name == '__fn_ptr__' and arg_count > 0:
                    first_arg = args[0]
                    if isinstance(first_arg, tuple) and first_arg[0] == 'ref':
                        fn_name = first_arg[1]
                        fn_args = args[1:]
                        if fn_name in self.functions:
                            func = self.functions[fn_name]
                            new_locals = dict(zip([p[0] for p in func.params], fn_args))
                            result = self.run(func.instructions, new_locals)
                            if func.return_type != 'void':
                                stack.append(result)
                        else:
                            raise RuntimeError(f"Unknown function: {fn_name}")
                    else:
                        raise RuntimeError(f"Invalid function pointer call: {first_arg}")
                elif arg_count > 0 and isinstance(args[0], str):
                    fn_name = args[0]
                    fn_args = args[1:]
                    if fn_name in self.functions:
                        func = self.functions[fn_name]
                        new_locals = dict(zip([p[0] for p in func.params], fn_args))
                        result = self.run(func.instructions, new_locals)
                        if func.return_type != 'void':
                            stack.append(result)
                    else:
                        raise RuntimeError(f"Unknown function: {fn_name}")
                elif arg_count > 0 and isinstance(args[0], tuple) and args[0][0] == 'ref':
                    fn_name = args[0][1]
                    fn_args = args[1:]
                    if fn_name in self.functions:
                        func = self.functions[fn_name]
                        new_locals = dict(zip([p[0] for p in func.params], fn_args))
                        result = self.run(func.instructions, new_locals)
                        if func.return_type != 'void':
                            stack.append(result)
                    else:
                        raise RuntimeError(f"Unknown function: {fn_name}")
                elif arg_count > 0 and isinstance(args[0], dict) and '_class' in args[0]:
                    obj = args[0]
                    method_name = func_name
                    method_args = args[1:]
                    
                    cls = obj['_class']
                    
                    if hasattr(cls, 'methods') and method_name in cls.methods:
                        func = cls.methods[method_name]
                        pc_stack.append(i)
                        call_stack.append(locals_stack[-1] if len(locals_stack) > 1 else {})
                        instruction_stack.append(current_instructions)
                        
                        new_locals = {'self': obj}
                        for j, (pname, ptype) in enumerate(func.params):
                            if j < len(method_args):
                                new_locals[pname] = method_args[j]
                        locals_stack.append(new_locals)
                        
                        current_instructions = func.instructions
                        i = 0
                        continue
                    else:
                        raise RuntimeError(f"Unknown method: {method_name}")
                elif func_name in self.functions:
                    func = self.functions[func_name]
                    new_locals = dict(zip([p[0] for p in func.params], args))
                    result = self.run(func.instructions, new_locals)
                    if func.return_type != 'void':
                        stack.append(result)
                elif func_name in self.classes:
                    cls = self.classes[func_name]
                    obj = {'_class': cls, '_attrs': {}}
                    stack.append(obj)
                else:
                    raise RuntimeError(f"Unknown function: {func_name}")
            
            elif op.opcode == OpCode.RETURN:
                result = stack[-1] if stack else None
                
                if instruction_stack:
                    saved_locals = locals_stack.pop() if len(locals_stack) > 1 else None
                    prev_pc = pc_stack.pop() if pc_stack else 0
                    stack.clear()
                    current_instructions = instruction_stack.pop()
                    if call_stack:
                        prev_locals = call_stack.pop()
                        if len(locals_stack) > 0:
                            locals_stack[-1] = prev_locals
                    i = prev_pc + 1
                    stack.append(result)
                    continue
                return result
            
            elif op.opcode == OpCode.PRINT:
                values = []
                for _ in range(op.args[0] if op.args else 1):
                    val = stack.pop()
                    if isinstance(val, tuple) and len(val) == 3 and val[0] == 'ref':
                        values.append(f"{val[1]}@{id(val)}")
                    elif isinstance(val, tuple) and len(val) == 2 and val[0] == 'method':
                        values.append(f"<method {val[2]}>")
                    elif isinstance(val, dict) and '_class' in val:
                        values.append(f"<{val['_class'].name}>")
                    else:
                        values.append(val)
                values.reverse()
                print(*values, sep='', end='')
            
            elif op.opcode == OpCode.MAKE_LIST:
                count = op.args[0]
                items = []
                for _ in range(count):
                    items.append(stack.pop())
                stack.append(list(reversed(items)))
            
            elif op.opcode == OpCode.LIST_GET:
                idx = stack.pop()
                lst = stack.pop()
                stack.append(lst[int(idx)])
            
            elif op.opcode == OpCode.NEW:
                cls_name = op.args[0]
                if cls_name in self.classes:
                    cls = self.classes[cls_name]
                    obj = {'_class': cls, '_attrs': {}}
                    stack.append(obj)
                else:
                    raise RuntimeError(f"Unknown class: {cls_name}")
            
            elif op.opcode == OpCode.GET_ATTR:
                attr = op.args[0]
                obj = stack.pop()
                if isinstance(obj, dict):
                    if '_attrs' in obj and attr in obj['_attrs']:
                        stack.append(obj['_attrs'][attr])
                    elif '_class' in obj and hasattr(obj['_class'].methods.get(attr), '__call__'):
                        stack.append(('method', obj, attr))
                    else:
                        stack.append(None)
                else:
                    stack.append(getattr(obj, attr, None))
            
            elif op.opcode == OpCode.SET_ATTR:
                attr, obj = op.args[0], stack.pop()
                val = stack.pop()
                if isinstance(obj, dict) and '_attrs' in obj:
                    obj['_attrs'][attr] = val
            
            elif op.opcode == OpCode.ADDRESS_OF:
                var_name = op.args[0]
                if var_name in locals_stack[-1]:
                    stack.append(('ref', var_name, locals_stack[-1][var_name]))
                elif var_name in self.globals:
                    stack.append(('ref', var_name, self.globals[var_name]))
                else:
                    stack.append(('ref', var_name, None))
            
            elif op.opcode == OpCode.DEREFERENCE:
                ref = stack.pop()
                if isinstance(ref, tuple) and ref[0] == 'ref':
                    stack.append(ref[2])
                elif isinstance(ref, list):
                    stack.append(ref[0] if ref else None)
                else:
                    stack.append(None)
            
            elif op.opcode == OpCode.SIZEOF:
                type_name = op.args[0] if op.args else None
                if type_name:
                    sizes = {'int': 4, 'float': 4, 'double': 8, 'char': 1, 'bool': 1, 'void': 0}
                    stack.append(sizes.get(type_name, 4))
                else:
                    val = stack.pop()
                    if isinstance(val, list):
                        stack.append(len(val) * 4)
                    elif isinstance(val, dict):
                        stack.append(len(val) * 8)
                    elif isinstance(val, str):
                        stack.append(len(val))
                    else:
                        stack.append(4)
            
            elif op.opcode == OpCode.HALT:
                break
            elif op.opcode == OpCode.COMMENT:
                # Comment opcode - print for debugging
                print(f"// COMMENT: {op.args[0]}")
                pass
            
            i += 1
        
        return None

def assemble(instructions: list[Instruction]) -> list[tuple]:
    return [(op.opcode.name, op.args) for op in instructions]

def disassemble(code: list[tuple]) -> str:
    lines = []
    for i, (name, args) in enumerate(code):
        lines.append(f"  {i:4d}: {name:20} {args}")
    return "\n".join(lines)