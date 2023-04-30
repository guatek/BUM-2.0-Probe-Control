#ifndef _SENSORS

#define _SENSORS

#define INA260_SYS_ADDR 0x40


#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_INA260.h>

#include "Config.h"

Adafruit_BME280 _bme; // I2C
Adafruit_INA260 _ina260_sys = Adafruit_INA260();


class Sensors {

    private:
        bool sensorsValid;
   
    public:

        float voltage[1];
        float current[1];
        float power[1];
        float temperature;
        float pressure;
        float humidity;

        Sensors() {
            sensorsValid = false;

        }

        bool begin() {

            sensorsValid = true;
            
            if (!_ina260_sys.begin(INA260_SYS_ADDR)) {
                DEBUGPORT.println("Couldn't find INA260 A chip");
                sensorsValid = false;
            }
            else {
                DEBUGPORT.println("System INA260 OK");
            }

            // set the number of samples to average
            _ina260_sys.setAveragingCount(INA260_COUNT_256);
            // set the time over which to measure the current and bus voltage
            _ina260_sys.setVoltageConversionTime(INA260_TIME_558_us);
            _ina260_sys.setCurrentConversionTime(INA260_TIME_558_us);
            
            
            // default settings
            int status = _bme.begin(0x76, &Wire);  
            // You can also pass in a Wire library object like &Wire2
            // status = bme.begin(0x76, &Wire2)
            if (!status) {
                DEBUGPORT.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
                DEBUGPORT.print("SensorID was: 0x"); Serial.println(_bme.sensorID(),16);
                DEBUGPORT.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
                DEBUGPORT.print("   ID of 0x56-0x58 represents a BMP 280,\n");
                DEBUGPORT.print("        ID of 0x60 represents a BME 280.\n");
                DEBUGPORT.print("        ID of 0x61 represents a BME 680.\n");
                // while (1) delay(10);
                sensorsValid = false;
            }

            return sensorsValid;

        }

        void update() {
            if (!sensorsValid)
                return;
            temperature = _bme.readTemperature();
            pressure = _bme.readPressure();
            humidity = _bme.readHumidity();
            current[0] = _ina260_sys.readCurrent();
            voltage[0] = _ina260_sys.readBusVoltage();
            power[0] = _ina260_sys.readPower();

        }

        void printEnv() {
            if (!sensorsValid)
                return;
            temperature = _bme.readTemperature();
            pressure = _bme.readPressure();
            humidity = _bme.readHumidity();
            String output = "$BME280," + String(temperature) + "," + String(pressure) + "," + String(humidity);
            UI1.println(output);
            UI2.println(output);
        }

        void printPower() {
            if (!sensorsValid)
                return;
            current[0] = _ina260_sys.readCurrent();
            voltage[0] = _ina260_sys.readBusVoltage();
            power[0] = _ina260_sys.readPower();

            String output = "$PWR_SYS," + String(current[0]) + "," + String(voltage[0]) + "," + String(power[0]);
            UI1.println(output);
            UI2.println(output);
            
        }
};

#endif