import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def plot_vectors_3d(start_points, end_points):
    """
    Plot 3D vectors based on their starting and ending positions.

    :param start_points: List of tuples representing starting points [(x1, y1, z1), (x2, y2, z2), ...]
    :param end_points: List of tuples representing ending points [(x1, y1, z1), (x2, y2, z2), ...]
    """
    if len(start_points) != len(end_points):
        raise ValueError("Start points and end points must have the same length")

    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111, projection='3d')

    for start, end in zip(start_points, end_points):
        x_start, y_start, z_start = start
        x_end, y_end, z_end = end

        # Calculate the vector components
        dx = x_end - x_start
        dy = y_end - y_start
        dz = z_end - z_start

        # Plot the vector
        ax.quiver(x_start, y_start, z_start, dx, dy, dz, color='b')

    # Set plot limits for better visualization
    all_x = [x for x, y, z in start_points + end_points]
    all_y = [y for x, y, z in start_points + end_points]
    all_z = [z for x, y, z in start_points + end_points]

    ax.set_xlim(min(all_x) - 1, max(all_x) + 1)
    ax.set_ylim(min(all_y) - 1, max(all_y) + 1)
    ax.set_zlim(min(all_z) - 1, max(all_z) + 1)

    ax.set_xlabel('X-axis')
    ax.set_ylabel('Y-axis')
    ax.set_zlabel('Z-axis')
    ax.set_title('3D Vector Plot')
    plt.show()


# Example usage:
start_points =  [(0.533339, -0.411958, 0.409748),   (0.784067, -0.545836, 0.830608),                                    (0.783947, -0.545739, 0.830974),    (0.906054, -0.507553, 0.982147),                                    (0.906054, -0.507553, 0.982147)]
end_points =    [(0.784067, -0.545836, 0.830608),   (0.784067 - 0.218911, -0.545836 - 0.298953, 0.830608 - 0.150459),   (0.906054, -0.507553, 0.982147),    (0.906054 - 0.032676, -0.507553 + 0.149498, 0.982147 - 0.369274),   (0.906054 + 0.404474, -0.507553 + 0.126490, 0.982147 + 0.500750)]

plot_vectors_3d(start_points, end_points)
