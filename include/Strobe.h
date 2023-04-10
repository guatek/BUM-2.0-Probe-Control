/** @file strobe.h
 *  @brief Functions for controller triggering flashes and camera
 *  @author pldr
 *  @copyright Scripps Institution of Oceanography
 */

#ifndef _STROBE

#define _STROBE

#include <Arduino.h>
#include "Config.h"

#define MAX_FLASH 20000 /**< The longest flash duration supported for any flash type */
#define MIN_FLASH 50    /**< The shorted flash duration supported for any flash type */ 


/** 
 * Record an ambient light image
 *
 * @param dur The duration of the delay while recording
 */
void recordAmbient(int dur) {
	digitalWrite(CAMERA_TRIG,1);
	delayMicroseconds(300);
    delayMicroseconds(dur);
    digitalWrite(CAMERA_TRIG,0);
}

/** 
 * Record an white LED image
 *
 * @param dur The duration of LED flash while recording
 */
void recordWhite(int dur) {
    digitalWrite(CAMERA_TRIG,1);
    delayMicroseconds(300);
    digitalWrite(WHITE_FLASH_TRIG,1);
    delayMicroseconds(dur);
    digitalWrite(WHITE_FLASH_TRIG,0);
    digitalWrite(CAMERA_TRIG,0);
}

/** 
 * Record an measuring blue LED image
 *
 * @param dur The duration of LED flash while recording
 */
void recordUV(int dur, int power) {
    digitalWrite(CAMERA_TRIG,1);
    delayMicroseconds(300);
    digitalWrite(UV_FLASH_TRIG,1);
    delayMicroseconds(dur);
    digitalWrite(UV_FLASH_TRIG,0);
    digitalWrite(CAMERA_TRIG,0);
}





#endif