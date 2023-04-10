#ifndef _UTILS

#define _UTILS

#define PORT_BREAK_CHAR 5

#include <Arduino.h>
#include "MillisTimer.h"

void Blink(int DELAY_MS, byte loops)
{
    return;
    pinMode(LED_BUILTIN, OUTPUT);
    while (loops--)
    {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(DELAY_MS);
        digitalWrite(LED_BUILTIN,LOW);
        delay(DELAY_MS);  
    }
}

/** 
 * @brief check of an escape char received on UI or USB_UI ports
 * 
 * @return true if either port received 27 (Esc), false otherwise
 */
bool escapeReceived(Stream * in) {

	if (in->available()) {
		char c = in->read();
		if (c == 27)
			return true;
	}

    return false;
}

/** 
 * @brief Parse an int from a string with min/max range check
 * 
 * @param arg   The character array representing the string
 * @param val   The pointer to the int to store the parsed value in
 * @param min   The minimum int value that will be accepted
 * @param max   The maximum int value that will be accepted
 *
 * @return bool True when the parse was succeful, false otherwise 
 */
bool parseIntVal(char * arg,int * val, int min, int max) {

    int tmp;
    // Get the argument from the command line
    if (arg != NULL) {
        tmp = String(arg).toInt();    // Converts a char string to an integer
        if (tmp >= min && tmp <= max) {
            *val = tmp;
            return true;
        }
    }

    return false;
    
}

/** 
 * @brief check of an escape char received all UI ports
 * 
 * @return true if either port received 27 (Esc), false otherwise
 */
bool escapeReceived() {

    if (escapeReceived(&UI1) || escapeReceived(&UI2) || escapeReceived(&DEBUGPORT)) {
        return true;
    }
    else {
        return false;
    }
}

void portpass(Stream * in, Stream * out, bool localecho = false) {
    while (true) {
        if (in->available()) {
            unsigned char c = in->read();
            if (c == PORT_BREAK_CHAR) {
                break;
            }
            else{
                out->write(c);
            }
        }
        if (out->available()) {
            in->write(out->read());
        }
    }
}

bool confirm(Stream * in, const char * prompt, unsigned int cmdTimeout) {
    unsigned long startTimer = millis();
    in->println();
    in->print(prompt);

    while (startTimer <= millis() && millis() - startTimer < cmdTimeout) {

        // Wait on user input
        if (in->available()) {
            // Read the next char and reset timer          
            char c = in->read();
            if (c == 'Y' || c == 'y') {
                return true;
            }
            else {
                return false;
            }
        }
    }

    return false;
}

void printAllPorts(const char output[]) {
    UI1.println(output);
    UI2.println(output);
    DEBUGPORT.println(output);
}

int strncmp_ci(const char * input, const char * command, unsigned int n) {
    
    // string and command must match in length
    if (strlen(input) < n)
        return -1;
        
    if (strlen(command) < n)
        return -1;

    if (strlen(input) != strlen(command))
        return -1;

    for (unsigned int i=0; i < n; i++) {
        if (tolower(input[i]) < tolower(command[i]))
            return -1;
        if (tolower(input[i]) > tolower(command[i]))
            return 1;
    }

    // string match up to n chars
    return 0;
}

/**
 * @brief Sleep for given amount of time with periodic checks for exit
 *
 * @param us The duration to sleep in microseconds
 */
bool wakeable_sleep(int us) {
	int wake_period = 50;
	if (us > wake_period) { // Split up the delay into 50 us parts
		int iterations = us / wake_period;
		for (int i=0; i < iterations; i++) {
			if (escapeReceived())
				return true;
			delayMicroseconds(wake_period);
		}
	}
	else {
		delayMicroseconds(us);
	}

	return false;
}

/**
 * @brief Sleep for extended amount of time with periodic checks for exit
 */
bool wakeable_long_sleep(int ms) {
	
	int wake_period = 100;
	MillisTimer wakeTimer;
	
	if (ms > wake_period) { // Split up the delay into 100 ms parts

		int iterations = ms / wake_period;

		for (int i=0; i < iterations; i++) {

			wakeTimer.reset();
			wakeTimer.start();

			if (escapeReceived())
				return true;

			int delta = wake_period - wakeTimer.elapsed();
			if (delta > 0)
				delay(delta);
		}
	}
	else {
		delay(ms);
	}

	return false;
}

/** 
 * @brief Get input from the user over the serial port
 *
 * This function creates a very simple text input UI with
 * backspace handled properly and timeouts applied.
 *
 * It is intended to be used either for user interaction,
 * or for taking input from another program line-by-line
 *
 * The function fills a buffer while reading chars and
 * exits when it times out or if it recevies a 
 * @verbatim '\r' or '\n' @endverbatim
 *
 * Valid characters are echoed on the serial port in a
 * way similar to a terminal prompt
 *
 * @param port      The serial port to send/receive on
 * @param buf       The character buffer to store input in
 * @param maxLength The length of the character buffer
 *
 * @return  True when a line of characters followed by a @verbatim '\r' or '\n' @endverbatim
 *          was received before timeout.
 */
bool readUIInput(Stream * port, char * buf, int maxLength) {

    char c;
    int count = 0;
    int avail = 0;
    bool recvd = false;
    MillisTimer uiTimer;

	if (!port->available())
		return false;


    uiTimer.reset();
    uiTimer.start();
    while (uiTimer.elapsed() < 60000 && count < maxLength) {
        avail = port->available();
        if (avail > 0) {
            c = port->read();
			
            if (c == '\n' || c == '\r') {
				uiTimer.reset();
                uiTimer.start();
                buf[count++] = '\0';
                recvd = true;
                break;
            }
			// Handle backspace
			if (c == '\b' || c == 127) {
                count--;
                if (count < 0)
                    count = 0;
                else
                    port->print("\b \b");
            }
            else {
                port->print(c);
                buf[count++] = tolower(c);
            }
        }
    }
    port->flush();
    return recvd;
}



#endif