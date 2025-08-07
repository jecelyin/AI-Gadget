#include "SC7A20.h"

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // 初始化为震动模式
    SC7A20_Init(SC7A20_MODE_VIBRATION);

    // 如果需要单击模式，使用以下代码：
    // SC7A20_Init(SC7A20_MODE_SINGLE_CLICK);
}

void loop() {
    if (SC7A20_NewDataReady()) {
        int16_t acc_x_raw, acc_y_raw, acc_z_raw;
        float acc_x, acc_y, acc_z;

        // 读取原始数据
        SC7A20_ReadAccelerationRaw(&acc_x_raw, &acc_y_raw, &acc_z_raw);
        Serial.print("Raw X: "); Serial.print(acc_x_raw);
        Serial.print(" | Raw Y: "); Serial.print(acc_y_raw);
        Serial.print(" | Raw Z: "); Serial.println(acc_z_raw);

        // 读取浮点值
        SC7A20_ReadAcceleration(&acc_x, &acc_y, &acc_z);
        Serial.print("X: "); Serial.print(acc_x); Serial.print(" g, ");
        Serial.print("Y: "); Serial.print(acc_y); Serial.print(" g, ");
        Serial.print("Z: "); Serial.print(acc_z); Serial.println(" g");
    }

    delay(500);  // 延迟 500ms
}
