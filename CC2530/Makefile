# Root build entry for the CC2530 project.

FIRMWARE_DIR := firmware/baremetal
MIN_LED_DIR  := firmware/examples/min_led

.PHONY: all firmware flash erase test reset read backup example-min-led flash-min-led clean distclean help

all: firmware

firmware:
	$(MAKE) -C $(FIRMWARE_DIR)

flash:
	$(MAKE) -C $(FIRMWARE_DIR) flash

erase:
	$(MAKE) -C $(FIRMWARE_DIR) erase

test:
	$(MAKE) -C $(FIRMWARE_DIR) test

reset:
	$(MAKE) -C $(FIRMWARE_DIR) reset

read:
	$(MAKE) -C $(FIRMWARE_DIR) read

backup:
	$(MAKE) -C $(FIRMWARE_DIR) backup

example-min-led:
	$(MAKE) -C $(MIN_LED_DIR)

flash-min-led:
	$(MAKE) -C $(MIN_LED_DIR) flash


clean:
	$(MAKE) -C $(FIRMWARE_DIR) clean
	$(MAKE) -C $(MIN_LED_DIR) clean

distclean:
	$(MAKE) -C $(FIRMWARE_DIR) distclean
	$(MAKE) -C $(MIN_LED_DIR) clean

help:
	@echo "Targets:"
	@echo "  make              Build main OLED firmware"
	@echo "  make flash        Build, erase/write/verify, then reset target"
	@echo "  make test         Detect programmer and target"
	@echo "  make reset        Reset target"
	@echo "  make backup       Read full flash to firmware/baremetal/backup/"
	@echo "  make example-min-led  Build minimal LED example into build/"
	@echo "  make flash-min-led    Build and flash minimal LED example"
	@echo "  make clean        Remove generated build outputs"
