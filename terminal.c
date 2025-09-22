#include <stdbool.h>
#include "terminal.h"
#include "uart0.h"
#include "onboard_leds.h"

// function to store the string of characters received from UART0
void getsUart0(USER_DATA *d)
{
    uint8_t i = 0;
    char ch;
    while (1)
    {

        ch = getcUart0();
        if ((ch == 8 || ch == 127) && i > 0)    // backspace character
        {
            i--;
            putcUart0(ch);
        }
        else if (ch == 13)  // carriage return character
        {
            d->buffer[i] = 0;
            return;
        }
        else if (ch >= 32) // space character and further
        {
            d->buffer[i] = ch;
            putcUart0(ch);
            i++;
            if (i == MAX_CHARS)
            {
                d->buffer[i] = '\0';
                return;
            }
        }
        else
            continue;
    }
}

// function which parses the given string
// used to process the commands
void parseFields(USER_DATA *d)
{
    uint8_t i = 0;
    char prev = 0;
    d->fieldCount = 0;
    while (d->buffer[i] != '\0')
    {
        if ((d->fieldCount) >= MAX_FIELDS)
        {
            break;
        }

        char temp = d->buffer[i];

        if (((temp >= 97 && temp <= 122) || (temp >= 65 && temp <= 90))
                && prev != 'a')
        {
            prev = 'a';
            d->fieldType[(d->fieldCount)] = 'a';
            d->fieldPosition[(d->fieldCount)] = i;
            d->fieldCount += 1;
        }

        else if ((temp >= 48 && temp <= 57) && prev != 'n')
        {
            prev = 'n';
            d->fieldType[d->fieldCount] = 'n';
            d->fieldPosition[d->fieldCount] = i;
            d->fieldCount += 1;
        }
        else if (!((temp >= 97 && temp <= 122) || (temp >= 65 && temp <= 90))
                && !(temp >= 48 && temp <= 57))
        {
            prev = 0;
            d->buffer[i] = '\0';
        }
        i++;
    }
}

// function that gets the string from the input command, given the fieldNumber
char* getFieldString(USER_DATA *data, uint8_t fieldNumber)
{
    if (fieldNumber <= data->fieldCount)
    {
        return &(data->buffer[data->fieldPosition[fieldNumber]]);
    }
    else
        return -1;
}

// works like itoa() (supports 32bit int)
char* integerToAlphabet(uint32_t decInt, char* outStr)
{
    int8_t order = 0;
    char temp[MAX_INT_STR_LENGTH + 1];
    while(order <= MAX_INT_STR_LENGTH)
    {
        temp[order] = (decInt % 10) + '0';
        decInt = decInt / 10;
        order++;
        if(decInt == 0)
            break;
    }

    uint8_t i = 0;
    while(order >= 0)
    {
        outStr[i] = temp[--order];
        i++;
    }
    outStr[i] = 0;
    return outStr;
}

// works like atoi()
int32_t alphabetToInteger(char *numStr)
{
    int32_t num = 0;
    while (*numStr != 0)
    {
        if (*numStr >= 48 && *numStr <= 57)
        {
            num = num * 10 + ((*numStr) - 48);
            numStr++;
        }

    }
    return num;
}

//returns true if given two strings are equal
//false if not equal
bool stringCompare(const char *str1, const char *str2)
{
    bool equal = true;
    while (*str1 != 0 || *str2 != 0)
    {
        if ((*str1 == 0 && *str2 != 0) || (*str1 != 0 && *str2 == 0))
            return false;

        if (!(*str1 == *str2 || (*str1 + 32) == *str2 || *str1 == (*str2 + 32)
                || (*str1 - 32) == *str2 || *str1 == (*str2 - 32)))
        {
            equal = false;
            break;
        }

        str1++;
        str2++;
    }
    return equal;
}

// takes in the fieldNumber and returns the integer in that field of the buffer[] in USER_DATA
int32_t getFieldInteger(USER_DATA *data, uint8_t fieldNumber)
{
    if (fieldNumber <= data->fieldCount && data->fieldType[fieldNumber] == 'n')
    {
        return alphabetToInteger(getFieldString(data, fieldNumber));
    }
    else
        return 0;
}

// prints the decimal equivalent of the given char* with the hexadecimal number
uint32_t hexStrToInt(const char hex[])
{
    uint8_t i = 0;
    char c;
    uint32_t out = 0;
    
    while((c = hex[i]) != 0)
    {
        out <<= 4;

        if (c >= '0' && c <= '9')
            out += (c - '0');
        else if (c >= 'A' && c <= 'F')
            out += (c - 'A');
        else if (c >= 'a' && c <= 'f')
            out += (c - 87) << 4;
        else
            return 0;

        i++;
    }
    return out;

}

/*
uint32_t hexToInt(char *hex)
{
    uint32_t dec = 0;
    uint8_t count = 0;

    while (*hex != 0)
    {
        hex++;
        count++;
    }

    hex = hex - count;

    while (*hex != 0)
    {
        uint8_t character = *hex;
        uint32_t temp;

        if (character >= 48 && character <= 57)
        {
            temp = character - 48;
            uint8_t i = 0;
            for (i = 0; i < count - 1; i++)
            {
                temp = temp * 16;
            }

            count--;
        }

        else if ((character >= 65 && character <= 70)
                || (character >= 97 && character <= 102))
        {
            if (character >= 65 && character <= 70)
                temp = character - 55;

            else
                temp = character - 87;

            uint8_t i = 0;
            for (i = 0; i < count - 1; i++)
            {
                temp = temp * 16;
            }
            count--;
        }
        dec = dec + temp;
        hex++;
    }
    return dec;
}
*/

/*
* converts a 32-bit word to hex string and prints it to UART0
*/
void printHex(uint32_t word)
{
    uint8_t i;
    
    putsUart0("0x");
    for(i = 0; i < 8; i++)
    {
        uint8_t nibble = (word & 0xF0000000) >> 28;
        
        if (nibble <= 9)
            putcUart0(nibble + '0');         // 0-9
        else
            putcUart0(nibble + 'A');         // A-F
        
        word = word << 4;
    }
    putsUart0("\n\r");
}

/*function to check whether the entered command matches any of the shell commands
 * returns true if the entered command is valid
 * false if invalid*/
bool isCommand(USER_DATA *data, const char strCommand[], uint8_t minArguments)
{
    if (stringCompare(strCommand, getFieldString(data, 0))
            && (data->fieldCount) > minArguments)
        return true;
    return false;
}

// Receives, parses and executes user commands received in the UART0 terminal
void startShell()
{
    USER_DATA data;
    while (1)
    {
        // User prompt for new command
        putsUart0(CARRIAGE_RETURN);
        putsUart0("user@tivaC> ");
        getsUart0(&data);
        putsUart0(CARRIAGE_RETURN_AND_NEWLINE);
        // putsUart0(data.buffer);
        // putsUart0(CARRIAGE_RETURN_AND_NEWLINE);

        // parse the received command
        parseFields(&data);

        bool valid = false;

        if (data.buffer[0] == 0)
        {
            continue;
        }

        else if (isCommand(&data, "ps", 0))
        {
            ps();
            valid = true;
        }

        else if (isCommand(&data, "ipcs", 0))
        {
            ipcs();
            valid = true;
        }

        else if (isCommand(&data, "kill", 1))
        {
            kill(alphabetToInteger(getFieldString(&data, 1)));
            valid = true;
        }

        else if (isCommand(&data, "pkill", 1))
        {
            pkill(getFieldString(&data, 1));
            valid = true;
        }

        else if (isCommand(&data, "pi", 1))
        {
            char *ONOFF = getFieldString(&data, 1);
            valid = true;
            if (stringCompare(ONOFF, "on"))
            {
                pi(true);
            }
            else if (stringCompare(ONOFF, "off"))
            {
                pi(false);
            }
            else
            {
                valid = false;
            }
        }

        else if (isCommand(&data, "preempt", 1))
        {
            char *ONOFF = getFieldString(&data, 1);
            valid = true;
            if (stringCompare(ONOFF, "on"))
            {
                preempt(true);
            }
            else if (stringCompare(ONOFF, "off"))
            {
                preempt(false);
            }
            else
            {
                valid = false;
            }
        }

        else if (isCommand(&data, "sched", 1))
        {
            char *scheduling = getFieldString(&data, 1);
            valid = true;
            if (stringCompare(scheduling, "prio"))
            {
                sched(true);
            }
            else if (stringCompare(scheduling, "rr"))
            {
                sched(false);
            }
            else
            {
                valid = false;
            }
        }

        else if (isCommand(&data, "pidof", 1))
        {
            pidof(getFieldString(&data, 1));
            valid = true;
        }

        else if (isCommand(&data, "run", 1))
        {
            run(getFieldString(&data, 1));
            valid = true;
        }

        else if (isCommand(&data, "reboot", 0))
        {
            valid = true;
        }

        if (!valid)
            putsUart0("\rInvalid command\n");

    }

}

void ps()
{
    putsUart0("ps called.\n\r");
}

void ipcs()
{
    putsUart0("ipcs called.\n\r");
}

void kill(uint32_t pid)
{
    char pidStr[MAX_INT_STR_LENGTH + 1];
    putsUart0("Process #");
    putsUart0(integerToAlphabet(pid, pidStr));
    putsUart0(" killed.\n\r");
}

void pkill(const char proc_name[])
{
    putsUart0((char*)proc_name);
    putsUart0(" killed.\n\r");
}

void pi(bool on)
{
    on ? putsUart0("pi ON.\n\r") : putsUart0("pi OFF.\n\r");
}

void preempt(bool on)
{
    on ? putsUart0("preempt ON.\n\r") : putsUart0("preempt OFF.\n\r");
}

void sched(bool prio_on)
{
    prio_on ? putsUart0("sched prio.\n\r") : putsUart0("sched rr.\n\r");
}

void pidof(const char proc_name[])
{
    putsUart0((char*)proc_name);
    putsUart0(" launched.\n\r");
}

void run(const char proc_name[])
{
    ledState(RED, ON);
}

void reboot()
{
    putsUart0("Rebooting!\n\r");
}
