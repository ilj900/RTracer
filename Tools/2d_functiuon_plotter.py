import numpy as np
import matplotlib.pyplot as plt


# Define the function
def f(x):
    return x ** 2


# Generate x values (e.g., from -10 to 10)
x = np.linspace(-10, 10, 100)

# Compute y values
y = f(x)

# Create the plot
plt.plot(x, y, label='f(x) = x^2')

# Add labels and title
plt.xlabel('x')
plt.ylabel('f(x)')
plt.title('Plot of f(x) = x^2')

# Add a legend
plt.legend()

# Show the plot
plt.grid(True)
plt.show()