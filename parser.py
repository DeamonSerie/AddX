from tokenizer import TokenType, tokenize
from ast_nodes import *
from ast_nodes import ModanNode

class AddXParser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0
    
    def peek(self, off=0):
        i = self.pos + off
        return self.tokens[i] if i < len(self.tokens) else self.tokens[-1]
    
    def consume(self):
        t = self.peek()
        self.pos += 1
        return t
    
    def check(self, ttype, val=None):
        t = self.peek()
        return t.type == ttype and (val is None or t.value == val)
    
    def skip_nl(self):
        while self.check(TokenType.NEWLINE):
            self.consume()
    
    def parse(self):
        stmts = []
        while not self.check(TokenType.EOF):
            self.skip_nl()
            st = self.stmt()
            if st: stmts.append(st)
        return ProgramNode(stmts)
    
    def stmt(self):
        if self.check(TokenType.EOF): return None
        if self.check(TokenType.RBRACE): return None
        # Check for [Modan] modifier
        if self.check(TokenType.LBRACKET):
            if self.peek(1).type == TokenType.KEYWORD and self.peek(1).value == 'modan':
                return self.modan_stmt()
        if self.check(TokenType.KEYWORD):
            v = self.peek().value
            if v == 'def': return self.fun()
            if v == 'if': return self.if_()
            if v == 'for': return self.for_()
            if v == 'while': return self.while_()
            if v == 'return': return self.ret()
            if v == 'print': return self.print_()
            if v == 'static': return self.parse_static()
            if v == 'const': return self.parse_const()
            if v == 'class': return self.class_()
            if v == 'inherit': return self.inherit_stmt()
            if v == 'new': return self.parse_new()
            if v == 'delete': return self.parse_delete()
        return self.expr_stmt()
    
    def modan_stmt(self):
        self.consume()  # consume [
        self.consume()  # consume 'modan'
        # Now parse what the modan is applied to
        if self.check(TokenType.KEYWORD) and self.peek().value == 'def':
            return self.fun()
        if self.check(TokenType.KEYWORD) and self.peek().value == 'class':
            return self.class_()
        if self.check(TokenType.IDENTIFIER):
            # This is a modan call: [Modan] func() -> shift(...)
            return self.modan_call()
        return self.expr_stmt()
    
    def modan_call(self):
        target = self.expect(TokenType.IDENTIFIER).value
        # Check for -> shift(...)
        self.expect(TokenType.ARROW)
        self.expect(TokenType.KEYWORD)  # shift
        self.expect(TokenType.LPAREN)
        # Parse line numbers
        lines = []
        while not self.check(TokenType.RPAREN):
            if lines: self.expect(TokenType.COMMA)
            # Parse Ln: number
            self.expect(TokenType.IDENTIFIER)  # Ln
            self.expect(TokenType.COLON)
            lines.append(int(self.expect(TokenType.NUMBER).value))
        self.expect(TokenType.RPAREN)
        return ModanNode("modan", target, lines)
    
    def fun(self):
        self.consume()
        name = self.expect(TokenType.IDENTIFIER).value
        self.expect(TokenType.LPAREN)
        ps = []
        while not self.check(TokenType.RPAREN):
            if ps: self.expect(TokenType.COMMA)
            pn = self.expect(TokenType.IDENTIFIER).value
            self.expect(TokenType.COLON)
            pt = self.expect(TokenType.KEYWORD).value
            ps.append((pn, pt))
        self.expect(TokenType.RPAREN)
        rt = 'void'
        if self.check(TokenType.ARROW):
            self.consume()
            if self.check(TokenType.LBRACE):
                self.consume()  # consume {
                inherit_type = self.expect(TokenType.IDENTIFIER).value
                self.expect(TokenType.RBRACE)  # consume }
                rt = f'inherit:{inherit_type}'
            else:
                rt = self.expect(TokenType.KEYWORD).value
        
        # Accept either LBRACE (C-style) or COLON (Python-style)
        if self.check(TokenType.LBRACE):
            self.consume()
            bd = self.block()
        elif self.check(TokenType.COLON):
            self.consume()
            bd = self.block(use_colon=True)
        else:
            self.expect(TokenType.LBRACE)
        
        return FunctionDefNode(name, ps, rt, bd)
    
    def block(self, stop_at=None, use_colon=False):
        if stop_at is None:
            stop_at = set()
        bd = []
        depth = 0
        
        while True:
            self.skip_nl()
            t = self.peek()
            if self.check(TokenType.EOF): break
            
            if self.check(TokenType.LBRACE):
                self.consume()
                depth += 1
                continue
            if self.check(TokenType.RBRACE):
                if depth == 0:
                    self.consume()
                    break
                self.consume()
                depth -= 1
                continue
            if self.check(TokenType.KEYWORD):
                v = self.peek().value
                if depth == 0 and v in stop_at:
                    break
            s = self.stmt()
            if s: bd.append(s)
        return bd
    
    def block_for_control(self, stop_at=None):
        if stop_at is None:
            stop_at = {'elif', 'else', 'return', 'def', 'class'}
        bd = []
        newlines = 0
        depth = 0
        while True:
            if self.check(TokenType.EOF): break
            t = self.peek()
            if t.type == TokenType.LBRACE:
                self.consume()
                depth += 1
                continue
            if t.type == TokenType.RBRACE:
                if depth == 0:
                    # Consume the RBRACE and break
                    self.consume()
                    break
                self.consume()
                depth -= 1
                continue
            if t.type == TokenType.NEWLINE:
                if depth == 0:
                    newlines += 1
                    if newlines >= 2:
                        break
                self.consume()
                continue
            newlines = 0
            if t.type == TokenType.KEYWORD and t.value in stop_at and depth == 0:
                break
            s = self.stmt()
            if s: bd.append(s)
        return bd
    
    def class_(self):
        self.consume()
        name = self.expect(TokenType.IDENTIFIER).value
        
        # Check for inheritance
        base_class = None
        if self.check(TokenType.KEYWORD) and self.peek().value == 'inherit':
            self.consume()  # consume 'inherit'
            # Can be IDENTIFIER or TYPE_KEYWORD (like simple, int, str, etc.)
            if self.check(TokenType.IDENTIFIER):
                base_class = self.consume().value
            elif self.check(TokenType.KEYWORD):
                base_class = self.consume().value
        
        # Skip to first non-newline token, check for LBRACE or COLON
        while self.check(TokenType.NEWLINE):
            self.consume()
        
        # Parse class body - accept either LBRACE (C-style) or COLON (Python-style)
        if self.check(TokenType.LBRACE):
            self.consume()
        elif self.check(TokenType.COLON):
            self.consume()
        else:
            self.expect(TokenType.LBRACE)
        
        # Parse members (fields and methods) - simplified approach
        members = []
        while True:
            self.skip_nl()
            if self.check(TokenType.EOF) or self.check(TokenType.RBRACE):
                break
            
            # Handle def for methods
            if self.check(TokenType.KEYWORD) and self.peek().value == 'def':
                self.consume()  # consume 'def'
                method_name = self.expect(TokenType.IDENTIFIER).value
                
                # Parse parameters - handle methods with implicit 'self'
                # def set(self, x: int) -> void  (self has no type)
                # def foo(a: int, b: int) -> int (all params have types)
                self.expect(TokenType.LPAREN)
                params = []
                while not self.check(TokenType.RPAREN):
                    if params: self.expect(TokenType.COMMA)
                    
                    pname = self.expect(TokenType.IDENTIFIER).value
                    
                    # Check if next is COLON (has type) or COMMA/RPAREN (self param)
                    if self.check(TokenType.COLON):
                        self.consume()  # consume COLON
                        ptype = self.expect(TokenType.KEYWORD).value
                        params.append((pname, ptype))
                    else:
                        # No type - this is like 'self' in a method
                        # For now, treat as type 'self'
                        params.append((pname, 'self'))
                self.expect(TokenType.RPAREN)
                
                # Return type
                ret_type = 'void'
                if self.check(TokenType.ARROW):
                    self.consume()
                    ret_type = self.expect(TokenType.KEYWORD).value
                
                # Parse method body - accept either LBRACE or COLON
                if self.check(TokenType.LBRACE):
                    self.consume()
                    method_body = self.block(stop_at=set())
                    if self.check(TokenType.RBRACE):
                        self.consume()
                elif self.check(TokenType.COLON):
                    self.consume()
                    method_body = self.block(stop_at=set(), use_colon=True)
                    if self.check(TokenType.RBRACE):
                        self.consume()
                else:
                    self.expect(TokenType.LBRACE)
                
                # If first param is 'self', remove it from stored params but add to body
                # For now, just store all params as-is
                members.append(FunctionDefNode(method_name, params, ret_type, method_body))
            else:
                # Skip any other tokens (fields, etc) - simplified for now
                self.consume()
        
        if self.check(TokenType.RBRACE):
            self.consume()
        
        return ClassDefNode(name, base_class, members)

    def inherit_stmt(self):
        # Handle inherit statement: inherit <type>
        self.consume()  # consume 'inherit'
        inherited_type = self.expect(TokenType.IDENTIFIER).value
        # For now, we'll treat this as a standalone statement that could be used in classes
        # In a more complete implementation, this would modify the current class being defined
        # For simplicity, we'll create a placeholder node
        return InheritNode("__current_class__", inherited_type)
    
    def if_(self):
        self.consume()
        c = self.exp()
        
        # Brace-based: if x > 0 { ... } else { ... }
        self.expect(TokenType.LBRACE)
        tb = self.block_for_control()
        eb = []
        ob = None
        if self.check(TokenType.KEYWORD, 'else'):
            self.consume()
            self.expect(TokenType.LBRACE)
            ob = self.block_for_control()
            # Block already consumes the closing RBRACE
        
        return IfNode(c, tb, ob, eb)
    
    def for_(self):
        self.consume()
        vn = self.expect(TokenType.IDENTIFIER).value
        self.expect(TokenType.KEYWORD, 'in')
        if self.check(TokenType.KEYWORD, 'range'):
            self.consume()
            self.expect(TokenType.LPAREN)
            ag = []
            while not self.check(TokenType.RPAREN):
                if ag: self.expect(TokenType.COMMA)
                ag.append(self.exp())
            self.expect(TokenType.RPAREN)
            st = NumberNode(0) if len(ag) < 2 else ag[0]
            en = ag[-1]
            sp = ag[1] if len(ag) == 3 else None
        else:
            en = self.exp()
            st = NumberNode(0)
            sp = None
        
        # Handle both colon and brace syntax
        # Style 1: for i in range(3): ... (Python style)
        # Style 2: for i in range(3) { ... } (brace style)
        # Style 3: for i in range(3): { ... } (brace with colon)
        if self.check(TokenType.LBRACE):
            self.consume()
            bd = self.block_for_control()
            if self.check(TokenType.RBRACE):
                self.consume()
        elif self.check(TokenType.COLON):
            self.consume()
            self.skip_nl()
            if self.check(TokenType.LBRACE):
                self.consume()
                bd = self.block_for_control()
                if self.check(TokenType.RBRACE):
                    self.consume()
            else:
                bd = self.block_for_control()
        else:
            self.expect(TokenType.COLON)
            self.skip_nl()
            bd = self.block_for_control()
        
        return ForNode(vn, st, en, sp, bd)
    
    def while_(self):
        self.consume()
        c = self.exp()
        
        # Handle both colon and brace syntax
        # Style 1: while i < 3: ... (Python style)
        # Style 2: while i < 3 { ... } (brace style)
        # Style 3: while i < 3: { ... } (brace with colon)
        if self.check(TokenType.LBRACE):
            self.consume()
            bd = self.block_for_control()
            if self.check(TokenType.RBRACE):
                self.consume()
        elif self.check(TokenType.COLON):
            self.consume()
            self.skip_nl()
            if self.check(TokenType.LBRACE):
                self.consume()
                bd = self.block_for_control()
                if self.check(TokenType.RBRACE):
                    self.consume()
            else:
                bd = self.block_for_control()
        else:
            self.expect(TokenType.COLON)
            self.skip_nl()
            bd = self.block_for_control()
        
        return WhileNode(c, bd)
    
    def ret(self):
        self.consume()
        v = None
        if not self.check(TokenType.NEWLINE) and not self.check(TokenType.EOF):
            v = self.exp()
        return ReturnNode(v)
    
    def print_(self):
        self.consume()
        self.expect(TokenType.LPAREN)
        ag = []
        while not self.check(TokenType.RPAREN):
            if ag: self.expect(TokenType.COMMA)
            ag.append(self.exp())
        self.expect(TokenType.RPAREN)
        return PrintNode(ag)
    
    def expr_stmt(self):
        e = self.exp()
        if self.check(TokenType.OPERATOR, '='):
            self.consume()
            v = self.exp()
            if isinstance(e, IdentifierNode):
                return AssignmentNode(e.name, v)
            elif isinstance(e, AttributeAccessNode):
                # Attribute assignment: obj.attr = value
                return AssignmentNode(e, v)
        return e
    
    def exp(self): return self.or_()
    def or_(self):
        l = self.and_()
        while self.check(TokenType.KEYWORD, 'or'):
            self.consume()
            r = self.and_()
            l = BinaryOpNode('or', l, r)
        return l
    def and_(self):
        l = self.not_()
        while self.check(TokenType.KEYWORD, 'and'):
            self.consume()
            r = self.not_()
            l = BinaryOpNode('and', l, r)
        return l
    def not_(self):
        if self.check(TokenType.KEYWORD, 'not'):
            self.consume()
            return UnaryOpNode('not', self.not_())
        return self.cmp()
    def cmp(self):
        l = self.add()
        while self.check(TokenType.OPERATOR):
            op = self.peek().value
            if op in ('==','!=','<','>','<=','>='):
                self.consume()
                r = self.add()
                l = BinaryOpNode(op, l, r)
            else: break
        return l
    def add(self):
        l = self.mul()
        while self.check(TokenType.OPERATOR):
            op = self.peek().value
            if op in ('+','-'):
                self.consume()
                r = self.mul()
                l = BinaryOpNode(op, l, r)
            else: break
        return l
    def mul(self):
        l = self.unary()
        while self.check(TokenType.OPERATOR):
            op = self.peek().value
            if op in ('*','/','%'):
                self.consume()
                r = self.unary()
                l = BinaryOpNode(op, l, r)
            else: break
        return l
    def unary(self):
        if self.check(TokenType.OPERATOR, '-'):
            self.consume()
            return UnaryOpNode('-', self.unary())
        return self.post()
    def post(self):
        n = self.prim()
        while True:
            if self.check(TokenType.LBRACKET):
                self.consume()
                i = self.exp()
                self.expect(TokenType.RBRACKET)
                n = IndexAccessNode(n, i)
            elif self.check(TokenType.LPAREN):
                self.consume()
                a = []
                while not self.check(TokenType.RPAREN):
                    if a: self.expect(TokenType.COMMA)
                    a.append(self.exp())
                self.expect(TokenType.RPAREN)
                n = CallNode(n, a)
            elif self.check(TokenType.DOT):
                self.consume()
                at = self.expect(TokenType.IDENTIFIER).value
                n = AttributeAccessNode(n, at)
            else: break
        return n
    def prim(self):
        t = self.peek()
        if t.type == TokenType.NUMBER:
            self.consume()
            return NumberNode(float(t.value))
        if t.type == TokenType.STRING:
            self.consume()
            return StringNode(t.value)
        if t.type == TokenType.KEYWORD:
            if t.value == 'True':
                self.consume()
                return BoolNode(True)
            if t.value == 'False':
                self.consume()
                return BoolNode(False)
            if t.value == 'None':
                self.consume()
                return NoneNode()
            if t.value == 'nullptr':
                self.consume()
                return NullptrNode()
            if t.value == 'simple':
                self.consume()
                self.expect(TokenType.LPAREN)
                val = self.expect(TokenType.STRING).value
                self.expect(TokenType.RPAREN)
                return SimpleNode(val)
            if t.value == 'sizeof':
                self.consume()
                self.expect(TokenType.LPAREN)
                tn = None
                ex = None
                if self.check(TokenType.KEYWORD):
                    tn = self.expect(TokenType.KEYWORD).value
                else:
                    ex = self.exp()
                self.expect(TokenType.RPAREN)
                return SizeofNode(tn, ex)
            if t.value == 'new':
                self.consume()
                # Type can be KEYWORD (like 'int') or IDENTIFIER (like class names)
                if self.check(TokenType.KEYWORD):
                    tn = self.expect(TokenType.KEYWORD).value
                elif self.check(TokenType.IDENTIFIER):
                    tn = self.expect(TokenType.IDENTIFIER).value
                else:
                    tn = 'auto'  # Default type
                args = []
                if self.check(TokenType.LPAREN):
                    self.consume()
                    while not self.check(TokenType.RPAREN):
                        if args: self.expect(TokenType.COMMA)
                        args.append(self.exp())
                    self.expect(TokenType.RPAREN)
                return NewNode(tn, args)
            if t.value == 'delete':
                self.consume()
                ex = self.exp()
                return DeleteNode(ex)
            if t.value == 'static':
                return self.parse_static()
            if t.value == 'const':
                return self.parse_const()
        if t.type == TokenType.IDENTIFIER:
            self.consume()
            nm = t.value
            if self.check(TokenType.COLON):
                self.consume()
                vt = self.expect(TokenType.KEYWORD).value
                if self.check(TokenType.OPERATOR, '='):
                    self.consume()
                    return VarDeclNode(nm, vt, self.exp())
                return VarDeclNode(nm, vt, None)
            return IdentifierNode(nm)
        if t.type == TokenType.KEYWORD and t.value == 'simple':
            self.consume()
            if self.check(TokenType.LPAREN):
                self.consume()
                val = self.expect(TokenType.STRING).value
                self.expect(TokenType.RPAREN)
                return SimpleNode(val)
            elif self.check(TokenType.OPERATOR, '='):
                self.consume()
                # Check if it's a string or number
                if self.check(TokenType.STRING):
                    val = self.expect(TokenType.STRING).value
                    return SimpleNode(val)
                elif self.check(TokenType.NUMBER):
                    num_val = self.expect(TokenType.NUMBER).value
                    # Create SimpleNumNode for numeric simple types
                    return SimpleNumNode(float(num_val))
                return SimpleNode("")
            return SimpleNode("")
        if t.type == TokenType.AMPERSAND:
            self.consume()
            vn = self.expect(TokenType.IDENTIFIER).value
            return AddressOfNode(vn)
        if t.type == TokenType.POINTER:
            self.consume()
            return DereferenceNode(self.prim())
        if self.check(TokenType.LBRACKET):
            self.consume()
            el = []
            while not self.check(TokenType.RBRACKET):
                if el: self.expect(TokenType.COMMA)
                el.append(self.exp())
            self.expect(TokenType.RBRACKET)
            return ListNode(el)
        if self.check(TokenType.LPAREN):
            self.consume()
            ex = self.exp()
            self.expect(TokenType.RPAREN)
            return ex
        raise SyntaxError(f"Bad token: {t.value}")
    def expect(self, tt, vv=None):
        t = self.peek()
        if t.type != tt or (vv and t.value != vv):
            raise SyntaxError(f"Want {tt.name}{' '+vv if vv else ''}")
        return self.consume()
    
    def parse_static(self):
        self.consume()
        nm = self.expect(TokenType.IDENTIFIER).value
        self.expect(TokenType.COLON)
        vt = self.expect(TokenType.KEYWORD).value
        val = None
        if self.check(TokenType.OPERATOR, '='):
            self.consume()
            val = self.exp()
        return StaticVarNode(nm, vt, val)
    
    def parse_const(self):
        self.consume()
        nm = self.expect(TokenType.IDENTIFIER).value
        self.expect(TokenType.COLON)
        vt = self.expect(TokenType.KEYWORD).value
        self.expect(TokenType.OPERATOR, '=')
        val = self.exp()
        return ConstNode(nm, vt, val)
    
    def parse_new(self):
        self.consume()
        type_name = self.expect(TokenType.KEYWORD).value
        args = []
        if self.check(TokenType.LPAREN):
            self.consume()
            while not self.check(TokenType.RPAREN):
                if args: self.expect(TokenType.COMMA)
                args.append(self.exp())
            self.expect(TokenType.RPAREN)
        return NewNode(type_name, args)
    
    def parse_delete(self):
        self.consume()
        expr = self.exp()
        return DeleteNode(expr)

def parse(src, tokens=None):
    if tokens is None:
        tokens = tokenize(src)
    return AddXParser(tokens).parse()