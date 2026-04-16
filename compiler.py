from ast_nodes import *
from tokenizer import TokenType
from vm import Instruction, OpCode, Function, Class as AddXClass

class Compiler:
    def __init__(self):
        self.instructions = []
        self.functions: dict[str, Function] = {}
        self.classes: dict[str, AddXClass] = {}
        self.local_vars = set()
    
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
        else:
            # Other expression statement - compile for side effects
            self.compile_expression(node)
    
    def compile_function(self, node: FunctionDefNode):
        compiler = Compiler()
        compiler.local_vars = {p[0] for p in node.params}
        
        for stmt in node.body:
            compiler.compile_statement(stmt)
        
        if not any(isinstance(s, ReturnNode) for s in node.body):
            compiler.emit(OpCode.RETURN)
        
        func = Function(node.name, node.params, node.return_type, compiler.instructions)
        self.functions[node.name] = func
    
    def compile_class(self, node: ClassDefNode):
        methods = {}
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
    
    def compile_var_decl(self, node: VarDeclNode):
        if node.value:
            self.compile_expression(node.value)
            self.emit(OpCode.STORE_GLOBAL, node.name)
    
    def compile_assignment(self, node: AssignmentNode):
        self.compile_expression(node.value)
        if node.target in self.local_vars:
            self.emit(OpCode.STORE_LOCAL, node.target)
        else:
            self.emit(OpCode.STORE_GLOBAL, node.target)
    
    def compile_if(self, node: IfNode):
        self.compile_expression(node.condition)
        
        if node.else_branch:
            else_branch_start = len(self.instructions) + 1
            self.emit(OpCode.JUMP_IF_FALSE, else_branch_start)
        else:
            after_if = len(self.instructions) + 1
            self.emit(OpCode.JUMP_IF_FALSE, 0)
        
        for stmt in node.then_branch:
            self.compile_statement(stmt)
        
        if node.else_branch:
            end_jump_pos = len(self.instructions)
            self.emit(OpCode.JUMP, 0)
            
            self.instructions[else_branch_start - 1] = Instruction(OpCode.JUMP_IF_FALSE, (len(self.instructions),), ())
            
            for stmt in node.else_branch:
                self.compile_statement(stmt)
            
            self.instructions[end_jump_pos] = Instruction(OpCode.JUMP, (len(self.instructions),), ())
        else:
            self.instructions[-1] = Instruction(OpCode.JUMP_IF_FALSE, (len(self.instructions),), ())
    
    def compile_while(self, node: WhileNode):
        loop_start = len(self.instructions)
        
        self.compile_expression(node.condition)
        loop_end = len(self.instructions)
        self.emit(OpCode.JUMP_IF_FALSE, 0)
        
        for stmt in node.body:
            self.compile_statement(stmt)
        
        self.emit(OpCode.JUMP, loop_start)
        self.instructions[loop_end] = Instruction(OpCode.JUMP_IF_FALSE, (len(self.instructions),), ())
    
    def compile_for(self, node: ForNode):
        start_val = self.eval_const_expr(node.start) if node.start else 0
        self.emit(OpCode.LOAD_CONST, start_val)
        self.emit(OpCode.STORE_LOCAL, node.var_name)
        
        loop_start = len(self.instructions)
        self.emit(OpCode.LOAD_LOCAL, node.var_name)
        self.compile_expression(node.end)
        self.emit(OpCode.LT)
        loop_end = len(self.instructions)
        self.emit(OpCode.JUMP_IF_FALSE, 0)
        
        for stmt in node.body:
            self.compile_statement(stmt)
        
        self.emit(OpCode.LOAD_LOCAL, node.var_name)
        self.emit(OpCode.LOAD_CONST, 1)
        self.emit(OpCode.ADD)
        self.emit(OpCode.STORE_LOCAL, node.var_name)
        
        self.emit(OpCode.JUMP, loop_start)
        self.instructions[loop_end] = Instruction(OpCode.JUMP_IF_FALSE, (len(self.instructions),), ())
    
    def compile_return(self, node: ReturnNode):
        if node.value:
            self.compile_expression(node.value)
        self.emit(OpCode.RETURN)
    
    def compile_static_var(self, node: StaticVarNode):
        if node.value:
            self.compile_expression(node.value)
        else:
            self.emit(OpCode.LOAD_CONST, 0)
        self.emit(OpCode.STORE_GLOBAL, node.name)
    
    def compile_const(self, node: ConstNode):
        self.compile_expression(node.value)
        self.emit(OpCode.STORE_GLOBAL, node.name)
    
    def compile_print(self, node: PrintNode):
        for arg in node.args:
            self.compile_expression(arg)
        self.emit(OpCode.PRINT, len(node.args))
    
    def compile_expression(self, node: ASTNode):
        if isinstance(node, NumberNode):
            self.emit(OpCode.LOAD_CONST, node.value)
        elif isinstance(node, StringNode):
            self.emit(OpCode.LOAD_CONST, node.value)
        elif isinstance(node, BoolNode):
            self.emit(OpCode.LOAD_CONST, node.value)
        elif isinstance(node, NoneNode):
            self.emit(OpCode.LOAD_CONST, None)
        elif isinstance(node, NullptrNode):
            self.emit(OpCode.LOAD_CONST, None)
        elif isinstance(node, IdentifierNode):
            if node.name in self.local_vars:
                self.emit(OpCode.LOAD_LOCAL, node.name)
            else:
                self.emit(OpCode.LOAD_GLOBAL, node.name)
        elif isinstance(node, BinaryOpNode):
            self.compile_expression(node.left)
            self.compile_expression(node.right)
            op_map = {
                '+': OpCode.ADD, '-': OpCode.SUB, '*': OpCode.MUL,
                '/': OpCode.DIV, '%': OpCode.MOD,
                '==': OpCode.EQ, '!=': OpCode.NE, '<': OpCode.LT,
                '>': OpCode.GT, '<=': OpCode.LE, '>=': OpCode.GE,
                'and': OpCode.AND, 'or': OpCode.OR
            }
            self.emit(op_map.get(node.op, OpCode.ADD))
        elif isinstance(node, UnaryOpNode):
            self.compile_expression(node.operand)
            if node.op == '-':
                self.emit(OpCode.LOAD_CONST, -1)
                self.emit(OpCode.MUL)
            elif node.op == 'not':
                self.emit(OpCode.NOT)
        elif isinstance(node, ListNode):
            for elem in node.elements:
                self.compile_expression(elem)
            self.emit(OpCode.MAKE_LIST, len(node.elements))
        elif isinstance(node, CallNode):
            if isinstance(node.func, IdentifierNode):
                # Regular function call
                # Compile arguments
                for arg in node.args:
                    self.compile_expression(arg)
                self.emit(OpCode.CALL, node.func.name, len(node.args))
            elif isinstance(node.func, DereferenceNode):
                # Function pointer call: *fn(args) or *add(args)
                # First compile the pointer expression (which gives us a ref to the function)
                # BUT we don't actually dereference - we just keep the ref for the CALL to use
                # So we just compile the pointer without emitting DEREFERENCE
                self.compile_expression(node.func.pointer)
                # Compile arguments
                for arg in node.args:
                    self.compile_expression(arg)
                # Emit CALL - the first arg will be a ref to the function
                self.emit(OpCode.CALL, '__fn_ptr__', len(node.args) + 1)
            elif isinstance(node.func, AttributeAccessNode):
                # Method call: object.method(args)
                # Compile the object (receiver) - this will be 'self'
                self.compile_expression(node.func.object)
                # Compile arguments
                for arg in node.args:
                    self.compile_expression(arg)
                # Emit CALL with method name and arg count (+1 for self)
                self.emit(OpCode.CALL, node.func.attribute, len(node.args) + 1)
        elif isinstance(node, IndexAccessNode):
            self.compile_expression(node.object)
            self.compile_expression(node.index)
            self.emit(OpCode.LIST_GET)
        elif isinstance(node, AttributeAccessNode):
            self.compile_expression(node.object)
            self.emit(OpCode.GET_ATTR, node.attribute)
        elif isinstance(node, AddressOfNode):
            self.emit(OpCode.ADDRESS_OF, node.variable)
        elif isinstance(node, DereferenceNode):
            self.compile_expression(node.pointer)
            self.emit(OpCode.DEREFERENCE)
        elif isinstance(node, SizeofNode):
            if node.type_name:
                self.emit(OpCode.SIZEOF, node.type_name)
            else:
                self.compile_expression(node.expr)
                self.emit(OpCode.SIZEOF)
        elif isinstance(node, NewNode):
            # Handle primitive types vs classes
            # new always returns a pointer to heap-allocated memory
            heap_var = f"__heap_{id(node)}"
            if node.type_name == 'int':
                if node.args:
                    self.compile_expression(node.args[0])
                else:
                    self.emit(OpCode.LOAD_CONST, 0)
                # Store to a "heap" location and take its address
                self.emit(OpCode.STORE_GLOBAL, heap_var)
                self.emit(OpCode.ADDRESS_OF, heap_var)
            elif node.type_name == 'float':
                if node.args:
                    self.compile_expression(node.args[0])
                else:
                    self.emit(OpCode.LOAD_CONST, 0.0)
                self.emit(OpCode.STORE_GLOBAL, heap_var)
                self.emit(OpCode.ADDRESS_OF, heap_var)
            elif node.type_name == 'str':
                if node.args:
                    self.compile_expression(node.args[0])
                else:
                    self.emit(OpCode.LOAD_CONST, "")
                self.emit(OpCode.STORE_GLOBAL, heap_var)
                self.emit(OpCode.ADDRESS_OF, heap_var)
            else:
                # It's a class type
                for arg in node.args:
                    self.compile_expression(arg)
                self.emit(OpCode.NEW, node.type_name)
        elif isinstance(node, DeleteNode):
            # For now, delete just pops and ignores (GC handles memory)
            pass
    
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