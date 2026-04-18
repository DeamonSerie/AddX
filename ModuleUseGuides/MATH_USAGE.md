# AddX Math Module

Mathematical functions and constants.

## Basic Constants

| Name | Value |
|------|-------|
| `Math.PI` | 3.141592653589793 |
| `Math.E` | 2.718281828459045 |
| `Math.TAU` | 6.283185307179586 (2π) |

## Square Roots

| Name | Value |
|------|-------|
| `Math.SQRT2` | 1.414213562373095 |
| `Math.SQRT3` | 1.732050807568877 |
| `Math.SQRT_PI` | 1.772453850905516 |
| `Math.SQRT_E` | 1.648721270700128 |

## Logarithms

| Name | Value |
|------|-------|
| `Math.LN2` | 0.693147180559945 |
| `Math.LN10` | 2.302585092994046 |
| `Math.LOG2E` | 1.442695040889 |
| `Math.LOG10E` | 0.434294481903252 |

## Golden Ratio

| Name | Value |
|------|-------|
| `Math.PHI` | 1.618033988749895 |
| `Math.PHI_INV` | 0.618033988749895 |

## Other Constants

| Name | Value |
|------|-------|
| `Math.EPSILON` | 1e-15 |
| `Math.INFINITY` | Large number |
| `Math.NAN` | Not a number |

## Conversions

| Name | Value |
|------|-------|
| `Math.DEG_TO_RAD` | π/180 |
| `Math.RAD_TO_DEG` | 180/π |

## Trig Functions

```
Math.sin(0)       # 0.0
Math.cos(0)      # 1.0
Math.tan(0)      # 0.0
Math.asin(0.5)   # arcsine
Math.acos(0.5)   # arccosine
Math.atan(1.0)   # arctangent
```

## Power & Roots

```
Math.pow(2, 3)   # 8.0
Math.sqrt(16)    # 4.0
Math.SQRT2      # 1.414...
```

## Exp & Log

```
Math.exp(1)      # 2.718...
Math.log(1)      # 0.0
Math.log(2)      # ~0.693 (LN2)
Math.log10(100)  # 2.0
```

## Rounding

```
Math.abs(-5)     # 5
Math.floor(3.7)  # 3
Math.ceil(3.2)   # 4
Math.round(3.5) # 4
```

## Example: Circle Calculations

```
def main()
    radius = 5.0
    area = Math.PI * Math.pow(radius, 2)
    circumference = Math.TAU * radius
    
    print("Radius: " + str(radius))
    print("Area: " + str(area))
    print("Circumference: " + str(circumference))
    
    return 0
```

## Example: Convert Degrees to Radians

```
def degToRad(degrees: float) -> float
    return degrees * Math.DEG_TO_RAD

def main()
    print(degToRad(90))    # ~1.571
    print(degToRad(180))  # ~3.142 (PI)
    return 0
```

## Example: Using Golden Ratio

```
def main()
    print(Math.PHI)         # 1.618...
    print(1 / Math.PHI)    # ~0.618 (PHI_INV)
    return 0
```

## Quick Reference

| Function | Description |
|----------|-------------|
| sin, cos, tan | Trigonometric |
| asin, acos, atan | Inverse trig |
| pow(x, y) | x^y |
| sqrt(x) | Square root |
| exp(x) | e^x |
| log(x) | Natural log |
| log10(x) | Base 10 log |
| abs(x) | Absolute value |
| floor(x) | Round down |
| ceil(x) | Round up |
| round(x) | Nearest integer |
| PI, E, TAU | Basic constants |
| LN2, LN10 | Log constants |
| PHI | Golden ratio |