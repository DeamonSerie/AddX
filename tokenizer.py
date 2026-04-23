from enum import Enum, auto
from dataclasses import dataclass

class TokenType(Enum):
    IDENTIFIER = auto()
    KEYWORD = auto()
    NUMBER = auto()
    STRING = auto()
    OPERATOR = auto()
    LPAREN = auto()
    RPAREN = auto()
    LBRACE = auto()
    RBRACE = auto()
    LBRACKET = auto()
    RBRACKET = auto()
    LBRACKET_SQUARE = auto()
    RBRACKET_SQUARE = auto()
    COLON = auto()
    COMMA = auto()
    ARROW = auto()
    DOT = auto()
    NEWLINE = auto()
    EOF = auto()
    REFERENCE = auto()
    POINTER = auto()
    AMPERSAND = auto()

@dataclass
class Token:
    type: TokenType
    value: str
    line: int
    column: int

KEYWORDS = {'def', 'if', 'elif', 'else', 'for', 'while', 'return', 'print',
            'class', 'True', 'False', 'None', 'and', 'or', 'not', 'in', 'range',
            'nullptr', 'sizeof', 'static', 'const', 'new', 'delete', 'struct', 'inherit', 'simple', 'modan'}
TYPE_KEYWORDS = {'int', 'float', 'str', 'bool', 'void', 'list', 'dict', 'auto', 'simple', 'modan'}

def tokenize(source):
    lines = source.replace('\r\n', '\n').replace('\r', '\n').split('\n')
    tokens = []
    
    for line_num, line in enumerate(lines, 1):
        stripped = line.lstrip()
        
        if not stripped or stripped.startswith('#') or stripped.startswith('//'):
            tokens.append(Token(TokenType.NEWLINE, '', line_num, 1))
            continue
        
        i = 0
        col = 1
        while i < len(stripped):
            ch = stripped[i]
            
            if ch.isdigit():
                start = col
                val = ''
                while i < len(stripped) and (stripped[i].isdigit() or stripped[i] == '.'):
                    val += stripped[i]
                    i += 1
                    col += 1
                tokens.append(Token(TokenType.NUMBER, val, line_num, start))
                continue
            
            if ch in '"\'':
                start = col
                quote = ch
                val = ''
                i += 1
                col += 1
                while i < len(stripped) and stripped[i] != quote:
                    if stripped[i] == '\\' and i + 1 < len(stripped):
                        val += stripped[i+1]
                        i += 2
                        col += 2
                    else:
                        val += stripped[i]
                        i += 1
                        col += 1
                if i < len(stripped):
                    i += 1
                    col += 1
                tokens.append(Token(TokenType.STRING, val, line_num, start))
                continue
            
            if ch.isalpha() or ch == '_':
                start = col
                val = ''
                while i < len(stripped) and (stripped[i].isalnum() or stripped[i] == '_'):
                    val += stripped[i]
                    i += 1
                    col += 1
                if val in KEYWORDS or val in TYPE_KEYWORDS:
                    tokens.append(Token(TokenType.KEYWORD, val, line_num, start))
                else:
                    tokens.append(Token(TokenType.IDENTIFIER, val, line_num, start))
                continue
            
            if ch == '#':
                break
            
            if ch == '(':
                tokens.append(Token(TokenType.LPAREN, '(', line_num, col))
                i += 1
                col += 1
                continue
            if ch == ')':
                tokens.append(Token(TokenType.RPAREN, ')', line_num, col))
                i += 1
                col += 1
                continue
            if ch == '{':
                tokens.append(Token(TokenType.LBRACE, '{', line_num, col))
                i += 1
                col += 1
                continue
            if ch == '}':
                tokens.append(Token(TokenType.RBRACE, '}', line_num, col))
                i += 1
                col += 1
                continue
            if ch == '[':
                tokens.append(Token(TokenType.LBRACKET, '[', line_num, col))
                i += 1
                col += 1
                continue
            if ch == ']':
                tokens.append(Token(TokenType.RBRACKET, ']', line_num, col))
                i += 1
                col += 1
                continue
            if ch == ':':
                tokens.append(Token(TokenType.COLON, ':', line_num, col))
                i += 1
                col += 1
                continue
            if ch == ',':
                tokens.append(Token(TokenType.COMMA, ',', line_num, col))
                i += 1
                col += 1
                continue
            
            if ch == '-' and i + 1 < len(stripped) and stripped[i + 1] == '>':
                tokens.append(Token(TokenType.ARROW, '->', line_num, col))
                i += 2
                col += 2
                continue
            
            if ch == '&':
                tokens.append(Token(TokenType.AMPERSAND, '&', line_num, col))
                i += 1
                col += 1
                continue
            
            if ch == '*':
                tokens.append(Token(TokenType.POINTER, '*', line_num, col))
                i += 1
                col += 1
                continue
            
            if ch == '.':
                tokens.append(Token(TokenType.DOT, '.', line_num, col))
                i += 1
                col += 1
                continue
            
            if ch in '+-*/%=<>!':
                start = col
                val = ''
                while i < len(stripped) and stripped[i] in '+-*/%=<>!':
                    val += stripped[i]
                    i += 1
                    col += 1
                tokens.append(Token(TokenType.OPERATOR, val, line_num, start))
                continue
            
            i += 1
            col += 1
        
        tokens.append(Token(TokenType.NEWLINE, '', line_num, len(line) + 1))
    
    tokens.append(Token(TokenType.EOF, '', len(lines), 1))
    return tokens