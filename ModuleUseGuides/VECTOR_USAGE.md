# AddX Vector Module

2D/3D vector operations, color conversions, shader properties, and light controls.

## Vector2

### Creating Vector2

```
v = Vector2.new(3.0, 4.0)
```

### Operations

```
a = Vector2.new(1.0, 2.0)
b = Vector2.new(3.0, 4.0)

# Addition
c = a.add(b)  # (4.0, 6.0)

# Subtraction
c = a.sub(b)  # (-2.0, -2.0)

# Scalar multiplication
c = a.mul(2)  # (2.0, 4.0)

# Dot product
dot = a.dot(b)  # 1*3 + 2*4 = 11

# Cross product (2D)
cross = a.cross(b)  # 1*4 - 2*3 = -2
```

### Properties

```
v = Vector2.new(3.0, 4.0)

length = v.length()      # 5.0
normalized = v.normalize()  # (0.6, 0.8)

dist = v.distance(Vector2.new(0, 0))  # 5.0

lerped = v.lerp(Vector2.new(6.0, 8.0), 0.5)  # (4.5, 6.0)
```

## Vector3

### Creating Vector3

```
v = Vector3.new(1.0, 2.0, 3.0)
```

### Operations

```
a = Vector3.new(1.0, 2.0, 3.0)
b = Vector3.new(4.0, 5.0, 6.0)

c = a.add(b)   # (5, 7, 9)
c = a.sub(b)   # (-3, -3, -3)
c = a.mul(2)   # (2, 4, 6)

dot = a.dot(b)  # 1*4 + 2*5 + 3*6 = 32

cross = a.cross(b)  # (2*6-3*5, 3*4-1*6, 1*5-2*4) = (-3, 6, -3)
```

### Properties

```
v = Vector3.new(3.0, 4.0, 0.0)

length = v.length()       # 5.0
normalized = v.normalize()  # (0.6, 0.8, 0)
dist = v.distance(Vector3.new(0, 0, 0))
lerped = v.lerp(Vector3.new(6, 8, 0), 0.5)
```

### Utility Functions

```
# Distance between vectors
d = vector2_distance(v1, v2)
d = vector3_distance(v1, v2)

# Dot products
dot2 = vector2_dot(v1, v2)
dot3 = vector3_dot(v1, v2)
```

## Color

### Creating Colors

```
# RGB
c = Color.new(1.0, 0.0, 0.0, 1.0)  # Red

# From RGB values
c = Color.from_rgb(255, 0, 0, 255)

# From HSL
c = Color.from_hsl(0, 1.0, 0.5)  # Red

# From CMYK
c = Color.from_cmyk(0, 1, 1, 0)  # Red

# From hex
c = Color.from_hex("#FF0000")
c = Color.from_hex("FF0000FF")
```

### Color Properties

```
c = Color.new(1.0, 0.5, 0.25, 1.0)

r = c.red
g = c.green  
b = c.blue
a = c.alpha
```

### Color Conversions

```
c = Color.new(1.0, 0.0, 0.0, 1.0)

# To RGB
rgb = c.to_rgb()         # (255, 0, 0)
rgba = c.to_rgba()      # (255, 0, 0, 255)

# To HSL
hsl = c.to_hsl()        # (0, 1.0, 0.5)

# To CMYK
cmyk = c.to_cmyk()      # (0, 1, 1, 0)

# To Hex
hex = c.to_hex()        # "FF0000"
hexa = c.to_hex_a()     # "FF0000FF"
```

### Color Blending

```
c1 = Color.new(1, 0, 0, 1)  # Red
c2 = Color.new(0, 0, 1, 1)  # Blue

blended = c1.blend(c2, 0.5)  # Purple
```

## Shader Properties

```
shader = Shader.new()

# Set float
shader.set_float("mix", 0.5)
shader.set_int("iterations", 10)
shader.set_color("color", Color.new(1, 0, 0, 1))
shader.set_vector2("uv", Vector2.new(0.5, 0.5))
shader.set_vector3("pos", Vector3.new(1, 2, 3))

# Get values
val = shader.get_float("mix")
val = shader.get_int("iterations")
col = shader.get_color("color")
uv = shader.get_vector2("uv")
pos = shader.get_vector3("pos")
```

## Light Sources

### Creating Lights

```
# Point light
light = Light.new_point(Vector3.new(0, 5, 5), Color.new(1, 1, 1, 1), 100)

# Directional light
light = Light.new_directional(Vector3.new(-1, -1, -1), Color.new(1, 1, 1, 1))

# Spot light
light = Light.new_spot(Vector3.new(0, 5, 0), Vector3.new(0, -1, 0), 30, Color.new(1, 1, 1, 1), 50)
```

### Light Properties

```
light = Light.new_point(pos, color, intensity)

pos = light.position
color = light.color
intensity = light.intensity
type = light.light_type  # "point", "directional", "spot"
```

## Example: 3D Scene

```
def main()
    # Create vectors for a triangle
    v1 = Vector3.new(0, 1, 0)
    v2 = Vector3.new(-1, 0, 0)
    v3 = Vector3.new(1, 0, 0)
    
    # Calculate face normal
    edge1 = v2.sub(v1)
    edge2 = v3.sub(v1)
    normal = edge1.cross(edge2).normalize()
    
    # Create color for the triangle
    triangleColor = Color.from_rgb(255, 128, 0)
    
    # Calculate lighting
    lightDir = Vector3.new(0.5, 1.0, 0.5).normalize()
    brightness = normal.dot(lightDir)
    
    litColor = triangleColor.mul(brightness)
    
    print("Normal: " + normal.to_string())
    print("Brightness: " + brightness)
    
    return 0
```

## Example: Animation

```
def animate(t: float) -> Vector2
    # Circular motion
    x = Math.cos(t) * 100
    y = Math.sin(t) * 100
    return Vector2.new(x + 400, y + 300)
```