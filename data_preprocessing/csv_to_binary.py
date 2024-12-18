# import struct
# import csv
# import os

# def convert_csv_to_bin(input_csv, output_bin):
#     radars = []
#     radar_format = "<ffBBBH"  # Little-endian: 2 floats (4 bytes each), 3 uint8, 1 uint16
#     radar_size = struct.calcsize(radar_format)  # 13 bytes per radar

#     # Step 1: Read CSV and parse data
#     with open(input_csv, 'r') as csv_file:
#         csv_reader = csv.reader(csv_file)
#         header = next(csv_reader)  # Skip the header row

#         for row in csv_reader:
#             x = float(row[0])  # Longitude
#             y = float(row[1])  # Latitude
#             node_type = int(row[2])
#             speed = int(row[3])
#             dir_type = int(row[4])
#             direction = int(row[5])
            
#             # Append the radar data as a tuple
#             radars.append((x, y, node_type, speed, dir_type, direction))

#     # Step 2: Sort radars by latitude (y)
#     radars.sort(key=lambda radar: radar[1])

#     # Step 3: Write to binary file
#     with open(output_bin, 'wb') as bin_file:
#         for radar in radars:
#             bin_file.write(struct.pack(radar_format, *radar))

#     # Step 4: Log information
#     total_radars = len(radars)
#     expected_size = total_radars * radar_size
#     actual_size = os.path.getsize(output_bin)

#     print("Conversion complete!")
#     print(f"Total radars: {total_radars}")
#     print(f"Expected binary file size: {expected_size} bytes")
#     print(f"Actual binary file size: {actual_size} bytes")

#     # Check for discrepancies
#     if expected_size == actual_size:
#         print("File size check PASSED.")
#     else:
#         print("File size check FAILED! Please verify data.")

# # Input and output file names
# input_csv = "data_output/coordinates.csv"
# output_bin = "data_output/radars.bin"

# # Run the conversion
# convert_csv_to_bin(input_csv, output_bin)



import struct
import csv
import os

def convert_csv_to_bin(input_csv, output_bin, output_csv):
    radars = []
    radar_format = "<ffBBBH"  # Little-endian: 2 floats (4 bytes each), 3 uint8, 1 uint16
    radar_size = struct.calcsize(radar_format)  # 13 bytes per radar

    # Step 1: Read CSV and parse data
    with open(input_csv, 'r') as csv_file:
        csv_reader = csv.reader(csv_file)
        header = next(csv_reader)  # Skip the header row

        for row in csv_reader:
            x = float(row[0])  # Longitude
            y = float(row[1])  # Latitude
            node_type = int(row[2])
            speed = int(row[3])
            dir_type = int(row[4])
            direction = int(row[5])
            
            # Append the radar data as a tuple
            radars.append((x, y, node_type, speed, dir_type, direction))

    # Step 2: Sort radars by latitude (y)
    radars.sort(key=lambda radar: radar[1])

    # Step 3: Write to binary file
    with open(output_bin, 'wb') as bin_file:
        for radar in radars:
            bin_file.write(struct.pack(radar_format, *radar))

    # Step 4: Write the sorted radars to a new CSV file
    with open(output_csv, 'w', newline='') as csv_file:
        csv_writer = csv.writer(csv_file)
        csv_writer.writerow(["Longitude", "Latitude", "Node Type", "Speed", "Direction Type", "Direction"])  # Write header
        for radar in radars:
            csv_writer.writerow(radar)

    # Step 5: Log information
    total_radars = len(radars)
    expected_size = total_radars * radar_size
    actual_size = os.path.getsize(output_bin)

    print("Conversion complete!")
    print(f"Total radars: {total_radars}")
    print(f"Expected binary file size: {expected_size} bytes")
    print(f"Actual binary file size: {actual_size} bytes")

    # Check for discrepancies
    if expected_size == actual_size:
        print("File size check PASSED.")
    else:
        print("File size check FAILED! Please verify data.")

# Input, output binary, and output CSV file names
input_csv = "data_output/coordinates.csv"
output_bin = "data_output/radars.bin"
output_csv = "data_output/radars_sorted.csv"

# Run the conversion
convert_csv_to_bin(input_csv, output_bin, output_csv)
