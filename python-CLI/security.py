import os

# Default API key
DEFAULT_API_KEY = "WatkinsLabsLEDRack2025"

def get_api_key():
    """Get API key from environment variable or use default."""
    # Check if API key is in environment variable
    env_key = os.environ.get("LED_MATRIX_API_KEY")
    if env_key:
        return env_key
    return DEFAULT_API_KEY

