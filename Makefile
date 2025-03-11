# Makefile for LED Matrix Client Testing
# Host configuration
HOST = 192.168.1.205
API_KEY = WatkinsLabsLEDRack2025

# Python script path
CLIENT = -m python-CLI

# Common arguments
ARGS = --host $(HOST) --api-key $(API_KEY)

# Default target
.PHONY: help
help:
	@echo "LED Matrix Client Testing Makefile"
	@echo ""
	@echo "Usage:"
	@echo "  make <target>"
	@echo ""
	@echo "Targets:"
	@echo "  status               - Check device status"
	@echo "  get-all              - Get all settings"
	@echo "  get-mode             - Get current mode"
	@echo "  get-text             - Get current text"
	@echo "  update-text TEXT=... - Update display text"
	@echo "  text-left            - Set text left aligned"
	@echo "  text-center          - Set text center aligned"
	@echo "  text-right           - Set text right aligned"
	@echo "  text-scroll          - Set text scrolling"
	@echo "  twinkle              - Switch to twinkle mode"
	@echo "  bright-low           - Set brightness to low (2)"
	@echo "  bright-med           - Set brightness to medium (8)"
	@echo "  bright-high          - Set brightness to high (15)"
	@echo "  invert-on            - Turn invert mode on"
	@echo "  invert-off           - Turn invert mode off"
	@echo "  display-on           - Turn display on"
	@echo "  display-off          - Turn display off"
	@echo "  demo                 - Run text demo"
	@echo "  twinkle-demo         - Run twinkle demo"
	@echo "  list-files           - List files on device"
	@echo "  config-main          - Download main config"
	@echo "  config-security      - Download security config"
	@echo "  wifi-update          - Update WiFi credentials (interactive)"
	@echo "  manual-reset         - Perform manual WiFi reset"
	@echo "  update-hostname NAME - Update device hostname to NAME"
	@echo "  reboot                - Reboot the device"
	@echo "  multi-items-demo      - All item typoes in a demo"
	@echo "  download-config       - pull the stored config for the device"

reboot:
	python $(CLIENT) $(ARGS) reboot

update-hostname:
	@if [ -z "$(NAME)" ]; then \
		echo "Error: Hostname not specified. Use make update-hostname NAME=your-hostname"; \
	else \
		python $(CLIENT) $(ARGS) update-hostname $(NAME); \
	fi
# Basic commands
status:
	python $(CLIENT) $(ARGS) status

get-all:
	python $(CLIENT) $(ARGS) get

get-mode:
	python $(CLIENT) $(ARGS) get --param mode

get-text:
	python $(CLIENT) $(ARGS) get --param text

list-files:
	python $(CLIENT) $(ARGS) list-files

# Config commands
config-main:
	python $(CLIENT) $(ARGS) download-config --type main

config-security:
	python $(CLIENT) $(ARGS) download-config --type security

# Text mode commands
TEXT ?= "LED Matrix Test"

update-text:
	python $(CLIENT) $(ARGS) update --mode text --text $(TEXT)

text-left:
	python $(CLIENT) $(ARGS) update --mode text --alignment left

text-center:
	python $(CLIENT) $(ARGS) update --mode text --alignment center

text-right:
	python $(CLIENT) $(ARGS) update --mode text --alignment right

text-scroll:
	python $(CLIENT) $(ARGS) update --mode text --alignment scroll_left

# Twinkle mode
twinkle:
	python $(CLIENT) $(ARGS) update --mode twinkle

twinkle-dense:
	python $(CLIENT) $(ARGS) update --mode twinkle --twinkle-density 40

twinkle-fast:
	python $(CLIENT) $(ARGS) update --mode twinkle --twinkle-min-speed 30 --twinkle-max-speed 100

# Display control
bright-low:
	python $(CLIENT) $(ARGS) update --brightness 2

bright-med:
	python $(CLIENT) $(ARGS) update --brightness 8

bright-high:
	python $(CLIENT) $(ARGS) update --brightness 15

invert-on:
	python $(CLIENT) $(ARGS) update --invert true

invert-off:
	python $(CLIENT) $(ARGS) update --invert false

display-on:
	python $(CLIENT) $(ARGS) update --display-on true

display-off:
	python $(CLIENT) $(ARGS) update --display-on false

# Demo commands
demo:
	python $(CLIENT) $(ARGS) demo

twinkle-demo:
	python $(CLIENT) $(ARGS) twinkle-demo

# WiFi commands
wifi-update:
	@echo "Updating WiFi credentials..."
	@read -p "Enter SSID: " ssid; \
	read -p "Enter Password: " pass; \
	python $(CLIENT) $(ARGS) update-wifi --ssid "$$ssid" --password "$$pass"

manual-reset:
	python $(CLIENT) $(ARGS) manual-reset

multi-items-demo:
	python $(CLIENT) $(ARGS) multi-items-demo


download-config:
	python $(CLIENT) $(ARGS) download-config