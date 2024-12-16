from fetch_radar_data import get_token, validate_token, download_csv_with_token
from csv_to_flat_kdtree import build_tree_from_csv
from dotenv import load_dotenv
import os

# Load environment variables from .env
load_dotenv()

# Define constants for the radar data fetching process
MAPARADAR_USER = os.getenv("MAPARADAR_USER", "usuario")
TOKEN_URL = f"https://v1.api.maparadar.com/Token?usuario={MAPARADAR_USER}"
TOKEN_VALIDATION_URL = "https://v1.api.maparadar.com/TokenValido"
BASE_URL = "https://v1.api.maparadar.com/Exporta"
CSV_FILE = "data_output/coordinates.csv"
KD_TREE_FILE = "data_output/radars.bin"

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
    print_step(TOKEN_URL)
    token = get_token(TOKEN_URL)
    if not token:
        print_error("Failed to retrieve token. Exiting.")
        return False

    print_step("Step 2: Validating token...")
    if not validate_token(TOKEN_VALIDATION_URL, token):
        print_error("Token validation failed. Exiting.")
        return False

    print_step("Step 3: Downloading radar data as CSV...")
    download_csv_with_token(BASE_URL, token, CSV_FILE, MAPARADAR_USER)
    print_success(f"CSV file saved to {CSV_FILE}")
    return True

def process_csv_to_kdtree():
    print_step("Step 4: Converting CSV to binary KD-tree...")
    if not os.path.exists(CSV_FILE):
        print_warning(f"CSV file '{CSV_FILE}' not found. Did the download fail?")
        return False

    global expected_points_count
    expected_points_count = build_tree_from_csv(CSV_FILE, KD_TREE_FILE)
    return True

def main():
    print(f"{BOLD}{CYAN}=== Radar Data Processing Pipeline ==={RESET}")

    # if not fetch_and_validate_data():
    #     print_error("Data retrieval process failed. Aborting.")
    #     return

    if not process_csv_to_kdtree():
        print_error("CSV to KD-tree conversion failed. Aborting.")
        return

    print_success("=== Pipeline Completed ===")
    print_success(f"CSV saved as: {CSV_FILE}")
    print_success(f"KD-tree saved as: {KD_TREE_FILE}")

if __name__ == "__main__":
    main()
