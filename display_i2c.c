#include "display_i2c.h"
#include "driver/i2c.h"
#include "esp32/rom/ets_sys.h"
#include "esp_log.h"

#define I2C_MASTER_NUM I2C_NUM_0    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency 100kHz */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */

#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */
#define ACK_CHECK_EN 0x1	   /*!< I2C master will check ack from slave */
#define ACK_CHECK_DIS 0x0	   /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0		   /*!< I2C ack value */
#define NACK_VAL 0x1		   /*!< I2C nack value */

uint8_t _gpio_sda;
uint8_t _gpio_scl;
uint8_t _addr;
uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;
uint8_t _cols;
uint8_t _rows;
uint8_t _charsize;
uint8_t _backlightval;

static esp_err_t i2c_master_driver_initialize(void);
static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t data_wr);

static void lcd_send(uint8_t value, uint8_t mode);
static void lcd_write4bits(uint8_t value);
static void lcd_expanderWrite(uint8_t _data);
static void lcd_pulseEnable(uint8_t _data);

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

void lcd_init(uint8_t gpio_sda, uint8_t gpio_scl, uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows, uint8_t charsize)
{
	_gpio_sda = gpio_sda;
	_gpio_scl = gpio_scl;
	_addr = lcd_addr;
	_cols = lcd_cols;
	_rows = lcd_rows;
	_charsize = charsize;
	_backlightval = LCD_BACKLIGHT;
}

void lcd_begin(void)
{
	ESP_ERROR_CHECK(i2c_master_driver_initialize());
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if (_rows > 1)
	{
		_displayfunction |= LCD_2LINE;
	}

	// for some 1 line displays you can select a 10 pixel high font
	if ((_charsize != 0) && (_rows == 1))
	{
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	ets_delay_us(50000);

	// Now we pull both RS and R/W low to begin commands
	lcd_expanderWrite(_backlightval); // reset expanderand turn backlight off (Bit 8 =1)
	ets_delay_us(1000000);

	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	lcd_write4bits(0x03 << 4);
	ets_delay_us(4500); // wait min 4.1ms

	// second try
	lcd_write4bits(0x03 << 4);
	ets_delay_us(4500); // wait min 4.1ms

	// third go!
	lcd_write4bits(0x03 << 4);
	ets_delay_us(150);

	// finally, set to 4-bit interface
	lcd_write4bits(0x02 << 4);

	// set # lines, font size, etc.
	lcd_command(LCD_FUNCTIONSET | _displayfunction);

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	lcd_display();

	// clear it off
	lcd_clear();

	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	lcd_command(LCD_ENTRYMODESET | _displaymode);

	lcd_home();
}

/********** high level commands, for the user! */
void lcd_clear(void)
{
	lcd_command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
	ets_delay_us(2000);	       // this command takes a long time!
}

void lcd_home(void)
{
	lcd_command(LCD_RETURNHOME); // set cursor position to zero
	ets_delay_us(2000);	     // this command takes a long time!
}

// Turn the display on/off (quickly)
void lcd_noDisplay(void)
{
	_displaycontrol &= ~LCD_DISPLAYON;
	lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void lcd_display(void)
{
	_displaycontrol |= LCD_DISPLAYON;
	lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void lcd_noBlink(void)
{
	_displaycontrol &= ~LCD_BLINKON;
	lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void lcd_blink(void)
{
	_displaycontrol |= LCD_BLINKON;
	lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void lcd_noCursor(void)
{
	_displaycontrol &= ~LCD_CURSORON;
	lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void lcd_cursor(void)
{
	_displaycontrol |= LCD_CURSORON;
	lcd_command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void lcd_scrollDisplayLeft(void)
{
	lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void lcd_scrollDisplayRight(void)
{
	lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void lcd_leftToRight(void)
{
	_displaymode |= LCD_ENTRYLEFT;
	lcd_command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void lcd_rightToLeft(void)
{
	_displaymode &= ~LCD_ENTRYLEFT;
	lcd_command(LCD_ENTRYMODESET | _displaymode);
}

// Turn the (optional) backlight off/on
void lcd_noBacklight(void)
{
	_backlightval = LCD_NOBACKLIGHT;
	lcd_expanderWrite(0);
}
void lcd_backlight(void)
{
	_backlightval = LCD_BACKLIGHT;
	lcd_expanderWrite(0);
}

// This will 'right justify' text from the cursor
void lcd_autoscroll(void)
{
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	lcd_command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void lcd_noAutoscroll(void)
{
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	lcd_command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void lcd_createChar(uint8_t location, uint8_t charmap[])
{
	location &= 0x7; // we only have 8 locations 0-7
	lcd_command(LCD_SETCGRAMADDR | (location << 3));
	for (uint8_t i = 0; i < 8; i++)
	{
		lcd_write(charmap[i]);
	}
}

void lcd_setCursor(uint8_t col, uint8_t row)
{
	uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
	if (row > _rows)
	{
		row = _rows - 1; // we count rows starting w/0
	}
	lcd_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void lcd_print(char *str)
{
	for (; *str; str++)
		lcd_write(*str);
}

void lcd_blink_on(void)
{
	lcd_blink();
}
void lcd_blink_off(void)
{
	lcd_noBlink();
}

void lcd_cursor_on(void)
{
	lcd_cursor();
}
void lcd_cursor_off(void)
{
	lcd_noCursor();
}

void lcd_setBacklight(uint8_t new_val)
{
	if (new_val)
	{
		lcd_backlight(); // turn backlight on
	}
	else
	{
		lcd_noBacklight(); // turn backlight off
	}
}

void lcd_load_custom_character(uint8_t char_num, uint8_t *rows)
{
	lcd_createChar(char_num, rows);
}

/*********** mid level commands, for sending data/cmds */

void lcd_write(uint8_t value)
{
	lcd_send(value, Rs);
}

void lcd_command(uint8_t value)
{
	lcd_send(value, 0);
}

/************ low level data pushing commands **********/

// write either command or data
static void lcd_send(uint8_t value, uint8_t mode)
{
	uint8_t highnib = value & 0xf0;
	uint8_t lownib = (value << 4) & 0xf0;
	lcd_write4bits((highnib) | mode);
	lcd_write4bits((lownib) | mode);
}

static void lcd_write4bits(uint8_t value)
{
	lcd_expanderWrite(value);
	lcd_pulseEnable(value);
}

static void lcd_expanderWrite(uint8_t _data)
{
	ESP_ERROR_CHECK(i2c_master_write_slave(I2C_MASTER_NUM, _data | _backlightval));
}

static void lcd_pulseEnable(uint8_t _data)
{
	lcd_expanderWrite(_data | En); // En high
	ets_delay_us(1);	       // enable pulse must be >450ns

	lcd_expanderWrite(_data & ~En); // En low
	ets_delay_us(50);		// commands need > 37us to settle
}

/******************** I2C commands *********************/

/**
  * @brief I2C master initialization
  */
static esp_err_t i2c_master_driver_initialize(void)
{
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = _gpio_sda;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = _gpio_scl;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	i2c_param_config(I2C_MASTER_NUM, &conf);
	return i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
  * @brief Write to the display via I2C bus.
  *
  * ________________________________________________________________
  * | start | slave_addr + wr_bit + ack | write byte + ack  | stop |
  * --------|---------------------------|-------------------|------|
  *
  */
static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t data_wr)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, data_wr, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}
