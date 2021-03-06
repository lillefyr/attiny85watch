/*
 * Ref.:
 * DigisparkOLED: https://github.com/digistump/DigistumpArduino/tree/master/digistump-avr/libraries/DigisparkOLED
 * SSD1306 data sheet: https://www.adafruit.com/datasheets/SSD1306.pdf
 */

#include "ssd1306.h"
#include <avr/pgmspace.h>
#include <TinyWireM.h>

/*
 * Software Configuration, data sheet page 64
 */
/*
static const uint8_t ssd1306_configuration[] PROGMEM = {

#ifdef SCREEN128X64
  0xA8, 0x3F,   // Set MUX Ratio, 0F-3F
#else // SCREEN128X32 / SCREED64X32
  0xA8, 0x1F,   // Set MUX Ratio, 0F-3F
#endif

  0xD3, 0x00,   // Set Display Offset
  0x40,         // Set Display Start line
  0xA1,         // Set Segment re-map, mirror, A0/A1
  0xC8,         // Set COM Output Scan Direction, flip, C0/C8

#ifdef SCREEN128X64
  0xDA, 0x02,   // Set COM Pins hardware configuration, Sequential
#else // SCREEN128X32 / SCREED64X32
  0xDA, 0x12,   // Set Com Pins hardware configuration, Alternative
#endif

  0x81, 0x01,   // Set Contrast Control, 01-FF
  0xA4,         // Disable Entire Display On, 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
  0xA6,         // Set Display Mode. A6=Normal; A7=Inverse
  0xD5, 0x80,   // Set Osc Frequency
  0x8D, 0x14,   // Enable charge pump regulator
  0xAF          // Display ON in normal mode
};
*/

static const uint8_t ssd1306_configuration[] PROGMEM = {

//0xE4, //reset???

0xAE, // display off
0xD5, // set display clock div
0x80, // osc frequency suggested ratio
0xA8, // set multiplex

0x3F, // set height -1 ( 64 == 0x40 )

0xD3, // set display offset
0x00, // no offset  // can be 0b01000 and will move something to a higher column or something
0x40, // Set Display Start line
0x8D, // charge pump

0x14, // 0x10 does not work (externalVCC)

0x20, // some memory mode
0x00, // Horizontal is easier to understand (maybe)
0xA1, // Set Segment re-map, mirror, A0/A1
0xC8, // Set COM Output Scan Direction, flip, C0/C8 up or down

0xDA, // set COM pins
0x12, // 128x64 Sequential

0x81, // set contrast
0xAF, //9F, // 0x01 ??? Set Contrast Control, 01-FF

0xD9, // set precharge??? D'oh
0x22, // external VCC

0xDB, // VCOM detect
0x40,
0xA4, // Disable Entire Display On, 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
      // changed to A5, now all screen is white
      // Back to A4
0xA6, // Set Display Mode. A6=Normal; A7=Inverse
//0x2E, // stop scroll, (last time f*cked up display)

0xAF  // Display ON in normal mode
};

SSD1306::SSD1306(void) {}

void SSD1306::begin(void)
{
  for (uint8_t i = 0; i < sizeof (ssd1306_configuration); i++) {
    ssd1306_send_command(pgm_read_byte_near(&ssd1306_configuration[i]));
  }
}

void SSD1306::ssd1306_send_command_start(void) {
  TinyWireM.beginTransmission(SSD1306_I2C_ADDR);
  TinyWireM.send(0x00); //command
}

void SSD1306::ssd1306_send_command_stop(void) {
  TinyWireM.endTransmission();
}

void SSD1306::ssd1306_send_command(uint8_t command)
{
  ssd1306_send_command_start();
  TinyWireM.send(command);
  ssd1306_send_command_stop();
}

void SSD1306::ssd1306_send_data_start(void)
{
  TinyWireM.beginTransmission(SSD1306_I2C_ADDR);
  TinyWireM.send(0x40); //data from address 0
}

void SSD1306::ssd1306_send_data_stop(void)
{
  TinyWireM.endTransmission();
}

void SSD1306::ssd1306_send_data_byte(uint8_t data)
{
  if (TinyWireM.write(data) == 0) {
    // push data if detect buffer used up
    ssd1306_send_data_stop();
    ssd1306_send_data_start();
    TinyWireM.write(data);
  }
}


void SSD1306::set_all_area(uint8_t col, uint8_t len, uint8_t page)
{
  ssd1306_send_command_start();
  TinyWireM.send(0x20); // Set Memory Addressing Mode
  TinyWireM.send(0x01); // Vertical addressing mode (write whole column before going to next)
  TinyWireM.send(0x21); // Set Column Address
  TinyWireM.send(col); // start with offset=0 and column=0
  TinyWireM.send(len); // WIDTH - 1 
  TinyWireM.send(0x22); // Set Page Address
  TinyWireM.send(page); // start with page 0
  TinyWireM.send(page); // PAGES - 1  (3) 
  ssd1306_send_command_stop();
}

void SSD1306::set_all_area2(uint8_t colstart, uint8_t colend, uint8_t pagestart, uint8_t pageend)
{
  ssd1306_send_command_start();
  TinyWireM.send(0x20); // Set Memory Addressing Mode
  TinyWireM.send(0x00); // Horizontal
  TinyWireM.send(0x21); // Set Column Address
  TinyWireM.send(colstart); // start with offset=0 and column=0
  TinyWireM.send(colend); // WIDTH - 1 
  TinyWireM.send(0x22); // Set Page Address
  TinyWireM.send(pagestart); // start with page 0
  TinyWireM.send(pageend); // PAGES - 1  (7) 
  ssd1306_send_command_stop();
}

void SSD1306::set_area(uint8_t col, uint8_t page, uint8_t col_range_minus_1, uint8_t page_range_minus_1)
{
  ssd1306_send_command_start();
  TinyWireM.send(0x20); // Set Memory Addressing Mode
  TinyWireM.send(0x01); // Vertical addressing mode (write whole column before going to next)
  TinyWireM.send(0x21); // Set Column Address
  TinyWireM.send(XOFFSET + col); // start with offset=0 and column=0
  TinyWireM.send(XOFFSET + col + col_range_minus_1); // WIDTH -1 (6 or 14)
  TinyWireM.send(0x22); // Set Page Address
  TinyWireM.send(page); // start with page 0
  TinyWireM.send(PAGES - page -1); // page + page_range_minus_1); // PAGES - 1  (7) 
  ssd1306_send_command_stop();
}

void SSD1306::fill(uint8_t data)
{
  for (uint8_t page = 0; page <= PAGES; page++)
  {
    set_all_area(0, 127, page);
    ssd1306_send_data_start();
    for (uint8_t cnt=0; cnt<127; cnt++) {
      ssd1306_send_data_byte(data);
    }
    ssd1306_send_data_stop();
    delay(10);
  }
/*  for (uint8_t cnt=0; cnt<5; cnt++) {
    draw_digit_24x32(cnt,0,data,false);
    draw_digit_24x32(cnt,1,data,false);
  }
  */
}

void SSD1306::v_line(uint8_t col, uint8_t data)
{
  set_area(col, 0, 0, PAGES);
  ssd1306_send_data_start();
  for (uint8_t i = 0; i <= PAGES; i++)
  {
    ssd1306_send_data_byte(data);
  }
  ssd1306_send_data_stop();
}

void SSD1306::draw_pattern(uint8_t col, uint8_t page, uint8_t width, uint8_t pattern) {
  set_area(col, page, width, 0);
  ssd1306_send_data_start();
  for (uint8_t i = 0; i < width; i++) {
    ssd1306_send_data_byte(pattern);
  }
  ssd1306_send_data_stop();
}

void SSD1306::draw_pattern2(uint8_t pattern) {
  ssd1306_send_data_start();
  ssd1306_send_data_byte(pattern);
  ssd1306_send_data_stop();
}

int SSD1306::get_offset(byte theChar){
  // calculated based on the array in watchdigit.h
  if (( theChar >= 0) && ( theChar <= 9)) { return theChar*96; }
  if (theChar == 0x20) { return 10*96; }
  if (theChar == ';')  { return 11*96; }
  if (theChar == '.')  { return 12*96; }
  if (theChar == 'V')  { return 13*96; }
  return -1;  
}

void SSD1306::draw_digit_24x32(uint8_t col, uint8_t row, uint8_t digit, bool invert_color)
{
  int offset = get_offset(digit);
  col=col*24; // size of char
  if (( offset == -1) || (col > 100) || ( row > 1)) { return; } //cant print here
  
  for (uint8_t page = row*4; page< ((row*4)+4); page++) {
    set_all_area(col, 127, page); //0 127 4
    ssd1306_send_data_start();
    for (int i=0; i < 24; i++){
      if (page > 3) {
        ssd1306_send_data_byte(pgm_read_word_near(&font24x32[offset+i+(24*(page-4))])); // 0+0+(24*4)
      }
      else
      {
        ssd1306_send_data_byte(pgm_read_word_near(&font24x32[offset+i+(24*page)])); // 0+0+(24*4)
      }
    }
    ssd1306_send_data_stop();
    delay(10);
  }
}


void SSD1306::draw_digit(uint8_t col, uint8_t page, uint8_t digit, bool invert_color)
{
  set_area(col, page, FONTWIDTH - 1, 1 - 1);
  uint16_t offset = digit * FONTWIDTH;
  uint8_t data;

  ssd1306_send_data_start();
  for (uint8_t i = 0; i < FONTWIDTH; i++) // 8x7 font size data: (8 / 8) * 7 = 7
  {
    data = pgm_read_byte_near(&watch_digit[offset++]);
    if (invert_color) data = ~ data; // invert
    ssd1306_send_data_byte(data);
  }
  ssd1306_send_data_stop();
}

void SSD1306::print_digits(uint8_t col, uint8_t page, uint8_t font_size, uint32_t factor, uint32_t digits, bool invert_color) {
  uint16_t cur_digit = digits / factor;

  if (font_size == 1) {
    draw_digit(col, page, cur_digit, invert_color);
  }

  if (factor > 1) {
    if (font_size == 1) {
      print_digits(col + FONTWIDTH, page, font_size, factor / 10, digits - (cur_digit * factor), invert_color);
    } else {
      print_digits(col + FONT3XWIDTH, page, font_size, factor / 10, digits - (cur_digit * factor), invert_color);
    }
  }
}

void SSD1306::off(void)
{
  ssd1306_send_command(0xAE);
}

void SSD1306::on(void)
{
  ssd1306_send_command(0xAF);
}
