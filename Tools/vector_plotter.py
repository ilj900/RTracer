import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


def plot_vectors_3d(vector_data):
    """
    Plot 3D vectors based on their starting and ending positions and colors.

    :param vector_data: List of tuples, where each tuple contains
                        ((x_start, y_start, z_start), (x_end, y_end, z_end), 'color')
    """
    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111, projection='3d')

    for data in vector_data:
        start, end, color = data
        x_start, y_start, z_start = start
        x_end, y_end, z_end = end

        # Calculate the vector components
        dx = x_end - x_start
        dy = y_end - y_start
        dz = z_end - z_start

        # Plot the vector
        ax.quiver(x_start, y_start, z_start, dx, dy, dz, color=color)

    # Set plot limits for better visualization
    all_x = [x for (start, end, _) in vector_data for x, _, _ in (start, end)]
    all_y = [y for (start, end, _) in vector_data for _, y, _ in (start, end)]
    all_z = [z for (start, end, _) in vector_data for _, _, z in (start, end)]

    ax.set_xlim(min(all_x) - 1, max(all_x) + 1)
    ax.set_ylim(min(all_y) - 1, max(all_y) + 1)
    ax.set_zlim(min(all_z) - 1, max(all_z) + 1)

    ax.set_xlabel('X-axis')
    ax.set_ylabel('Y-axis')
    ax.set_zlabel('Z-axis')
    ax.set_aspect('equal')
    ax.set_title('3D Vector Plot')
    plt.show()


# Example usage:
vector_data = [
    ((0, 0, 0), (-0.080753, 0.156862, 0.984314), 'r'),  # NewNormal
    ((-0.016088, -0.667443, -0.744487), (0, 0, 0), 'g'),  # TangentSpaceViewDirection before
    ((0, 0, 0), (-0.151140, -0.405106, 0.901690), 'b'),  # TangentSpaceViewDirection after
    ((0, 0, 0), (0.282505, 0.112946, -0.952593), 'c'),  # NewNormal
    ((-0.151140, -0.405106, 0.901690), (0, 0, 0), 'm'),  # TangentSpaceViewDirection before
    ((0, 0, 0), (0.384149, -0.191097, -0.903278), 'y'),  # TangentSpaceViewDirection after
    ((0, 0, 0), (-0.480330, 0.090232, 0.872434), 'k'),  # NewNormal
    ((0.384149, -0.191097, -0.903278), (0, 0, 0), 'navy'),  # TangentSpaceViewDirection before
    ((0, 0, 0), (-0.566723, -0.012472, 0.823814), 'slategrey'),  # TangentSpaceViewDirection after
    ((0, 0, 0), (0.654147, 0.075835, -0.752556), 'pink'),  # NewNormal
    ((-0.566723, -0.012472, 0.823814), (0, 0, 0), 'orange'),  # TangentSpaceViewDirection before
    ((0, 0, 0), (0.730623, 0.137928, -0.668704), 'black'),  # TangentSpaceViewDirection after

]

plot_vectors_3d(vector_data)
