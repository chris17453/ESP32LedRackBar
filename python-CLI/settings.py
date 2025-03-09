import os
import requests
import json

def get_setting(host, param, api_key, retries=3):
    """Get a specific setting from the LED matrix with retry logic."""
    headers = {"X-API-Key": api_key}
    
    for attempt in range(retries):
        try:
            if attempt > 0:
                print(f"Retrying parameter request (attempt {attempt+1}/{retries})...")
                
            response = requests.get(f"http://{host}/get?param={param}", headers=headers, timeout=10)
            if response.status_code == 200:
                data = response.json()
                print(f"📊 {param}: {data.get(param, 'Not found')}")
                return data
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid API key")
                print("Check your API key and try again")
                return None
            else:
                print(f"❌ Error: Received status code {response.status_code}")
                if response.text:
                    print(f"   Message: {response.text}")
                if attempt < retries - 1:
                    print("Retrying in 1 second...")
                    time.sleep(1)
        except requests.exceptions.RequestException as e:
            print(f"❌ Connection Error: {e}")
            if attempt < retries - 1:
                print("Retrying in 1 second...")
                time.sleep(1)
    
    print(f"Failed to retrieve '{param}' setting after multiple attempts")
    return None

def get_all_settings(host, api_key, retries=3):
    """Get all current settings from the LED matrix with retry logic."""
    headers = {"X-API-Key": api_key}
    
    for attempt in range(retries):
        try:
            if attempt > 0:
                print(f"Retrying settings request (attempt {attempt+1}/{retries})...")
                
            response = requests.get(f"http://{host}/settings", headers=headers, timeout=10)
            if response.status_code == 200:
                data = response.json()
                print("📊 Current Settings:")
                print(json.dumps(data, indent=2))
                return data
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid API key")
                print("Check your API key and try again")
                return None
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
    
    print("Failed to retrieve settings after multiple attempts")
    return None

def change_api_key(host, current_api_key, new_api_key, retries=3):
    """Change the API key on the device."""
    headers = {"X-API-Key": current_api_key}
    
    if len(new_api_key) < 8:
        print("❌ Error: New API key must be at least 8 characters long")
        return False
        
    for attempt in range(retries):
        try:
            print(f"Sending API key change request (attempt {attempt+1}/{retries})...")
            response = requests.post(f"http://{host}/change_api_key", 
                                    json={"new_key": new_api_key},
                                    headers=headers,
                                    timeout=10)
            if response.status_code == 200:
                print("✅ API key updated successfully!")
                print("➡️ Remember to update your environment variable or use the --api-key parameter with the new key")
                return True
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid current API key")
                return False
            else:
                print(f"❌ Error: Received status code {response.status_code}")
                if response.text:
                    print(f"   Message: {response.text}")
                if attempt < retries - 1:
                    print(f"Retrying in 1 second...")
                    time.sleep(1)
        except requests.exceptions.RequestException as e:
            print(f"❌ Connection Error: {e}")
            if attempt < retries - 1:
                print(f"Retrying in 1 second...")
                time.sleep(1)
    
    print("Failed to change API key after multiple attempts")
    return False

def trigger_factory_reset(host, api_key, retries=3):
    """Trigger a factory reset on the device."""
    headers = {"X-API-Key": api_key}
    
    print("⚠️ WARNING: This will reset all settings including WiFi credentials!")
    confirm = input("Are you sure you want to proceed? (y/n): ")
    if confirm.lower() != 'y':
        print("Factory reset cancelled.")
        return False
        
    for attempt in range(retries):
        try:
            print(f"Sending factory reset request (attempt {attempt+1}/{retries})...")
            response = requests.post(f"http://{host}/factory_reset", 
                                    headers=headers,
                                    timeout=10)
            if response.status_code == 200:
                print("✅ Factory reset initiated!")
                print("The device will restart and reset all settings to default")
                print("You will need to reconnect to the WiFi access point")
                return True
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid API key")
                return False
            else:
                print(f"❌ Error: Received status code {response.status_code}")
                if response.text:
                    print(f"   Message: {response.text}")
                if attempt < retries - 1:
                    print(f"Retrying in 1 second...")
                    time.sleep(1)
        except requests.exceptions.RequestException as e:
            print(f"❌ Connection Error: {e}")
            if attempt < retries - 1:
                print(f"Retrying in 1 second...")
                time.sleep(1)
    
    print("Failed to initiate factory reset after multiple attempts")
    return False

def trigger_manual_factory_reset(host, api_key, retries=3):
    """Trigger a manual factory reset on the device (WiFi reset only)."""
    headers = {"X-API-Key": api_key}
    
    print("⚠️ WARNING: This will reset WiFi credentials and restart the device!")
    confirm = input("Are you sure you want to proceed? (y/n): ")
    if confirm.lower() != 'y':
        print("Manual factory reset cancelled.")
        return False
        
    for attempt in range(retries):
        try:
            print(f"Sending manual factory reset request (attempt {attempt+1}/{retries})...")
            response = requests.post(f"http://{host}/manual_factory_reset", 
                                    headers=headers,
                                    timeout=10)
            if response.status_code == 200:
                print("✅ Manual factory reset initiated!")
                print("The device will restart with WiFi settings cleared")
                print("You will need to reconnect to the WiFi access point")
                return True
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid API key")
                return False
            else:
                print(f"❌ Error: Received status code {response.status_code}")
                if response.text:
                    print(f"   Message: {response.text}")
                if attempt < retries - 1:
                    print(f"Retrying in 1 second...")
                    time.sleep(1)
        except requests.exceptions.RequestException as e:
            print(f"❌ Connection Error: {e}")
            if attempt < retries - 1:
                print(f"Retrying in 1 second...")
                time.sleep(1)
    
    print("Failed to initiate manual factory reset after multiple attempts")
    return False

def download_config_file(host, api_key, config_type, output_file=None, retries=3):
    """Download config file from the device."""
    headers = {"X-API-Key": api_key}
    
    if config_type == "main":
        endpoint = "/download_config"
        default_filename = "config.json"
    elif config_type == "security":
        endpoint = "/download_security_config"
        default_filename = "security_config.json"
    else:
        print(f"❌ Error: Unknown config type '{config_type}'")
        return False
    
    # If no output file is specified, use the default
    if output_file is None:
        output_file = default_filename
    
    for attempt in range(retries):
        try:
            print(f"Downloading {config_type} config file (attempt {attempt+1}/{retries})...")
            response = requests.get(f"http://{host}{endpoint}", 
                                  headers=headers,
                                  timeout=10,
                                  stream=True)
            
            if response.status_code == 200:
                # Save the file
                with open(output_file, 'wb') as f:
                    for chunk in response.iter_content(chunk_size=8192):
                        f.write(chunk)
                
                print(f"✅ Config file downloaded to '{output_file}'")
                # Try to parse and display the JSON
                try:
                    with open(output_file, 'r') as f:
                        data = json.load(f)
                        print("\nFile content:")
                        print(json.dumps(data, indent=2))
                except json.JSONDecodeError:
                    print("Warning: File is not valid JSON or is empty")
                except Exception as e:
                    print(f"Error displaying file content: {e}")
                    
                return True
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid API key")
                return False
            elif response.status_code == 404:
                print("❌ Error: Config file not found or empty on the device")
                return False
            else:
                print(f"❌ Error: Received status code {response.status_code}")
                if response.text:
                    print(f"   Message: {response.text}")
                if attempt < retries - 1:
                    print(f"Retrying in 1 second...")
                    time.sleep(1)
        except requests.exceptions.RequestException as e:
            print(f"❌ Connection Error: {e}")
            if attempt < retries - 1:
                print(f"Retrying in 1 second...")
                time.sleep(1)
    
    print("Failed to download config file after multiple attempts")
    return False

def list_files(host, api_key, retries=3):
    """List all files on the device's filesystem."""
    headers = {"X-API-Key": api_key}
    
    for attempt in range(retries):
        try:
            print(f"Requesting file list (attempt {attempt+1}/{retries})...")
            response = requests.get(f"http://{host}/list_files", 
                                   headers=headers,
                                   timeout=10)
            
            if response.status_code == 200:
                data = response.json()
                print("\n📁 Files on device:")
                if "files" in data and data["files"]:
                    # Calculate the longest filename for nice formatting
                    max_name_len = max(len(file["name"]) for file in data["files"])
                    
                    # Print header
                    print(f"\n{'Filename':<{max_name_len+2}} {'Size':>10}")
                    print("-" * (max_name_len+2 + 10 + 1))
                    
                    # Print each file
                    for file in data["files"]:
                        name = file["name"]
                        size = file["size"]
                        print(f"{name:<{max_name_len+2}} {size:>10} bytes")
                else:
                    print("No files found on device")
                return True
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid API key")
                return False
            else:
                print(f"❌ Error: Received status code {response.status_code}")
                if response.text:
                    print(f"   Message: {response.text}")
                if attempt < retries - 1:
                    print(f"Retrying in 1 second...")
                    time.sleep(1)
        except requests.exceptions.RequestException as e:
            print(f"❌ Connection Error: {e}")
            if attempt < retries - 1:
                print(f"Retrying in 1 second...")
                time.sleep(1)
    
    print("Failed to list files after multiple attempts")
    return False

def update_wifi_settings(host, ssid, password, api_key, retries=3):
    """Update WiFi credentials on the device."""
    headers = {"X-API-Key": api_key}
    
    # Validate inputs
    if not ssid:
        print("❌ Error: SSID cannot be empty")
        return False
        
    if password and len(password) < 8:
        print("❌ Error: Password must be at least 8 characters or empty for open networks")
        return False
    
    # Confirm with user
    print("⚠️ Warning: You are about to update WiFi credentials")
    print(f"SSID: {ssid}")
    print(f"Password: {'*' * len(password) if password else '(empty - open network)'}")
    confirm = input("Are you sure you want to proceed? The device will reconnect. (y/n): ")
    if confirm.lower() != 'y':
        print("WiFi update cancelled.")
        return False
        
    for attempt in range(retries):
        try:
            print(f"Sending WiFi update request (attempt {attempt+1}/{retries})...")
            response = requests.post(
                f"http://{host}/update_wifi", 
                json={"ssid": ssid, "password": password},
                headers=headers,
                timeout=10
            )
            
            if response.status_code == 200:
                print("✅ WiFi settings updated successfully!")
                print("The device is attempting to connect to the new network.")
                print("If connection fails, the device will revert to Access Point mode on next restart.")
                print("Note: Your current connection to the device may be lost if it connects to a different network.")
                return True
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid API key")
                return False
            else:
                print(f"❌ Error: Received status code {response.status_code}")
                if response.text:
                    print(f"   Message: {response.text}")
                if attempt < retries - 1:
                    print(f"Retrying in 1 second...")
                    time.sleep(1)
        except requests.exceptions.RequestException as e:
            print(f"❌ Connection Error: {e}")
            if attempt < retries - 1:
                print(f"Retrying in 1 second...")
                time.sleep(1)
    
    print("Failed to update WiFi settings after multiple attempts")
    return False

def update_hostname(host, new_hostname, api_key, retries=3):
    """Update the device hostname."""
    headers = {"X-API-Key": api_key}
    
    # Validate hostname (simple validation)
    if not new_hostname or len(new_hostname) > 32:
        print("❌ Error: Hostname must be between 1 and 32 characters")
        return False
    
    # Check hostname format (alphanumeric + hyphen)
    if not all(c.isalnum() or c == '-' for c in new_hostname):
        print("❌ Error: Hostname must contain only alphanumeric characters and hyphens")
        return False
    
    print(f"⚠️ Updating hostname to: {new_hostname}")
    print(f"Note: After updating, the device will be accessible at http://{new_hostname}.local")
    confirm = input("Are you sure you want to proceed? (y/n): ")
    if confirm.lower() != 'y':
        print("Hostname update cancelled.")
        return False
        
    for attempt in range(retries):
        try:
            print(f"Sending hostname update request (attempt {attempt+1}/{retries})...")
            response = requests.post(
                f"http://{host}/update_hostname", 
                json={"hostname": new_hostname},
                headers=headers,
                timeout=10
            )
            
            if response.status_code == 200:
                data = response.json()
                print("✅ Hostname updated successfully!")
                print(f"  New hostname: {data.get('hostname', new_hostname)}")
                print(f"  The device is now accessible at: http://{new_hostname}.local")
                print("  Note: mDNS can take a moment to propagate on your network")
                return True
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid API key")
                return False
            else:
                print(f"❌ Error: Received status code {response.status_code}")
                if response.text:
                    print(f"   Message: {response.text}")
                if attempt < retries - 1:
                    print(f"Retrying in 1 second...")
                    time.sleep(1)
        except requests.exceptions.RequestException as e:
            print(f"❌ Connection Error: {e}")
            if attempt < retries - 1:
                print(f"Retrying in 1 second...")
                time.sleep(1)
    
    print("Failed to update hostname after multiple attempts")
    return False