/*
 *      Filename: isr.h
 *
 *      Created on: Sep 21, 2025
 *      Author: Abhishek Dhital
 */

#ifndef ISR_H_
#define ISR_H_

#include <stdint.h>

void busFaultISR(void);
void usageFaultISR(void);
void hardFaultISR(void);
void mpuFaultISR(void);
void pendSvISR(void);
void showStackDump(uint32_t *);
void enableFaults(void);

#endif