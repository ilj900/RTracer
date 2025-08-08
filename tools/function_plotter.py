import numpy as np
import matplotlib.pyplot as plt

# Define the function in Python
def DistributionGGX(Roughness):
    NDotH = 0.5
    A = Roughness * Roughness
    A2 = A * A
    NDotH2 = NDotH * NDotH
    Nom = A2
    Denom = (NDotH2 * (A2 - 1) + 1)
    Denom = 3.14159265358 * Denom * Denom
    return Nom / Denom
    return

# Generate x values
x = np.linspace(0, 1, 1000)

# Compute y values
y = DistributionGGX(x)

# Plot
plt.plot(x, y, label='f(x) = x² + 2x + 1')
plt.xlabel('x')
plt.ylabel('f(x)')
plt.title('Plot of the C++ Function in Python')
plt.legend()
plt.grid(True)
plt.show()