# AddX Tokenizer Module

Lexical analysis and tokenization for text parsing.

## Token Types

| Type | Description |
|------|-------------|
| EOF | End of file |
| IDENTIFIER | Variable/function names |
| NUMBER | Numeric literals |
| STRING | Quoted string literals |
| OPERATOR | Operators (+, -, *, /, etc.) |
| PUNCTUATION | Brackets, commas, etc. |
| KEYWORD | Reserved words (if, while, def, etc.) |
| WHITESPACE | Spaces/tabs |
| COMMENT | Comments |
| UNKNOWN | unrecognized characters |

## Basic Usage

### Tokenize String

```
tokens = tokenize("x = 10 + 5")
for t in tokens do
    print(t.toString())
```

### Tokenize File

```
tokens = tokenizeFile("source.addx")
```

### Tokens to JSON

```
json = tokensToJson(tokens)
print(json)
```

## Token Structure

```
Token {
    type: string      # Token type (e.g., "IDENTIFIER", "NUMBER")
    value: string     # Token text
    line: int         # Line number
    column: int       # Column number
}
```

## Example

```
def main()
    source = "x = 10 + y"
    tokens = tokenize(source)
    
    for t in tokens do
        print(t.type + ": " + t.value + " at " + str(t.line) + ":" + str(t.column))
    
    return 0
```

Output:
```
IDENTIFIER: x at 1:1
PUNCTUATION: = at 1:3
NUMBER: 10 at 1:5
OPERATOR: + at 1:8
IDENTIFIER: y at 1:10
EOF: at 1:12
```

## Tokenize Keywords

Recognized keywords: if, else, while, for, def, return, class, import, True, False, None

## String Handling

Handles escape sequences: \n, \t, \\, \", \'

## Custom Tokenizer

```
t = Tokenizer.new(source)
result = t.tokenize()
```

Creates a Tokenizer instance and tokenizes the source string.