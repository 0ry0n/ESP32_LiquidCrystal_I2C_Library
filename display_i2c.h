#ifndef __DISPLAY_I2C_H__
#define __DISPLAY_I2C_H__

#include <stdint.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x06
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04 /*!< Set 5x10 dot matrix */
#define LCD_5x8DOTS 0x00  /*!< Set 5x8 dot matrix */

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04 //B00000100 Enable bit
#define Rw 0x02 //B00000010 Read/Write bit
#define Rs 0x01 //B00000001 Register select bit

/**
  * This is the driver for the Liquid Crystal LCD displays Hitachi HD44780 (or a compatible)
  * that use PCF8574 expander for the I2C bus.
  *
  * After calling lcd_init(), first call lcd_begin() before anything else.
  * The backlight is on by default, since that is the most likely operating mode in
  * most cases.
  */

/**
  * @brief Initialize I2C bus, must be called before anything else is done.
  * 
  * @param gpio_sda GPIO pin used for SDA I2C pin.
  * @param gpio_scl GPIO pin used for SCL I2C pin.
  * @param lcd_addr I2C slave address of the LCD display. Most likely printed on the
  * 		    LCD circuit board, or look in the supplied LCD documentation.
  * @param lcd_cols Number of columns your LCD display has.
  * @param lcd_rows Number of rows your LCD display has.
  * @param charsize The size in dots that the display has, use LCD_5x10DOTS or LCD_5x8DOTS.
  *
  * @return none
  */
void lcd_init(uint8_t gpio_sda, uint8_t gpio_scl, uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows, uint8_t charsize);

/**
  * @brief  Set the LCD display in the correct begin state, must be called after lcd_init().
  *
  * @param  none
  *
  * @return none
  */
void lcd_begin(void);

/**
  * @brief  Remove all the characters currently shown. Next print/write operation will start
  *         from the first position on LCD display.  
  *
  * @param  none
  *
  * @return none
  */
void lcd_clear(void);

/**
  * @brief  Next print/write operation will will start from the first position on the LCD display.
  *
  * @param  none
  *
  * @return none
  */
void lcd_home(void);

/**
  * @brief  Do not show any characters on the LCD display. Backlight state will remain unchanged.
  *         Also all characters written on the display will return, when the display in enabled again.
  *
  * @param  none
  *
  * @return none
  */
void lcd_noDisplay(void);

/**
  * @brief  Show the characters on the LCD display, this is the normal behaviour. This method should
  *         only be used after lcd_noDisplay() has been used.
  *
  * @param  none
  *
  * @return none
  */
void lcd_display(void);

/**
  * @brief  Do not blink the cursor indicator.
  *
  * @param  none
  *
  * @return none
  */
void lcd_noBlink(void);

/**
  * @brief  Start blinking the cursor indicator.
  *
  * @param  none
  *
  * @return none
  */
void lcd_blink(void);

/**
  * @brief  Do not show a cursor indicator.
  *
  * @param  none
  *
  * @return none
  */
void lcd_noCursor(void);

/**
  * @brief  Show a cursor indicator, cursor can blink on not blink. Use the
  *         methods lcd_blink() and lcd_noBlink() for changing cursor blink.
  *
  * @param  none
  *
  * @return none
  */
void lcd_cursor(void);

/**
  * @brief  Scroll the display to left.
  *
  * @param  none
  *
  * @return none
  */
void lcd_scrollDisplayLeft(void);

/**
  * @brief  Scroll the display to right.
  *
  * @param  none
  *
  * @return none
  */
void lcd_scrollDisplayRight(void);

/**
  * @brief  This is for text that flows Left to Right.
  *
  * @param  none
  *
  * @return none
  */
void lcd_leftToRight(void);

/**
  * @brief  This is for text that flows Right to Left.
  *
  * @param  none
  *
  * @return none
  */
void lcd_rightToLeft(void);

/**
  * @brief  Turn the backlight off.
  *
  * @param  none
  *
  * @return none
  */
void lcd_noBacklight(void);

/**
  * @brief  Turn the backlight on.
  *
  * @param  none
  *
  * @return none
  */
void lcd_backlight(void);

/**
  * @brief  This will 'right justify' text from the cursor.
  *
  * @param  none
  *
  * @return none
  */
void lcd_autoscroll(void);

/**
  * @brief  This will 'left justify' text from the cursor.
  *
  * @param  none
  *
  * @return none
  */
void lcd_noAutoscroll(void);

/**
  * @brief  Allows us to fill the first 8 CGRAM locations with custom characters.
  *
  * @param  location We only have 8 locations 0-7.
  * @param  charmap  Your custom char.
  *
  * @return none
  */
void lcd_createChar(uint8_t location, uint8_t charmap[]);

/**
  * @brief  Set cursor position.
  *
  * @param  col Destination column.
  * @param  row Destination row.
  *
  * @return none
  */
void lcd_setCursor(uint8_t col, uint8_t row);

/**
  * @brief  Print a string.
  *
  * @param  str String to print.
  *
  * @return none
  */
void lcd_print(char *str);

/**
  * @brief  Alias for lcd_blink().
  *
  * @param  none
  *
  * @return none
  */
void lcd_blink_on(void);

/**
  * @brief  Alias for lcd_noBlink().
  *
  * @param  none
  *
  * @return none
  */
void lcd_blink_off(void);

/**
  * @brief  Alias for lcd_cursor().
  *
  * @param  none
  *
  * @return none
  */
void lcd_cursor_on(void);

/**
  * @brief  Alias for lcd_noCursor().
  *
  * @param  none
  *
  * @return none
  */
void lcd_cursor_off(void);

// Compatibility API function aliases
/**
  * @brief  Alias for lcd_backlight() and lcd_noBacklight().
  *
  * @param  new_val Value for display backlight.
  *
  * @return none
  */
void lcd_setBacklight(uint8_t new_val);

/**
  * @brief  Alias for lcd_createChar().
  *
  * @param  char_num
  * @param  rows
  *
  * @return none
  */
void lcd_load_custom_character(uint8_t char_num, uint8_t *rows);

/**
  * @brief  Mid level function for sending data.
  *
  * @param  value Data to send.
  *
  * @return none
  */
void lcd_write(uint8_t value);

/**
  * @brief  Mid level function for sending command
  *
  * @param  value Command to send.
  *
  * @return none
  */
void lcd_command(uint8_t value);

#endif /* __DISPLAY_I2C_H__ */