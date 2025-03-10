import numpy as np
from PIL import Image

def create_roughness_texture(size, center_size):
    roughness = np.zeros((size, size))
    start_idx = (size - center_size) // 2
    end_idx = start_idx + center_size
    roughness[start_idx:end_idx, start_idx:end_idx] = 0.25
    return roughness

# Parameters
size = 1024
center_size = 768

# Create the roughness texture
roughness_texture = create_roughness_texture(size, center_size)

# Convert the texture to an image
roughness_image = Image.fromarray((roughness_texture * 255).astype(np.uint8))  # Scaling values to 255 for grayscale

# Save the image
roughness_image.save("roughness_texture_pil.png")