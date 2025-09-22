/*
 *      Filename: isr.c
 *
 *      Created on: Sep 21, 2025
 *      Author: Abhishek Dhital
 */

#include "tm4c123gh6pm.h"
#include "isr.h"
#include "mpu.h"
#include "uart0.h"
#include "terminal.h"

void showStackDump(uint32_t *pspAddress)
{
    putsUart0("xPSR: ");
    printHex(*(pspAddress + 7));

    putsUart0("PC:   ");
    printHex(*(pspAddress + 6));

    putsUart0("LR:   ");
    printHex(*(pspAddress + 5));

    putsUart0("R12:  ");
    printHex(*(pspAddress + 4));

    putsUart0("R3:   ");
    printHex(*(pspAddress + 3));

    putsUart0("R2:   ");
    printHex(*(pspAddress + 2));

    putsUart0("R1:   ");
    printHex(*(pspAddress + 1));

    putsUart0("R0:   ");
    printHex(*pspAddress);
}

void busFaultISR()
{
    putsUart0("[INFO] Bus fault in process N\n\r");
    putsUart0("[INFO] Bus fault occurred when the process tried to access ");
    printHex(NVIC_FAULT_ADDR_R);
}

void usageFaultISR()
{
    putsUart0("Usage fault in process N\n\r");
    printHex(NVIC_FAULT_STAT_R);

    if (NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_DIV0)
        putsUart0("Process attempted to perform a division by 0\n\r");
}

/* 
* The Hard Fault handler
*/
void hardFaultISR()
{
    putsUart0("Hard fault in process N\n\r");
    uint32_t address;
    
    // print MSP
    address = getMSPaddress();
    putsUart0("MSP: ");
    printHex(address);
    
    // print PSP
    address = getPSPaddress();
    putsUart0("PSP: ");
    printHex(address);

    // Process Stack Dump
    showStackDump((uint32_t *) address);

    // Hard Fault Flag
    putsUart0("Hard Fault Flag: ");
    printHex(NVIC_HFAULT_STAT_R);
}

void mpuFaultISR()
{
    putsUart0("MPU fault in process N\n\r");
    uint32_t address;
    
    // print MSP
    address = getMSPaddress();
    putsUart0("MSP: ");
    printHex(address);
    
    // print PSP
    address = getPSPaddress();
    putsUart0("PSP: ");
    printHex(address);

    // Offending instruction and data address
    uint32_t *pspAddress = (uint32_t *) address;

    putsUart0("Offending instruction address: ");
    printHex(*(pspAddress + 6));

    putsUart0("Offending Data address: ");
    printHex(NVIC_MM_ADDR_R);

    putsUart0("MFAULT Flag: ");
    printHex(NVIC_FAULT_STAT_R & 0xFF);     // get the least significant byte from the FAULT STAT Register

    // Process Stack Dump
    showStackDump(pspAddress);

    // clear the MPU fault pending bit
    NVIC_SYS_HND_CTRL_R  &= ~NVIC_SYS_HND_CTRL_MEMP;

    // trigger a pendSV ISR call
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_ISR_PEND;
}

void pendSvISR()
{
    putsUart0("PendSV in process N\n\r");

    // if the DERR or IERR bits are set, it means pendsv was called by MPU Fault handler
    if ((NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_DERR) || (NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_IERR))
    {
        putsUart("Called From MPU");
        NVIC_FAULT_STAT_R |= NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR;       // clear the DERR and IERR bits by writing 1
    }
}

// enables the specific faults by setting bits in the NVIC_SYS_HND_CTRL_R
// without it, all faults are treated as a hard fault
void enableFaults()
{
    NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_USAGE;     // enable the USAGE FAULT
    NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_BUS;       // enable the BUS FAULT
    NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEM;       // enable the MEM FAULT

    // NVIC_CFG_CTRL_R |= NVIC_CFG_CTRL_DIV0 | NVIC_CFG_CTRL_UNALIGNED;        // enable traps on division by 0 and unaligned halfword and word access
}