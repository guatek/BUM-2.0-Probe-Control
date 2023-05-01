#ifndef _CONFIG

#define _CONFIG

#include <Arduino.h>
#include "wiring_private.h" // pinPeripheral() function

// Define additional serial ports

// Serial2
#define PIN_SERIAL2_RX       (5ul)
#define PIN_SERIAL2_TX       (2ul)
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)

// Serial3
#define PIN_SERIAL3_RX       (12ul)
#define PIN_SERIAL3_TX       (10ul)
#define PAD_SERIAL3_TX       (UART_TX_PAD_2)
#define PAD_SERIAL3_RX       (SERCOM_RX_PAD_3)

// Serial objects
Uart Serial2( &sercom2, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX ) ;
Uart Serial3( &sercom1, PIN_SERIAL3_RX, PIN_SERIAL3_TX, PAD_SERIAL3_RX, PAD_SERIAL3_TX ) ;

// Set SERCOM peripherals
void configSerialPins() {
    pinPeripheral(5, PIO_SERCOM);
    pinPeripheral(2, PIO_SERCOM);
    pinPeripheral(12, PIO_SERCOM);
    pinPeripheral(10, PIO_SERCOM);
}

// Serial handlers
void SERCOM2_Handler()
{
  Serial2.IrqHandler();
}

void SERCOM1_Handler()
{
  Serial3.IrqHandler();
}

// Macros
#define MAX_MACROS 6

// Power Control
#define LED1_EN 13
#define LED2_EN 11


// Define GPIOs
#define GPIO_1_IO 42
#define GPIO_2_IO SWIO
#define GPIO_3_IO SWCLK

// Define Triggers
#define TRIG_4_0 A1
#define TRIG_4_1 A2
#define TRIG_1_0 8
#define TRIG_1_1 9
#define TRIG_0_0 4
#define TRIG_0_1 3

// Define flash triggers
#define WHITE_FLASH_TRIG 7
#define UV_FLASH_TRIG 6
#define CAMERA_TRIG 4 

// Serial ports
#define DEBUGPORT Serial
#define HWPORT0 Serial0
#define HWPORT1 Serial1
#define HWPORT2 Serial2
#define HWPORT3 Serial3

// Mapping serial ports to UI ports
#define UI1 HWPORT0
#define UI2 HWPORT2
#define JETSONPORT HWPORT1
#define RBRPORT HWPORT3

// Define Config Settings
#define LOGINT "LOGINT"
#define DEPTHCHECKINTERVAL "DEPTHCHECKINTERVAL"
#define DEPTHTHRESHOLD "DEPTHTHRESHOLD"
#define LOCALECHO "LOCALECHO"
#define CMDTIMEOUT "CMDTIMEOUT"
#define HWPORT0BAUD "HWPORT0BAUD"
#define HWPORT1BAUD "HWPORT1BAUD"
#define HWPORT2BAUD "HWPORT2BAUD"
#define HWPORT3BAUD "HWPORT3BAUD"
#define STROBEDELAY "STROBEDELAY"
#define TRIGENABLED "TRIGENABLED"
#define FRAMERATE "FRAMERATE"
#define TRIGWIDTH "TRIGWIDTH"
#define WHITEFLASH "WHITEFLASH"
#define UVFLASH "UVFLASH"
#define AMBIENT "AMBIENT"
#define IMAGINGMODE "IMAGINGMODE"
#define RECORDAMBIENT "RECORDAMBIENT"
#define HIGHMAGCOLORFLASH "HIGHMAGCOLORFLASH"
#define HIGHMAGREDFLASH "HIGHMAGREDFLASH"
#define FLASHTYPE "FLASHTYPE"
#define FOCUSPOS "FOCUSPOS"
#define MAXREPEAT "MAXREPEAT"
#define MAXDELAY  "MAXDELAY"
#define MAXLONGDELAY "MAXLONGDELAY"
#define FOCUSINC "FOCUSINC"
#define LOWVOLTAGE "LOWVOLTAGE"
#define STANDBY "STANDBY"
#define CHECKHOURLY "CHECKHOURLY"
#define STARTUPTIME "STARTUPTIME"
#define WATCHDOG "WATCHDOG"
#define CAMGUARD "CAMGUARD"
#define TEMPLIMIT "TEMPLIMIT"
#define HUMLIMIT "HUMLIMIT"
#define CHECKINTERVAL "CHECKINTERVAL"


// Define Commands
#define CFG "CFG"
#define PORTPASS "PORTPASS"
#define SETTIME "SETTIME"
#define WRITECONFIG "WRITECONFIG"
#define READCONFIG "READCONFIG"
#define CAMERAON "CAMERAON"
#define CAMERAOFF "CAMERAOFF"
#define SHUTDOWNJETSON "SHUTDOWNJETSON"
#define NEWEVENT "NEWEVENT"
#define PRINTEVENTS "PRINTEVENTS"
#define CLEAREVENTS "CLEAREVENTS"
#define GOTOSLEEP "GOTOSLEEP"
#define TESTFLASH "TESTFLASH"
#define LOADSEQ "LOADSEQ"
#define RUNSEQ "RUNSEQ"
#define OPTOTUNE "OPTOTUNE"
#define MOVELENS "MOVELENS"


#endif