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
    if line.startswith('print('):
        s = line[6:-1]
        if s.startswith('"'):
            ops.append((Op.PRINT, s.strip('"')))
        elif s.isdigit():
            ops.append((Op.PRINT, int(s)))
        else:
            ops.append((Op.PRINT, s))
    elif '=' in line and ':' not in line:
        var, expr = line.split('=', 1)
        var = var.strip()
        expr = expr.strip()
        if expr.isdigit():
            ops.append((Op.LOAD, int(expr)))
        elif expr[0] == '"':
            ops.append((Op.LOAD, expr.strip('"')))
        else:
            ops.append((Op.LOAD, expr))
        ops.append((Op.STORE, var))

def run(ops):
    stack = []
    vars = {}
    pc = 0
    while pc < len(ops):
        op = ops[pc]
        if op[0] == Op.LOAD:
            stack.append(op[1])
        elif op[0] == Op.STORE:
            vars[op[1]] = stack.pop()
        elif op[0] == Op.PRINT:
            v = op[1]
            if isinstance(v, str) and v in vars:
                print(vars[v])
            elif isinstance(v, str):
                print(v)
            else:
                print(v)
        elif op[0] == Op.HLT:
            break
        pc += 1

def run_addx(source):
    ops = assemble(source)
    run(ops)

if __name__ == '__main__':
    import sys
    src = open(sys.argv[1]).read() if len(sys.argv) > 1 else sys.stdin.read()
    run_addx(src)