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

int sendCommand(char * cmd, int len) {
    return port->println(cmd);
}

public:

Optotune(Stream * port) {
    this->port = port;
}

void move(int position) {
    
}

};

#endif