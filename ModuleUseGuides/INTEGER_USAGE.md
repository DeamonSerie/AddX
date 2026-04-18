# AddX Integer Module

Integer handling, big integers, bit operations, and number theory.

## BigInt

Arbitrary precision integers for calculations beyond standard integer limits.

### Creating BigInt

```
# From integer
n = BigInt.new(12345678901234567890)

# From string
n = BigInt.fromString("99999999999999999999999999")
n = BigInt.fromString("ff", 16)  # hex, base 16
```

### Basic Operations

```
a = BigInt.new(100)
b = BigInt.new(50)

sum = a.add(b)           # 150
diff = a.subtract(b)     # 50
prod = a.multiply(b)    # 5000
quot = a.divide(b)       # 2
mod = a.mod(b)           # 0
```

### Comparison

```
a = BigInt.new(10)
b = BigInt.new(20)

a.compare(b)   # -1 (a < b)
b.compare(a)   # 1  (b > a)
a.compare(a)   # 0  (equal)
```

### Properties

```
n = BigInt.new(-42)

n.isZero()       # false
n.isNegative()   # true
n.isPositive()   # false
n.abs()          # BigInt(42)
n.negate()       # BigInt(42)
```

### String Conversion

```
n = BigInt.new(255)
n.toString()           # "255"
n.toString(16)         # "ff"
n.toString(2)          # "11111111"
```

### Example: Factorial

```
def factorial(n: int) -> BigInt
    result = BigInt.new(1)
    for i in range(2, n + 1):
        result = result.multiply(BigInt.new(i))
    return result

fact100 = factorial(100)
print(fact100.toString())
```

## Bit Operations

```
a = 0b1100  # 12
b = 0b1010  # 10

bitAnd(a, b)    # 0b1000 = 8
bitOr(a, b)     # 0b1110 = 14
bitXor(a, b)    # 0b0110 = 6
bitNot(a)       # -13 (two's complement)
bitShiftLeft(a, 2)  # 0b110000 = 48
bitShiftRight(a, 1) # 0b110 = 6
bitRotateLeft(a, 4) # rotate bits
bitRotateRight(a, 4)
```

## Number Theory

### Primality

```
isPrime(17)           # true
isPrime(100)          # false
nextPrime(100)        # 101
prevPrime(100)        # 97
primesUpTo(30)       # [2,3,5,7,11,13,17,19,23,29]
primeFactors(60)      # [2,2,3,5]
isPrimeFermat(n)      # Fermat test
isPrimeMillerRabin(n) # Miller-Rabin test
```

### GCD & LCM

```
gcd(48, 18)           # 6
gcdExtended(48, 18)   # returns (g, x, y) where ax + by = g
lcm(12, 18)           # 36
```

### Fibonacci

```
fib(0)      # 0
fib(1)      # 1
fib(10)     # 55
fibBig(100) # very large number
fibMatrix(n) # fast doubling method
```

### Combinatorics

```
factorial(5)      # 120
factorial(100)    # huge number
combination(10, 3) # 120 (10 choose 3)
permutation(10, 3) # 720 (P(10,3))
```

### Modular Arithmetic

```
modPow(2, 10, 1000)     # 2^10 mod 1000 = 24
modInverse(3, 11)       # 4 (3*4 mod 11 = 1)
modSqrt(4, 13)          # 5 or 8 (square roots mod 13)
```

### Number Tests

```
isPerfectSquare(64)     # true
isPerfectSquare(65)    # false
isPowerOf2(32)         # true
isPowerOf2(33)         # false
isPalindrome(121)      # true
isArmstrong(153)      # true (1^3+5^3+3^3=153)
```

### Conversions

```
intToBin(42)            # "101010"
binToInt("101010")      # 42
intToHex(255)           # "ff"
hexToInt("ff")          # 255
intToRoman(1994)        # "MCMXCIV"
romanToInt("MCMXCIV")   # 1994
```

## Example: RSA-like Operations

```
def main()
    # Large number operations
    p = BigInt.fromString("34906635940920929547629589004300843370291951805196748099584793378822127419835438820022653622449753669540652908579")
    q = BigInt.fromString("3121583517424958662613143135909442103742553183210979474216188833019494784656121669492544406520575669239101306500")
    
    n = p.multiply(q)
    print("n: " + n.toString())
    
    # GCD example
    a = BigInt.new(48)
    b = BigInt.new(18)
    g = Integer.gcd(a, b)
    print("GCD: " + g.toString())  # 6
    
    return 0
```