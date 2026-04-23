from enum import Enum

class Op(Enum):
    LOAD = 0
    STORE = 1
    PRINT = 2
    ADD = 3
    SUB = 4
    MUL = 5
    DIV = 6
    JMP = 7
    JZ = 8
    HLT = 9
    ADDR = 10      # &var - get address of variable
    DEREF = 11     # *ptr - dereference pointer
    PTR_CALL = 12   # Call function via pointer

def assemble(code):
    # First pass: find all [Modan] declarations
    modan_shifts = {}
    for line in code.split('\n'):
        stripped = line.strip()
        if stripped.startswith('[Modan]'):
            # Parse: [Modan] func() -> shift(Ln: 2, Ln: 3) or [Modan] Class.method() -> shift(...)
            parts = stripped.split('-> shift(')
            if len(parts) > 1:
                func_part = parts[0].replace('[Modan]', '').strip()
                # Remove () from function name
                func_name = func_part.replace('()', '').strip()
                shift_part = parts[1].rstrip(')').replace('Ln: ', '')
                line_nums = [int(x.strip()) for x in shift_part.split(',')]
                # Store as 0-indexed
                modan_shifts[func_name] = [x - 1 for x in line_nums]
    
    # Second pass: collect code lines
    lines = []
    for l in code.split('\n'):
        stripped = l.strip()
        # Skip comments, [Modan] lines
        if stripped and not stripped.startswith('#') and not stripped.startswith('//') and not stripped.startswith('['):
            lines.append(stripped)
    
    ops = []
    current_func_lines = []
    current_func = None
    in_class = False
    class_name = None
    
    for line in lines:
        if line.startswith('class '):
            # Process previous function with modan shift if any
            if current_func and current_func_lines:
                if modan_shifts.get(current_func):
                    for idx in modan_shifts[current_func]:
                        if 0 <= idx < len(current_func_lines):
                            parse_line(current_func_lines[idx], ops)
                else:
                    for l in current_func_lines:
                        parse_line(l, ops)
            current_func = None
            current_func_lines = []
            class_name = line.split(' ')[1].rstrip(':')
            in_class = True
            continue
        elif line.startswith('def '):
            # Process previous function/method with modan shift if any
            if current_func and current_func_lines:
                if modan_shifts.get(current_func):
                    for idx in modan_shifts[current_func]:
                        if 0 <= idx < len(current_func_lines):
                            parse_line(current_func_lines[idx], ops)
                else:
                    for l in current_func_lines:
                        parse_line(l, ops)
            
            # Start new function/method
            func_def = line.split('(')[0].replace('def ', '')
            if in_class and class_name:
                current_func = f"{class_name}.{func_def}"
            else:
                current_func = func_def
            current_func_lines = []
        else:
            # Inside function - collect lines
            if current_func:
                current_func_lines.append(line)
            else:
                parse_line(line, ops)
    
    # Handle last function
    if current_func and current_func_lines:
        if modan_shifts.get(current_func):
            for idx in modan_shifts[current_func]:
                if 0 <= idx < len(current_func_lines):
                    parse_line(current_func_lines[idx], ops)
        else:
            for l in current_func_lines:
                parse_line(l, ops)
    
    ops.append((Op.HLT,))
    return ops

def parse_line(line, ops):
    # Pointer: &var (get address)
    if line.startswith('&') and not line.startswith('def '):
        var_name = line[1:].strip()
        ops.append((Op.ADDR, var_name))
        return
    
    # Dereference in print: *ptr
    stripped = line.strip()
    if stripped.startswith('print(*') and stripped.endswith(')'):
        ptr_expr = stripped[6:-1]  # Extract ptr from print(*ptr)
        if ptr_expr.startswith('*'):
            ptr_expr = ptr_expr[1:]  # Remove * prefix
        ops.append((Op.DEREF, ptr_expr))
        ops.append((Op.PRINT, ""))
        return
    
    # Dereference without print (assignment): *ptr = value  
    if '*' in line and '=' in line and not line.startswith('def '):
        parts = line.split('=', 1)
        dest = parts[0].strip()[1:].strip()  # *ptr -> ptr
        src = parts[1].strip()
        if src.isdigit():
            ops.append((Op.LOAD, int(src)))
        else:
            ops.append((Op.LOAD, src))
        ops.append((Op.STORE, dest))
        return
    
    if line.startswith('print('):
        s = line[6:-1]
        if s.startswith('"'):
            ops.append((Op.PRINT, s.strip('"')))
        elif s.isdigit():
            ops.append((Op.PRINT, int(s)))
        elif s.startswith('&'):
            # Print address: print(&var)
            var_name = s[1:].strip()
            ops.append((Op.ADDR, var_name))
            ops.append((Op.PRINT, 'PRINT_STACK'))  # Marker to print from stack
        elif s.startswith('*'):
            # Print deref: print(*ptr)
            ptr_name = s[1:].strip()
            ops.append((Op.DEREF, ptr_name))
            ops.append((Op.PRINT, 'PRINT_STACK'))
        else:
            ops.append((Op.PRINT, s))
    elif '=' in line and ':' not in line and not line.startswith('def '):
        var, expr = line.split('=', 1)
        var = var.strip()
        expr = expr.strip()
        if expr.isdigit():
            ops.append((Op.LOAD, int(expr)))
        elif expr[0] == '"':
            ops.append((Op.LOAD, expr.strip('"')))
        elif expr.startswith('&'):
            addr_var = expr[1:].strip()
            ops.append((Op.ADDR, addr_var))
        else:
            ops.append((Op.LOAD, expr))
        ops.append((Op.STORE, var))

def run(ops):
    stack = []
    vars = {}
    addr_of = {}  # Maps ptr var -> original var it points to
    pc = 0
    while pc < len(ops):
        op = ops[pc]
        opcode = op[0]
        args = op[1:] if len(op) > 1 else ()
        
        if opcode == Op.LOAD:
            stack.append(args[0])
        elif opcode == Op.STORE:
            stored_var = args[0]
            # Check what we're storing before popping
            val = stack[-1] if stack else None
            if isinstance(val, str) and val in vars:
                # Likely storing an address like &x
                addr_of[stored_var] = val
            vars[stored_var] = stack.pop()
        elif opcode == Op.ADDR:
            var_name = args[0]
            # Push the variable name as address
            stack.append(var_name)
        elif opcode == Op.DEREF:
            var_name = args[0]
            val = vars.get(var_name)
            stack.append(val)
        elif opcode == Op.PRINT:
            if args and args[0] == 'PRINT_STACK':
                v = stack.pop()
                if isinstance(v, str):
                    # Print address with & prefix, value without
                    if v in vars:
                        print(f"&{v}")
                    else:
                        print(v)
                else:
                    print(v)
            else:
                v = args[0] if args else None
                if isinstance(v, str):
                    if v in vars:
                        print(vars[v])
                    else:
                        print(v)
                else:
                    print(v)
        elif opcode == Op.HLT:
            break
        pc += 1

def run_addx(source):
    ops = assemble(source)
    run(ops)

if __name__ == '__main__':
    import sys
    src = open(sys.argv[1]).read() if len(sys.argv) > 1 else sys.stdin.read()
    run_addx(src)