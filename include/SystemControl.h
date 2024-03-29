#ifndef _SYSTEMCONTROL

#define _SYSTEMCONTROL

#include <Arduino.h>
#include <RTCZero.h>
#include <RTCLib.h>
#include <WDTZero.h>
#include "Config.h"
#include "DeepSleep.h"
#include "SPIFlash.h"
#include "Sensors.h"
#include "Stats.h"
#include "SystemConfig.h"
#include "SystemTrigger.h"
#include "RBRInstrument.h"
#include "SBE39.h"
#include "Utils.h"
#include "Optotune.h"
#include "Sequence.h"

#define CMD_CHAR '!'
#define SET_CHAR '#'
#define PROMPT "PCTL > "
#define LOG_PROMPT "$PCTL"
#define CMD_BUFFER_SIZE 128

#define STROBE_POWER 7
#define CAMERA_POWER 6

// Global Sensors
Sensors _sensors;

// Global RTCZero
RTCZero _zerortc;

//Global RTCLib
RTC_DS3231 _ds3231;

// Global watchdog timer with 8 second hardware timeout
WDTZero _watchdog;

// RBR instrument
RBRInstrument _rbr;

// SBE39 CTD
SBE39 _sbe39;

// Optotune lens
Optotune _etl;

// Sequence processors
Sequence _seq[MAX_MACROS];

class SystemControl
{
    private:
    float lastDepth;
    float currentDepth;
    bool systemOkay;
    bool ds3231Okay;
    bool cameraOn;
    bool pendingPowerOff;
    bool pendingPowerOn;
    bool lowVoltage;
    bool badEnv;
    char cmdBuffer[CMD_BUFFER_SIZE];
    bool rbrData;
    int state;
    unsigned long timestamp;
    unsigned long lastDepthCheck;
    unsigned long startupTimer;
    unsigned long lastPowerOnTime;
    unsigned long lastPowerOffTime;
    unsigned long pendingPowerOffTimer;
    unsigned long pendingPowerOnTimer;
    unsigned long clockSyncTimer;
    unsigned long envTimer;
    unsigned long voltageTimer;

    unsigned long imageCounter;

    int lastFlashType;

    MovingAverage<float> avgVoltage;
    MovingAverage<float> avgTemp;
    MovingAverage<float> avgHum;
    MovingAverage<float> avgDepth;
    
    void readInput(Stream *in) {
      
        if (in != NULL && in->available() > 0) {
            char c = in->read();

            if (c == SET_CHAR) {
                unsigned long startTimer = millis();
                int index = 0;
                while (startTimer <= millis() && millis() - startTimer < (unsigned int)(cfg.getInt("CMDTIMEOUT"))) {

                    if (cfg.getInt(WATCHDOG) > 0) {
                        _watchdog.clear();
                    }

                    // Break if we have exceed the buffer size
                    if (index >= CMD_BUFFER_SIZE)
                        break;

                    // Wait on user input
                    if (in->available()) {
                        // Read the next char and reset timer          
                        c = in->read();
                        startTimer = millis();
                    }
                    else {
                        continue;
                    }

                    // Exit command loop on repeat command char
                    if (c == CMD_CHAR) {
                        break;
                    }
                    
                    if (c == '\r') {
                        // Command ended try to 
                        if (index <= CMD_BUFFER_SIZE)
                            cmdBuffer[index++] = '\0';
                        else
                            cmdBuffer[CMD_BUFFER_SIZE-1] = '\0';
                        
                        // Parse Command menus
                        char * rest;
                        char * cmd = strtok_r(cmdBuffer,",",&rest);

                        // CFG (configuration commands)
                        if (cmd != NULL && strncmp_ci(cmd,SET, 3) == 0) {
                            if (rest != NULL) {
                                cfg.parseConfigCommand(rest, in);
                            }
                            else {
                                char timeString[64];
                                getTimeString(timeString);
                                cfg.printConfig(in, timeString);

                            }
                            
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,STEPLENS,8) == 0) {
                            float num;
                            sscanf(rest,"%f",&num);
                            if (num >= -2.0 && num < 3.0) {
                                _etl.step(num);
                            }
                            
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,MOVELENS,8) == 0) {
                            float num, inc;
                            sscanf(rest,"%f,%f",&num,&inc);
                            if (num >= -2.0 && num < 3.0) {
                                _etl.move(num, inc);
                            }
                            
                        }

                        // Break from loop in set command since there is no CLI
                        break;
                    }
                    // Handle backspace
                    if (c == '\b') {
                        index -= 1;
                        if (index < 0) {
                            index = 0;
                        }
                        else if ( index >= 0 && cfg.getInt(LOCALECHO)) {
                           in->write("\b \b");
                        }
                    }
                    else {
                        cmdBuffer[index++] = c;
                        if (cfg.getInt(LOCALECHO))
                            in->write(c);
                    }
                }
            }

            if (c == CMD_CHAR) {

                // Don't echo the command char
                //if (cfg.getInt("LOCALECHO"))
                //    in->write(c);
              
                // Print the prompt
                in->write(PROMPT);


                unsigned long startTimer = millis();
                int index = 0;
                while (startTimer <= millis() && millis() - startTimer < (unsigned int)(cfg.getInt("CMDTIMEOUT"))) {

                    if (cfg.getInt(WATCHDOG) > 0) {
                        _watchdog.clear();
                    }

                    // Break if we have exceed the buffer size
                    if (index >= CMD_BUFFER_SIZE)
                        break;

                    // Wait on user input
                    if (in->available()) {
                        // Read the next char and reset timer          
                        c = in->read();
                        startTimer = millis();
                    }
                    else {
                        continue;
                    }

                    // Exit command loop on repeat command char
                    if (c == CMD_CHAR) {
                        break;
                    }
                    
                    if (c == '\r') {
                        // Command ended try to 
                        if (index <= CMD_BUFFER_SIZE)
                            cmdBuffer[index++] = '\0';
                        else
                            cmdBuffer[CMD_BUFFER_SIZE-1] = '\0';
                        
                        // Parse Command menus
                        char * rest;
                        char * cmd = strtok_r(cmdBuffer,",",&rest);

                        // CFG (configuration commands)
                        if (cmd != NULL && strncmp_ci(cmd,CFG, 3) == 0) {
                            if (rest != NULL) {
                                cfg.parseConfigCommand(rest, in);
                            }
                            else {
                                char timeString[64];
                                getTimeString(timeString);
                                cfg.printConfig(in, timeString);

                            }
                            
                        }

                        // PORTPASS (pass through to other serial ports)
                        if (cmd != NULL && strncmp_ci(cmd,PORTPASS, 8) == 0) {
                            doPortPass(in, rest);
                        }

                        //LOADSEQ,num
                        else if (cmd != NULL && strncmp_ci(cmd,LOADSEQ, 7) == 0) {
                            int num;
                            sscanf(rest,"%d",&num);
                            in->print("\n");
                            if (num >= 0 && num < MAX_MACROS) {
                                _seq[num].load_sequence(in);
                            }
                        }

                        //RUNSEQ,num
                        else if (cmd != NULL && strncmp_ci(cmd,RUNSEQ, 6) == 0) {
                            int num;
                            sscanf(rest,"%d",&num);
                            if (num >= 0 && num < MAX_MACROS) {
                                _seq[num].run_sequence(0,_seq[num].getIdx());
                            }
                        }

                        // SETTIME (set time from string)
                        else if (cmd != NULL && strncmp_ci(cmd,SETTIME, 7) == 0) {
                            setTime(rest, in);
                        }

                        // WRITECONFIG (save the current config to EEPROM)
                        else if (cmd != NULL && strncmp_ci(cmd,WRITECONFIG, 11) == 0) {
                            writeConfig();
                        }

                        // READCONFIG (read the current config to EEPROM)
                        else if (cmd != NULL && strncmp_ci(cmd,READCONFIG, 10) == 0) {
                            readConfig();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,CAMERAON,8) == 0) {
                            if (confirm(in, "Are you sure you want to power ON camera ? [y/N]: ", cfg.getInt(CMDTIMEOUT)))
                                turnOnCamera();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,CAMERAOFF,9) == 0) {
                            if (confirm(in, "Are you sure you want to power OFF camera ? [y/N]: ", cfg.getInt(CMDTIMEOUT)))
                                turnOffCamera();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,TESTFLASH,9) == 0) {
                            testFlash();
                        } 

                        else if (cmd != NULL && strncmp_ci(cmd,GOTOSLEEP,9) == 0) {
                            goToSleep();
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,OPTOTUNE,8) == 0) {
                            _etl.sendCommand(rest);
                        }

						else if (cmd != NULL && strncmp_ci(cmd,FOCALSWEEP,8) == 0) {
                            _etl.focalSweep();
                        }
                        else if (cmd != NULL && strncmp_ci(cmd,MOVELENS,8) == 0) {
                            float num;
                            sscanf(rest,"%f",&num);
                            if (num >= -2.0 && num < 3.0) {
                                _etl.move(num, 0.05);
                            }
                            
                        }
						
						else if (cmd != NULL && strncmp_ci(cmd,STEPLENS,8) == 0) {
                            float num;
                            sscanf(rest,"%f",&num);
                            if (num >= -2.0 && num < 3.0) {
                                _etl.step(num);
                            }
                            
                        }

                        else if (cmd != NULL && strncmp_ci(cmd,"resetopto",9) == 0) {
                            sendBreak();
                        }
                        // Reset the buffer and print out the prompt
                        if (c == '\n')
                            in->write('\r');
                        else
                            in->write("\r\n");

                        in->write(PROMPT);

                        index = 0;                       
                        startTimer = millis();
                        continue;
                    }
                    
                    // Handle backspace
                    if (c == '\b') {
                        index -= 1;
                        if (index < 0) {
                            index = 0;
                        }
                        else if ( index >= 0 && cfg.getInt(LOCALECHO)) {
                           in->write("\b \b");
                        }
                    }
                    else {
                        cmdBuffer[index++] = c;
                        if (cfg.getInt(LOCALECHO))
                            in->write(c);
                    }
                }
            }
        }
    }

    void doPortPass(Stream * in, char * cmd) {
        char * rest;
        char * num = strtok_r(cmd,",",&rest);
        in->print("Passing through to hardware port ");
        in->println(num);
        in->println();
        if (num != NULL) {
            char portNum = *num;
            switch (portNum) {
                case '0':
                    portpass(in, &HWPORT0, cfg.getInt(LOCALECHO) == 1);
                    break;
                case '1':
                    portpass(in, &HWPORT1, cfg.getInt(LOCALECHO) == 1);
                    break;
                case '2':
                    portpass(in, &HWPORT2, cfg.getInt(LOCALECHO) == 1);
                    break;
                case '3':
                    portpass(in, &HWPORT3, cfg.getInt(LOCALECHO) == 1);
                    break;
            }
        }
    }

    void setTime(char * timeString, Stream * ui) {
        if (timeString != NULL) {
            // if we have ds3231 set that first
            DateTime dt(timeString);
            if (dt.isValid()) {
                ui->println("\nUpdating clock...\n");
                if (ds3231Okay) {
                    _ds3231.adjust(dt.unixtime());
                }
                _zerortc.setEpoch(dt.unixtime());
            }
        }
    }
                      
 
    public:

    SystemConfig cfg;
    int trigWidth;
    int lastStrobeDuration;
    int flashType;
    int frameRate;
  
    SystemControl() {
        systemOkay = false;
        rbrData = false;
        state = 0;
        timestamp = 0;
        ds3231Okay = false;
        pendingPowerOff = false;
        cameraOn = false;
        lowVoltage = false;
        badEnv = false;
        imageCounter = 0;
    }

    void configurePins() {
        //Turn off strobe and camera power
        pinMode(CAMERA_POWER, OUTPUT);
        pinMode(STROBE_POWER, OUTPUT);
        pinMode(LED1_EN, OUTPUT);
        pinMode(LED2_EN, OUTPUT);
        pinMode(WHITE_FLASH_TRIG, OUTPUT);
        pinMode(UV_FLASH_TRIG, OUTPUT);
        pinMode(CAMERA_TRIG, OUTPUT);

        digitalWrite(LED1_EN, LOW);
        digitalWrite(LED2_EN, LOW);
        digitalWrite(WHITE_FLASH_TRIG, LOW);
        digitalWrite(UV_FLASH_TRIG, LOW);
        digitalWrite(CAMERA_TRIG, LOW);
    }
    
	void sendBreak() {
	        HWPORT3.end();
	        delay(1000);
	        pinMode(10,INPUT);
	        digitalWrite(10,HIGH);
	        pinMode(12,INPUT);
	        digitalWrite(12,HIGH);
	        delay(1000);
	        HWPORT3.begin(cfg.getInt(HWPORT3BAUD));
	        pinPeripheral(12, PIO_SERCOM);
	        pinPeripheral(10, PIO_SERCOM);
    }

    bool begin() {

        // Start RTC
        _zerortc.begin();

        // Start DS3231
        ds3231Okay = true;
        if (!_ds3231.begin()) {
            DEBUGPORT.println("Could not init DS3231, time will be lost on power cycle.");
            ds3231Okay = false;
        }
        else {
            // sync rtczero to DS3231
            _zerortc.setEpoch(_ds3231.now().unixtime());
        }

        // set the startup timer
        startupTimer = _zerortc.getEpoch();
        lastPowerOffTime = _zerortc.getEpoch();
        lastPowerOnTime = _zerortc.getEpoch();
        lastDepthCheck = _zerortc.getEpoch();
        voltageTimer = _zerortc.getEpoch();
        envTimer = _zerortc.getEpoch();
        clockSyncTimer = _zerortc.getEpoch();

        lastDepth = -10.0;

        systemOkay = true;
        if (_flash.initialize()) {
            DEBUGPORT.println("Flash Init OK.");
        }
            
        else {
            DEBUGPORT.print("Init FAIL, expectedDeviceID(0x");
            DEBUGPORT.print(_expectedDeviceID, HEX);
            DEBUGPORT.print(") mismatched the read value: 0x");
            DEBUGPORT.println(_flash.readDeviceId(), HEX);
        }

        // Start sensors
        _sensors.begin();

        // Initialize sequences
        for (int i = 0; i < MAX_MACROS; i++) {
            _seq[i].init(&cfg, &_etl);
        }
            
        return true;

    }

    void configWatchdog() {
        // enable hardware watchdog if requested
        if (cfg.getInt(WATCHDOG) > 0) {
            _watchdog.setup(WDT_HARDCYCLE8S);
        }
    }

    void clearWatchdog() {
        // clear watchdog timer if requested
        if (cfg.getInt(WATCHDOG) > 0) {
            _watchdog.clear();
        }
    }

    bool turnOnCamera() {
        if (_zerortc.getEpoch() - lastPowerOffTime > (unsigned int)cfg.getInt(CAMGUARD) && !cameraOn) {
            DEBUGPORT.println("Turning ON camera power...");
            cameraOn = true;
            digitalWrite(LED1_EN, HIGH);
            digitalWrite(LED2_EN, HIGH);

            delay(1000);

            // Setup ETL after opening hardware serial port
            _etl.setPort(&HWPORT3);

            lastPowerOnTime = _zerortc.getEpoch();
            return true;
        }
        else {
            return false;
        }
    }

    bool turnOffCamera() {
        if (_zerortc.getEpoch() - lastPowerOnTime > (unsigned int)cfg.getInt(CAMGUARD) && cameraOn) {
            DEBUGPORT.println("Turning OFF camera power...");
            cameraOn = false;
            digitalWrite(LED1_EN, LOW);
            digitalWrite(LED2_EN, LOW);
            lastPowerOffTime = _zerortc.getEpoch();
            return true;
        }
        else {
            return false;
        }
    }

    void getTimeString(char * timeString) {
        
        sprintf(timeString,"%s","YYYY-MM-DD hh:mm:ss");
        if (ds3231Okay) {
            DateTime now = _ds3231.now();
            now.toString(timeString);
        }
        else {
            sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d", 
                _zerortc.getYear(),
                _zerortc.getMonth(),
                _zerortc.getDay(),
                _zerortc.getHours(),
                _zerortc.getMinutes(),
                _zerortc.getSeconds()
            );
        }
    }

    bool update() {

        // Run updates and check for new data
        _sensors.update();

        // Build log string and send to UIs
        char output[256];

        char timeString[64];
        getTimeString(timeString);



        // The system log string, note this requires enabling printf_float build
        // option work show any output for floating point values
        sprintf(output, "%s,%s.%03u,%0.3f,%0.3f,%0.2f,%0.2f,%0.2f,%d",

            LOG_PROMPT,
            timeString,
            ((unsigned int) millis()) % 1000,
            _sensors.temperature, // In C
            _sensors.pressure / 1000.0, // in kPa
            _sensors.humidity, // in %
            _sensors.voltage[0] / 1000.0, // In Volts
            _sensors.power[0] / 1000.0, // in W
            int(cameraOn)
            
        );

        // Send output
        printAllPorts(output);

        return true;
    }

    void writeConfig() {
        if (systemOkay) {
            cfg.writeConfig();
        }
    }

    void readConfig() {
        if (systemOkay)
            cfg.readConfig();
    }

    void checkInput() {
        if (DEBUGPORT.available() > 0) {
            readInput(&DEBUGPORT);
        }
        if (UI1.available() > 0) {
            _rbr.disableEcho();
            readInput(&UI1);
        }
        if (UI2.available() > 0) {
            _rbr.disableEcho();
            readInput(&UI2);
        }
    }

    void checkCameraPower() {


    }

    void checkEnv() {
        if (_zerortc.getEpoch() - startupTimer <= (unsigned int)cfg.getInt(STARTUPTIME))
            return;


        // Update moving average of temperature
        float latestTemp = avgTemp.update(_sensors.temperature);
        float latestHum = avgHum.update(_sensors.humidity);
        //float latestVoltage = _sensors.voltage[0];

        // Make sure this check happens AFTER updating the average measurement, otherwise
        // the average will not be calculated properly
        if (_zerortc.getEpoch() - envTimer <= (unsigned int)cfg.getInt(CHECKINTERVAL))
            return;

        // Reset check timer
        envTimer = _zerortc.getEpoch();

        if (latestTemp > cfg.getInt(TEMPLIMIT)) {
            char output[64];
            sprintf(output,"Temperature %0.2f C exceeds limit of %0.2f C", latestTemp, (float)cfg.getInt(TEMPLIMIT));
            printAllPorts(output);
            badEnv = true;
            if (cameraOn) {
                printAllPorts("Shuting down camera...");
                sendShutdown();
            }
        }

        if (latestHum > cfg.getInt(HUMLIMIT)) {
            char output[64];
            sprintf(output,"Humidity %0.2f %% exceeds limit of %0.2f %%", latestHum, (float)cfg.getInt(HUMLIMIT));
            printAllPorts(output);
            badEnv = true;
            if (cameraOn) {
                printAllPorts("Shuting down camera...");
                sendShutdown();
            }
        }

        badEnv = false;
        
    }

    void checkVoltage() {

        if (_zerortc.getEpoch() - startupTimer <= (unsigned int)cfg.getInt(STARTUPTIME))
            return;

        // Update moving average of voltage
        float latestVoltage = avgVoltage.update(_sensors.voltage[0]);
        //float latestVoltage = _sensors.voltage[0];

        // Make sure this check happens AFTER updating the average measurement, otherwise
        // the average will not be calculated properly
        if (_zerortc.getEpoch() - voltageTimer <= (unsigned int)cfg.getInt(CHECKINTERVAL))
            return;
        
        // Reset check timer
        voltageTimer = _zerortc.getEpoch();

        if (latestVoltage < 6000.0) {
            // likely on USB power, note voltage is in mV
            return;
        }

        // If battery voltage is too low, notify and sleep
        // If the camera is running at this point, shut it down first
        if (latestVoltage < cfg.getInt(LOWVOLTAGE)) {
            char output[256];
            sprintf(output,"Voltage %f below threshold %d", latestVoltage, cfg.getInt(LOWVOLTAGE));
            printAllPorts(output);
            if (cameraOn) {
                sendShutdown();
            }
            if (cfg.getInt(STANDBY) == 1 && !cameraOn) {
                goToSleep();
            }
        }
    }

    void goToSleep() {
        
        printAllPorts("Going to sleep...");
        _zerortc.setAlarmTime(0, 0, 0);
        if (cfg.getInt(CHECKHOURLY) == 1) {
            printAllPorts("Alarm Set for 1 Hour");
            _zerortc.enableAlarm(RTCZero::MATCH_MMSS);
        }
        else {
            printAllPorts("Alarm Set for 1 Minute");
            _zerortc.enableAlarm(RTCZero::MATCH_SS);
        }
        if (cfg.getInt(STANDBY) == 1) {
            _zerortc.standbyMode();
        }
    }

    void sendShutdown() {
        if (cameraOn) {
            DEBUGPORT.println("Sending to Jetson: sudo shutdown -h now");
            JETSONPORT.println("sudo shutdown -h now\n");
            pendingPowerOff = true;
            pendingPowerOffTimer = _zerortc.getEpoch();
        }
        else {
            DEBUGPORT.println("Camera not powered on, not sending shutdown command");
        }
    }

    void configureFlashDurations() {
        // Set global delays for ISRs
        trigWidth = cfg.getInt(TRIGWIDTH);
        flashType = cfg.getInt(FLASHTYPE);
        if (flashType == 0) {
            lastStrobeDuration = cfg.getInt(WHITEFLASH);

        }
        else {
            lastStrobeDuration = cfg.getInt(UVFLASH);
        }
    }

    void setTriggers() {
        frameRate = cfg.getInt(FRAMERATE); 
        configTriggers(cfg.getInt(FRAMERATE));
    }

    void testFlash() {
        int uv_mod = 20;
        int flashCounter = 0;
        DEBUGPORT.flush();
        while (DEBUGPORT.available() <= 0) {
            if (flashCounter % uv_mod == 0) {
                doFlash(UV_FLASH_TRIG, 5000, 20);
            }
            else {
                doFlash(WHITE_FLASH_TRIG, 5000, 20);
            }
            flashCounter++;
            delay(50);
        }
    }

void triggerImage() {

    if (!(cfg.getInt(TRIGENABLED) == 1)) {
        return;
    }

    // Get variable before any timing events
    int whiteFlashDelay = cfg.getInt(WHITEFLASH);
    int uvFlashDelay = cfg.getInt(UVFLASH);
    int imagingMode = cfg.getInt(IMAGINGMODE);

    digitalWrite(CAMERA_TRIG,HIGH);
    delayMicroseconds(300);
    switch(imagingMode) {
        case 0:
            digitalWrite(WHITE_FLASH_TRIG,HIGH);
            delayMicroseconds(whiteFlashDelay);
            digitalWrite(WHITE_FLASH_TRIG,LOW);
            break;
        case 1:
            digitalWrite(UV_FLASH_TRIG,HIGH);
            delayMicroseconds(uvFlashDelay);
            digitalWrite(UV_FLASH_TRIG,LOW);
            break;
        case 2:
            if (imageCounter % 2 == 0) {
                digitalWrite(WHITE_FLASH_TRIG,HIGH);
                delayMicroseconds(whiteFlashDelay);
                digitalWrite(WHITE_FLASH_TRIG,LOW);
            }
            else {
                digitalWrite(UV_FLASH_TRIG,HIGH);
                delayMicroseconds(uvFlashDelay);
                digitalWrite(UV_FLASH_TRIG,LOW);
            }
            break;
    }
    imageCounter++;
    digitalWrite(CAMERA_TRIG,LOW);
}   

};

#endif
