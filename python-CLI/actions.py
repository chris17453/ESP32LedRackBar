import json
import requests
import time
import os

def update_display(host, settings, api_key, retries=3, timeout=10):
    """Update display settings with retry logic."""
    headers = {"X-API-Key": api_key}
    
    for attempt in range(retries):
        try:
            print(f"Sending update request (attempt {attempt+1}/{retries})...")
            response = requests.post(f"http://{host}/update_display", 
                                    json=settings,
                                    headers=headers,
                                    timeout=timeout)
            if response.status_code == 200:
                print("✅ Settings updated successfully!")
                return True
            elif response.status_code == 401:
                print("❌ Error: Unauthorized - Invalid API key")
                print("Check your API key and try again")
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
    
    print("Failed to update settings after multiple attempts")
    return False

def reboot_device(host, api_key, retries=3):
    """Reboot the device."""
    headers = {"X-API-Key": api_key}
    
    print("⚠️ You are about to reboot the device")
    confirm = input("Are you sure you want to proceed? (y/n): ")
    if confirm.lower() != 'y':
        print("Reboot cancelled.")
        return False
        
    for attempt in range(retries):
        try:
            print(f"Sending reboot request (attempt {attempt+1}/{retries})...")
            response = requests.post(
                f"http://{host}/reboot", 
                headers=headers,
                timeout=10
            )
            
            if response.status_code == 200:
                print("✅ Reboot command sent successfully!")
                print("The device is now restarting...")
                print("It should be available again in a few seconds.")
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
    
    print("Failed to send reboot command after multiple attempts")
    return False
