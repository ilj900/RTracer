import numpy as np
from PIL import Image
import matplotlib.pyplot as plt


def create_roughness_texture(size=512, center_square_ratio=0.3, save_path="roughness_texture.png"):
    """
    Create a square roughness texture where the edges are 0 and a center square is 1.

    Parameters:
    -----------
    size : int
        Size of the output image (will be size x size pixels)
    center_square_ratio : float
        Ratio of the center square size to the full image size (0.0 to 1.0)
    save_path : str
        Path to save the output image

    Returns:
    --------
    numpy.ndarray
        The generated roughness texture
    """
    # Create an empty image (all zeros)
    texture = np.zeros((size, size), dtype=np.float32)

    # Calculate the center square dimensions
    center_size = int(size * center_square_ratio)
    start_idx = (size - center_size) // 2
    end_idx = start_idx + center_size

    # Set the center square to 1
    texture[start_idx:end_idx, start_idx:end_idx] = 0.25

    # Save the image
    plt.figure(figsize=(6, 6))
    plt.imshow(texture, cmap='gray', vmin=0, vmax=1)
    plt.title("Roughness Texture")
    plt.colorbar(label="Roughness Value")
    plt.savefig(save_path, dpi=300, bbox_inches='tight')
    plt.close()

    # Also save as a raw PNG (without matplotlib formatting)
    im = Image.fromarray((texture * 255).astype(np.uint8))
    im.save(save_path.replace(".png", "_raw.png"))

    print(f"Roughness texture saved to {save_path} and {save_path.replace('.png', '_raw.png')}")

    return texture


# Example usage
if __name__ == "__main__":
    texture = create_roughness_texture(
        size=1024,
        center_square_ratio=0.75,
        save_path="roughness_texture.png"
    )