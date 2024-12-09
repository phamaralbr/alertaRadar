from fetch_radar_data import get_token, validate_token, download_csv_with_token
from csv_to_kdtree import csv_to_binary_kd_tree
from validate_kdtree import validate_kd_tree
from dotenv import load_dotenv
import os

# Load environment variables from .env
load_dotenv()

# Define constants for the radar data fetching process
MAPARADAR_USER = os.getenv("MAPARADAR_USER", "default_user")
TOKEN_URL = f"https://v1.api.maparadar.com/Token?usuario={MAPARADAR_USER}"
TOKEN_VALIDATION_URL = "https://v1.api.maparadar.com/TokenValido"
BASE_URL = "https://v1.api.maparadar.com/Exporta"
CSV_FILE = "data_output/coordinates.csv"
KD_TREE_FILE = "data_output/kd_tree.bin"

# ANSI escape codes for colors and styles
RESET = "\033[0m"
BOLD = "\033[1m"
CYAN = "\033[36m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
RED = "\033[31m"

def print_step(step_message):
    """Prints step messages with styling for better readability."""
    print(f"{BOLD}{CYAN}{step_message}{RESET}")

def print_success(message):
    """Prints success messages in green."""
    print(f"{GREEN}{message}{RESET}")

def print_warning(message):
    """Prints warnings in yellow."""
    print(f"{YELLOW}{message}{RESET}")

def print_error(message):
    """Prints errors in red."""
    print(f"{RED}{message}{RESET}")

def fetch_and_validate_data():
    print_step("Step 1: Fetching token...")
    token = get_token(TOKEN_URL)
    if not token:
        print_error("Failed to retrieve token. Exiting.")
        return False

    print_step("Step 2: Validating token...")
    if not validate_token(TOKEN_VALIDATION_URL, token):
        print_error("Token validation failed. Exiting.")
        return False

    print_step("Step 3: Downloading radar data as CSV...")
    download_csv_with_token(BASE_URL, token, CSV_FILE)
    print_success(f"CSV file saved to {CSV_FILE}")
    return True

def process_csv_to_kdtree():
    print_step("Step 4: Converting CSV to binary KD-tree...")
    if not os.path.exists(CSV_FILE):
        print_warning(f"CSV file '{CSV_FILE}' not found. Did the download fail?")
        return False

    global expected_points_count
    expected_points_count = csv_to_binary_kd_tree(CSV_FILE, KD_TREE_FILE)
    return True

def validate_kd_tree_file(expected_points_count):
    print_step("Step 5: Validating the KD-tree for correctness...")
    if not os.path.exists(KD_TREE_FILE):
        print_warning(f"KD-tree file '{KD_TREE_FILE}' not found. Aborting validation.")
        return False

    if validate_kd_tree(KD_TREE_FILE, expected_points_count):
        print_success("KD-tree validation passed successfully!")
        return True
    else:
        print_error("KD-tree validation failed. Data may be incorrect or incomplete.")
        return False

def main():
    print(f"{BOLD}{CYAN}=== Radar Data Processing Pipeline ==={RESET}")

    if not fetch_and_validate_data():
        print_error("Data retrieval process failed. Aborting.")
        return

    if not process_csv_to_kdtree():
        print_error("CSV to KD-tree conversion failed. Aborting.")
        return

    if not validate_kd_tree_file(expected_points_count):
        print_error("KD-tree validation failed. Please check the input data.")
    else:
        print_success("=== Pipeline Completed Successfully ===")
        print_success(f"CSV saved as: {CSV_FILE}")
        print_success(f"KD-tree saved as: {KD_TREE_FILE}")

if __name__ == "__main__":
    main()
