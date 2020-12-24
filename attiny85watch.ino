#include <avr/sleep.h>
#include <TinyWireM.h>
#include "ssd1306.h"
#include "WDT_Time.h"

#define TIMEOUT 5000 // 5 seconds
static uint32_t display_timeout;

#define UNUSEDPIN 1
#define SETBUTTON 3
#define UPBUTTON  4

SSD1306 oled;
bool sleeping = false;

void set_display_timeout() {
  display_timeout = millis() + TIMEOUT;
}

/*
 * UI related
 */

// 0: time; 1: debug
static uint8_t display_mode = 0;
static uint8_t last_display_mode = 0;
// 0: none; 1: year; 2: month; 3: day; 4: hour; 5: minute
static uint8_t selected_field = 0;

void draw_oled() {
  if (display_mode != last_display_mode) {
    oled.fill(0x00);
    last_display_mode = display_mode;
  }
  if (display_mode == 0) {
    // 1st rows: print date
    /* print_digits(col, page, size, factor, digits) */
    oled.print_digits(0, 0, 1, 1000, year(), (selected_field == 1) ? true : false);
    /* draw_pattern(col, page, width, pattern) */
    oled.draw_pattern(29, 0, 2, 0b00010000); // dash
    oled.print_digits(32, 0, 1, 10, month(), (selected_field == 2) ? true : false);
    oled.draw_pattern(47, 0, 2, 0b00010000); // dash
    oled.print_digits(50, 0, 1, 10, day(), (selected_field == 3) ? true : false);
    // 2nd-4th rows: print time
    oled.print_digits(0, 1, 3, 10, hour(), (selected_field == 4) ? true : false);
    oled.draw_pattern(31, 2, 2, (second() & 1) ? 0b11000011 : 0); // blink colon
    oled.print_digits(34, 1, 3, 10, minute(), (selected_field == 5) ? true : false);
  } else if (display_mode == 1) {
    oled.print_digits(0, 0, 1, 10000, wdt_get_interrupt_count(), false);
    oled.print_digits(0, 1, 1, 10000, readVcc(), false);
  }
}

void cls(){
  // black screen (first time)
  delay(200);
  for (uint8_t page=0; page<8; page++){
    oled.set_all_area(0, 127, page);
    oled.ssd1306_send_data_start();
    for (int i=0; i < 128; i++){
      oled.ssd1306_send_data_byte(0x00);
    }
    oled.ssd1306_send_data_stop();
    delay(100);
  }
}

void setup() {

    pinMode(PORTB0, OUTPUT); // SDA
    pinMode(PORTB2, OUTPUT); // SCL
    digitalWrite(PORTB2, LOW);
      
//  wdt_setup();

  TinyWireM.begin();
  oled.begin();
/*
  cls();
  uint8_t cnt=0;

  // write lines
  for (uint8_t page=0; page<8; page++){
    //(uint8_t colstart, uint8_t colend, uint8_t pagestart, uint8_t pageend)
    oled.set_all_area(0, 127, page);
    oled.ssd1306_send_data_start();
    for (int i=1; i < 128; i++){
      oled.ssd1306_send_data_byte(0b00010001);
    }
    oled.ssd1306_send_data_stop();
    delay(100);
  }

  cnt=1;
  if (cnt>100) {
    for (uint8_t page=0; page<8; page++){
      oled.set_all_area(8, 16, page);
      oled.ssd1306_send_data_start();
      for (uint8_t col = 0; col < 16; col++) {
        oled.ssd1306_send_data_byte(0b01010101);
      }
      oled.ssd1306_send_data_stop();
    }
    delay(2000);

    for (uint8_t page=0; page<8; page++){
      oled.set_all_area(24, 32, page);
      oled.ssd1306_send_data_start();
      for (uint8_t col = 0; col < 32; col++) {
        oled.ssd1306_send_data_byte(0b10101010);
      }
      oled.ssd1306_send_data_stop();
    }
    delay(2000);

    for (uint8_t page=0; page<8; page++){
      oled.set_all_area(56, 48, page);
      oled.ssd1306_send_data_start();
      for (uint8_t col = 0; col < 48; col++) {
        oled.ssd1306_send_data_byte(0b01010101);
      }
      oled.ssd1306_send_data_stop();
    }
    delay(2000);
  }


  for (uint8_t page=0; page<8; page++){
    //(uint8_t colstart, uint8_t colend, uint8_t pagestart, uint8_t pageend)
    oled.set_all_area(0, 127, page);
    oled.ssd1306_send_data_start();
    for (int i=0; i < 16; i++){
      oled.ssd1306_send_data_byte(0b00011000);
      oled.ssd1306_send_data_byte(0b00100100);
      oled.ssd1306_send_data_byte(0b00100100);
      oled.ssd1306_send_data_byte(0b00111100);
      oled.ssd1306_send_data_byte(0b00100100);
      oled.ssd1306_send_data_byte(0b00100100);
      oled.ssd1306_send_data_byte(0b00100100);
      oled.ssd1306_send_data_byte(0b00000000);
    }
    oled.ssd1306_send_data_stop();
    delay(100);
  }
 
  

  // letter A
  for (uint8_t page=0; page<8; page=page+2){
    //(uint8_t colstart, uint8_t colend, uint8_t pagestart, uint8_t pageend)
    oled.set_all_area(0, 127, page);
    oled.ssd1306_send_data_start();
    for (int i=0; i < 16; i++){
      expand1to3(0b00000000);oled.ssd1306_send_data_byte(word1);oled.ssd1306_send_data_byte(word2);oled.ssd1306_send_data_byte(word3);
      expand1to3(0b00000000);oled.ssd1306_send_data_byte(word1);oled.ssd1306_send_data_byte(word2);oled.ssd1306_send_data_byte(word3);
      expand1to3(0b01111110);oled.ssd1306_send_data_byte(word1);oled.ssd1306_send_data_byte(word2);oled.ssd1306_send_data_byte(word3);
      expand1to3(0b00010001);oled.ssd1306_send_data_byte(word1);oled.ssd1306_send_data_byte(word2);oled.ssd1306_send_data_byte(word3);
      expand1to3(0b00010001);oled.ssd1306_send_data_byte(word1);oled.ssd1306_send_data_byte(word2);oled.ssd1306_send_data_byte(word3);
      expand1to3(0b01111110);oled.ssd1306_send_data_byte(word1);oled.ssd1306_send_data_byte(word2);oled.ssd1306_send_data_byte(word3);
      expand1to3(0b00000000);oled.ssd1306_send_data_byte(word1);oled.ssd1306_send_data_byte(word2);oled.ssd1306_send_data_byte(word3);
      expand1to3(0b00000000);oled.ssd1306_send_data_byte(word1);oled.ssd1306_send_data_byte(word2);oled.ssd1306_send_data_byte(word3);
    }
    oled.ssd1306_send_data_stop();
    delay(100);
  }
*/
  cls();

  for (uint8_t cnt=0; cnt<5; cnt++) {
    oled.draw_digit_24x32(cnt,0,cnt,false);
    oled.draw_digit_24x32(cnt,1,cnt+5,false);
  }
/*
  cls();
  for (uint8_t page=0; page<8; page=page+4){
    //(uint8_t colstart, uint8_t colend, uint8_t pagestart, uint8_t pageend)
    oled.set_all_area(0, 127, page);
    oled.ssd1306_send_data_start();
    for (int i=0; i < 127; i++){
      oled.ssd1306_send_data_byte(0b11111111);
    }
    oled.ssd1306_send_data_stop();
    delay(1000);
    cls();
  }
*/
  while (true) {
//    cls();

//    for (uint8_t i=0; i<10; i++){
//      oled.draw_digit_24x32(i*8,0,i,false);
//      delay(500);
//    }
    delay(1000);
  }
}

void loop() {
//  if (sleeping) {
//    system_sleep();
//  } else {
//    if (millis() > display_timeout) {
//      enter_sleep();
//    } else {
display_mode = 0;
      draw_oled();
//    }
//  }
}

void enter_sleep() {
  oled.fill(0x00); // clear screen to avoid show old time when wake up
  oled.off();
  delay(2); // wait oled stable

  sleeping = true;
}

void wake_up() {
  sleeping = false;

  delay(2); // wait oled stable
  oled.on();

  set_display_timeout();
}


// PIN CHANGE interrupt event function
ISR(PCINT0_vect)
{
 // sleep_disable();

//  if (sleeping) {
//    wake_up();
//  } else {
//    if (digitalRead(SETBUTTON) == LOW) { // SET button pressed
//      selected_field++;
//      if (selected_field > 5) selected_field = 0;
//      if (selected_field > 0) display_mode = 0;
//    } else if (digitalRead(UPBUTTON) == LOW) { // UP button pressed
//      if (selected_field == 0) {
//        display_mode++; // toggle mode;
//        if (display_mode > 1) display_mode = 0;
//      } else {
        int set_year = year();
        int set_month = month();
        int set_day = day();
        int set_hour = hour();
        int set_minute = minute();
  
        if (selected_field == 1) {
          set_year++; // add year
          if (set_year > 2069) set_year = 1970; // loop back
        } else if (selected_field == 2) {
          set_month++; // add month
          if (set_month > 12) set_month = 1; // loop back
        } else if (selected_field == 3) {
          set_day++; // add day
          if (set_day > getMonthDays(CalendarYrToTm(set_year), set_month)) set_day = 1; // loop back
        } else if (selected_field == 4) {
          set_hour++; // add day
          if (set_hour > 23) set_hour = 0; // loop back
        } else if (selected_field == 5) {
          set_minute++; // add day
          if (set_minute > 59) set_minute = 0; // loop back
        }
        setTime(set_hour, set_minute, second(), set_day, set_month, set_year);
//      }
//    }
//  }

//  set_display_timeout(); // extent display timeout while user input
//  sleep_enable();
}
