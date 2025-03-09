

def calculate_wait_time(item):
    """Calculate a reasonable wait time based on display settings."""
    mode = item.get("mode", "text")
    alignment = item.get("alignment", "scroll_left")
    
    if mode == "text":
        # If it's scrolling text, calculate time based on text length and scroll speed
        if alignment in ["scroll_left", "scroll_right"]:
            text_length = len(item.get("text", ""))
            scroll_speed = item.get("scrollSpeed", 50)
            pause_time = item.get("pauseTime", 2000) / 1000  # Convert ms to seconds
            
            # Rough estimate: longer text with slower speed (higher value) needs more time
            # Plus the pause time at the end
            wait_time = (text_length * scroll_speed / 1000) + pause_time
            
            # Cap at reasonable values
            wait_time = max(3, min(wait_time, 5))
            return wait_time
        else:
            # Static text just needs a short time to view
            return 3
    elif mode == "twinkle":
        # For twinkle, base it on the max speed
        max_speed = item.get("twinkleMaxSpeed", 300)
        # Allow at least a couple of cycles to see the effect
        wait_time = max(3, min(max_speed * 3 / 1000, 5))
        return wait_time
    else:
        # Default wait time
        return 3