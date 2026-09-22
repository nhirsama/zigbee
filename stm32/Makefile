# Root build entry for STM32F103RC + WB-01S WiFi + ST7735S screen.
# Run all commands from STM32/: make, make test, make flash-openocd, ...

PROJECT       ?= stm32_wifi_screen
FIRMWARE_DIR  := firmware
BUILD_DIR     ?= $(FIRMWARE_DIR)/build
TEST_BUILD_DIR ?= $(FIRMWARE_DIR)/tests/build

PREFIX        ?= arm-none-eabi-
CC            := $(PREFIX)gcc
AS            := $(PREFIX)gcc
OBJCOPY       := $(PREFIX)objcopy
OBJDUMP       := $(PREFIX)objdump
SIZE          := $(PREFIX)size
HOST_CC       ?= cc
OPENOCD       ?= openocd
STM32FLASH    ?= stm32flash
PORT          ?= /dev/ttyUSB0
BAUD          ?= 115200
# CH340/USB-TTL boards with RTS/DTR wired to BOOT0/RESET can auto-enter
# the STM32 ROM bootloader with this sequence. Set STM32FLASH_SEQ= to disable.
STM32FLASH_SEQ ?= rts,dtr,-rts,-dtr,rts,dtr,,
FLASH_SERIAL_INIT := $(if $(strip $(STM32FLASH_SEQ)),-i "$(STM32FLASH_SEQ)",)

CPU           := -mcpu=cortex-m3 -mthumb
DEFS          := -DSTM32F10X_HD -DUSE_STDPERIPH_DRIVER
EXTRA_DEFS    ?=

# Friendly Goster-IoT build knobs. Example:
#   make clean
#   make GOSTER=1 GOSTER_HOST=192.168.1.10 GOSTER_UUID=dev-1 GOSTER_TOKEN=token-dev-1
GOSTER        ?= 0
GOSTER_HOST   ?=
GOSTER_PORT   ?=
GOSTER_UUID   ?=
GOSTER_TOKEN  ?=
GOSTER_INCLUDE_TOKEN ?=
ifeq ($(strip $(GOSTER)),1)
EXTRA_DEFS    += -DAPP_CLOUD_BACKEND=APP_CLOUD_BACKEND_GOSTER_MQTT
endif
ifneq ($(strip $(GOSTER_HOST)),)
EXTRA_DEFS    += -DAPP_GOSTER_MQTT_HOST=\"$(GOSTER_HOST)\"
endif
ifneq ($(strip $(GOSTER_PORT)),)
EXTRA_DEFS    += -DAPP_GOSTER_MQTT_PORT=$(GOSTER_PORT)u
endif
ifneq ($(strip $(GOSTER_UUID)),)
EXTRA_DEFS    += -DAPP_GOSTER_DEVICE_UUID=\"$(GOSTER_UUID)\"
endif
ifneq ($(strip $(GOSTER_TOKEN)),)
EXTRA_DEFS    += -DAPP_GOSTER_DEVICE_TOKEN=\"$(GOSTER_TOKEN)\"
endif
ifneq ($(strip $(GOSTER_INCLUDE_TOKEN)),)
EXTRA_DEFS    += -DAPP_GOSTER_INCLUDE_TOKEN_IN_PAYLOAD=$(GOSTER_INCLUDE_TOKEN)u
endif
INCLUDES      := \
  -I$(FIRMWARE_DIR)/app \
  -I$(FIRMWARE_DIR)/drivers/board \
  -I$(FIRMWARE_DIR)/drivers/wifi \
  -I$(FIRMWARE_DIR)/drivers/mqtt \
  -I$(FIRMWARE_DIR)/drivers/display \
  -I$(FIRMWARE_DIR)/drivers/sensor \
  -I$(FIRMWARE_DIR)/services/bemfa \
  -I$(FIRMWARE_DIR)/services/goster \
  -I$(FIRMWARE_DIR)/assets/images \
  -I$(FIRMWARE_DIR)/port/cmsis \
  -I$(FIRMWARE_DIR)/third_party/ugui \
  -I$(FIRMWARE_DIR)/third_party/cjson \
  -I$(FIRMWARE_DIR)/third_party/paho_mqtt_packet/src \
  -I$(FIRMWARE_DIR)/third_party/STM32F10x_StdPeriph_Driver/inc

# 原始字库/字符串包含 GBK/GB2312 内容，默认按 GBK 编译。
CHARSET       ?= -finput-charset=GBK -fexec-charset=GBK

CFLAGS        := $(CPU) $(DEFS) $(EXTRA_DEFS) $(INCLUDES) $(CHARSET) -std=gnu99 -O2 -g3 \
                 -ffunction-sections -fdata-sections -Wall -Wextra \
                 -Wno-unused-parameter -Wno-missing-field-initializers \
                 -Wno-missing-braces -Wno-unterminated-string-initialization
ASFLAGS       := $(CPU) -x assembler-with-cpp -g3
LDSCRIPT      := $(FIRMWARE_DIR)/linker/STM32F103RC_FLASH.ld
LDFLAGS       := $(CPU) -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map \
                 -Wl,--gc-sections -Wl,--print-memory-usage
LDLIBS        := --specs=nano.specs -lc -lm -lnosys

STARTUP_SRCS  := $(FIRMWARE_DIR)/startup/startup_stm32f103xc_gcc.s
PORT_SRCS     := $(FIRMWARE_DIR)/port/syscalls.c
APP_SRCS      := \
  $(FIRMWARE_DIR)/app/system_stm32f10x.c \
  $(FIRMWARE_DIR)/app/stm32f10x_it.c \
  $(FIRMWARE_DIR)/app/main.c \
  $(FIRMWARE_DIR)/app/app_ui.c
BOARD_SRCS    := \
  $(FIRMWARE_DIR)/drivers/board/beep.c \
  $(FIRMWARE_DIR)/drivers/board/key.c \
  $(FIRMWARE_DIR)/drivers/board/usart1.c \
  $(FIRMWARE_DIR)/drivers/board/led.c \
  $(FIRMWARE_DIR)/drivers/board/delay.c
DISPLAY_SRCS  := \
  $(FIRMWARE_DIR)/drivers/display/lcd.c \
  $(FIRMWARE_DIR)/drivers/display/spi.c \
  $(FIRMWARE_DIR)/drivers/display/ugui_port.c
WIFI_SRCS     := $(FIRMWARE_DIR)/drivers/wifi/wifi.c
SENSOR_SRCS   := $(FIRMWARE_DIR)/drivers/sensor/sensor.c
MQTT_SRCS     := \
  $(FIRMWARE_DIR)/drivers/mqtt/mqtt_client.c \
  $(FIRMWARE_DIR)/drivers/mqtt/mqtt_wifi_transport.c
BEMFA_SRCS    := \
  $(FIRMWARE_DIR)/services/bemfa/bemfa_protocol.c \
  $(FIRMWARE_DIR)/services/bemfa/bemfa_wifi.c
GOSTER_SRCS   := \
  $(FIRMWARE_DIR)/services/goster/goster_mqtt.c
PAHO_MQTT_SRCS := \
  $(FIRMWARE_DIR)/third_party/paho_mqtt_packet/src/MQTTPacket.c \
  $(FIRMWARE_DIR)/third_party/paho_mqtt_packet/src/MQTTConnectClient.c \
  $(FIRMWARE_DIR)/third_party/paho_mqtt_packet/src/MQTTSerializePublish.c \
  $(FIRMWARE_DIR)/third_party/paho_mqtt_packet/src/MQTTDeserializePublish.c \
  $(FIRMWARE_DIR)/third_party/paho_mqtt_packet/src/MQTTSubscribeClient.c \
  $(FIRMWARE_DIR)/third_party/paho_mqtt_packet/src/MQTTUnsubscribeClient.c
UGUI_SRCS     := $(FIRMWARE_DIR)/third_party/ugui/ugui.c
PERIPH_SRCS   := $(wildcard $(FIRMWARE_DIR)/third_party/STM32F10x_StdPeriph_Driver/src/*.c)

SRCS          := $(STARTUP_SRCS) $(PORT_SRCS) $(APP_SRCS) $(BOARD_SRCS) \
                 $(DISPLAY_SRCS) $(WIFI_SRCS) $(SENSOR_SRCS) $(MQTT_SRCS) \
                 $(BEMFA_SRCS) $(GOSTER_SRCS) $(PAHO_MQTT_SRCS) $(UGUI_SRCS) $(PERIPH_SRCS)
C_SRCS        := $(filter %.c,$(SRCS))
S_SRCS        := $(filter %.s,$(SRCS))
OBJS          := $(patsubst $(FIRMWARE_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS)) \
                 $(patsubst $(FIRMWARE_DIR)/%.s,$(BUILD_DIR)/%.o,$(S_SRCS))

ELF           := $(BUILD_DIR)/$(PROJECT).elf
HEX           := $(BUILD_DIR)/$(PROJECT).hex
BIN           := $(BUILD_DIR)/$(PROJECT).bin
LST           := $(BUILD_DIR)/$(PROJECT).lst

TEST_BEMFA    := $(TEST_BUILD_DIR)/test_bemfa_protocol
TEST_GOSTER   := $(TEST_BUILD_DIR)/test_goster_mqtt

.PHONY: all firmware check test flash-openocd flash-serial erase-openocd reset-openocd size clean distclean help

all: firmware

firmware: $(ELF) $(HEX) $(BIN) $(LST) size

check:
	@command -v $(CC) >/dev/null || { echo "missing $(CC)"; exit 1; }
	@command -v $(OBJCOPY) >/dev/null || { echo "missing $(OBJCOPY)"; exit 1; }
	@command -v $(SIZE) >/dev/null || { echo "missing $(SIZE)"; exit 1; }

$(BUILD_DIR)/%.o: $(FIRMWARE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(FIRMWARE_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -MMD -MP -c $< -o $@

$(ELF): check $(OBJS) $(LDSCRIPT)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(LST): $(ELF)
	$(OBJDUMP) -S $< > $@

size: $(ELF)
	$(SIZE) $<

test: $(TEST_BEMFA) $(TEST_GOSTER)
	$(TEST_BEMFA)
	$(TEST_GOSTER)

$(TEST_BEMFA): $(FIRMWARE_DIR)/tests/test_bemfa_protocol.c \
               $(FIRMWARE_DIR)/services/bemfa/bemfa_protocol.c \
               $(FIRMWARE_DIR)/services/bemfa/bemfa_protocol.h
	@mkdir -p $(TEST_BUILD_DIR)
	$(HOST_CC) -I$(FIRMWARE_DIR)/services/bemfa -std=c99 -Wall -Wextra -Werror -g \
	  $(FIRMWARE_DIR)/tests/test_bemfa_protocol.c \
	  $(FIRMWARE_DIR)/services/bemfa/bemfa_protocol.c -o $@

$(TEST_GOSTER): $(FIRMWARE_DIR)/tests/test_goster_mqtt.c \
                $(FIRMWARE_DIR)/services/goster/goster_mqtt.c \
                $(FIRMWARE_DIR)/services/goster/goster_mqtt.h
	@mkdir -p $(TEST_BUILD_DIR)
	$(HOST_CC) -I$(FIRMWARE_DIR)/services/goster -std=c99 -Wall -Wextra -Werror -g \
	  $(FIRMWARE_DIR)/tests/test_goster_mqtt.c \
	  $(FIRMWARE_DIR)/services/goster/goster_mqtt.c -o $@

flash-openocd: $(ELF)
	$(OPENOCD) -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program $(ELF) verify reset exit"

flash-serial: $(BIN)
	$(STM32FLASH) -b $(BAUD) $(FLASH_SERIAL_INIT) -w $(BIN) -v -g 0x08000000 $(PORT)

erase-openocd:
	$(OPENOCD) -f interface/stlink.cfg -f target/stm32f1x.cfg -c "init; reset halt; stm32f1x mass_erase 0; reset run; exit"

reset-openocd:
	$(OPENOCD) -f interface/stlink.cfg -f target/stm32f1x.cfg -c "init; reset run; exit"

clean:
	rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR)

distclean: clean

help:
	@echo "make                    build firmware ELF/HEX/BIN from STM32/"
	@echo "make test               run host-side tests"
	@echo "make flash-openocd      flash through ST-Link/OpenOCD"
	@echo "make flash-serial PORT=/dev/ttyUSB0  flash through STM32 serial bootloader"
	@echo "make clean              remove build artifacts"

-include $(OBJS:.o=.d)
