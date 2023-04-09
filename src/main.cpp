#include "Config.h"
#include "SystemControl.h"
#include "Sensors.h"
#include "SystemTrigger.h"
#include "SystemConfig.h"
#include "Utils.h"

// Global system control variable
SystemControl sys;

// High Mag Trigger Callback
void HighMagCallback()
{
    digitalWrite(HIGH_MAG_CAM_TRIG,HIGH);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(HIGH_MAG_STROBE_TRIG,HIGH);
    if (sys.lastStrobeDuration-FLASH_DELAY_OFFSET >= MIN_FLASH_DURATION)
        delayMicroseconds(sys.lastStrobeDuration-FLASH_DELAY_OFFSET);
    digitalWrite(HIGH_MAG_STROBE_TRIG,LOW);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(HIGH_MAG_CAM_TRIG,LOW);
}

// Low Mag Trigger Callback
void LowMagCallback()
{
    digitalWrite(LOW_MAG_CAM_TRIG,HIGH);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(LOW_MAG_STROBE_TRIG,HIGH);
    if (sys.lastStrobeDuration-FLASH_DELAY_OFFSET >= MIN_FLASH_DURATION)
        delayMicroseconds(sys.lastStrobeDuration-FLASH_DELAY_OFFSET);
    digitalWrite(LOW_MAG_STROBE_TRIG,LOW);
    delayMicroseconds(sys.trigWidth/2);
    digitalWrite(LOW_MAG_CAM_TRIG,LOW);
}

// Wrapper for updaing timers and flashes from callback functions
void setTriggers() {
    sys.setTriggers();
}

void setFlashes() {
    sys.configureFlashDurations();
}

void setCTDType() {
    sys.setCTDType();
}

void setPolling() {
    sys.setPolling();
}

// wrapper for turning system on
void turnOnCamera() {
    if (sys.cfg.getInt("PROFILEMODE") == 1) {
        sys.turnOnCamera();
    }
}



void setup() {

    //Turn off strobe and camera power
    pinMode(CAMERA_POWER, OUTPUT);
    pinMode(STROBE_POWER, OUTPUT);
    pinMode(LED1_EN, OUTPUT);
    pinMode(LED2_EN, OUTPUT);
    pinMode(WHITE_FLASH_TRIG, OUTPUT);
    pinMode(UV_FLASH_TRIG, OUTPUT);

    digitalWrite(LED1_EN, LOW);
    digitalWrite(LED2_EN, LOW);
    digitalWrite(WHITE_FLASH_TRIG, LOW);
    digitalWrite(UV_FLASH_TRIG, LOW);


    // Setup Sd Card Pins
    //pinMode(SDCARD_DETECT, INPUT_PULLUP);

    // Start the debug port
    DEBUGPORT.begin(115200);

    delay(4000);

    // Wait until serial port is opened
    //while (!Serial) { delay(10); }
    //while (!Serial0) { delay(10); }

    // Startup all system processes
    sys.begin();

    // Add config parameters for system
    // IMPORTANT: add parameters at t he end of the list, otherwise you'll need to reflash the saved params in EEPROM before reading
    sys.cfg.addParam(LOGINT, "Time in ms between log events", "ms", 0, 100000, 250);
    sys.cfg.addParam(POLLFREQ, "Rate of polling instruments", "Hz", 1, 50, 10, false, setPolling);
    sys.cfg.addParam(DEPTHCHECKINTERVAL, "Time in seconds between depth checks for testing ascent/descent", "s", 10, 300, 30);
    sys.cfg.addParam(DEPTHTHRESHOLD, "Depth change threshold to denote ascent or descent", "mm", 500, 10000, 1000);
    sys.cfg.addParam(LOCALECHO, "When > 0, echo serial input", "", 0, 1, 1);
    sys.cfg.addParam(CMDTIMEOUT, "time in ms before timeout waiting for user input", "ms", 1000, 100000, 10000);
    sys.cfg.addParam(HWPORT0BAUD, "Serial Port 0 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam(HWPORT1BAUD, "Serial Port 1 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam(HWPORT2BAUD, "Serial Port 2 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam(HWPORT3BAUD, "Serial Port 3 baud rate", "baud", 9600, 115200, 115200);
    sys.cfg.addParam(STROBEDELAY, "Time between camera trigger and strobe trigger in us", "us", 5, 1000, 50, false, setFlashes);
    sys.cfg.addParam(FRAMERATE, "Camera frame rate in Hz", "Hz", 1, 30, 10, false, setTriggers);
    sys.cfg.addParam(TRIGWIDTH, "Width of the camera trigger pulse in us", "us", 30, 10000, 100, false, setFlashes);
    sys.cfg.addParam(AMBIENT, "Width of the ambient light exposure in us", "us", 30, 10000, 100, false, setFlashes);
    sys.cfg.addParam(WHITEFLASH, "Width of the white flash in us", "us", 1, 100000, 10, false, setFlashes);
    sys.cfg.addParam(UVFLASH, "Width of the uv flash in us", "us", 1, 100000, 10, false, setFlashes);
    sys.cfg.addParam(FLASHTYPE, "0 = white strobes, 1 = uv strobes","", 0, 1, 0, false, setFlashes);
    sys.cfg.addParam(FOCUSPOS, "Position of the lens focus relative to the view port in um", "um", 25000, 30000, 35000);
    sys.cfg.addParam(FOCUSINC, "Minimum increment in focus position in um", "um", 1, 100, 5000);
    sys.cfg.addParam(MAXREPEAT, "Maximum number of cycles in sequence REPEAT cmd.", "cycles", 0, 1000, 100000);
    sys.cfg.addParam(MAXDELAY, "Maximum ms delay in sequence DELAY cmd", "ms", 0, 1000, 10000);
    sys.cfg.addParam(MAXLONGDELAY, "Maximum seconds in sequence LONGDELAY cmd.", "s", 0, 3600, 10000);
    sys.cfg.addParam(LOWVOLTAGE, "Voltage in mV where we shut down system", "mV", 10000, 14000, 11500);
    sys.cfg.addParam(STANDBY, "If voltage is low go into standby mode", "", 0, 1, 0);
    sys.cfg.addParam(CHECKHOURLY, "0 = check every minute, 1 = check every hour", "", 0, 1, 0);
    sys.cfg.addParam(CHECKINTERVAL, "Time in seconds between checking system health", "s", 10, 60, 3600);
    sys.cfg.addParam(STARTUPTIME, "Time in seconds before performing any system checks", "s", 0, 60, 10);
    sys.cfg.addParam(WATCHDOG, "0 = no watchdog, 1 = hardware watchdog timer with 8 sec timeout","", 0, 1, 0);
    sys.cfg.addParam(TEMPLIMIT, "Temerature in C where controller will shutdown and power off camera","C", 0, 80, 55);
    sys.cfg.addParam(HUMLIMIT, "Humidity in % where controller will shutdown and power off camera","%", 0, 100, 60);


    // configure watchdog timer if enabled
    sys.configWatchdog();

    // Start the remaining serial ports
    HWPORT0.begin(sys.cfg.getInt(HWPORT0BAUD));
    HWPORT1.begin(sys.cfg.getInt(HWPORT1BAUD));
    HWPORT2.begin(sys.cfg.getInt(HWPORT2BAUD));
    HWPORT3.begin(sys.cfg.getInt(HWPORT3BAUD));

    // Config the SERCOM muxes AFTER starting the ports
    configSerialPins();

    // Load the last config from EEPROM
    sys.readConfig();

    sys.loadScheduler();

    // Setup flashes triggers and polling
    setFlashes();
    setTriggers();
    setPolling();
    
}

void loop() {

    sys.update();
    sys.checkInput();
    sys.checkVoltage();
    sys.checkEnv();
    sys.checkEvents();
    sys.checkCameraPower(); 

    int logInt = sys.cfg.getInt(LOGINT);

    delay(logInt);
    Blink(10, 1);

}
