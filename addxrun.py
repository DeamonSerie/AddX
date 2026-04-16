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
    lines = [l.strip() for l in code.split('\n') if l.strip() and not l.strip().startswith('#')]
    ops = []
    for line in lines:
        if line.startswith('def '):
            continue
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
        elif line.startswith('if '):
            pass
        elif line.startswith('while '):
            pass
        elif line.startswith('for '):
            pass
    ops.append((Op.HLT,))
    return ops

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