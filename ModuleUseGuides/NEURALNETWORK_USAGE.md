# AddX Neural Network Module

Create and train neural networks for machine learning tasks.

## Basic Usage

### Create a Neural Network

```
nn = NeuralNetwork.new([784, 128, 10], 0.01)
```

Creates a network with:
- Input layer: 784 neurons
- Hidden layer: 128 neurons  
- Output layer: 10 neurons
- Learning rate: 0.01

### Forward Pass

```
inputs = [0.1, 0.2, 0.3, ...]  # 784 values
outputs = nn.forward(inputs)
```

### Predict

```
predictedClass = nn.predict(inputs)
```

### Train

```
nn.train(inputData, targetData)
```

## Layer Class

### Creating a Layer

```
layer = Layer.new(inputSize, outputSize, activation)
```

Activations: "relu", "sigmoid", "tanh", "softmax"

### Forward

```
outputs = layer.forward(inputs)
```

## Activation Functions

### Using Activation Class

```
sig = Activation.sigmoid(0.5)
relu = Activation.relu(-1.0)
tanh = Activation.tanh(0.0)
```

### Available Functions

| Function | Description |
|----------|-------------|
| `sigmoid(x)` | Sigmoid activation |
| `sigmoidDeriv(x)` | Derivative for backprop |
| `relu(x)` | ReLU (max(0,x)) |
| `reluDeriv(x)` | Derivative |
| `leakyRelu(x, alpha)` | Leaky ReLU |
| `tanh(x)` | Hyperbolic tangent |
| `softmax(inputs)` | Softmax for output layer |

## Loss Functions

### Using Loss Class

```
mse = Loss.mse(predictions, targets)
loss = Loss.crossEntropy(predictions, targets)
bce = Loss.binaryCrossEntropy(pred, target)
```

## Utilities

### One Hot Encoding

```
encoded = oneHotEncode(index, size)
# index=2, size=4 -> [0.0, 0.0, 1.0, 0.0]
```

### Argmax

```
predicted = argmax(outputs)
# Returns index of highest value
```

### Normalize

```
normalized = normalize(data)
# Scales values to [0, 1] range
```

### Shuffle Data

```
shuffled = shuffleData(data)
# Randomizes training data order
```

## Example: XOR Problem

```
def main()
    # XOR training data
    trainingData = [
        ([0.0, 0.0], [0.0]),
        ([0.0, 1.0], [1.0]),
        ([1.0, 0.0], [1.0]),
        ([1.0, 1.0], [0.0])
    ]
    
    # Create network: 2 inputs -> 4 hidden -> 1 output
    nn = NeuralNetwork.new([2, 4, 1], 0.5)
    
    # Train
    for epoch in range(1000) do
        for inputs, targets in trainingData do
            nn.train(inputs, targets)
    
    # Test
    test1 = nn.predict([0.0, 0.0])
    print("0 XOR 0 = " + str(test1))
    
    test2 = nn.predict([1.0, 1.0])
    print("1 XOR 1 = " + str(test2))
    
    return 0
```

## Example: Digit Recognition

```
def main()
    # Simple MNIST-like example
    # Input: 28x28 = 784 pixels
    # Output: 10 digits (0-9)
    
    nn = NeuralNetwork.new([784, 256, 128, 10], 0.01)
    
    # Load training data (simplified)
    print("Training network...")
    
    # Train for some epochs
    for epoch in range(10) do
        print("Epoch " + str(epoch))
        # nn.trainBatch(trainingData)
    
    # Predict
    image = [0.1, 0.2, ...]  # 784 pixel values
    digit = nn.predict(image)
    print("Predicted digit: " + str(digit))
    
    return 0
```

## Example: Multi-class Classification

```
def main()
    # 4 input features, 3 output classes
    nn = NeuralNetwork.new([4, 8, 3], 0.1)
    
    # Sample data
    data = [
        ([1, 0, 0, 1], [1, 0, 0]),
        ([0, 1, 0, 1], [0, 1, 0]),
        ([0, 0, 1, 1], [0, 0, 1])
    ]
    
    # Train
    for i in range(500) do
        for inputs, targets in data do
            nn.train(inputs, targets)
    
    # Predict
    result = nn.predict([1, 0, 0, 1])
    print("Predicted class: " + str(result))
    
    return 0
```

## Hyperparameters

### Learning Rate
- Too high: Training may not converge
- Too low: Training is slow
- Recommended: 0.001 to 0.1

### Network Architecture
- More layers = more capacity
- More neurons per layer = more features
- Too few: underfitting
- Too many: overfitting

### Training Tips
1. Normalize input data to [0,1] or [-1,1]
2. Use appropriate activation functions
3. For classification: use softmax + cross-entropy loss
4. Monitor training/validation accuracy
5. Use mini-batches for large datasets

## Activation Quick Reference

| Name | Formula | Use Case |
|------|---------|----------|
| Sigmoid | 1/(1+e^-x) | Binary output |
| ReLU | max(0,x) | Hidden layers (default) |
| Leaky ReLU | x>0?x:0.01x | Prevents dead neurons |
| Tanh | tanh(x) | Hidden layers |
| Softmax | e^x/sum(e^x) | Output layer (classification) |