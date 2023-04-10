/** @file Sequence.h
 * @brief Imaging sequence functions for loading, parsing, and running commands 
 *
 * This is a C++ rework of an existing C implementation 
 *
 * START 								-- Start the sequence \n
 * REPEAT, [iterations] 				-- Repeat the subsequent sequence from the next command till reaching END,  [iterations] times \n
 * AMBIENT, [duration]					-- Ambient light image with no flash \n
 * FLUOR, [duration]                    -- UV light image with [duration] us flash \n
 * WHITE, [duration]					-- White light image with [duration] us flash \n
 * MOVE, [pos]							-- Move actuators to [pos] \n
 * FOCALSTACK, [start], [stop], [inc]	-- Record focal stack while repeating commands at [inc] steps between [start] and [stop] actuator positions \n
 * DELAY, [duration]					-- Delay for [duration] us \n
 * LONG_DELAY, [duration]				-- Delay for [duration] s \n
 * END									-- End a sequence  \n
 * 
 * @author pldr
 * @copyright 2020 Scripps Institution of Oceanography
 * @copyright 2023 Guatek
 * 
 */
#ifndef _SEQUENCE

#define _SEQUENCE

#include "Config.h"
#include "SystemControl.h"
#include "Utils.h"
#include "Strobe.h"
#include "Optotune.h"

#define MAX_STRING_LEN 128  /**< Maximum length of string for pritning status */
#define MAX_COMMANDS 512    /**< Maximum number of commands in a sequence */
#define MAX_RECURSION 16    /**< Maximum number of recursions in a sequence */


class Sequence {

private:
    /** SequenceCommandType */
    typedef enum {
        CMD_START = 0,      /**< Start the sequence (needed for REPEAT cmd) */
        CMD_END = 1,        /**< End the sequence (need to return back to UI) */
        CMD_REPEAT = 2,     /**< Repeat a block of the sequence delimited by by START */
        CMD_DELAY = 3,      /**< A short delay up to 1 second */
        CMD_LONGDELAY = 4,  /**< A long delay up to 1 hour */
        CMD_MOVE = 5,       /**< Move the actuator */
        CMD_FOCALSTACK = 6, /**< Record a focal stack while repeating a block of commands */
        CMD_FLUOR = 7,      /**< Fluorescence image */
        CMD_AMBIENT = 8,    /**< trigger camera */
        CMD_WHITE = 9      /**< Fire a white LED flash and trigger camera */
    } SequenceCommandType;

    /** Command Object */
    struct Command {
        SequenceCommandType cmd;    /**< type of the command */
        unsigned long dur;          /**< duration of command (if applicable) */ 
        unsigned short start;       /**< start position of lens (if applicable) */ 
        unsigned short stop;        /**< end position of lens (if applicable) */ 
        unsigned short inc;         /**< increment of actuator (if applicable) */ 
    };

    SystemControl * sys;                /**< Pointer to the system config object */
    Optotune * etl;                     /**< Pointer to the optotune ETL object */
    bool end;                           /**< True when sequence should end */
    int idx;                            /**< Index of mos recent command */
    int startIdx;                       /**< Index of most recent start command */
    int startIndexList[MAX_RECURSION];  /**< Start index array for recursion */
    Command commands[MAX_COMMANDS];     /**< Command array, up to MAX_COMMANDS len. */

public:
    /**
     * @brief Default constructor
    */
    Sequence() {
        this->end = false;
        this->idx = 0;
        this->startIdx = 0;
    }


    /**
     * @brief Inititalizes the object with sys and etl pointers
     * 
     * @param cfg The already instantiated system config object
    */
    void init(SystemControl * sys, Optotune * etl) {
        this->sys = sys;
        this->etl = etl;
    }


    /**
     * @brief print out the string repesentation of the command
     *
     * @param command The command to print
     */
    void print_sequence_command(Command command) {
        
        char output[MAX_STRING_LEN];
        
        switch(command.cmd) {
            case CMD_START:
                printAllPorts("START\r\n");
                break;
            case CMD_END:
                printAllPorts("END\r\n");
                break;
            case CMD_REPEAT:
                sprintf(output, "REPEAT,%lu\r\n", command.dur);
                printAllPorts(output);
                break;
            case CMD_DELAY:
                sprintf(output, "DELAY,%lu\r\n", command.dur);
                printAllPorts(output);
                break;
            case CMD_LONGDELAY:
                sprintf(output, "LONGDELAY,%lu\r\n", command.dur);
                printAllPorts(output);
                break;
            case CMD_MOVE:
                sprintf(output, "MOVE,%u\r\n", command.start);
                printAllPorts(output);
                break;
            case CMD_FOCALSTACK:
                sprintf(output, "FOCALSTACK,%u,%u,%u\r\n", command.start, command.stop, command.inc);
                printAllPorts(output);
                break;
            case CMD_WHITE:
                sprintf(output, "WHITE,%lu\r\n", command.dur);
                printAllPorts(output);
                break;
            case CMD_FLUOR:
                sprintf(output, "FLUOR,%lu\r\n", command.dur);
                printAllPorts(output);
                break;
            case CMD_AMBIENT:
                sprintf(output, "AMBIENT,%lu\r\n", command.dur);
                printAllPorts(output);
                break;
        }
    }


    /**
     * @brief run the command sequence stored in SEQ starting at given index
     *
     * This is a recursive function to process commands stored in the SEQ
     * global variable.
     *
     * @param startIndex The start index into this->commands
     * @param endIndex The end index into SED.commands
     */
    bool run_sequence(int startIndex, int endIndex) {

        char output[MAX_STRING_LEN];

        // Check if we halted the sequence early
        if (this->end)
            return true;

        for (int i = startIndex; i < endIndex; i++) {

            int nextStart = -1;
            sprintf(output,"%d : ", i);
            printAllPorts(output);
            print_sequence_command(this->commands[i]);

            if (i >= 0 && i < MAX_COMMANDS) {
                switch (this->commands[i].cmd) {
                    case CMD_START:
                        if (this->startIdx >= 0 && this->startIdx < MAX_RECURSION)
                            this->startIndexList[this->startIdx++] = i;
                        break;
                    case CMD_END:
                        this->end = true;
                        return true;
                    case CMD_REPEAT:
                        if (this->startIdx > 0) {
                            nextStart = this->startIndexList[this->startIdx-- - 1]; // Pop right
                        }
                        for (unsigned int j=0; j < this->commands[i].dur - 1; j++) {
                            if (run_sequence(nextStart+1, i))  { // repeat sub-sequence 
                                this->end = true;
                                return true;
                            }
                        }
                        break;
                    case CMD_DELAY:
                        if (wakeable_sleep(this->commands[i].dur)) {
                            this->end = true;
                            return true;
                        }
                        break;
                    case CMD_LONGDELAY:
                        if (wakeable_long_sleep(1000 * this->commands[i].dur)) {
                            this->end = true;
                            return true;

                        }
                        break;
                    /* Important Note:
                    *
                    * The flash commands below need to set the values
                    * for the SP parameters AND call the associated record
                    * function. This will ensure that subsequent calls
                    * to focal stack will use the last flash settings
                    */
                    case CMD_WHITE:
                        sys->cfg.set(WHITEFLASH, this->commands[i].dur);
                        sys->cfg.set(FLASHTYPE, 0);
                        recordWhite(this->commands[i].dur);
                        break;
                    case CMD_FLUOR:
                        sys->cfg.set(UVFLASH, this->commands[i].dur);
                        sys->cfg.set(FLASHTYPE, 1);
                        recordWhite(this->commands[i].dur);
                        break;
                    case CMD_AMBIENT:
                        sys->cfg.set(AMBIENT, this->commands[i].dur);
                        sys->cfg.set(FLASHTYPE, 2);
                        recordAmbient(this->commands[i].dur);
                        break;
                    case CMD_MOVE:
                        etl->move(this->commands[i].start);
                        break;
                    case CMD_FOCALSTACK:
                        {
                            int start = this->commands[i].start;
                            int stop = this->commands[i].stop;
                            int inc = this->commands[i].inc;
                            etl->move(start);

                            if (this->startIdx > 0) {
                                nextStart = this->startIndexList[this->startIdx-- - 1]; // Pop right
                            }

                            while (start <= stop) {
                                if (escapeReceived())
                                    return true;
                                //triggerSystem();
                                /* Use FOCALSTACK like REPEAT but with movement between iterations */
                                if (run_sequence(nextStart+1, i))  { // repeat sub-sequence 
                                    this->end = true;
                                    return true;
                                }							
                                start += inc;
                                etl->move(start);
                                // delay for the frame rate
                                int frameRate = sys->cfg.getInt(FRAMERATE);
                                if (frameRate > 0) {
                                    delayMicroseconds(1000000/frameRate);
                                }
                            }
                        }
                        break;

                }
            }

            // delay for the frame rate
            int frameRate = sys->cfg.getInt(FRAMERATE);
            if (frameRate > 0) {
                delayMicroseconds(1000000/frameRate);
            }

        }
        
        return false;

    }



    /**
     * @brief Parse a command string and update the SEQ object
     *
     * @param buf The character buffer holding the NULL terminated command
     * @param online If true, immediately execute the command in buf
     */
    bool parse_cmd(char * buf, bool online=false) {

        char * rem;
        bool okay = false;

        // Get the command name
        char * tok = strtok_r(buf,",", &rem);

        // START
        if (strncmp_ci(tok, "start", 5) == 0) {
            this->commands[this->idx++].cmd = CMD_START;
            okay = true;
        }
        // END
        else if (strncmp_ci(tok, "end", 3) == 0) {
            this->commands[this->idx++].cmd = CMD_END;
            okay = true;
        }
        // REPEAT
        else if (strncmp_ci(tok, "repeat", 6) == 0) {
            int iterations;
            if (parseIntVal(rem,&iterations, 0, sys->cfg.getInt(MAXREPEAT))) {
                this->commands[this->idx].cmd = CMD_REPEAT;
                this->commands[this->idx++].dur = iterations; // stick iteraions in dur field
                okay = true;
            }
        }
        // DELAY
        else if (strncmp_ci(tok, "delay", 5) == 0) {
            int us_delay;
            if (parseIntVal(rem,&us_delay, 0, sys->cfg.getInt(MAXDELAY))) {
                this->commands[this->idx].cmd = CMD_DELAY;
                this->commands[this->idx++].dur = us_delay;
                okay = true;

                if (okay && online) {
                    wakeable_sleep(us_delay);
                }
            }
        }
        // LONG_DELAY
        else if (strncmp_ci(tok, "longdelay", 10) == 0) {
            int s_delay;
            if (parseIntVal(rem,&s_delay, 0, sys->cfg.getInt(MAXLONGDELAY))) {
                this->commands[this->idx].cmd = CMD_LONGDELAY;
                this->commands[this->idx++].dur = s_delay;
                okay = true;

                if (okay && online) {
                    wakeable_long_sleep(1000*s_delay);
                }
            }
        }
        // MOVE
        else if (strncmp_ci(tok, "move", 4) == 0) {
            int pos;
            if (parseIntVal(rem,&pos, sys->cfg.getIntMin(FOCUSPOS), sys->cfg.getIntMax(FOCUSPOS))) {
                this->commands[this->idx].cmd = CMD_MOVE;
                this->commands[this->idx++].start = pos;
                okay = true;

                if (okay && online) {
                    etl->move(pos);
                }
            }
        }
        // WHITE
        else if (strncmp_ci(tok, "white", 5) == 0) {
            int dur;
            if (parseIntVal(rem,&dur, sys->cfg.getIntMin(WHITEFLASH), sys->cfg.getIntMax(WHITEFLASH))) {
                this->commands[this->idx].cmd = CMD_WHITE;
                this->commands[this->idx++].dur = dur;
                okay = true;

                if (okay && online) {
                    sys->cfg.set(WHITEFLASH, dur);
                    sys->cfg.set(FLASHTYPE,0);
                    recordAmbient(dur);
                }
                
            }
        }
        // FLUOR
        else if (strncmp_ci(tok, "meas", 4) == 0) {
            int dur;
            if (parseIntVal(rem,&dur, sys->cfg.getIntMin(UVFLASH), sys->cfg.getIntMax(UVFLASH))) {
                this->commands[this->idx].cmd = CMD_FLUOR;
                this->commands[this->idx++].dur = dur;
                okay = true;

                if (okay && online) {
                    sys->cfg.set(UVFLASH, dur);
                    sys->cfg.set(FLASHTYPE,1);
                    recordAmbient(dur);
                }
                
            }

        }
        // AMBIENT
        else if (strncmp_ci(tok, "ambient", 7) == 0) {
            int dur;
            if (parseIntVal(rem,&dur, sys->cfg.getIntMin(AMBIENT), sys->cfg.getIntMax(AMBIENT))) {
                this->commands[this->idx].cmd = CMD_AMBIENT;
                this->commands[this->idx++].dur = dur;
                okay = true;

                if (okay && online) {
                    sys->cfg.set(AMBIENT, dur);
                    sys->cfg.set(FLASHTYPE,2);
                    recordAmbient(dur);
                }
                
            }
        }
        // FOCALSTACK
        else if (strncmp_ci(tok, "focalstack", 10) == 0) {
            int start;
            int stop;
            int inc;
            okay = true;
            tok = strtok_r(rem,",", &rem);
            if (!parseIntVal(tok,&start, sys->cfg.getIntMin(FOCUSPOS), sys->cfg.getIntMax(FOCUSPOS)))
                okay = false;
            tok = strtok_r(rem,",", &rem);
            if (!okay || !parseIntVal(tok,&stop, sys->cfg.getIntMin(FOCUSPOS), sys->cfg.getIntMax(FOCUSPOS)))
                okay = false;
            if (!okay || !parseIntVal(rem,&inc, sys->cfg.getIntMin(FOCUSINC), sys->cfg.getIntMax(FOCUSINC)))
                okay = false;
            if (okay) {

                this->commands[this->idx].cmd = CMD_FOCALSTACK;
                this->commands[this->idx].start = start;
                this->commands[this->idx].stop = stop;
                this->commands[this->idx++].inc = inc;

            }

            if (okay && online) {
                while (start <= stop) {
                    if (escapeReceived())
                        break;
                    sys->triggerSystem();			
                    start += inc;
                    etl->move(start);
                    int frameRate = sys->cfg.getInt(FRAMERATE);
                    if (frameRate > 0) {
                        delayMicroseconds(1000000/frameRate);
                    }
                }
            }
        }

        else {
            okay = false;
        }

        if (!okay) {
            printAllPorts("Invalid Command.");
        }

        return okay;
    }

    /**
     * @brief Load sequence commands from UI_PORT but don't execute them
     */
    bool load_sequence(Stream * in) {
        char buf[256];
        // Disable camera timer triggered images
        //int lastTriggerState = SP.triggerEnabled;
        //SP.triggerEnabled = 0;
        //cameraTriggerTimer.end();

        // clear out any old sequence values:
        this->idx = 0;
        this->startIdx = 0;
        this->end = false;

        // read commands, with 60 second timeout
        MillisTimer uiTimer;

        while (uiTimer.elapsed() < (unsigned int)(sys->cfg.getInt("CMDTIMEOUT"))) {

            printAllPorts("\rLOAD > ");

            bool haveCmd = false;

            if (!haveCmd)
                haveCmd = readUIInput(in, buf, 256);

            if (haveCmd) {
                
                // end the sequence when we get the "END" command
                if (strncmp_ci(buf, "end", 3) == 0)
                    break;

                parse_cmd(buf);

                // reset timer
                uiTimer.reset();
                uiTimer.start();
                printAllPorts("\r\n");

            }
            else {
                delay(250);
            }
        }

        printAllPorts("\r\n");

        return true;
    }

};

#endif