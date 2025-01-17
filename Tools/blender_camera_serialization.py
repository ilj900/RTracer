import bpy
import json
from mathutils import Matrix

# Specify the directory for saving the files
output_directory = bpy.path.abspath("//camera_exports/")

# Ensure the directory exists (create it if it doesn't)
import os
if not os.path.exists(output_directory):
    os.makedirs(output_directory)

# Function to calculate direction, right, and up vectors
def get_camera_vectors(camera_obj):
    # Get the camera's world matrix
    matrix_world = camera_obj.matrix_world
    
    # Direction vector (negative Z-axis in Blender's local space)
    direction = -matrix_world.col[2].yzx
    
    # Right vector (positive X-axis in Blender's local space)
    right = matrix_world.col[0].yzx
    
    # Up vector (positive Y-axis in Blender's local space)
    up = matrix_world.col[1].yzx
    
    return {
        "Direction": list(direction),
        "Right": list(right),
        "Up": list(up)
    }

# Loop through all objects in the scene
counter = 0
for obj in bpy.data.objects:
    if obj.type == 'CAMERA':  # Check if the object is a camera
        # Extract camera vectors
        vectors = get_camera_vectors(obj)
        
        # Extract other camera data
        camera_data = {
            "Origin": list(obj.location.yzx),
            "FocalDistance": obj.data.lens / 1000,
            "SensorSizeX": obj.data.sensor_width / 1000,
            "SensorSizeY": obj.data.sensor_height / 1000
        }
        
        # Add vectors to the data
        camera_data.update(vectors)
        
        # Define file name and path
        file_name = "Blender_" + str(counter) + ".json"
        file_path = os.path.join(output_directory, file_name)
        
        # Save the data to a JSON file
        with open(file_path, 'w') as file:
            json.dump(camera_data, file, indent=4)
        
        print(f"Exported camera data for '{obj.name}' to {file_path}")
        counter += 1

print(f"All camera data has been exported to: {output_directory}")
