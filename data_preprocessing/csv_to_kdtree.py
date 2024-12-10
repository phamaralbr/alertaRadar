# import csv
# import pickle  # For binary serialization

# # Define a Point structure to store data
# class Point:
#     def __init__(self, x, y, point_type, speed, dir_type, direction):
#         self.x = x  # Longitude
#         self.y = y  # Latitude
#         self.point_type = point_type
#         self.speed = speed
#         self.dir_type = dir_type
#         self.direction = direction

#     def __repr__(self):
#         return f"Point(x={self.x}, y={self.y}, type={self.point_type}, speed={self.speed}, dir_type={self.dir_type}, direction={self.direction})"

# # Define a k-d tree node
# class KdNode:
#     def __init__(self, point, left=None, right=None):
#         self.point = point
#         self.left = left
#         self.right = right

# # Build the k-d tree
# def build_kd_tree(points, depth=0):
#     if not points:
#         return None

#     # Alternate between splitting by x (longitude) and y (latitude)
#     axis = depth % 2
#     points.sort(key=lambda p: p.x if axis == 0 else p.y)
#     median_idx = len(points) // 2

#     # Recursive construction
#     return KdNode(
#         point=points[median_idx],
#         left=build_kd_tree(points[:median_idx], depth + 1),
#         right=build_kd_tree(points[median_idx + 1:], depth + 1),
#     )

# # Serialize the k-d tree to a binary file
# def save_kd_tree_to_file(root, filename):
#     with open(filename, "wb") as f:
#         pickle.dump(root, f)

# # Deserialize the k-d tree from a binary file
# def load_kd_tree_from_file(filename):
#     with open(filename, "rb") as f:
#         return pickle.load(f)

# # Read CSV and create points
# def read_csv_to_points(csv_filename):
#     points = []
#     with open(csv_filename, "r") as csvfile:
#         reader = csv.DictReader(csvfile)
#         for row in reader:
#             point = Point(
#                 x=float(row["X"]),
#                 y=float(row["Y"]),
#                 point_type=int(row["TYPE"]),
#                 speed=int(row["SPEED"]),
#                 dir_type=int(row["DirType"]),
#                 direction=int(row["Direction"]),
#             )
#             points.append(point)
#     return points

# # Main conversion process
# def csv_to_binary_kd_tree(csv_filename, output_filename):
#     # Step 1: Read the CSV file
#     points = read_csv_to_points(csv_filename)
#     total_points = len(points)
#     print(f"Total coordinates in CSV: {total_points}")

#     # Step 2: Build the k-d tree
#     kd_tree_root = build_kd_tree(points)

#     # Count the nodes added to the k-d tree
#     def count_nodes(node):
#         if node is None:
#             return 0
#         return 1 + count_nodes(node.left) + count_nodes(node.right)

#     added_points = count_nodes(kd_tree_root)
#     print(f"Total coordinates added to the k-d tree: {added_points}")

#     # Step 3: Save the k-d tree to a binary file
#     save_kd_tree_to_file(kd_tree_root, output_filename)
#     print(f"K-d tree saved to {output_filename}")

#     return total_points

# # Example usage
# if __name__ == "__main__":
#     csv_filename = "coordinates.csv"  # Replace with your CSV file
#     output_filename = "kd_tree.bin"   # Output binary file

#     csv_to_binary_kd_tree(csv_filename, output_filename)

#     # Load and verify
#     root = load_kd_tree_from_file(output_filename)
#     print("Loaded k-d tree root:", root.point)



import struct

# Define a RadarPoint class to store the radar data
class RadarPoint:
    def __init__(self, x, y, point_type, speed, dir_type, direction):
        self.x = x  # longitude
        self.y = y  # latitude
        self.point_type = point_type
        self.speed = speed
        self.dir_type = dir_type
        self.direction = direction

    def to_bytes(self):
        # Serialize as a binary string with 4 bytes for x, 4 bytes for y, and 2 bytes each for other fields
        return struct.pack('>ffHHHH', self.x, self.y, self.point_type, self.speed, self.dir_type, self.direction)

# K-D Tree Node class
class KdNode:
    def __init__(self, point, left=None, right=None):
        self.point = point
        self.left = left
        self.right = right

# Function to build the KD Tree
def build_kd_tree(points, depth=0):
    if not points:
        return None

    # Select axis based on depth (0 = x, 1 = y)
    axis = depth % 2
    points.sort(key=lambda p: p.x if axis == 0 else p.y)  # Sort by x or y
    median = len(points) // 2

    # Create a node at the median
    node = KdNode(points[median])

    # Recursively build left and right subtrees
    node.left = build_kd_tree(points[:median], depth + 1)
    node.right = build_kd_tree(points[median + 1:], depth + 1)

    return node

# Function to serialize the KD Tree
def serialize_kd_tree(node, radar_counter=None):
    if radar_counter is None:
        radar_counter = {'count': 0}

    if node is None:
        return b'\x00'  # Null node

    # Serialize current node's point
    point_data = node.point.to_bytes()
    radar_counter['count'] += 1

    # Recursively serialize left and right children
    left_data = serialize_kd_tree(node.left, radar_counter)
    right_data = serialize_kd_tree(node.right, radar_counter)

    # Return combined data: point + left + right
    return b'\x01' + point_data + left_data + right_data

# Read data from CSV
def read_csv(filename):
    radar_points = []
    with open(filename, 'r') as file:
        lines = file.readlines()
        for line in lines[1:]:  # Skip header
            x, y, point_type, speed, dir_type, direction = line.strip().split(',')
            radar_points.append(RadarPoint(float(x), float(y), int(point_type), int(speed), int(dir_type), int(direction)))
    return radar_points

# # Main process
# def main():
#     # Load CSV data
#     radar_points = read_csv('coordinates.csv')
#     print(f"Total radars in CSV: {len(radar_points)}")

#     # Build the KD tree
#     kd_tree = build_kd_tree(radar_points)

#     # Serialize the KD tree to binary format
#     radar_counter = {'count': 0}
#     serialized_data = serialize_kd_tree(kd_tree, radar_counter)

#     # Write serialized data to a binary file
#     with open('kd_tree.bin', 'wb') as f:
#         f.write(serialized_data)

#     print(f"Total radars added to binary file: {radar_counter['count']}")
#     print("KD Tree serialized and written to 'kd_tree.bin'")

# if __name__ == '__main__':
#     main()


def csv_to_binary_kd_tree(csv_filename, output_filename):
    # Step 1: Read the CSV file
    radar_points = read_csv(csv_filename)
    print(f"Total radars in CSV: {len(radar_points)}")

    # Step 2: Build the k-d tree
    kd_tree = build_kd_tree(radar_points)

    # Serialize the KD tree to binary format
    radar_counter = {'count': 0}
    serialized_data = serialize_kd_tree(kd_tree, radar_counter)

    # Write serialized data to a binary file
    with open(output_filename, 'wb') as f:
        f.write(serialized_data)

    print(f"Total radars added to binary file: {radar_counter['count']}")
    print(f"K-d tree serialized and saved to {output_filename}")
