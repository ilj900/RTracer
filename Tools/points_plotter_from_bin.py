import struct
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


# Function to read binary file and extract 3D points
def read_binary_file(filename):
    points = []
    with open(filename, 'rb') as file:
        while chunk := file.read(12):  # 3 floats of 4 bytes each
            if len(chunk) == 12:
                x, y, z = struct.unpack('fff', chunk)
                points.append((x, y, z))
            else:
                raise ValueError("File size is not a multiple of 12 bytes (3 floats per point).")
    return points


# Function to plot 3D points
def plot_points(points):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # Unpack points into x, y, z lists
    x_coords, y_coords, z_coords = zip(*points)

    ax.scatter(x_coords, y_coords, z_coords, c='blue', marker='o')
    ax.set_xlabel('X Coordinate')
    ax.set_ylabel('Y Coordinate')
    ax.set_zlabel('Z Coordinate')

    ax.set_box_aspect((1, 0.5, 1))
    plt.title("3D Point Cloud")
    plt.show()


# Main execution
if __name__ == "__main__":
    # Replace 'points.bin' with your binary file's path
    binary_file = 'Sampled3DSphere.bin'

    try:
        points = read_binary_file(binary_file)
        print(f"Read {len(points)} points from the binary file.")
        plot_points(points)
    except Exception as e:
        print(f"An error occurred: {e}")