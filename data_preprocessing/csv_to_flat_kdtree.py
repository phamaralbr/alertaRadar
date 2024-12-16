import struct
import csv
import math
import os

# Define the structure for the KD-Tree node
NODE_SIZE = 21  # 4 + 4 + 1 + 1 + 1 + 2 + 4 + 4 bytes
STRUCT_FORMAT = "<ffBBBHII"  # Longitude, Latitude, Node Type, Speed, Dir Type, Direction, Left Offset, Right Offset

# Haversine formula to calculate distance between two lat, lon points in kilometers
def haversine(lat1, lon1, lat2, lon2):
    R = 6371  # Earth's radius in km
    dlat = math.radians(lat2 - lat1)
    dlon = math.radians(lon2 - lon1)
    a = math.sin(dlat / 2) ** 2 + math.cos(math.radians(lat1)) * math.cos(math.radians(lat2)) * math.sin(dlon / 2) ** 2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    return R * c


# Recursively build the KD-Tree and write nodes to file
def build_kd_tree(file, points, depth=0):
    if not points:
        return None  # No points to process

    # Select axis based on depth
    axis = depth % 2  # 0 = Longitude, 1 = Latitude
    points.sort(key=lambda x: x[axis])  # Sort points by the current axis

    # Find the median and split points
    median_idx = len(points) // 2
    median_point = points[median_idx]

    # Default offsets for children
    left_offset = 0
    right_offset = 0

    # Placeholder node data: (Lon, Lat, Node Type, Speed, Dir Type, Direction, Left Offset, Right Offset)
    node_data = (
        median_point[0],  # Longitude
        median_point[1],  # Latitude
        median_point[2],  # Node Type
        median_point[3],  # Speed
        median_point[4],  # Dir Type
        median_point[5],  # Direction
        left_offset,      # Left offset (placeholder)
        right_offset      # Right offset (placeholder)
    )

    # Write current node to file and get its offset
    current_offset = file.tell()
    file.write(struct.pack(STRUCT_FORMAT, *node_data))

    # Recursively build the left subtree and get its offset
    left_child_offset = build_kd_tree(file, points[:median_idx], depth + 1)
    if left_child_offset is not None:
        left_offset = left_child_offset

    # Recursively build the right subtree and get its offset
    right_child_offset = build_kd_tree(file, points[median_idx + 1:], depth + 1)
    if right_child_offset is not None:
        right_offset = right_child_offset

    # Update current node with correct offsets
    updated_node_data = (
        median_point[0],  # Longitude
        median_point[1],  # Latitude
        median_point[2],  # Node Type
        median_point[3],  # Speed
        median_point[4],  # Dir Type
        median_point[5],  # Direction
        left_offset,      # Left offset (updated)
        right_offset      # Right offset (updated)
    )

    # Rewrite the updated node at the same offset
    file.seek(current_offset)
    file.write(struct.pack(STRUCT_FORMAT, *updated_node_data))
    file.seek(0, 2)  # Move back to the end of the file

    return current_offset


# Main function to build the KD-Tree from CSV and save it to a binary file
def build_tree_from_csv(csv_file, output_bin_file):
    # Read CSV data
    points = []
    with open(csv_file, mode='r') as file:
        reader = csv.reader(file)
        next(reader)  # Skip header
        for row in reader:
            try:
                lon = float(row[0])
                lat = float(row[1])
                node_type = int(row[2])
                speed = int(row[3])
                dir_type = int(row[4])
                direction = int(row[5])
                points.append((lon, lat, node_type, speed, dir_type, direction))
            except ValueError:
                print(f"Skipping invalid row: {row}")

    num_radars = len(points)  # Total number of radars
    print(f"Number of radars in the CSV file: {num_radars}")

    # Open binary file to build the KD Tree
    with open(output_bin_file, 'wb') as bin_file:
        build_kd_tree(bin_file, points)
        print(f"KD-Tree built and saved to {output_bin_file}.")

    # Check file size to confirm all nodes are written
    expected_size = NODE_SIZE * num_radars
    actual_size = os.path.getsize(output_bin_file)
    print(f"Expected file size: {expected_size} bytes")
    print(f"Actual file size: {actual_size} bytes")

    if actual_size == expected_size:
        print("All nodes have been successfully written to the binary file!")
    else:
        print("Warning: File size mismatch! Some nodes may be missing.")


if __name__ == "__main__":
    import os

    csv_file = 'data_output/coordinates.csv'  # Path to your CSV file
    output_bin_file = 'data_output/radars.bin'  # Output binary file path
    
    build_tree_from_csv(csv_file, output_bin_file)
