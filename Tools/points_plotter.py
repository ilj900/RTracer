import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

# Function to read the 3D points from the file
def read_points_from_file(file_path):
    points = []
    with open(file_path, 'r') as file:
        for line in file:
            # Assuming the file contains x, y, z coordinates separated by spaces
            x, y, z = map(float, line.split())
            points.append([x, y, z])
    return np.array(points)


# Function to plot the 3D points
def plot_3d_points(points):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # Split the points into x, y, z arrays
    xs = points[:, 0]
    ys = points[:, 1]
    zs = points[:, 2]

    # Scatter plot of the points
    ax.scatter(xs, ys, zs, c='r', marker='o')

    ax.set_xlabel('X Label')
    ax.set_ylabel('Y Label')
    ax.set_zlabel('Z Label')

    ax.set_box_aspect((np.ptp(xs), np.ptp(ys), np.ptp(zs)))
    plt.show()


# Example usage:
file_path = 'points.txt'  # Path to your text file containing 3D points
points = read_points_from_file(file_path)
plot_3d_points(points)