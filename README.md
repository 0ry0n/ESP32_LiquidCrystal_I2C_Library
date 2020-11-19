# ESP32_LiquidCrystal_I2C_Library
 Library for Liquid Crystal LCD displays Hitachi HD44780 (or a compatible) that use PCF8574 expander for the I2C bus.
 
## Usage ##
Include library in your code.
```
#include "display_i2c.h"
```

Set I2C addr, SDA and SCL GPIO numbers.
```
lcd_init(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, ESP_SLAVE_ADDR, 16, 2, LCD_5x8DOTS);
```

Init display.
```
lcd_begin();
```

---
This project is based on Arduino [LiquidCrystal](https://github.com/arduino-libraries/LiquidCrystal) library.
