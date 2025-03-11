
import os
import time
import requests 
import json

def get_items(host, api_key, retries=3):
    """Get all display items from the LED matrix."""
    headers = {"X-API-Key": api_key}
    
    for attempt in range(retries):
        try:
            if attempt > 0:
                print(f"Retrying items request (attempt {attempt+1}/{retries})...")
                
            response = requests.get(f"http://{host}/items", headers=headers, timeout=10)
            if response.status_code == 200:
                data = response.json()
                print("📋 Display Items:")
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
    
    print("Failed to retrieve items after multiple attempts")
    return None

def add_item(host, item, api_key, retries=3):
    """Add a new item to the display playlist."""
    headers = {"X-API-Key": api_key}
    
    for attempt in range(retries):
        try:
            print(f"Adding new display item (attempt {attempt+1}/{retries})...")
            response = requests.post(f"http://{host}/items", 
                                    json=item,
                                    headers=headers,
                                    timeout=10)
            if response.status_code == 200:
                data = response.json()
                print("✅ Item added successfully!")
                print(f"   Item index: {data.get('index')}")
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
    
    print("Failed to add item after multiple attempts")
    return False

def delete_item(host, index, api_key, retries=3):
    """Delete an item from the display playlist."""
    headers = {"X-API-Key": api_key}
    
    for attempt in range(retries):
        try:
            print(f"Deleting display item at index {index} (attempt {attempt+1}/{retries})...")
            response = requests.post(f"http://{host}/items/delete", 
                                    json={"index": index},
                                    headers=headers,
                                    timeout=10)
            if response.status_code == 200:
                data = response.json()
                print("✅ Item deleted successfully!")
                print(f"   Remaining items: {data.get('remaining')}")
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
    
    print("Failed to delete item after multiple attempts")
    return False

def replace_all_items(host, items, api_key, retries=3):
    """Replace all display items with a new set."""
    headers = {"X-API-Key": api_key}
    
    for attempt in range(retries):
        try:
            print(f"Replacing all display items (attempt {attempt+1}/{retries})...")
            response = requests.post(f"http://{host}/items_replace", 
                                    json={"items": items},
                                    headers=headers,
                                    timeout=10)
            if response.status_code == 200:
                data = response.json()
                print(data)
                print("✅ Items replaced successfully!")
                print(f"   Total items: {data.get('count')}")
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
    
    print("Failed to replace items after multiple attempts")
    return False

def setup_temporary_item(host, api_key):
    """Set up a temporary item that will delete itself after one play."""
    print("🔄 Setting up a temporary one-time display item...")
    
    # Define the item
    item = {
        "mode": "text",
        "text": "ONE TIME ALERT - Will delete after showing once!",
        "alignment": "scroll_left",
        "brightness": 15,
        "scrollSpeed": 30,  # Faster scroll for alert
        "pauseTime": 1000,
        "duration": 15000,  # 15 seconds
        "deleteAfterPlay": True,  # Delete after playing
        "maxPlays": 1,      # Only play once
        "playCount": 0
    }
    
    # Add this as a new item
    success = add_item(host, item, api_key)
    
    if success:
        print("✅ Temporary item added successfully!")
        print("  This item will show once and then delete itself from the playlist.")
        print("  You can check the items after it plays to verify it was removed.")
    else:
        print("❌ Failed to add temporary item")
    
    return success

def setup_multiple_items(host, api_key):
    """Set up multiple items for testing: a text, twinkle, and another text."""
    print("📋 Setting up multiple display items...")
    
    # Define the items
    items = [
        {
            "mode": "text", 
            "text": "Watkins Labs", 
            "alignment": "scroll_right",
            "brightness": 12,
            "scrollSpeed": 50,
            "pauseTime": 2000,
            "duration": 10000,
            "deleteAfterPlay": False,
            "maxPlays": 0
        },
        {
            "mode": "twinkle",
            "brightness": 10,
            "twinkleDensity": 50,
            "twinkleMinSpeed": 50,
            "twinkleMaxSpeed": 300,
            "duration": 4000, 
            "deleteAfterPlay": False,
            "maxPlays": 0
        },
        {
            "mode": "text",
            "text": "Admiral Rackbar the LED Rack Bar",
            "alignment": "scroll_left",
            "brightness": 15,
            "scrollSpeed": 40,
            "pauseTime": 1000,
            "duration": 10000,
            "deleteAfterPlay": False,
            "maxPlays": 0
        },
        {
            "mode": "twinkle",
            "brightness": 10,
            "twinkleDensity": 80,
            "twinkleMinSpeed": 50,
            "twinkleMaxSpeed": 300,
            "duration": 4000, 
            "deleteAfterPlay": False,
            "maxPlays": 0
        },
    ]
    
    # Send the request to replace all items
# Send the request to replace all items
    success = replace_all_items(host, items, api_key)
    
    if success:
        print("✅ Multiple items set up successfully!")
        print("\nThe display will cycle through these items in sequence.")
        # Get the updated settings to verify
        #get_items(host, api_key)
    else:
        print("❌ Failed to set up multiple items")
    
    return success

