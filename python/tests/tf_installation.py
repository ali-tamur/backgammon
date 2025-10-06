import tensorflow as tf
from tensorflow import keras

print("TensorFlow version:", tf.__version__)
print("Keras version:", keras.__version__)

# Create a simple tensor
hello_world = tf.constant("Hello, TensorFlow!")
print(hello_world)
