SDCC    ?= sdcc
PACKIHX ?= packihx
CCTOOL  ?= cc-tool
SUDO    ?= sudo
TARGET  ?= build/led_on
HEX     := $(TARGET).hex
IHX     := $(TARGET).ihx

CFLAGS  := -mmcs51 --model-small --std-sdcc99 --opt-code-size -I.
SRCS    := main.c led.c
OBJS    := $(patsubst %.c,build/%.rel,$(SRCS))

.PHONY: all flash test clean

all: $(HEX)

build/%.rel: %.c cc2530_min.h led.h
	@mkdir -p $(dir $@)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(IHX): $(OBJS)
	$(SDCC) $(CFLAGS) $(OBJS) -o $(IHX)

$(HEX): $(IHX)
	$(PACKIHX) $(IHX) > $(HEX)

flash: $(HEX)
	$(SUDO) $(CCTOOL) -n CC2530 -e -w $(HEX) -v read --reset

test:
	$(SUDO) $(CCTOOL) -t

clean:
	rm -rf build *.asm *.ihx *.hex *.lk *.lst *.map *.mem *.rel *.rst *.sym
