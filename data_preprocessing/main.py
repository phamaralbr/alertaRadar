from fetch_radar_data import get_token, validate_token, download_csv_with_token
from csv_to_kdtree import csv_to_binary_kd_tree
import os

# Load environment variables from .env
load_dotenv()

# Define constants for the radar data fetching process
MAPARADAR_USER = os.getenv("MAPARADAR_USER", "default_user")  # Fallback to "default_user" if not set
TOKEN_URL = "https://v1.api.maparadar.com/Token?usuario={MAPARADAR_USER}"  # URL to fetch the token
TOKEN_VALIDATION_URL = "https://v1.api.maparadar.com/TokenValido"  # URL to validate the token
BASE_URL = "https://v1.api.maparadar.com/Exporta"  # URL to download the CSV
CSV_FILE = "coordinates.csv"  # Local path to save the downloaded CSV
KD_TREE_FILE = "kd_tree.bin"  # Local path to save the binary KD-tree

def fetch_and_validate_data():
    """
    Fetches the token, validates it, and downloads the radar data as a CSV file.
    """
    print("Step 1: Fetching token...")
    token = get_token(TOKEN_URL)
    if not token:
        print("Failed to retrieve token. Exiting.")
        return False

    print("Step 2: Validating token...")
    if not validate_token(TOKEN_VALIDATION_URL, token):
        print("Token validation failed. Exiting.")
        return False

    print("Step 3: Downloading radar data as CSV...")
    download_csv_with_token(BASE_URL, token, CSV_FILE)
    return True

def process_csv_to_kdtree():
    """
    Converts the downloaded CSV file into a binary KD-tree format.
    """
    print("Step 4: Converting CSV to binary KD-tree...")
    if not os.path.exists(CSV_FILE):
        print(f"CSV file '{CSV_FILE}' not found. Did the download fail?")
        return False

    csv_to_binary_kd_tree(CSV_FILE, KD_TREE_FILE)
    return True

def main():
    """
    Main function to orchestrate radar data download and preprocessing.
    """
    print("=== Radar Data Processing Pipeline ===")

    # Step 1: Fetch, validate token, and download radar data
    if not fetch_and_validate_data():
        print("Data retrieval process failed. Aborting.")
        return

    # Step 2: Process CSV to KD-tree
    if not process_csv_to_kdtree():
        print("CSV to KD-tree conversion failed. Aborting.")
        return

    print("=== Pipeline Completed Successfully ===")
    print(f"CSV saved as: {CSV_FILE}")
    print(f"KD-tree saved as: {KD_TREE_FILE}")

if __name__ == "__main__":
    main()
