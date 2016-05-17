#include "CleanCrystal.h"
#include "Arduino.h"

CleanCrystal::CleanCrystal() {
  init();
}

void CleanCrystal::init() {
  pinMode(LCD_PIN_RESET, OUTPUT);
  pinMode(LCD_PIN_ENABLE, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
    
  begin();  
}

void CleanCrystal::begin() {
  delayMicroseconds(50000);
  
  digitalWrite(LCD_PIN_RESET, LOW);
  digitalWrite(LCD_PIN_ENABLE, LOW);
  
  write4bits(0x03);
  delayMicroseconds(4500); // wait min 4.1ms
  write4bits(0x03);
  delayMicroseconds(4500); // wait min 4.1ms
  write4bits(0x03); 
  delayMicroseconds(150);
  write4bits(0x02); 

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

}

/********** high level commands, for the user! */
void CleanCrystal::clear() {
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);
}

void CleanCrystal::home() {
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);
}

void CleanCrystal::setCursor(char col, char row) {
  if ( row > 1 ) {
    row = 1;    // we count rows starting w/0
  }
  
  command(LCD_SETDDRAMADDR | (col + row * 0x40));
  
  _cursorPos = col;
}

char CleanCrystal::getCursor() {
  return _cursorPos;
}

// Turn the display on/off (quickly)
void CleanCrystal::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CleanCrystal::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void CleanCrystal::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CleanCrystal::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void CleanCrystal::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void CleanCrystal::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void CleanCrystal::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void CleanCrystal::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void CleanCrystal::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void CleanCrystal::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void CleanCrystal::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void CleanCrystal::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void CleanCrystal::createChar(char location, char charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

void CleanCrystal::createChar(char location, const char charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

void CleanCrystal::command(char value) {
  digitalWrite(LCD_PIN_RESET, LOW);
  write4bits(value>>4);
  write4bits(value);
}

void CleanCrystal::write(char value) {
  digitalWrite(LCD_PIN_RESET, HIGH);  
  write4bits(value>>4);
  write4bits(value);
  _cursorPos++;
}

void CleanCrystal::writeBlank() {
  digitalWrite(LCD_PIN_RESET, HIGH);  
  write4bits(' '>>4);
  write4bits(' ');
  _cursorPos++;
}
  
void CleanCrystal::print(const char * value) {
  digitalWrite(LCD_PIN_RESET, HIGH);
  for (_bufferPos = 0; _bufferPos < 16; _bufferPos++) {
    if (value[_bufferPos] == '\0') return;
    write4bits(value[_bufferPos]>>4);
    write4bits(value[_bufferPos]);
    _cursorPos++;
  }
}

void CleanCrystal::print(int value, char precede) {
  digitalWrite(LCD_PIN_RESET, HIGH);  
  _bufferPos = (value / 10000);
  if (_bufferPos || (precede > 3)) {
	_bufferPos += '0';
    write4bits(_bufferPos>>4);
    write4bits(_bufferPos);
    _cursorPos++;
  }
  
  value %= 10000;
  _bufferPos = (value / 1000);
  if (_bufferPos || (precede > 2)) {
	_bufferPos += '0';
    write4bits(_bufferPos>>4);
    write4bits(_bufferPos);
    _cursorPos++;
  }
  
  value %= 1000;
  _bufferPos = (value / 100);
  if (_bufferPos || (precede > 1)) {
	_bufferPos += '0';
    write4bits(_bufferPos>>4);
    write4bits(_bufferPos);
    _cursorPos++;
  }
  
  value %= 100;
  _bufferPos = (value / 10);
  if (_bufferPos || (precede > 0)) {
	_bufferPos += '0';
    write4bits(_bufferPos>>4);
    write4bits(_bufferPos);
    _cursorPos++;
  }
  
  value %= 10;
  value += '0';
  write4bits(value>>4);
  write4bits(value);
  _cursorPos++;
}

void CleanCrystal::print(char value, char precede) {
  digitalWrite(LCD_PIN_RESET, HIGH);  
  _bufferPos = (value / 100);
  if (_bufferPos || (precede > 1)) {
	_bufferPos += '0';
    write4bits(_bufferPos>>4);
    write4bits(_bufferPos);
    _cursorPos++;
  }
  
  value %= 100;
  _bufferPos = (value / 10);
  if (_bufferPos || (precede > 0)) {
	_bufferPos += '0';
    write4bits(_bufferPos>>4);
    write4bits(_bufferPos);
    _cursorPos++;
  }
  
  value %= 10;
  value += '0';
  write4bits(value>>4);
  write4bits(value);
  _cursorPos++;
}

/************ low level data pushing commands **********/

void CleanCrystal::pulseEnable(void) {
  digitalWrite(LCD_PIN_ENABLE, LOW);
  delayMicroseconds(1);    
  digitalWrite(LCD_PIN_ENABLE, HIGH);
  delayMicroseconds(1);    // enable pulse must be >450ns
  digitalWrite(LCD_PIN_ENABLE, LOW);
  delayMicroseconds(100);   // commands need > 37us to settle
}

void CleanCrystal::write4bits(uint8_t value) {
  for (int i = 4; i < 8; i++) {
    digitalWrite(i, (value >> i - 4) & 0x01);
  }
  pulseEnable();
}
