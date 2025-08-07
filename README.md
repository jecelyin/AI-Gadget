# AI摆件


**它可以做什么：**
- 查看时间、天气、温度湿度、空气质量
- AI聊天（使用小智开源代码，感谢作者的奉献）
- 音乐播放器（可以给娃听书，需要使用Python建立索引，支持400+文件列表）

本工程使用2个ESP32 S3，LCD板负责RGB屏幕，所有UI都在该主控上面。主板负责网络相关（小智AI，天气，NTP等），温湿度，麦克风相关通讯，所有软件硬件已经开源：

[https://oshwhub.com/jecelyin/esp32-ai-gadget](https://oshwhub.com/jecelyin/esp32-ai-gadget)