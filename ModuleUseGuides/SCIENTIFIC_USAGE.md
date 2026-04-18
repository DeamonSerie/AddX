# AddX Scientific Notation Module

Handle scientific notation and floating point numbers.

## Basic Usage

### Create ScientificFloat

```
sci = ScientificFloat.new(1.23, -4)  # 1.23e-4
```

### Convert to String

```
sci = ScientificFloat.new(1.23, -4)
print(sci.toString())  # "1.23e-4"
```

### Convert to Double

```
sci = ScientificFloat.new(1.23, -4)
val = sci.toDouble()  # 0.000123
```

## Converting Numbers

### Float to Scientific

```
sci = toScientific(12345.6789)
print(sci.toString())
```

### Parse String

```
sci = parseScientific("1.23e-4")
print(sci.toDouble())  # 0.000123
```

## Formatting

### Scientific Format

```
formatScientific(0.000123)    # "1.23e-4"
formatScientific(1234000)    # "1.23e6"
formatScientific(0.000123, 4) # "1.2300e-4"
```

### Engineering Format (exponents in multiples of 3)

```
formatEngineering(1234000)  # "1.234e6"
formatEngineering(0.00123) # "1.23e-3"
```

### SI Prefix Format

```
formatSI(1234000)  # "1.23M"
formatSI(0.00123) # "1.23m"
formatSI(1000000) # "1M"
formatSI(0.000001) # "1u"
```

| Prefix | Value |
|--------|-------|
| Y | 10^24 |
| Z | 10^21 |
| E | 10^18 |
| P | 10^15 |
| T | 10^12 |
| G | 10^9 |
| M | 10^6 |
| k | 10^3 |
| (none) | 1 |
| m | 10^-3 |
| u | 10^-6 |
| n | 10^-9 |
| p | 10^-12 |
| f | 10^-15 |
| a | 10^-18 |
| z | 10^-21 |
| y | 10^-24 |

## Arithmetic

### Using Functions

```
result = scientificAdd(1e5, 2e3)    # 1.02e5
result = scientificSub(1e5, 5e4)  # 5e4
result = scientificMul(2e3, 3e2)  # 6e5
result = scientificDiv(6e5, 2e2)  # 3e3
```

### Direct Operations

```
a = toScientific(1e5)
b = toScientific(2e3)
sum = a.add(b)
diff = a.subtract(b)
prod = a.multiply(b)
quot = a.divide(b)
```

## Comparison

### Compare

```
cmp = scientificCompare(1e5, 1e4)  # 1 (greater)
cmp = scientificCompare(1e5, 1e5) # 0 (equal)
cmp = scientificCompare(1e4, 1e5) # -1 (less)
```

### Equal with Tolerance

```
equal = scientificEqual(1.0e5, 100001.0, 0.01)  # true (within tolerance)
```

## Examples

### Very Large Numbers

```
def main()
    avogadro = 6.022e23
    sci = toScientific(avogadro)
    print(sci.toString())  # "6.022e23"
    print(formatSI(avogadro))  # "602.2Y" (actually should be ~602.2N)
    return 0
```

### Very Small Numbers

```
def main()
    electronMass = 9.109e-31
    sci = toScientific(electronMass)
    print(sci.toString())  # "9.109e-31"
    print(formatSI(electronMass))  # "910.9y"
    return 0
```

### Unit Conversion

```
def main()
    # Light speed in km/s
    c = 299792.458
    print(formatSI(c))  # "299.792k"
    
    # Planck's constant
    h = 6.626e-34
    print(formatSI(h))  # "662.6f"
    
    return 0
```

### Precision Calculations

```
def main()
    # Add very different magnitudes
    result = scientificAdd(1e10, 1e-10)
    print(result.toString())  # "1.0e10"
    
    # Multiply
    result = scientificMul(2.5e5, 4.0e-3)
    print(result.toString())  # "1.0e3"
    
    return 0
```

## Quick Reference

| Function | Description |
|----------|-------------|
| `ScientificFloat.new(m, e)` | Create with mantissa and exponent |
| `toScientific(val)` | Convert float to scientific |
| `parseScientific(str)` | Parse string to scientific |
| `formatScientific(val)` | Format with scientific notation |
| `formatEngineering(val)` | Format with engineering notation |
| `formatSI(val)` | Format with SI prefixes |
| `scientificAdd(a, b)` | Add two numbers |
| `scientificMul(a, b)` | Multiply two numbers |
| `scientificCompare(a, b)` | Compare (-1, 0, 1) |
| `scientificEqual(a, b, tol)` | Check equality with tolerance |