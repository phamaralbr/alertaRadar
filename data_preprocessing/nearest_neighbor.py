import struct
import math

# Global variable to track the number of recursions
recursion_count = 0

def read_node(file, offset):
    """Reads a node from the file at the given offset."""
    file.seek(offset)
    data = file.read(21)  # Each node is exactly 21 bytes
    
    if len(data) != 21:
        return None  # Return None if the data is incomplete

    # Unpack the node data into respective values
    lon, lat, node_type, speed, direction_type, direction, left_offset, right_offset = struct.unpack('<ffBBBHII', data)
    
    # Return the node data as a dictionary
    return {
        'lon': lon,
        'lat': lat,
        'node_type': node_type,
        'speed': speed,
        'direction_type': direction_type,
        'direction': direction,
        'left_offset': left_offset,
        'right_offset': right_offset,
    }

def haversine(lon1, lat1, lon2, lat2):
    """Calculates the great-circle distance between two points using the Haversine formula."""
    R = 6371  # Earth radius in kilometers
    lon1, lat1, lon2, lat2 = map(math.radians, [lon1, lat1, lon2, lat2])
    dlon = lon2 - lon1
    dlat = lat2 - lat1
    a = math.sin(dlat / 2) ** 2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon / 2) ** 2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    return R * c  # Return the distance in kilometers

def nearest_neighbor_search(file, target_lon, target_lat, offset=0, depth=0, best=None, is_root=True):
    """Recursively search the KD Tree for the nearest neighbor."""
    global recursion_count
    recursion_count += 1  # Increment the recursion count each time this function is called

    if not is_root and offset == 0:
        return best  # Return the best result if no child exists

    node = read_node(file, offset)
    if node is None:
        return best  # Return best if reading the node fails

    # Compute distance from target to current node
    current_distance = haversine(target_lon, target_lat, node['lon'], node['lat'])

    # Update the best result if the current node is closer
    if best is None or current_distance < best['distance']:
        best = {'node': node, 'distance': current_distance}  # Save the entire node instead of just coordinates

    # Determine which subtree to search based on the current depth (longitude or latitude split)
    axis = depth % 2
    target_value = target_lon if axis == 0 else target_lat
    node_value = node['lon'] if axis == 0 else node['lat']

    # Choose primary and secondary subtrees based on target's value
    if target_value < node_value:
        primary_offset = node['left_offset']
        secondary_offset = node['right_offset']
    else:
        primary_offset = node['right_offset']
        secondary_offset = node['left_offset']

    # Recursively search the primary subtree
    best = nearest_neighbor_search(file, target_lon, target_lat, primary_offset, depth + 1, best, is_root=False)

    # Check if we need to search the secondary subtree
    if abs(target_value - node_value) < best['distance']:
        best = nearest_neighbor_search(file, target_lon, target_lat, secondary_offset, depth + 1, best, is_root=False)

    return best
    
def main():
    tree_file = 'data_output/radars.bin'  # File containing the KD tree data
    target_lon, target_lat = -47.809293, -15.882209  # Target coordinates for nearest neighbor search

    with open(tree_file, 'rb') as file:
        result = nearest_neighbor_search(file, target_lon, target_lat)  # Search for the nearest neighbor

    # Print the result
    if result:
        print(f"Nearest Neighbor: {result['node']}, Distance: {result['distance']:.3f} km")
    else:
        print("No nearest neighbor found.")
    
    # Log the number of recursions
    print(f"Number of recursions: {recursion_count}")

if __name__ == "__main__":
    main()
