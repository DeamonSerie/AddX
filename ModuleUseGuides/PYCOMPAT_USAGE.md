# AddX Python Compatibility Module

Provides Python-like functions that aren't built into AddX.

## List Functions

### append
```
nums = [1, 2, 3]
append(nums, 4)  # [1, 2, 3, 4]
```

### pop
```
nums = [1, 2, 3]
last = pop(nums)  # last=3
```

### insert, remove, extend

### index, count

### sort, reverse, clear, copy

## Dict Functions

### dictGet
```
dictGet(d, "key", "default")
```

### dictKeys, dictValues, dictItems

### dictPop, dictClear, dictUpdate

## Math Functions

### abs
```
abs(-5)  # 5
```

### min2, max2, minList, maxList

### pow, floor, ceil, round_

## Utility Functions

### enumerate
```
enumerate(["a", "b"])  # [(0,"a"), (1,"b")]
```

### zip
```
zip([1,2], ["a","b"])  # [(1,"a"), (2,"b")]
```

### sum, product

### sorted, reversed, filter, map

## Type Functions

### isinstance

## Example

```
def main()
    nums = [3, 1, 4]
    print(sorted(nums))
    print(sum(nums))
    return 0
```