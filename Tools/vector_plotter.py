import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Function to plot a plane
def plot_plane(ax, point, normal, size=1.0, color='blue', alpha=0.5):
    d = -np.dot(normal, point)  # Plane equation: Ax + By + Cz + D = 0

    # Generate grid of points in the plane
    xx, yy = np.meshgrid(np.linspace(-size, size, 10), np.linspace(-size, size, 10))
    zz = (-normal[0] * xx - normal[1] * yy - d) / normal[2]

    # Plot the plane
    ax.plot_surface(xx, yy, zz, color=color, alpha=alpha)

# Function to plot vectors
def plot_vector(ax, origin, vector, color='red', label=None):
    ax.quiver(
        origin[0], origin[1], origin[2],
        vector[0], vector[1], vector[2],
        color=color, length=np.linalg.norm(vector), normalize=True, label=label
    )

def main():
    # Create a 3D plot
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # Define a point and normal vector for the plane
    plane_point = np.array([0, 0, 0])  # Point on the plane
    plane_normal = np.array([0, 1, 0])  # Normal to the plane

    # Plot the plane
    plot_plane(ax, plane_point, plane_normal, size=2, color='blue', alpha=0.5)

    # Define some vectors to plot
    incoming = np.array([0.577350, 0.577350, 0.577350])
    outgoing = np.array([0.577350, -0.577350, 0.577350])
    normal = np.array([0.000000, 1.000000, 0.000000])

    # Plot the vectors
    plot_vector(ax, np.array([0, 0, 0]), incoming, color='red', label='Incoming')
    plot_vector(ax, np.array([0, 0, 0]), outgoing, color='green', label='Outgoing')
    plot_vector(ax, np.array([0, 0, 0]), normal, color='blue', label='Normal')

    # Set plot limits
    ax.set_xlim([-2, 2])
    ax.set_ylim([-2, 2])
    ax.set_zlim([2, -2])

    # Add labels and legend
    ax.set_xlabel('X-axis')
    ax.set_ylabel('Y-axis')
    ax.set_zlabel('Z-axis')
    ax.legend()

    # Show the plot
    plt.show()

if __name__ == "__main__":
    main()
