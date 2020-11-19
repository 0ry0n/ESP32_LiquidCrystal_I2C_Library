#include "display_i2c.h"

#define I2C_MASTER_SCL_IO 18 /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 19 /*!< gpio number for I2C master data  */
#define ESP_SLAVE_ADDR 0x27  /*!< ESP32 slave address, you can set any 7bit value */

void app_main(void)
{
	/* Display and bus I2C init */
	lcd_init(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, ESP_SLAVE_ADDR, 16, 2, LCD_5x8DOTS);
	lcd_begin();

	/* Custom character creation */
	uint8_t heart[8] = {0x0, 0xa, 0x1f, 0x1f, 0xe, 0x4, 0x0};
	lcd_createChar(0, heart);

	/* Clear Display */
	lcd_clear();

	/* Simple Print */
	lcd_print("Hello World!");

	/* Set cursor */
	lcd_setCursor(0, 1);

	/* Write heart */
	lcd_write(0);
}