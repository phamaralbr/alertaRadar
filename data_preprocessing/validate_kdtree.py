import pickle

# Define the Point class to match the serialized structure
class Point:
    def __init__(self, x, y, point_type, speed, dir_type, direction):
        self.x = x
        self.y = y
        self.point_type = point_type
        self.speed = speed
        self.dir_type = dir_type
        self.direction = direction

    def __repr__(self):
        return f"Point(x={self.x}, y={self.y}, type={self.point_type}, speed={self.speed}, dir_type={self.dir_type}, direction={self.direction})"

# Define the KdNode class to match the serialized structure
class KdNode:
    def __init__(self, point, left=None, right=None):
        self.point = point
        self.left = left
        self.right = right

# Define a recursive function to traverse the k-d tree
def traverse_kd_tree(node, points_set, duplicates):
    if node is None:
        return 0  # No nodes here

    # Convert the point data into a hashable tuple for tracking
    point_data = (node.point.x, node.point.y, node.point.point_type, 
                  node.point.speed, node.point.dir_type, node.point.direction)

    # Check for duplicates
    if point_data in points_set:
        duplicates.add(point_data)
    else:
        points_set.add(point_data)

    # Count this node and recurse into left and right subtrees
    return 1 + traverse_kd_tree(node.left, points_set, duplicates) + traverse_kd_tree(node.right, points_set, duplicates)

# Function to load and validate the k-d tree
def validate_kd_tree(binary_file, expected_points_count):
    """
    Validates the KD-tree:
    1. Ensures there are no duplicate points.
    2. Ensures the total number of points matches the expected count.

    :param binary_file: Path to the KD-tree binary file.
    :param expected_points_count: The expected number of points (from the CSV file).
    :return: True if the KD-tree is valid, False otherwise.
    """
    # Load the k-d tree from the binary file
    try:
        with open(binary_file, "rb") as f:
            kd_tree_root = pickle.load(f)
    except Exception as e:
        print_error(f"Failed to load KD-tree: {e}")
        return False

    # Initialize containers for checking
    points_set = set()
    duplicates = set()

    # Traverse the tree
    total_points = traverse_kd_tree(kd_tree_root, points_set, duplicates)

    # Log results
    print(f"Total points in KD-tree: {total_points}")
    print(f"Unique points: {len(points_set)}")
    print(f"Duplicate points: {len(duplicates)}")

    # Validation checks
    if duplicates:
        return False

    if total_points != expected_points_count:
        return False

    return True
