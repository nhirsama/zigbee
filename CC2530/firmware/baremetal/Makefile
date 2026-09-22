# CC2530F256 bare-metal SDCC development Makefile
# Usage:
#   make          Build build/firmware.hex
#   make flash    Build + burn by cc-tool
#   make test     Detect SmartRF/CC-Debugger and CC2530

PROJECT      ?= firmware
TARGET       := build/$(PROJECT)
HEX          := $(TARGET).hex
IHX          := $(TARGET).ihx

SDCC         ?= sdcc
PACKIHX      ?= packihx
CCTOOL       ?= cc-tool
TARGET_NAME  ?= CC2530
VERIFY       ?= read
SUDO         ?= sudo

INCLUDES     := -Iinclude -Idrivers
CFLAGS       := -mmcs51 --model-small --std-sdcc99 --opt-code-size $(INCLUDES) $(CFLAGS_EXTRA)

SRCS         := src/main.c drivers/delay.c drivers/display_ssd1306.c
OBJS         := $(patsubst %.c,build/%.rel,$(SRCS))

.PHONY: all flash erase test reset read backup check check-build check-flash clean distclean help

all: $(HEX)

check: check-build check-flash

check-build:
	@command -v $(SDCC) >/dev/null || { echo "Missing $(SDCC). Install on Arch: sudo pacman -S sdcc"; exit 1; }
	@command -v $(PACKIHX) >/dev/null || { echo "Missing $(PACKIHX). It is included in sdcc."; exit 1; }

check-flash:
	@command -v $(CCTOOL) >/dev/null || { echo "Missing $(CCTOOL). Install cc-tool-git from AUR."; exit 1; }

build/%.rel: %.c
	@mkdir -p $(dir $@)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(IHX): check-build $(OBJS)
	$(SDCC) $(CFLAGS) $(OBJS) -o $(IHX)

$(HEX): $(IHX)
	$(PACKIHX) $(IHX) > $(HEX)
	@echo "Built $(HEX)"

flash: check-flash $(HEX)
	@set -e; \
	out="$$(mktemp)"; \
	$(SUDO) $(CCTOOL) -n $(TARGET_NAME) -e -w $(HEX) -v $(VERIFY) | tee "$$out"; \
	if grep -Eq "Failed|Error occurred" "$$out"; then rm -f "$$out"; exit 1; fi; \
	rm -f "$$out"; \
	$(SUDO) $(CCTOOL) -n $(TARGET_NAME) --reset

erase: check-flash
	$(SUDO) $(CCTOOL) -n $(TARGET_NAME) -e

test: check-flash
	$(SUDO) $(CCTOOL) -t

reset: check-flash
	$(SUDO) $(CCTOOL) -n $(TARGET_NAME) --reset

read: check-flash
	@mkdir -p backup
	$(SUDO) $(CCTOOL) -n $(TARGET_NAME) -r backup/flash.bin,bin

backup: check-flash
	@mkdir -p backup
	$(SUDO) $(CCTOOL) -n $(TARGET_NAME) -r backup/cc2530_$$(date +%Y%m%d_%H%M%S).bin,bin

clean:
	rm -rf build/src build/drivers build/*.asm build/*.ihx build/*.hex build/*.lk build/*.lst build/*.map build/*.mem build/*.rel build/*.rst build/*.sym

distclean: clean
	rm -rf backup/*.bin

help:
	@echo "Targets:"
	@echo "  make        Compile to $(HEX)"
	@echo "  make flash  Compile, erase/write/verify, then reset"
	@echo "  make test   Detect programmer and target"
	@echo "  make reset  Reset target"
	@echo "  make backup Read full flash to backup/"
