import numpy as np
from PIL import Image


def create_roughness_texture(image_size=1024, center_square_ratio=0.75,
                             edge_roughness=0, center_roughness=1,
                             save_path="roughness_texture.png"):
    # Create texture with edge_roughness
    texture = np.ones((image_size, image_size), dtype=np.float32) * edge_roughness

    # Calculate center square dimensions
    center_size = int(image_size * center_square_ratio)
    start_pixel = (image_size - center_size) // 2
    end_pixel = start_pixel + center_size

    # Set center square to center_roughness
    texture[start_pixel:end_pixel, start_pixel:end_pixel] = center_roughness

    # Apply inverse gamma correction to counteract sRGB conversion
    gamma_corrected = np.power(texture, 1 / 2.2)

    # Convert to 8-bit grayscale image
    texture_image = (gamma_corrected * 255).astype(np.uint8)

    # Save the image
    img = Image.fromarray(texture_image, mode='L')
    img.save(save_path)

    print(f"Roughness texture saved to {save_path}")
    return texture


# Example usage
if __name__ == "__main__":
    # Default: edges=0, center=1
    create_roughness_texture(save_path="roughness_0_1.png")

    # Edges=0, center=0.5
    create_roughness_texture(center_roughness=0.5, save_path="roughness_0_0.5.png")

    # Edges=0, center=0.25
    create_roughness_texture(center_roughness=0.25, save_path="roughness_0_0.25.png")