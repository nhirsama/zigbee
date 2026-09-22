# CC2530 Zigbee 模块开发项目（Linux/SDCC）

本项目用于 CC2530F256 Zigbee 开发板裸机开发。当前主工程是 SPI OLED/SSD1306 显示屏测试固件。

## 快速开始

```bash
make          # 编译主显示屏固件：firmware/baremetal/build/firmware.hex
make test     # 检测 SmartRF04EB/CC-Debugger 和目标芯片
make flash    # 编译、擦写校验、成功后单独 reset
make clean    # 清理生成文件
```

最小点灯验证工程：

```bash
make example-min-led  # 编译到 firmware/examples/min_led/build/
make flash-min-led    # 编译并烧录最小点灯示例
```

## 主固件烧录流程

`make flash` 固定使用两步流程，避免 `cc-tool --reset` 在擦写前执行：

```bash
sudo cc-tool -n CC2530 -e -w build/firmware.hex -v read
sudo cc-tool -n CC2530 --reset
```

## 项目结构

```text
.
├── firmware/                 # 固件源码
│   ├── baremetal/            # 主显示屏工程（默认开发入口）
│   └── examples/min_led/     # 最小点灯验证工程
├── docs/                     # 文档和参考资料
├── Makefile                  # 根目录统一构建入口
└── .gitignore                # 构建产物忽略规则
```

## 目标硬件

- 芯片：CC2530F256，8051 内核
- 下载器：SmartRF04EB / CC-Debugger，`cc-tool` 烧录
- 显示屏：7 针 SPI OLED，SSD1306 兼容 128x64

文档入口：[`docs/README.md`](docs/README.md)。
