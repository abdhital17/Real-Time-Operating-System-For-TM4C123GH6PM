#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdbool.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
// RTOS Shell Variables/Macro/Structures/Functions
//-----------------------------------------------------------------------------
#define MAX_CHARS 80
#define MAX_FIELDS 5
#define CARRIAGE_RETURN "\r"
#define NEWLINE "\n"
#define CARRIAGE_RETURN_AND_NEWLINE "\n\r"
#define PRINT_NEWLINE putsUart0(CARRIAGE_RETURN_AND_NEWLINE)
#define MAX_INT_STR_LENGTH 10

typedef struct _USER_DATA
{
    char buffer[MAX_CHARS + 1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

// Functions to help receive characters and parse string
void getsUart0(USER_DATA *d);
void parseFields(USER_DATA *d);
char* getFieldString(USER_DATA *data, uint8_t fieldNumber);
int32_t alphabetToInteger(char *numStr);
bool stringCompare(const char *str1, const char *str2);
int32_t getFieldInteger(USER_DATA *data, uint8_t fieldNumber);
uint32_t hexStrToInt(const char hex[]);
char* integerToAlphabet(uint32_t decInt, char* outStr);
bool isCommand(USER_DATA *data, const char strCommand[], uint8_t minArguments);

// RTOS Shell functions
void startShell(void);
void ps(void);
void ipcs(void);
void kill(uint32_t pid);
void pkill(const char proc_name[]);
void pi(bool on);
void preempt(bool on);
void sched(bool prio_on);
void pidof(const char proc_name[]);
void run(const char proc_name[]);
void reboot(void);

#endif
