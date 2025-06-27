import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np


def plot_vectors(vector_data, figsize=(12, 9), colors=None):
    """
    Plot 3D vectors from vector data.

    Parameters:
    vector_data: list of tuples, where each tuple contains:
        - origin: tuple of 3 floats (x, y, z coordinates of vector origin)
        - direction: tuple of 3 floats (x, y, z components of vector direction)
        - color_index: int (index for color selection)
    figsize: tuple, figure size (width, height)
    colors: list of colors, if None uses default color cycle
    """

    # Create figure and 3D axis
    fig = plt.figure(figsize=figsize)
    ax = fig.add_subplot(111, projection='3d')

    # Default colors if none provided
    if colors is None:
        colors = ['red', 'blue', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan']

    # Extract and plot each vector
    for origin, direction, color_idx in vector_data:
        # Extract coordinates
        x0, y0, z0 = origin
        dx, dy, dz = direction

        # Select color based on index
        color = colors[color_idx % len(colors)]

        # Plot vector as arrow
        ax.quiver(x0, y0, z0, dx, dy, dz,
                  color=color, arrow_length_ratio=0.1, linewidth=2)

    # Set labels and title
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.set_title('3D Vector Plot')

    # Create legend
    unique_indices = sorted(set([color_idx for _, _, color_idx in vector_data]))
    legend_elements = [plt.Line2D([0], [0], color=colors[idx % len(colors)],
                                  linewidth=2, label=f'Color Index {idx}')
                       for idx in unique_indices]
    ax.legend(handles=legend_elements)

    # Set equal aspect ratio for better visualization
    ax.set_box_aspect([1, 1, 1])

    plt.tight_layout()
    plt.show()


# Example usage with your data
if __name__ == "__main__":
    vector_data = [
        ((-6.059510, 4.487666, 0.093596), (0.907407, -0.180059, -0.379725), 0),
        ((-5.152103, 4.307607, -0.286130), (-0.952127, 0.107609, -0.286137), 1),
        ((-5.152103, 4.307607, -0.286130), (0.973925, -0.163524, -0.157259), 2),
        ((-4.373211, 4.176829, -0.411897), (0.973925, -0.163524, -0.157259), 0),
        ((-3.399287, 4.013305, -0.569156), (-0.800739, 0.186701, 0.569174), 1),
        ((-3.399287, 4.013305, -0.569156), (0.987666, -0.138127, 0.073732), 2),
        ((-3.894635, 4.082581, -0.606135), (0.987666, -0.138127, 0.073732), 0),
        ((-2.906969, 3.944453, -0.532403), (-0.806987, -0.255552, -0.532415), 1),
        ((-2.906969, 3.944453, -0.532403), (0.969154, 0.012614, 0.246134), 2),
        ((-2.099964, 3.954957, -0.327450), (0.969154, 0.012614, 0.246134), 0),
        ((-1.130810, 3.967571, -0.081316), (-0.969207, 0.232434, 0.081317), 1),
        ((-1.130810, 3.967571, -0.081316), (0.910467, 0.158581, 0.381970), 2),
    ]

    # Plot the vectors
    plot_vectors(vector_data)