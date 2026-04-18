# AddX Crypto Module

Cryptographic functions and utilities for AddX.

## Hash Functions

### MD5

```
md5 = hash_md5("hello")
print(mdval)  # 5d41402abc4b2a76b9719d911017c592
```

### SHA-256

```
sha = hash_sha256("hello")
print(sha)  # 2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824
```

### SHA-512

```
sha = hash_sha512("hello")
```

### BLAKE2

```
blake = hash_blake2b("test", 32)  # 256-bit output
```

### CRC32

```
crc = hash_crc32("test")
```

## Symmetric Encryption

### AES

```
aes = AES.new("mysecretkey12345", "CBC")
encrypted = aes.encrypt("Hello, World!")
decrypted = aes.decrypt(encrypted)
print(decrypted)
```

### RC4

```
rc4 = RC4.new("key")
encrypted = rc4.encrypt("message")
decrypted = rc4.decrypt(encrypted)
```

### ChaCha20

```
chacha = ChaCha20.new("key", "nonce")
encrypted = chacha.encrypt("plaintext")
decrypted = chacha.decrypt(encrypted)
```

## Public Key Encryption

### RSA

```
rsa = RSA.new(2048)
publicKey = rsa.generateKeyPair()
encrypted = rsa.encrypt("message", publicKey)
decrypted = rsa.decrypt(encrypted, privateKey)

signature = rsa.sign(message, privateKey)
verified = rsa.verify(message, signature, publicKey)
```

### ECC (Elliptic Curve)

```
ecc = ECC.new("secp256r1")
keys = ecc.generateKeyPair()
encrypted = ecc.encrypt("data", keys.public)
decrypted = ecc.decrypt(encrypted, keys.private)
```

### Ed25519

```
ed = Ed25519.new()
keys = ed.generateKeyPair()
signed = ed.sign("message", keys.private)
valid = ed.verify("message", signed, keys.public)
```

## Encoding

### Base64

```
encoded = encode_base64("Hello")
decoded = decode_base64(encoded)
```

### Hex

```
hex = encode_hex("Hello")
plain = decode_hex(hex)
```

### URL

```
url_encoded = encode_url("hello world")
url_decoded = decode_url(url_encoded)
```

## Utilities

### Random

```
rng = SecureRandom.new(256)
bytes = rng.nextBytes(32)
num = rng.nextInt(100)
```

### PBKDF2

```
derived = pbkdf2("password", "salt", 100000, 32)
```

### BCrypt

```
hashed = bcrypt_hash("password", 12)
valid = bcrypt_verify("password", hashed)
```

## Example: Full Workflow

```
def main()
    # Hash a message
    digest = hash_sha256("secret data")
    print("SHA-256: " + digest)
    
    # AES encryption
    aes = AES.new("1234567890123456", "CBC")
    ciphertext = aes.encrypt("Hello AddX!")
    print("Encrypted: " + ciphertext)
    
    plaintext = aes.decrypt(ciphertext)
    print("Decrypted: " + plaintext)
    
    # RSA key pair
    rsa = RSA.new(2048)
    keys = rsa.generateKeyPair()
    
    signed = rsa.sign("message", keys.private)
    if rsa.verify("message", signed, keys.public) then
        print("Signature verified!")
    
    return 0
```