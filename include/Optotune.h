#ifndef _OPTOTUNE

#define _OPTOTUNE

#include <Arduino.h>
#include <RTCZero.h>
#include <RTCLib.h>
#include <WDTZero.h>
#include "Config.h"

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
    port->print(cmd);
    port->print("\r\n");
}

void setPort(Stream * port) {
    this->port = port;
    // send the start command to the etl
    sendCommand("start");
    delay(25);
    sendCommand("setfp=0.0");
    position = 0.0;

}

void stepLens(float pos) {

    if (pos >= -2.0 && pos <= 3.0) {
        char buffer[32];
        sprintf(buffer,"setfp=%0.3f",pos);
        sendCommand(buffer);
    }
}

void move(float newPosition, float inc = 0.05) {

    char buffer[32];

    if (newPosition < position) {
        while (position > newPosition) { 
            position -= inc;
            sprintf(buffer,"setfp=%0.3f",position);
            sendCommand(buffer);
            //wait for reply
            while (!port->available()) {};

        }
    }

    if (newPosition > position) {
        while (position < newPosition) { 
            position += inc;
            sprintf(buffer,"setfp=%0.3f",position);
            sendCommand(buffer);
            //wait for reply
            while (!port->available()) {};
            
        }
    }
    
}

};

#endif