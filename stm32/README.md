# STM32 WiFi 屏幕固件工程

这是 STM32 子项目唯一项目入口文档。构建入口也统一到本目录的唯一 `Makefile`。

```text
STM32/
├── Makefile              # 唯一构建/测试/烧录入口
├── README.md             # 唯一项目入口文档
├── docs/
│   ├── 开发文档.md       # 依赖、硬件连接、模块说明、MQTT/Bemfa 说明
│   ├── display/          # ST7735S、SPI、颜色表资料
│   ├── hardware/         # 最小系统原理图
│   ├── stm32/            # STM32 SPL 说明
│   └── wifi/             # WB-01S/AT 指令资料
├── firmware/
│   ├── app/              # main、系统文件、app_config.h
│   ├── drivers/          # board/wifi/display/sensor/mqtt 驱动，display 内含 UGUI 适配
│   ├── services/bemfa/   # Bemfa 协议格式化与发送
│   ├── startup/          # GCC 启动文件/中断向量表
│   ├── linker/           # STM32F103RC 链接脚本
│   ├── port/             # CMSIS/newlib 适配
│   ├── tests/            # 主机侧测试源码，不放 Makefile
│   ├── assets/           # LCD 图片数组
│   └── third_party/      # STM32 SPL、Paho MQTTPacket、cJSON、UGUI
└── tools/                # 辅助脚本
```

## 屏幕控制界面

固件启动后先在 ST7735S 上显示从 Apple 黑色 SVG 取模得到的 64×78 logo，并在每个模块实际初始化完成后推进进度条；
网络初始化后进入 `while (1)` 主循环，再显示 UGUI 风格控制页。控制页运行时不再每次整屏清空刷新：
菜单移动只重绘变化行，传感器/网络数值更新只重绘对应行，进入/返回菜单只刷新标题、内容区和底栏。

- 顶部显示 WiFi/Bemfa 连接状态。
- 两键菜单操作：`KEY1` 单击向下、双击向上；`KEY2` 单击进入/切换，双击返回上一级。
- 菜单分为 `Quick Ctrl`、`Sensors`、`Network`、`System`、`About`。
- 可控制 `LED2`、`LED3`、`BEEP`、`MQTT Pub`、`LED1 Blink`、`LCD Light`，并支持 `Beep Pulse`、`All Off` 操作。
- `MQTT` 关闭后会暂停周期上报和传感器回调上报；不影响本地传感器显示。

## 常用命令

所有命令都在 `STM32/` 下执行：

```bash
make
make test
make clean
make flash-openocd
make flash-serial PORT=/dev/ttyUSB0 BAUD=115200
```

`flash-serial` 默认使用 CH340/USB-TTL 常见的 RTS/DTR 自动进 Bootloader 时序：
`rts,dtr,-rts,-dtr,rts,dtr,,`。如果你的板子没有接自动下载电路，手动拉 BOOT0/复位时可关闭：

```bash
make flash-serial PORT=/dev/ttyUSB0 BAUD=115200 STM32FLASH_SEQ=
```

输出文件：

- `firmware/build/stm32_wifi_screen.elf`
- `firmware/build/stm32_wifi_screen.hex`
- `firmware/build/stm32_wifi_screen.bin`

## 配置入口

应用配置集中在：

```text
firmware/app/app_config.h
```

包括 WiFi、Bemfa host/port、UID、topic、上报周期等。

云端后端默认保持 Bemfa TCP 文本透传。如需切到 MQTT 3.1.1：

```bash
make clean
make EXTRA_DEFS="-DAPP_CLOUD_BACKEND=APP_CLOUD_BACKEND_MQTT"
```

如需接入 `Goster-IoT` 的 `protocol-ingress` MQTT adapter，先在 Goster-IoT 侧启用 MQTT embedded broker，并保证设备 UUID/token 已在 Core 中存在：

```bash
make clean
make GOSTER=1 GOSTER_HOST=<protocol-ingress所在IP> GOSTER_UUID=<设备UUID> GOSTER_TOKEN=<设备token>
```

固件会使用：

- MQTT client id：`GOSTER_UUID`
- MQTT username：`GOSTER_UUID`
- MQTT password：`GOSTER_TOKEN`
- 上报 topic：`goster/v1/{uuid}/telemetry`
- 心跳 topic：`goster/v1/{uuid}/heartbeat`
- 下行订阅：`goster/v1/{uuid}/downlink`

温湿度会上报为 Goster 可识别的 JSON 字段 `temperature` / `humidity`。

MQTT 相关宏：

- `APP_MQTT_HOST` / `APP_MQTT_PORT`
- `APP_MQTT_CLIENT_ID`
- `APP_MQTT_USERNAME` / `APP_MQTT_PASSWORD`
- `APP_MQTT_TOPIC` / `APP_MQTT_SUB_TOPIC`
- `APP_MQTT_QOS`

## USART3 下位机/网关通信

USART3 已按课程文档配置为 9600 8N1、无校验、无硬件流控，接口使用 PC10/PC11 部分重映射。应用层支持项目自定义二进制结构体透传：

- 兼容裸结构体：`[t_val, h_val]`、`[t_val, h_val, mode]`、`[addr_lo, addr_hi, t_val, h_val]`、`[addr_lo, addr_hi, t_val, h_val, mode]`。
- 支持带帧头格式：`0xA5, version, type, seq, addr_lo, addr_hi, len, payload..., checksum`。
- 收到温湿度数据后通过 `Sensor_SetUpdateCallback()` 更新本地 UI，并按当前 Bemfa topic 上报。

## 更多说明

见：`docs/开发文档.md`。
