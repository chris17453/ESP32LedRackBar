#!/usr/bin/env python3
import argparse
import json
import requests
import sys
import time
import os


from .common import calculate_wait_time
from .actions import reboot_device, update_display
from .security import get_api_key, DEFAULT_API_KEY
from .items import get_items,  add_item,  delete_item,  replace_all_items,  setup_temporary_item,  setup_multiple_items
from .settings import get_setting, get_all_settings, change_api_key, trigger_factory_reset, trigger_manual_factory_reset, download_config_file, list_files, update_wifi_settings, update_hostname
from .status import check_status




def main():
    parser = argparse.ArgumentParser(description='Control an ESP32 LED Matrix over HTTP API')
    
    # Host argument
    parser.add_argument('--host', type=str, default='ledmatrix.local',
                        help='Hostname or IP address of the LED matrix (default: ledmatrix.local)')
    
    # API key argument
    parser.add_argument('--api-key', type=str, 
                        help='API key for authentication (default: from LED_MATRIX_API_KEY env var or built-in default)')
    
    # Subcommands
    subparsers = parser.add_subparsers(dest='command', help='Command to execute')
    
    # Status command
    status_parser = subparsers.add_parser('status', help='Check if the LED matrix is online')
    
    # Get settings command
    get_parser = subparsers.add_parser('get', help='Get current settings')
    get_parser.add_argument('--param', type=str, 
                           help='Get a specific parameter (displayOn, loopItems, currentItemIndex, numItems, mode, text, alignment, etc.)')
    
    # Get items command
    get_items_parser = subparsers.add_parser('get-items', help='Get all display items')
    
    # Update settings command
    update_parser = subparsers.add_parser('update', help='Update display settings')
    update_parser.add_argument('--displayOn', type=lambda x: (str(x).lower() == 'true'),
                              help='Turn display on/off (true/false)')
    update_parser.add_argument('--loopItems', type=lambda x: (str(x).lower() == 'true'),
                              help='Loop through items when reaching the end (true/false)')
    update_parser.add_argument('--mode', type=str, choices=['text', 'twinkle'], 
                              help='Display mode for current item ("text" or "twinkle")')
    update_parser.add_argument('--text', type=str, 
                              help='Text to display (for text mode)')
    update_parser.add_argument('--alignment', type=str, choices=['left', 'center', 'right', 'scroll_left', 'scroll_right'],
                              help='Text alignment (for text mode)')
    update_parser.add_argument('--invert', type=lambda x: (str(x).lower() == 'true'),
                              help='Invert display (true/false)')
    update_parser.add_argument('--brightness', type=int, choices=range(0, 16),
                              help='Display brightness (0-15)')
    update_parser.add_argument('--twinkle-density', dest='twinkleDensity', type=int, choices=range(1, 101),
                              help='Twinkle density - percentage of LEDs active (1-100, for twinkle mode)')
    update_parser.add_argument('--twinkle-min-speed', dest='twinkleMinSpeed', type=int,
                              help='Minimum twinkle cycle speed in ms (for twinkle mode)')
    update_parser.add_argument('--twinkle-max-speed', dest='twinkleMaxSpeed', type=int,
                              help='Maximum twinkle cycle speed in ms (for twinkle mode)')
    update_parser.add_argument('--duration', type=int,
                              help='Duration to show item in ms (0 = forever)')
    update_parser.add_argument('--timeout', type=int, default=10,
                              help='HTTP request timeout in seconds (default: 10)')
    update_parser.add_argument('--retries', type=int, default=3,
                              help='Number of retry attempts for HTTP requests (default: 3)')
    
    # Add item command
    add_item_parser = subparsers.add_parser('add-item', help='Add a new display item')
    add_item_parser.add_argument('--mode', type=str, choices=['text', 'twinkle'], required=True,
                               help='Display mode ("text" or "twinkle")')
    add_item_parser.add_argument('--text', type=str, 
                               help='Text to display (required for text mode)')
    add_item_parser.add_argument('--alignment', type=str, choices=['left', 'center', 'right', 'scroll_left', 'scroll_right'], default='scroll_left',
                               help='Text alignment (for text mode, default: scroll_left)')
    add_item_parser.add_argument('--brightness', type=int, choices=range(0, 16), default=8,
                               help='Display brightness (0-15, default: 8)')
    add_item_parser.add_argument('--duration', type=int, default=0,
                               help='Duration to show item in ms (0 = forever, default: 0)')
    add_item_parser.add_argument('--delete-after', type=lambda x: (str(x).lower() == 'true'), default=False,
                               help='Delete after playing (default: false)')
    add_item_parser.add_argument('--max-plays', type=int, default=0,
                               help='Maximum times to play item (0 = unlimited, default: 0)')
    
    # Delete item command
    delete_item_parser = subparsers.add_parser('delete-item', help='Delete a display item')
    delete_item_parser.add_argument('--index', type=int, required=True,
                                  help='Index of the item to delete')
    
    # Replace all items command
    replace_items_parser = subparsers.add_parser('replace-items', help='Replace all display items with new ones')
    replace_items_parser.add_argument('--file', type=str, required=True,
                                    help='JSON file containing array of items to use')
    
    # WiFi update command
    update_wifi_parser = subparsers.add_parser('update-wifi', help='Update WiFi credentials')
    update_wifi_parser.add_argument('--ssid', type=str, required=True,
                                help='WiFi network name (SSID)')
    update_wifi_parser.add_argument('--password', type=str, default="",
                                help='WiFi password (leave empty for open networks)')
    
    # Update hostname command
    update_hostname_parser = subparsers.add_parser('update-hostname', help='Update device hostname')
    update_hostname_parser.add_argument('hostname', type=str, help='New hostname for the device')
    
    # Reboot device command
    reboot_parser = subparsers.add_parser('reboot', help='Reboot the device')
    
    # Demo command
    demo_parser = subparsers.add_parser('demo', help='Run a demonstration of different settings')
    
    # Twinkle Demo command
    twinkle_demo_parser = subparsers.add_parser('twinkle-demo', help='Run a demonstration of twinkle mode')
    
    # Multiple items demo command
    multi_items_parser = subparsers.add_parser('multi-items-demo', help='Set up multiple display items for testing')
    
    # One-time item demo command
    temp_item_parser = subparsers.add_parser('temp-item-demo', help='Add a temporary one-time display item')
    
    # Change API key command
    change_key_parser = subparsers.add_parser('change-key', help='Change the API key')
    change_key_parser.add_argument('new_key', type=str, help='New API key (minimum 8 characters)')
    
    # Factory reset command
    factory_reset_parser = subparsers.add_parser('factory-reset', help='Trigger a factory reset')
    
    # Manual factory reset command (WiFi reset only)
    manual_reset_parser = subparsers.add_parser('manual-reset', help='Trigger a manual factory reset (WiFi reset only)')
    
    # Download config file command
    download_config_parser = subparsers.add_parser('download-config', help='Download a config file from the device')
    download_config_parser.add_argument('--type', type=str, choices=['main', 'security'], default='main',
                                      help='Which config file to download (main or security)')
    download_config_parser.add_argument('--output', type=str, 
                                      help='Output filename (default: config.json or security_config.json)')
    
    # List files command
    list_files_parser = subparsers.add_parser('list-files', help='List all files on the device')
    
    args = parser.parse_args()
    
    # If no command is specified, show help
    if not args.command:
        parser.print_help()
        sys.exit(1)
    
    # Determine API key to use
    api_key = args.api_key if args.api_key else get_api_key()
    
    # Process commands
    if args.command == 'status':
        check_status(args.host)
    
    elif args.command == 'get':
        if args.param:
            get_setting(args.host, args.param, api_key)
        else:
            get_all_settings(args.host, api_key)
    
    elif args.command == 'get-items':
        get_items(args.host, api_key)
    
    elif args.command == 'update':
        # Build settings object with only provided arguments
        settings = {}
        if args.displayOn is not None:
            settings['displayOn'] = args.displayOn
        if args.loopItems is not None:
            settings['loopItems'] = args.loopItems
        
        # Current item settings
        if args.mode:
            settings['mode'] = args.mode
        if args.text:
            settings['text'] = args.text
        if args.alignment:
            settings['alignment'] = args.alignment
        if args.invert is not None:
            settings['invert'] = args.invert
        if args.brightness is not None:
            settings['brightness'] = args.brightness
        if args.twinkleDensity is not None:
            settings['twinkleDensity'] = args.twinkleDensity
        if args.twinkleMinSpeed is not None:
            settings['twinkleMinSpeed'] = args.twinkleMinSpeed
        if args.twinkleMaxSpeed is not None:
            settings['twinkleMaxSpeed'] = args.twinkleMaxSpeed
        if args.duration is not None:
            settings['duration'] = args.duration
        
        # Check if any setting was provided
        if not settings:
            print("❌ Error: At least one setting must be specified")
            update_parser.print_help()
            sys.exit(1)
        
        # Update display with provided settings
        timeout = args.timeout if hasattr(args, 'timeout') else 10
        retries = args.retries if hasattr(args, 'retries') else 3
        
        success = update_display(args.host, settings, api_key, retries=retries, timeout=timeout)
        if success:
            # Show the updated settings
            print("Retrieving updated settings...")
            time.sleep(1)  # Brief pause
            get_all_settings(args.host, api_key, retries=retries)
    
    elif args.command == 'add-item':
        # Build item object
        item = {
            "mode": args.mode,
            "brightness": args.brightness,
            "duration": args.duration,
            "deleteAfterPlay": args.delete_after,
            "maxPlays": args.max_plays
        }
        
        # Add text-specific settings
        if args.mode == 'text':
            if not args.text:
                print("❌ Error: Text is required for text mode")
                sys.exit(1)
            
            item["text"] = args.text
            item["alignment"] = args.alignment
        
        # Add the item
        add_item(args.host, item, api_key)
    
    elif args.command == 'delete-item':
        delete_item(args.host, args.index, api_key)
    
    elif args.command == 'replace-items':
        try:
            with open(args.file, 'r') as f:
                items_data = json.load(f)
            
            if not isinstance(items_data, list):
                print("❌ Error: File must contain a JSON array of items")
                sys.exit(1)
            
            replace_all_items(args.host, items_data, api_key)
        except json.JSONDecodeError:
            print("❌ Error: Invalid JSON file")
            sys.exit(1)
        except FileNotFoundError:
            print(f"❌ Error: File '{args.file}' not found")
            sys.exit(1)
    
    elif args.command == 'demo':
        print("🚀 Running LED Matrix Demo")
        
        # Check if device is online
        if not check_status(args.host):
            sys.exit(1)
        
        # Get initial settings to restore later
        initial_settings = get_all_settings(args.host, api_key)
        if not initial_settings:
            sys.exit(1)
        
        try:
            # Demo sequence
            print("\n📝 Demo 1: Changing text with different alignments")
            demos = [
                {"mode": "text", "text": "Left aligned", "alignment": "left", "brightness": 8},
                {"mode": "text", "text": "Center aligned", "alignment": "center", "brightness": 8},
                {"mode": "text", "text": "Right aligned", "alignment": "right", "brightness": 8},
                {"mode": "text", "text": "Scrolling text", "alignment": "scroll_left", "brightness": 8}
            ]
            
            for i, demo in enumerate(demos):
                print(f"\nRunning demo {i+1}/{len(demos)}: {demo['text']} ({demo['alignment']})")
                update_display(args.host, demo, api_key)
                wait_time = calculate_wait_time(demo)
                print(f"Waiting for {wait_time:.1f} seconds to observe the effect...")
                time.sleep(wait_time)
            
            print("\n💡 Demo 2: Brightness levels")
            for brightness in [2, 8, 15]:
                print(f"Setting brightness to {brightness}")
                update_display(args.host, {"brightness": brightness}, api_key)
                time.sleep(2)
            
            print("\n🔄 Demo 3: Invert display")
            update_display(args.host, {"mode": "text", "text": "Inverted text", "invert": True}, api_key)
            time.sleep(3)
            update_display(args.host, {"invert": False}, api_key)
            time.sleep(1)
            
            print("\n✅ Demo complete! Restoring original settings...")
            success = update_display(args.host, initial_settings, api_key, retries=3, timeout=10)
            if not success:
                print("\n⚠️ WARNING: Failed to restore original settings. You may need to manually reset the device.")
                print("Try running: python led_matrix_client.py --host", args.host, "update --mode text")
            
        except KeyboardInterrupt:
            print("\n\n⚠️ Demo interrupted by user")
            print("Restoring original settings...")
            update_display(args.host, initial_settings, api_key)
    
    elif args.command == 'twinkle-demo':
        print("✨ Running Twinkle Mode Demo")
        
        # Check if device is online
        if not check_status(args.host):
            sys.exit(1)
        
        # Get initial settings to restore later
        initial_settings = get_all_settings(args.host, api_key)
        if not initial_settings:
            sys.exit(1)
        
        try:
            # Base twinkle settings
            base_twinkle = {
                "mode": "twinkle", 
                "brightness": 10,
                "twinkleDensity": 15,
                "twinkleMinSpeed": 50,
                "twinkleMaxSpeed": 300
            }
            
            # Start with basic twinkle mode
            print("\n✨ Part 1: Basic twinkle effect")
            update_display(args.host, base_twinkle, api_key)
            print("Showing basic twinkle effect...")
            time.sleep(3)
            
            # Demonstrate different densities
            print("\n✨ Part 2: Different twinkle densities")
            densities = [10, 20, 40]
            
            for density in densities:
                settings = base_twinkle.copy()
                settings["twinkleDensity"] = density
                print(f"Setting twinkle density to {density}%")
                update_display(args.host, settings, api_key)
                print(f"Observing twinkle density of {density}% for 3 seconds...")
                time.sleep(3)
            
            # Demonstrate different speeds
            print("\n✨ Part 3: Different twinkle speeds")
            speeds = [
                {"min": 30, "max": 80, "desc": "Fast twinkling"},
                {"min": 80, "max": 200, "desc": "Medium twinkling"},
                {"min": 200, "max": 500, "desc": "Slow twinkling"}
            ]
            
            for speed in speeds:
                settings = base_twinkle.copy()
                settings["twinkleMinSpeed"] = speed["min"]
                settings["twinkleMaxSpeed"] = speed["max"]
                print(f"{speed['desc']} (Min: {speed['min']}ms, Max: {speed['max']}ms)")
                update_display(args.host, settings, api_key)
                
                # Calculate wait time based on the speeds
                wait_time = max(3, min(speed["max"] * 2 / 1000, 5))
                print(f"Observing {speed['desc']} for {wait_time:.1f} seconds...")
                time.sleep(wait_time)
            
            # Mixed speeds demo
            print("\n✨ Part 4: Mixed Speed Demo")
            mixed_speed = {
                "mode": "twinkle",
                "brightness": 15,
                "twinkleDensity": 30,
                "twinkleMinSpeed": 30,
                "twinkleMaxSpeed": 500
            }
            update_display(args.host, mixed_speed, api_key)
            print("Showing mixed speed twinkling (fast and slow together)...")
            print("Running for 4 seconds...")
            time.sleep(4)
            
            # Finale
            print("\n✨ Part 5: Spectacular finale")
            finale = {
                "mode": "twinkle",
                "brightness": 15,
                "twinkleDensity": 35,
                "twinkleMinSpeed": 40,
                "twinkleMaxSpeed": 400
            }
            update_display(args.host, finale, api_key)
            print("Enjoy the grand finale!")
            print("Running for 5 seconds...")
            time.sleep(5)
            
            print("\n✅ Twinkle demo complete! Restoring original settings...")
            update_display(args.host, initial_settings, api_key)
            
        except KeyboardInterrupt:
            print("\n\n⚠️ Demo interrupted by user")
            print("Restoring original settings...")
            update_display(args.host, initial_settings, api_key)
    
    elif args.command == 'multi-items-demo':
        setup_multiple_items(args.host, api_key)
    
    elif args.command == 'temp-item-demo':
        setup_temporary_item(args.host, api_key)
    
    elif args.command == 'change-key':
        print("🔑 Changing API key")
        change_api_key(args.host, api_key, args.new_key)
    
    elif args.command == 'factory-reset':
        print("⚠️ Factory Reset")
        trigger_factory_reset(args.host, api_key)
    
    elif args.command == 'manual-reset':
        print("⚠️ Manual Factory Reset (WiFi Reset)")
        trigger_manual_factory_reset(args.host, api_key)
    
    elif args.command == 'download-config':
        print(f"📥 Downloading {args.type} config file")
        download_config_file(args.host, api_key, args.type, args.output)
    
    elif args.command == 'list-files':
        print("📋 Listing files on device")
        list_files(args.host, api_key)
    
    elif args.command == 'update-wifi':
        print("📡 Updating WiFi Settings")
        update_wifi_settings(args.host, args.ssid, args.password, api_key)
    
    elif args.command == 'update-hostname':
        print("🏷️ Updating Device Hostname")
        update_hostname(args.host, args.hostname, api_key)
    
    elif args.command == 'reboot':
        print("🔄 Rebooting Device")
        reboot_device(args.host, api_key)
    
if __name__ == "__main__":
    main()