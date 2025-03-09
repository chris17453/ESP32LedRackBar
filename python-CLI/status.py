import requests 

def check_status(host, api_key=None, retries=3):
    """Check if the LED matrix is online and get its status with retry logic."""
    for attempt in range(retries):
        try:
            if attempt > 0:
                print(f"Retrying connection (attempt {attempt+1}/{retries})...")
            
            # Status endpoint doesn't require API key
            response = requests.get(f"http://{host}/status", timeout=10)
            if response.status_code == 200:
                data = response.json()
                print(f"✅ Device Status: Online")
                print(f"   IP Address: {data.get('ip', 'Not provided')}")
                return True
            else:
                print(f"❌ Error: Received status code {response.status_code}")
                if attempt < retries - 1:
                    print("Retrying in 1 second...")
                    time.sleep(1)
        except requests.exceptions.RequestException as e:
            print(f"❌ Connection Error: {e}")
            if attempt < retries - 1:
                print("Retrying in 1 second...")
                time.sleep(1)
    
    print("Failed to connect after multiple attempts")
    print("Check that the device is powered on and connected to your network")
    return False
