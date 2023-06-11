#ifndef _OPTOTUNE

#define _OPTOTUNE

#include <Arduino.h>
#include <RTCZero.h>
#include <RTCLib.h>
#include <WDTZero.h>
#include "Config.h"

#define MIN_FP -2.0
#define MAX_FP 3.0

class Optotune
{

private:

Stream * port;
char buffer[64];
float position;

public:

Optotune() {

}

int sendCommand(char * cmd) {
    port->flush();
    port->print(cmd);
    port->print("\r\n");
    port->flush();
}

void setPort(Stream * port) {
    this->port = port;
    // send the start command to the etl
    delay(250);
    sendCommand("start");
    delay(250);
    sendCommand("setfp=0.0");
    position = 0.0;

}

void move(float newPosition, float inc = 0.05) {

    char buffer[32];

    if (newPosition < position) {
        while (position > MIN_FP && position > newPosition) { 
            position -= inc;
            sprintf(buffer,"setfp=%0.3f",position);
            sendCommand(buffer);
            //wait for reply
            while (!port->available()) {};
            while (port->available()) {
                DEBUGPORT.write(port->read());
            };
            delay(20);

        }
    }

    if (newPosition > position) {
        while (position < MAX_FP && position < newPosition) { 
            position += inc;
            sprintf(buffer,"setfp=%0.3f",position);
            sendCommand(buffer);
            //wait for reply
            while (!port->available()) {};
            while (port->available()) {
                DEBUGPORT.write(port->read());
            };
            delay(20);
            
        }
    }
    
}

void step(float inc) {

    char buffer[32];

    float newPosition = position + inc;
    if (newPosition > MIN_FP && newPosition < MAX_FP) {
        position += inc;
        sprintf(buffer,"setfp=%0.3f",position);
        sendCommand(buffer);
        //wait for reply
        while (!port->available()) {};
        while (port->available()) {
                DEBUGPORT.write(port->read());
        };
        delay(20);
    }
    
}

void focalSweep() {

    char buffer[32];
    sprintf(buffer,"setfp=%0.3f",MAX_FP);
    sendCommand(buffer);
    position = 3.0;
    while (position > -2.0) { 
        position -= 0.05;
        sprintf(buffer,"setfp=%0.3f",position);
        sendCommand(buffer);
        //wait for reply
        while (!port->available()) {};
        delay(50);
            
    }
}


};

#endif