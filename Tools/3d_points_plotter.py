import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


def load_data(filename):
    """Load 3D vector data from a file."""
    data = np.loadtxt(filename)
    return data


def plot_3d_points(data):
    """Plot 3D points."""
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.scatter(data[:, 0], data[:, 1], data[:, 2], c='b', marker='o')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    plt.show()


if __name__ == "__main__":
    filename = "../data/debug/3d_points.txt"  # Change this to your file name
    data = load_data(filename)
    plot_3d_points(data)
