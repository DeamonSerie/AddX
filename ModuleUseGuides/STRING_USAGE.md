# AddX String Module

Python-like string manipulation and utilities.

## Basic Functions

### Length

```
len("hello")  # 5
```

### Split

```
split("a,b,c", ",")  # ["a", "b", "c"]
split("hello world")  # ["hello", "world"]
```

### Join

```
join(["a", "b", "c"], "-")  # "a-b-c"
join(["x", "y"], " ")  # "x y"
```

### Substring

```
substring("hello", 1, 4)  # "ell"
```

## Case Conversion

### Upper/Lower

```
upper("Hello")   # "HELLO"
lower("HELLO")   # "hello"
capitalize("hello")  # "Hello"
title("hello world")  # "Hello World"
```

## Search & Replace

### Find

```
find("hello world", "world")  # 6
find("hello world", "foo")    # -1 (not found)
```

### Contains

```
contains("hello", "ell")  # true
contains("hello", "foo")  # false
```

### Replace

```
replace("hello world", "world", "addx")  # "hello addx"
```

### StartsWith / EndsWith

```
startsWith("hello", "hel")  # true
endsWith("hello", "lo")     # true
```

## Whitespace

### Trim

```
trim("  hello  ")  # "hello"
lstrip("  hello")   # "hello"
rstrip("hello  ")   # "hello"
```

## Padding

### Ljust/Rjust/Center

```
ljust("hi", 5)    # "hi   "
rjust("hi", 5)    # "   hi"
center("hi", 5)    # " hi  "
```

### Zfill

```
zfill("42", 5)  # "00042"
```

## Character Operations

### Ord/Chr

```
ord("A")  # 65
chr(65)   # "A"
```

## Type Checking

### isAlpha, isDigit, isSpace, isAlnum

```
isAlpha("hello")   # true
isDigit("123")     # true
isSpace("   ")    # true
isAlnum("abc123") # true
```

## Formatting

### Format

```
format("Hello {0}!", ["World"])  # "Hello World!"
format("Value: {0}, Size: {1}", ["test", 5])  # "Value: test, Size: 5"
```

## Conversions

### To Int/Float

```
toInt("42")     # 42
toFloat("3.14") # 3.14
str(123)       # "123"
```

## Counting

### Count

```
count("hello llo", "l")  # 3
```

## Example: Word Counter

```
def countWords(s: string) -> int
    trimmed = trim(s)
    if len(trimmed) == 0 then
        return 0
    words = split(trimmed, " ")
    return len(words)

def main()
    text = "  hello world from addx  "
    print("Words: " + str(countWords(text)))
    return 0
```

## Example: Reverse String

```
def reverseString(s: string) -> string
    result = ""
    for i in range(len(s) - 1, -1, -1) do
        result = result + s[i]
    return result

def main()
    print(reverseString("hello"))  # "olleh"
    return 0
```

## Example: Palindrome Check

```
def isPalindrome(s: string) -> bool
    cleaned = lower(trim(s))
    reversed_ = ""
    for i in range(len(cleaned) - 1, -1, -1) do
        reversed_ = reversed_ + cleaned[i]
    return cleaned == reversed_

def main()
    print(isPalindrome("racecar"))  # true
    print(isPalindrome("hello"))   # false
    return 0
```