/*
Controller:  Arduino Uno rev3
Author:      Marcelo Furtado Lima
Date:        14 July 2015

Check SynthByte.fzz for a connection diagram
*/

#include "CleanCrystal.h"
#include <DFR_Key.h>

#define DATA_LENGTH 256

#define SPEAKER 11

#define ADD 0
#define AM 1
#define FM 2
#define PM 3

#define OFF 0
#define SQUARE 1
#define SAW 2
#define SINE 3

#define NO_WAVE 255
#define NO_CHANGE 255

prog_uchar SINE_WAVE[] PROGMEM = {128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124};

CleanCrystal lcd;
DFR_Key keypad;

//byte value = 0;
volatile unsigned int finalSignal = 0;
volatile unsigned int tempSignal = 0;
byte signalAdders = 1;

byte waveType[] = {SINE, OFF, OFF, OFF};
byte modulation[] = {ADD, ADD, ADD, ADD};
byte multiplier[] = {1, 1, 1, 1};
byte divider[] = {0, 0, 0, 0};
volatile byte count[] = {0, 0, 0, 0};
volatile byte divCount[] = {0, 0, 0, 0};
volatile byte signal[] = {0, 0, 0, 0};
volatile boolean ampMod = false;

const char* wavName[] = {"Off", "Square", "Saw", "Sine"};
const char modName[] = {'S', 'A', 'F', 'P'};

byte currentKey = 0;
byte lastKey = 0;

byte cursorLine = 0;
boolean cursorBottom = false;
byte lineChange = NO_CHANGE;
byte currentWave = NO_WAVE;
byte temp;
const byte settingsCursor[] = {6, 12, 12, 9};

ISR (TIMER1_COMPA_vect) {
  
  if (signalAdders < 0) {    
    OCR2A = 0;  
  } else {  
    ampMod = false;
    finalSignal = 0;
    
    for (byte i = 3; i < 4; i--) {
      
      if (waveType[i] == OFF) {  
        ampMod = false;
        signal[i] = 0x00;
        continue;
      }
      
      divCount[i] ++;
      if (divCount[i] >= divider[i]) {
        divCount[i] = 0;
        count[i] += multiplier[i];
      }
        
      if (ampMod) {
        switch (waveType[i]) {
          case SQUARE:
            signal[i] = count[i] < 0x80 ? 0x00 : signal[i];
            break;        
          case SAW:
            tempSignal = count[i] * signal[i];// & 0xFF00);// >> 8;
            signal[i] = tempSignal >> 8;
            break;
          case SINE:
            tempSignal = pgm_read_byte_near(SINE_WAVE + count[i]) * signal[i];// & 0xFF00);// >> 8;
            signal[i] = tempSignal >> 8;
            break;
        }
      } else {
        switch (waveType[i]) {
          case SQUARE:
            signal[i] = count[i] < 0x80 ? 0x00 : 0xFF;
            break;        
          case SAW:
            signal[i] = count[i];
            break;
          case SINE:
            signal[i] = pgm_read_byte_near(SINE_WAVE + count[i]);
            break;
        }
      }
      
      switch (modulation[i]) {
        case ADD:
          finalSignal += signal[i];
          ampMod = false;
          break;
        case AM:
          signal[i-1] = signal[i];
          ampMod = true;
          break;
        case FM:
          tempSignal = (signal[i] * multiplier[i-1]);
          count[i-1] = tempSignal >> 8;
          ampMod = false;
          break;
        case PM:
          count[i-1] += (signal[i] - 128) >> 2;
          ampMod = false;
          break;
      }
    }
    
    switch (signalAdders) {
      case 1:
        OCR2A = finalSignal;  
        break;
      case 2:
        OCR2A = finalSignal >> 1;
        break;
      case 3:
        OCR2A = finalSignal / 3;
        break;
      case 4:
        OCR2A = finalSignal >> 2;
        break;
    } 
  }
}

void setup() {
  
  //Configuring output pin
  pinMode(SPEAKER, OUTPUT);
  
  lcd.print("Malynthersizer");
  lcd.setCursor(11, 1);
  lcd.print("v0.01");
  
  delay(2000);

  displayMenu();

  cli();
  
  //SAMPLER Timer
  TCCR1A = 0;
  TCCR1B = 0;
  
  OCR1A = 0;               //Divide frequency by 1
  TCCR1B |= (1 << WGM12);  //Set to compare
  TCCR1B |= (1 << CS12);   //Run at 62.5 KHz (16 MHz / 256)
  TIMSK1 = (1 << OCIE1A);  //Compare and interrupt
  
  //TONE Timer
  TCCR2A = 0;
  TCCR2B = 0;
  
  OCR2A = 0x7F;            //Set to 50% duty cycle
  TCCR2A |= (1 << COM2A1); //Set PWM output to pin 11
  TCCR2A |= (1 << WGM20);  //Set fast PWM mode
  TCCR2A |= (1 << WGM21);  //Set fast PWM mode  
  TCCR2B |= (1 << CS20);   //Run at 16 MHz
  
  sei();	

}

void loop() {  
  if (lineChange != NO_CHANGE) {
    lcd.noBlink();
    
    //Main menu
    if (currentWave == NO_WAVE) {  
      
      //Top line
      lcd.setCursor(0, 0);
      lcd.write(cursorLine + 49);
      
      //Wave
      if (waveType[cursorLine] != waveType[lineChange]) {
        lcd.setCursor(3, 0);
        lcd.print(wavName[waveType[cursorLine]]);
        
        temp = strlen(wavName[waveType[lineChange]]) + 4;
        while (lcd.getCursor() < temp) {
          lcd.writeBlank();
        }
      }
      
      //Modulation
      if (waveType[cursorLine] != waveType[lineChange]) {
        if (waveType[cursorLine]) {
          if (waveType[lineChange]) {        
            lcd.setCursor(13, 0);
            lcd.write(modName[modulation[cursorLine]]);
          } else {
            lcd.setCursor(12, 0);
            lcd.write('(');
            lcd.write(modName[modulation[cursorLine]]);
            lcd.print("M)");
          }         
        } else {
          if (waveType[lineChange]) {      
            lcd.setCursor(12, 0);
            lcd.print("    ");
          } 
        }     
      }
      
      lineChange++;
        
      //Bottom line
      lcd.setCursor(0, 1);
      lcd.write(cursorLine + 50);
      
      //Wave
      temp = cursorLine + 1;
      if (waveType[temp] != waveType[lineChange]) {      
        lcd.setCursor(3, 1);
        lcd.print(wavName[waveType[temp]]);
        
        temp = strlen(wavName[waveType[lineChange]]) + 4;
        while (lcd.getCursor() < temp) {
          lcd.writeBlank();
        }
      }
      
      temp = cursorLine + 1;
      
      //Modulation
      if (waveType[temp] != waveType[lineChange]) {  
        if (waveType[temp]) {
          if (waveType[lineChange]) {        
            lcd.setCursor(13, 1);
            lcd.write(modName[modulation[temp]]);
          } else {
            lcd.setCursor(12, 1);
            lcd.write('(');
            lcd.write(modName[modulation[temp]]);
            lcd.print("M)");
          }         
        } else {
          if (waveType[lineChange]) {      
            lcd.setCursor(12, 1);
            lcd.print("    ");
          } 
        } 
      }
      lcd.setCursor(0, cursorBottom);
      
    //Settings
    } else {      
      lcd.clear();
      switch (cursorLine) {
        case 0:
          lcd.print("Type: ");
          lcd.print(wavName[waveType[currentWave]]);
          lcd.setCursor(0, 1);
          lcd.print("Modulation: ");
          lcd.write(modName[modulation[currentWave]]);
          lcd.write('M');
          break;
          
        case 1:
          lcd.print("Modulation: ");
          lcd.write(modName[modulation[currentWave]]);
          lcd.write('M');
          lcd.setCursor(0, 1);
          lcd.print("Multiplier: ");
          lcd.print(multiplier[currentWave]);
          break;
          
        case 2:
          lcd.print("Multiplier: ");
          lcd.print(multiplier[currentWave]);
          lcd.setCursor(0, 1);
          lcd.print("Divider: ");
          lcd.print(divider[currentWave]);
          break;
      }
      
      lcd.setCursor(settingsCursor[cursorLine + cursorBottom], cursorBottom);
    }    
    
    lineChange = NO_CHANGE;
    lcd.blink();
  }
  
  currentKey = keypad.getKey();
  if (currentKey != lastKey || lastKey == RIGHT_KEY || lastKey == LEFT_KEY) {
    
    if (currentKey == UP_KEY) {
      if (cursorBottom) {
        cursorBottom = false;
        if (currentWave == NO_WAVE) {
          lcd.setCursor(0, cursorBottom);
        } else {
          lcd.setCursor(settingsCursor[cursorLine + cursorBottom], cursorBottom);
        }
      } else if (cursorLine > 0) {
        lineChange = cursorLine;
        cursorLine--;
      }
    } else if (currentKey == DOWN_KEY) {
      if (!cursorBottom) {
        cursorBottom = true;
        if (currentWave == NO_WAVE) {
          lcd.setCursor(0, cursorBottom);
        } else {
          lcd.setCursor(settingsCursor[cursorLine + cursorBottom], cursorBottom);
        }
      } else if (cursorLine < 2) {
        lineChange = cursorLine;
        cursorLine++;
      }
    } else if (currentKey == RIGHT_KEY) {
      if (currentWave != NO_WAVE) {
        switch (cursorLine + cursorBottom) {
          case 0:
            if (waveType[currentWave] == OFF && modulation[currentWave] == ADD) signalAdders++;
            
            waveType[currentWave]++;
            if (waveType[currentWave] > SINE) {
              waveType[currentWave] = OFF;
              if (modulation[currentWave] == ADD) signalAdders--;
            }
            
            lcd.print(wavName[waveType[currentWave]]);
            while (lcd.getCursor() < 12) {
              lcd.writeBlank();
            }
            lcd.setCursor(6, cursorBottom);
            break;
            
          case 1:
            if (currentWave != 0) {
              if (modulation[currentWave] == ADD && waveType[currentWave] != OFF) signalAdders--;
              
              modulation[currentWave]++;
              if (modulation[currentWave] > PM) {
                modulation[currentWave] = ADD;
                if (waveType[currentWave] != OFF) signalAdders++;
              }
            }
            
            lcd.write(modName[modulation[currentWave]]);
            lcd.write('M');
            lcd.setCursor(12, cursorBottom);
            break;
            
          case 2:
            if (temp > 3) {
              multiplier[currentWave] += 5;
            } else {
                if (lastKey == RIGHT_KEY) temp++;
                multiplier[currentWave]++;
            }
            lcd.print(multiplier[currentWave]);
            lcd.setCursor(12, cursorBottom);
            break;
            
          case 3:
            if (temp > 3) {
              multiplier[currentWave] += 5;
            } else {
              if (lastKey == RIGHT_KEY) temp++;
              divider[currentWave]++;
            }
            lcd.print(divider[currentWave]);
            lcd.setCursor(9, cursorBottom);
            break;
        }
      }
    } else if (currentKey == LEFT_KEY) {
      if (currentWave != NO_WAVE) {
        switch (cursorLine + cursorBottom) {
          case 0:
            if (waveType[currentWave] == OFF && modulation[currentWave] == ADD) signalAdders++;
            
            waveType[currentWave]--;
            if (waveType[currentWave] > SINE) {
              waveType[currentWave] = SINE;
              if (modulation[currentWave] == ADD) signalAdders--;
            }
            
            lcd.print(wavName[waveType[currentWave]]);
            while (lcd.getCursor() < 12) {
              lcd.writeBlank();
            }
            lcd.setCursor(6, cursorBottom);
            break;
            
          case 1:
            if (currentWave != 0) {
              if (modulation[currentWave] == ADD && waveType[currentWave] != OFF) signalAdders--;
              
              modulation[currentWave]--;
              if (modulation[currentWave] > PM) {
                modulation[currentWave] = PM;
                if (waveType[currentWave] != OFF) signalAdders++;
              }
            }
            
            lcd.write(modName[modulation[currentWave]]);
            lcd.write('M');
            lcd.setCursor(12, cursorBottom);
            break;
            
          case 2:
            if (temp > 3) {
              multiplier[currentWave] -= 5;
            } else {
              if (lastKey == LEFT_KEY) temp++;
              multiplier[currentWave]--;
            }
            lcd.print(multiplier[currentWave]);
            lcd.setCursor(12, cursorBottom);
            break;
            
          case 3:
            if (temp > 3) {
              multiplier[currentWave] -= 5;
            } else {
              if (lastKey == LEFT_KEY) temp++;
              divider[currentWave]--;
            }
            lcd.print(divider[currentWave]);
            lcd.setCursor(9, cursorBottom);
            break;
        }
      }
    } else if (currentKey == SELECT_KEY) {
      if (currentWave == NO_WAVE) {
        currentWave = cursorLine + cursorBottom;
        lineChange = true;
        cursorLine = 0;
        cursorBottom = false;
      } else {
        currentWave = NO_WAVE;
        displayMenu();
      }
    } else {
      temp = 0;
    }
    
    lastKey = currentKey;
  }  
  
}

inline void displayMenu() {
  lcd.clear();
  lcd.noBlink(); 
  
  lcd.setCursor(0, 0);
  lcd.print("1:");
  lcd.setCursor(3, 0);
  lcd.print(wavName[waveType[0]]);
  if (waveType[0]) {
    lcd.setCursor(12, 0);
    lcd.write('(');
    lcd.write(modName[modulation[0]]);
    lcd.print("M)");
  }  
  
  lcd.setCursor(0, 1);
  lcd.print("2:");
  lcd.setCursor(3, 1);
  lcd.print(wavName[waveType[1]]);
  if (waveType[1]) {   
    lcd.setCursor(12, 1); 
    lcd.write('(');
    lcd.write(modName[modulation[1]]);
    lcd.print("M)");
  }
  
  lcd.setCursor(0, 0);
  lcd.blink();  
  
  cursorLine = 0;
  cursorBottom = false;
  lineChange = NO_CHANGE;
}
