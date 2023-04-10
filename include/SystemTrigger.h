#ifndef _SYSTEMTRIGGER

#define _SYSTEMTRIGGER

#include <Adafruit_ZeroTimer.h>

#define FLASH_DELAY_OFFSET 3
#define MIN_FLASH_DURATION 1

// Flash Triggers
Adafruit_ZeroTimer flashTimer = Adafruit_ZeroTimer(3);
void flashCallback();

//define the interrupt handlers
void TC3_Handler(){
  Adafruit_ZeroTimer::timerHandler(3);
}

void TC4_Handler(){
  Adafruit_ZeroTimer::timerHandler(4);
}

void TC5_Handler(){
  Adafruit_ZeroTimer::timerHandler(5);
}

void configTimer(float freq, uint16_t * divider, uint16_t * compare, tc_clock_prescaler * prescaler) {
       // Set up the flexible divider/compare
    //uint8_t divider  = 1;
    //uint16_t compare = 0;
    *prescaler = TC_CLOCK_PRESCALER_DIV1;

    if ((freq < 24000000) && (freq > 800)) {
        *divider = 1;
        *prescaler = TC_CLOCK_PRESCALER_DIV1;
        *compare = 48000000/freq;
    } else if (freq > 400) {
        *divider = 2;
        *prescaler = TC_CLOCK_PRESCALER_DIV2;
        *compare = (48000000/2)/freq;
    } else if (freq > 200) {
        *divider = 4;
        *prescaler = TC_CLOCK_PRESCALER_DIV4;
        *compare = (48000000/4)/freq;
    } else if (freq > 100) {
        *divider = 8;
        *prescaler = TC_CLOCK_PRESCALER_DIV8;
        *compare = (48000000/8)/freq;
    } else if (freq > 50) {
        *divider = 16;
        *prescaler = TC_CLOCK_PRESCALER_DIV16;
        *compare = (48000000/16)/freq;
    } else if (freq > 12) {
        *divider = 64;
        *prescaler = TC_CLOCK_PRESCALER_DIV64;
        *compare = (48000000/64)/freq;
    } else if (freq > 3) {
        *divider = 256;
        *prescaler = TC_CLOCK_PRESCALER_DIV256;
        *compare = (48000000/256)/freq;
    } else if (freq >= 0.75) {
        *divider = 1024;
        *prescaler = TC_CLOCK_PRESCALER_DIV1024;
        *compare = (48000000/1024)/freq;
    } else {
        DEBUGPORT.println("Invalid frequency");
    }
    DEBUGPORT.print("Divider:"); Serial.println(*divider);
    DEBUGPORT.print("Compare:"); Serial.println(*compare);
    DEBUGPORT.print("Final freq:"); Serial.println((int)(48000000/(*compare)));
}

void configTriggers(float freq) {

    Serial.println("Trigger Configuration");

    Serial.print("Desired freq (Hz):");
    Serial.println(freq);

    uint16_t divider  = 1;
    uint16_t compare = 0;
    tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV1;

    configTimer(freq, &divider, &compare, &prescaler);

    flashTimer.enable(false);
    flashTimer.configure(prescaler,       // prescaler
            TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
            TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
            );

    flashTimer.setCompare(0, compare);
    flashTimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, flashCallback);
    flashTimer.enable(true);

}

void doFlash(int flashType = WHITE_FLASH_TRIG, int triggerWidth = 1000, int flashDuration = 100) {
    digitalWrite(CAMERA_TRIG,HIGH);
    delayMicroseconds(triggerWidth/2);
    digitalWrite(flashType,HIGH);
    if (flashDuration-FLASH_DELAY_OFFSET >= MIN_FLASH_DURATION)
        delayMicroseconds(flashDuration-FLASH_DELAY_OFFSET);
    digitalWrite(flashType,LOW);
    delayMicroseconds(triggerWidth/2);
    digitalWrite(CAMERA_TRIG,LOW);
}

#endif



