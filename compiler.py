from ast_nodes import *
from tokenizer import TokenType
from vm import Instruction, OpCode, Function, Class as AddXClass

class Compiler:
    def __init__(self):
        self.instructions = []
        self.functions: dict[str, Function] = {}
        self.classes: dict[str, AddXClass] = {}
        self.local_vars = set()
        self.current_class: str = None
    
    def emit(self, opcode: OpCode, *args) -> int:
        self.instructions.append(Instruction(opcode, args if args else ()))
        return len(self.instructions) - 1
    
    def compile(self, ast: ProgramNode):
        for stmt in ast.statements:
            self.compile_statement(stmt)
        
        if not self.functions:
            self.emit(OpCode.HALT)
        
        return self.instructions, self.functions, self.classes
    
    def compile_statement(self, node: ASTNode):
        if isinstance(node, FunctionDefNode):
            self.compile_function(node)
        elif isinstance(node, ClassDefNode):
            self.compile_class(node)
        elif isinstance(node, VarDeclNode):
            self.compile_var_decl(node)
        elif isinstance(node, AssignmentNode):
            if isinstance(node.target, AttributeAccessNode):
                # Assignment to attribute: self.val = v
                self.compile_expression(node.value)
                self.compile_expression(node.target.object)
                self.emit(OpCode.SET_ATTR, node.target.attribute)
            else:
                self.compile_assignment(node)
        elif isinstance(node, IfNode):
            self.compile_if(node)
        elif isinstance(node, WhileNode):
            self.compile_while(node)
        elif isinstance(node, ForNode):
            self.compile_for(node)
        elif isinstance(node, ReturnNode):
            self.compile_return(node)
        elif isinstance(node, PrintNode):
            self.compile_print(node)
        elif isinstance(node, StaticVarNode):
            self.compile_static_var(node)
        elif isinstance(node, ConstNode):
            self.compile_const(node)
        elif isinstance(node, BreakNode):
            self.emit(OpCode.JUMP, 0)
        elif isinstance(node, ContinueNode):
            self.emit(OpCode.JUMP, 0)
        elif isinstance(node, CallNode):
            # Method call without assignment: c.init(5)
            self.compile_expression(node)
        elif isinstance(node, AttributeAccessNode):
            # Attribute access without assignment - just ignore
            pass
        elif isinstance(node, InheritNode):
            self.compile_inherit(node)
        elif isinstance(node, SimpleNode):
            self.compile_simple(node)
        elif isinstance(node, SimpleNumNode):
            self.compile_simple(node)
        elif isinstance(node, ModanNode):
            # Modan: modifies execution order - only execute specified lines
            # args[0] = function name, args[1] = list of line numbers to execute
            self.emit(OpCode.MODAN, node.target, node.lines)
        else:
            # Other expression statement - compile for side effects
            self.compile_expression(node)
    
    def compile_class(self, node: ClassDefNode):
        # Set current class for inherit statements
        previous_class = self.current_class
        self.current_class = node.name
        
        methods = {}
        
        # Add trait methods based on inheritance (base_class)
        if node.base_class:
            trait_methods = self.get_trait_methods(node.base_class)
            for method_name in trait_methods:
                # Create actual implementations for trait methods
                # These assume the object stores its value in a 'value' attribute
                if method_name == 'add':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.ADD),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'sub':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.SUB),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'mul':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.MUL),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'div':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.DIV),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'mod':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.MOD),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'eq':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.EQ),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'ne':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.NE),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'lt':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.LT),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'gt':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.GT),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'le':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.LE),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'ge':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.GE),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'and':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.AND),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'or':
                    methods[method_name] = Function(method_name, ['other'], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                        Instruction(OpCode.OR),
                        Instruction(OpCode.RETURN)
                    ])
                elif method_name == 'not':
                    methods[method_name] = Function(method_name, [], 'void', [
                        Instruction(OpCode.LOAD_ATTR, 'value'),
                        Instruction(OpCode.NOT),
                        Instruction(OpCode.RETURN)
                    ])
                else:
                    # Fallback to placeholder for unknown methods
                    methods[method_name] = Function(method_name, [], 'void', [
                        Instruction(OpCode.LOAD_CONST, 0),
                        Instruction(OpCode.RETURN)
                    ])
        
        # Process class body
        for stmt in node.body:
            if isinstance(stmt, FunctionDefNode):
                method_compiler = Compiler()
                method_compiler.local_vars = {'self'}
                method_compiler.local_vars.update(p[0] for p in stmt.params)
                
                for mstmt in stmt.body:
                    method_compiler.compile_statement(mstmt)
                
                if not any(isinstance(s, ReturnNode) for s in stmt.body):
                    method_compiler.emit(OpCode.RETURN)
                
                methods[stmt.name] = Function(stmt.name, stmt.params, stmt.return_type, 
                                               method_compiler.instructions)
        
        cls = AddXClass(node.name, methods)
        self.classes[node.name] = cls
        
        # Restore previous class
        self.current_class = previous_class

    def get_trait_methods(self, type_name):
        """Get trait methods for a given type"""
        type_traits = {
            'int': ['add', 'sub', 'mul', 'div', 'mod', 'eq', 'ne', 'lt', 'gt', 'le', 'ge'],
            'float': ['add', 'sub', 'mul', 'div', 'mod', 'eq', 'ne', 'lt', 'gt', 'le', 'ge'],
            'str': ['add', 'eq', 'ne', 'lt', 'gt', 'le', 'ge'],  # String concatenation + comparisons
            'bool': ['and', 'or', 'not', 'eq', 'ne'],
        }
        return type_traits.get(type_name, [])

    def compile_inherit(self, node: InheritNode):
        # Implement actual trait inheritance based on type
        # Add trait methods to the current class being compiled
        
        if not self.current_class:
            # If we're not in a class context, just emit a comment (for function-level inherit)
            type_traits = {
                'int': ['add', 'sub', 'mul', 'div', 'mod', 'eq', 'ne', 'lt', 'gt', 'le', 'ge'],
                'float': ['add', 'sub', 'mul', 'div', 'mod', 'eq', 'ne', 'lt', 'gt', 'le', 'ge'],
                'str': ['add', 'eq', 'ne', 'lt', 'gt', 'le', 'ge'],  # String concatenation + comparisons
                'bool': ['and', 'or', 'not', 'eq', 'ne'],
            }
            
            if node.inherited_type in type_traits:
                methods = type_traits[node.inherited_type]
                self.emit(OpCode.COMMENT, f"Inherit traits from {node.inherited_type}: {', '.join(methods)}")
            else:
                self.emit(OpCode.COMMENT, f"Inherit from unknown type: {node.inherited_type}")
            return
        
        # We're in a class context, add trait methods to the current class
        type_traits = {
            'int': ['add', 'sub', 'mul', 'div', 'mod', 'eq', 'ne', 'lt', 'gt', 'le', 'ge'],
            'float': ['add', 'sub', 'mul', 'div', 'mod', 'eq', 'ne', 'lt', 'gt', 'le', 'ge'],
            'str': ['add', 'eq', 'ne', 'lt', 'gt', 'le', 'ge'],  # String concatenation + comparisons
            'bool': ['and', 'or', 'not', 'eq', 'ne'],
        }
        
        if node.inherited_type not in type_traits:
            self.emit(OpCode.COMMENT, f"Inherit from unknown type: {node.inherited_type}")
            return
            
        trait_methods = type_traits[node.inherited_type]
        
        # Add the trait methods to the current class
        for method_name in trait_methods:
            # Create actual implementations for trait methods
            # These assume the object stores its value in a 'value' attribute
            if method_name == 'add':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.ADD),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'sub':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.SUB),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'mul':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.MUL),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'div':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.DIV),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'mod':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.MOD),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'eq':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.EQ),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'ne':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.NE),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'lt':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.LT),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'gt':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.GT),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'le':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.LE),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'ge':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.GE),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'and':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.AND),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'or':
                self.functions[self.current_class].methods[method_name] = Function(method_name, ['other'], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.LOAD_ATTR, 'value'),  # Load other.value
                    Instruction(OpCode.OR),
                    Instruction(OpCode.RETURN)
                ])
            elif method_name == 'not':
                self.functions[self.current_class].methods[method_name] = Function(method_name, [], 'void', [
                    Instruction(OpCode.LOAD_ATTR, 'value'),
                    Instruction(OpCode.NOT),
                    Instruction(OpCode.RETURN)
                ])
            else:
                # Fallback to placeholder for unknown methods
                self.functions[self.current_class].methods[method_name] = Function(method_name, [], 'void', [
                    Instruction(OpCode.LOAD_CONST, 0),
                    Instruction(OpCode.RETURN)
                ])
    
    def compile_simple(self, node):
        if isinstance(node, SimpleNode):
            # simple type for strings - stores as list of characters
            char_list = list(node.value)
            self.emit(OpCode.LOAD_CONST, char_list)
        elif isinstance(node, SimpleNumNode):
            # simple type for numbers - creates list from 0 to end value
            end_val = int(node.value)
            num_list = list(range(end_val + 1))  # [0, 1, 2, ..., end_val]
            self.emit(OpCode.LOAD_CONST, num_list)
    
    def eval_const_expr(self, node: ASTNode) -> float:
        if isinstance(node, NumberNode):
            return node.value
        return 0

def compile_program(ast: ProgramNode):
    compiler = Compiler()
    return compiler.compile(ast)

def compile_and_run(source: str):
    from parser import parse
    from vm import AddXVM
    
    ast = parse(source)
    instructions, functions, classes = compile_program(ast)
    
    vm = AddXVM()
    vm.functions = functions
    vm.classes = classes
    
    if 'main' in functions:
        vm.run(functions['main'].instructions)
    else:
        for func in functions.values():
            vm.run(func.instructions)

if __name__ == '__main__':
    import sys
    source = sys.stdin.read() if len(sys.argv) == 1 else open(sys.argv[1]).read()
    compile_and_run(source)