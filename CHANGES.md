# Changes to AddX Parser to Skip Tokenization Step

## Overview
Added a new function `parse_source` to the C parser that allows callers to optionally provide pre-tokenized tokens, thus skipping the tokenization step if tokens are already available.

## Files Modified

### c/parser.h
- Added declaration for `ASTNode* parse_source(const char* source, Tokens* tokens);`

### c/parser.c
- Implemented `parse_source` function:
  - If `tokens` is NULL, tokenizes the source internally and frees tokens after parsing
  - If `tokens` is provided, uses them directly without freeing (caller retains ownership)
- No changes to existing `parse(Tokens* tokens)` function for backward compatibility

## Usage Example

```c
#include "parser.h"
#include "tokenizer.h"

// Option 1: Let parse_source handle tokenization (original behavior)
ASTNode* ast1 = parse_source(source, NULL);

// Option 2: Provide pre-tokenized tokens to skip tokenization step
Tokens tokens = tokenize(source);
ASTNode* ast2 = parse_source(source, &tokens);
// Note: tokens are NOT freed by parse_source - caller must free them when done
free_tokens(&tokens);

// Option 3: Use the existing parse function directly with tokens
Tokens tokens2 = tokenize(source);
ASTNode* ast3 = parse(&tokens2);
free_tokens(&tokens2);
```

## Benefits
- Avoids redundant tokenization when parsing the same source multiple times
- Enables integration with external tokenizers or token caching mechanisms
- Maintains full backward compatibility with existing code