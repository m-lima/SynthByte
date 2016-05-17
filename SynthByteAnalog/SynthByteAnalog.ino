/*
Controller:  Arduino Uno rev3
Author:      Marcelo Furtado Lima
Date:        14 July 2015

Check SynthByte.fzz for a connection diagram
*/

#include <avr/pgmspace.h>

#define DATA_LENGTH 256

#define SPEAKER 11

#define R_LED_1 10
#define R_LED_2 9
#define Y_LED_1 6
#define Y_LED_2 5
#define G_LED_1 3
#define G_LED_2 2

#define BTN_1 8
#define BTN_2 7
#define BTN_3 4
#define BTN_4 12

#define ADD 0
#define AM 1
#define FM 2
#define PM 3

#define OFF 0
#define SQUARE 1
#define SAW 2
#define SINE 3

prog_uchar SINE_WAVE[] PROGMEM = {128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124};

//byte value = 0;
volatile unsigned int finalSignal = 0;
volatile unsigned int tempSignal = 0;
byte signalAdders = 1;

boolean btnPressed1 = false;
boolean btnPressed2 = false;
boolean btnPressed3 = false;

byte currentWave = 0;
byte waveType[] = {SINE, OFF, OFF, OFF};
byte modulation[] = {ADD, ADD, ADD, ADD};
byte multiplier[] = {1, 1, 1, 1};
byte divider[] = {0, 0, 0, 0};
volatile byte count[] = {0, 0, 0, 0};
volatile byte divCount[] = {0, 0, 0, 0};
volatile byte signal[] = {0, 0, 0, 0};
volatile boolean ampMod = false;

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
  
  //Configuring output pins
  pinMode(SPEAKER, OUTPUT);
  pinMode(R_LED_1, OUTPUT);
  pinMode(R_LED_2, OUTPUT);
  pinMode(Y_LED_1, OUTPUT);
  pinMode(Y_LED_2, OUTPUT);
  pinMode(G_LED_1, OUTPUT);
  pinMode(G_LED_2, OUTPUT);
  
  //Configuring input pins...
  pinMode(BTN_1, INPUT);
  pinMode(BTN_2, INPUT);  
  pinMode(BTN_3, INPUT);  
  pinMode(BTN_4, INPUT);  

  //...with pull-up resistors
  digitalWrite(BTN_1, HIGH);
  digitalWrite(BTN_2, HIGH);
  digitalWrite(BTN_3, HIGH);
  digitalWrite(BTN_4, HIGH);
      
  digitalWrite(R_LED_1, waveType[currentWave] & 1);
  digitalWrite(R_LED_2, (waveType[currentWave] >> 1) & 1);
      
  digitalWrite(Y_LED_1, modulation[currentWave] & 1);
  digitalWrite(Y_LED_2, (modulation[currentWave] >> 1) & 1);
      
  digitalWrite(G_LED_1, currentWave & 1);
  digitalWrite(G_LED_2, (currentWave >> 1) & 1);
  
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
  
  if (!digitalRead(BTN_1)) {
    if (!btnPressed1) {
      btnPressed1 = true;
      
      if (waveType[currentWave] == OFF && modulation[currentWave] == ADD) signalAdders++;
      
      waveType[currentWave]++;
      if (waveType[currentWave] > SINE) {
        waveType[currentWave] = OFF;
        if (modulation[currentWave] == ADD) signalAdders--;
      }
      
      digitalWrite(R_LED_1, waveType[currentWave] & 1);
      digitalWrite(R_LED_2, (waveType[currentWave] >> 1) & 1);
      
      delay(100);
    }
  } else {
    btnPressed1 = false;
  }
  
  if (!digitalRead(BTN_2)) {
    if (!btnPressed2) {
      btnPressed2 = true;
      
      if (currentWave == 0) return;
      
      if (modulation[currentWave] == ADD && waveType[currentWave] != OFF) signalAdders--;
      
      modulation[currentWave]++;
      if (modulation[currentWave] > PM) {
        modulation[currentWave] = ADD;
        if (waveType[currentWave] != OFF) signalAdders++;
      }
      
      digitalWrite(Y_LED_1, modulation[currentWave] & 1);
      digitalWrite(Y_LED_2, (modulation[currentWave] >> 1) & 1);   
      
      delay(150);   
    }
  } else {
    btnPressed2 = false;
  }
  
  if (!digitalRead(BTN_3)) {
    if (!btnPressed3) {
      btnPressed3 = true;
      
      currentWave++;
      if (currentWave > 3) {
        currentWave = 0;
      }
      
      digitalWrite(G_LED_1, currentWave & 1);
      digitalWrite(G_LED_2, (currentWave >> 1) & 1);
      
      digitalWrite(Y_LED_1, modulation[currentWave] & 1);
      digitalWrite(Y_LED_2, (modulation[currentWave] >> 1) & 1);
      
      digitalWrite(R_LED_1, waveType[currentWave] & 1);
      digitalWrite(R_LED_2, (waveType[currentWave] >> 1) & 1);
      
      delay(100);
    }
  } else {
    btnPressed3 = false;
  }
  
  if (!digitalRead(BTN_4)) {      
    multiplier[currentWave] = (analogRead(A5) >> 4) + 1;
    divider[currentWave] = analogRead(A4) >> 4;
  }
}
