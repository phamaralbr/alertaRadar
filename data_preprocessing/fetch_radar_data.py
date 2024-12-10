import requests
import os

# Step 1: Fetch the token
def get_token(token_url):
    """
    Fetch the authentication token from the server.

    Args:
        token_url (str): URL to fetch the token.

    Returns:
        str: Retrieved token or None if failed.
    """
    try:
        response = requests.get(token_url)
        response.raise_for_status()  # Raise an exception for HTTP errors
        data = response.json()  # Parse the JSON response
        token = data  # Replace "token" with the actual key in your response

        if not token:
            raise ValueError("Token not found in response.")
        print(f"Token retrieved: {token}")  # Debugging, remove in production
        return token
    except requests.RequestException as e:
        print(f"Failed to fetch token: {e}")
        return None

# Step 2: Validate the token
def validate_token(validation_url, token):
    """
    Validate the token using the server.

    Args:
        validation_url (str): URL to validate the token.
        token (str): Token to validate.

    Returns:
        bool: True if the token is valid (status 200), False otherwise.
    """
    url = f"{validation_url}?token={token}"
    try:
        response = requests.get(url)
        if response.status_code == 200:
            print("Token is valid.")
            return True
        else:
            print(f"Token validation failed with status code {response.status_code}.")
            return False
    except requests.RequestException as e:
        print(f"Failed to validate token: {e}")
        return False

# Step 3: Download the CSV file
def download_csv_with_token(base_url, token, save_path):
    """
    Download a CSV file using a token.

    Args:
        base_url (str): Base URL for the CSV file download.
        token (str): Token to authenticate the request.
        save_path (str): Path to save the downloaded CSV file.
    """
    # Build the complete URL with the token
    params = {
        "token": token,
        "usuario": "phamatal",
        "tipoExportacao": "igo8",
        "estado": "",
        "tipoAlerta": "1,2,4,5,6,7,9,",  # Added trailing comma
        # "tipoAlerta": "1,2,4,5,", # Sem dados de Lombada, Policia, Pedagio
        "direcao": "true",
        "arquivosExternos": "true",
    }

    # Add headers to mimic a browser request
    headers = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
        "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
        "Referer": "https://maparadar.com",  # Update with the correct origin page if necessary
    }

    try:
        # Make the GET request with the token and other query parameters
        response = requests.get(base_url, params=params, headers=headers, stream=True)
        response.raise_for_status()

        # Save the CSV file
        with open(save_path, "wb") as file:
            for chunk in response.iter_content(chunk_size=8192):
                file.write(chunk)

        print(f"CSV file saved to {save_path}")
    except requests.RequestException as e:
        print(f"Failed to download CSV: {e}")

# # Step 4: Main script
# if __name__ == "__main__":
#     # Define URLs
#     TOKEN_URL = "https://v1.api.maparadar.com/Token?usuario=user"  # Token URL
#     TOKEN_VALIDATION_URL = "https://v1.api.maparadar.com/TokenValido"  # Token validation URL
#     BASE_URL = "https://v1.api.maparadar.com/Exporta"  # CSV base URL

#     # Define save location for the CSV
#     SAVE_PATH = os.path.join(os.getcwd(), "coordinates.csv")

#     # Fetch the token
#     token = get_token(TOKEN_URL)

#     # Validate the token and download the CSV if the token is valid
#     if token and validate_token(TOKEN_VALIDATION_URL, token):
#         download_csv_with_token(BASE_URL, token, SAVE_PATH)
#     else:
#         print("Could not retrieve or validate token. Aborting.")
