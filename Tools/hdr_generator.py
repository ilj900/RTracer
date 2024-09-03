import os
os.environ["OPENCV_IO_ENABLE_OPENEXR"] = "1"

import cv2
import numpy as np

# Define image dimensions (e.g., 512x512)
height = 512
width = 512

# Create an empty array for HDR image with float32 values
hdr_image = np.zeros((height, width, 3), dtype=np.float32)

# Set the right half of the image to color values (10, 10, 10)
for i in range(height):
    for j in range(width):
        if j < width / 2:
            hdr_image[i, j] = [0, 0, 0]
        else:
            hdr_image[i, j] = [10, 10, 10]

# Save the image as EXR format
cv2.imwrite('hdr_image.exr', hdr_image)

print("HDR image created and saved as hdr_image.exr")