/**
 * @file MillisTimer.h
 * @brief Millisecond timer
 *
 * A samll helper class to implement a software timer
 * based on Arduino millis().
 *
 * @author pldr
 * @copyright 2020 Scripps Institution of Oceanography
 * @copyright 2023 Guatek
 */
#include "Arduino.h"

class MillisTimer {

	public:
	
        /**< Default constructor */
		MillisTimer() {
	        running = false;
	        elapsedMillis = 0;
	
        }

        /**< Start the timer */				
		void start() {
            lastMillis = millis();
            running = true;
        }		

        /**< Stop the timer */		
		void stop() {
            running = false;
        }			

        /**< Reset the timer */
		void reset() {
            running = false;
            elapsedMillis = 0;
        }		

        /**< Return the elasped ms of the timer */
		unsigned long elapsed() {
            if (!running)
                return elapsedMillis;
            
            unsigned long curMillis = millis();
            if (curMillis - lastMillis < 0) {
                elapsedMillis = 4294967295 - lastMillis;
                elapsedMillis += curMillis;
            }
            else
                elapsedMillis = curMillis - lastMillis;

            return elapsedMillis;
        }	
		
	private:
	
		unsigned long lastMillis; 		/**< The last millis() values */
		unsigned long elapsedMillis; 	/**< The elasped milliseconds */
		bool running;					/**< True when the timer is running */
		
};