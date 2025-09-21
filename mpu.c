// Abhishek Dhital
// Memory Protection Unit Implementation on TM4C123GH6PM (Arm Cortex-M4)
//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
//   UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port

/* 
*                       Memory Map
                        ----------
*   On-Chip Flash - 0x0000.0000
                    ...........
                    0x0003.FFFF

*   On-Chip SRAM    0x2000.0000
                    ...........
                    0x2000.7FFF

*   Peripherals     0x4002.0000     
                    ...........
                    0x400F.FFFF     

*   Peripheral      0x4200.0000
    BitBanded alias ...........
                    0x43FF.FFFF
*/

//                  The priority of the MPU regions is higher the region number,
//                  higher will be the priority of that region
//
//                  MPU Model implemented in this code showing all regions
//  *************************************************************************************
//  *                                                                                   *
//  * -1 default region covering all of addressable 4GiB of memory                      *
//  *  priv mode    rwx access                                                          *
//  *  Unpriv mode  no access                                                           *
//  *  ___________________________________________________________________________      *
//  * |  ********************                       region #0                     |     *
//  * |  * 256 KiB          *                       priv mode- r/w access         |     *
//  * |  * Flash Memory     *                       unpriv mode - r/w access      |     *
//  * |  * region #1                                                              |     *
//  * |  * priv mode- rwx   *                                                     |     *
//  * |  *unpriv mode - rwx *                                                     |     *
//  * |  ********************       _____________________________                 |     *
//  * |                             |  31 KiB SRAM (total 32KiB) |                |     *
//  * |                             |  divided into 32 subregions|                |     *
//  * |                             |         region #2-5        |                |     *
//  * |                             |  priv mode- rw access      |                |     *
//  * |                             |  unpriv mode - no access   |                |     *
//  * |                             |                            |                |     *
//  * |                             |____________________________|                |     *
//  * |                             |      remaining 1 KiB SRAM  |                |     *
//  * |                             |      priv mode- rw access, |                |     *
//  * |                             |      unpriv mode rw access |                |     *
//  * |                             ------------------------------                |     *
//  * |                                                                           |     *
//  * |                                     2GiB                                  |     *
//  * |                                     Peripherals/Bitband                   |     *
//  * |                                                                           |     *
//  * |                                                                           |     *
//  * |                                                                           |     *
//  * |___________________________________________________________________________|     *
//  *                                                                                   *
//  *************************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "mpu.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

/*
* Function: setBackgroundRule() 
* sets a background rule for all 4GiB of addressable memory
* RW access (no X access) for both privileged and unprivileged mode for RAM, bitband and peripherals 
*/
void setBackgroundRule()
{   
    // region #0 for the background region
    NVIC_MPU_NUMBER_R  = 0x0;        // select region 0
    NVIC_MPU_BASE_R    = 0x00000000; // addr=0x0000.0000 for complete 4GiB of memory, valid=0, region already set

    // S=0, C=1, B=0, size=31(11111 for 4GiB), XN=1(instruction fetch disabled), TEX=000, AP=11 for RW in both modes
    NVIC_MPU_ATTR_R   = ~NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_CACHEABLE | ~NVIC_MPU_ATTR_BUFFRABLE | NVIC_MPU_ATTR_XN | 
                        ~NVIC_MPU_ATTR_TEX_M | (NVIC_MPU_ATTR_SIZE_4GiB << 1) | NVIC_MPU_ATTR_AP_RW_RW; 
    NVIC_MPU_ATTR_R   |= NVIC_MPU_ATTR_ENABLE;        // MPU  region enable
}

/*
* Function: allowFlashAccess() 
* sets a MPU rule for the 256KiB of flash memory
* RWX access for both privileged and unprivileged mode
*/
void allowFlashAccess(void)
{
    // region #1 for the 256kiB flash memory
    NVIC_MPU_NUMBER_R  = 0x1;        //select region 1
    NVIC_MPU_BASE_R    = 0x00000000; //addr=0x00000000 for base address of flash bits, 256KiB, valid = 0, region already set in the NUMBER register
    
    // S=0, C=1, B=0, size=17(10001) for 256KB, XN=0(instruction fetch enabled), TEX=000
    NVIC_MPU_ATTR_R    = ~NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_CACHEABLE | ~NVIC_MPU_ATTR_BUFFRABLE | ~NVIC_MPU_ATTR_XN |
                         ~NVIC_MPU_ATTR_TEX_M | (NVIC_MPU_ATTR_SIZE_256KiB << 1) | NVIC_MPU_ATTR_AP_RW_RW; 
    NVIC_MPU_ATTR_R   |= NVIC_MPU_ATTR_ENABLE;        //MPU  region enable
}

/*
* Function: allowPeripheralAccess()
* 
*/
void allowPeripheralAccess(void)
{
    /*  Implementation Not required since background rule covers it 
     *  with RW access in both privileged and unprivileged mode
    */
}


/*
* Function: setupSramAccess()
* Creates 6 MPU regions to cover 32KiB of SRAM with 8 sub-regions each
* Sub-regions created as 8K, 4K, 4K, 4K, 4K, 8K in order from the start of SRAM region
*/
void setupSramAccess(void)
{
    /*
    * we divide the 32KiB SRAM into 6 different regions in the MPU
    * the first and last of those regions in the MPU will protect 8KiB of SRAM each
    * the other 4 regions will protect 4KiB of SRAM each
    * the 8KiB within the regions are further divided into 8 subregions of 1KiB each 
    */

    /*****************************************************/
    // first region of SRAM - MPU region 2 (8KiB)
    // 0x2000.0000-0x2000.1FFF
    /*****************************************************/
    NVIC_MPU_NUMBER_R  = 0x2;       //select region 2
    NVIC_MPU_BASE_R    = 0x20000000;//addr=0x20000000,  valid=0, region already set in NUMBER register
    
    // for internal SRAM S=1, C=1, B=0, size=12(1100) for 8KiB, XN=1(instruction fetch disabled), TEX=000
    NVIC_MPU_ATTR_R    = NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_CACHEABLE | ~NVIC_MPU_ATTR_BUFFRABLE | NVIC_MPU_ATTR_XN |
                         ~NVIC_MPU_ATTR_TEX_M | (NVIC_MPU_ATTR_SIZE_8KiB << 1) | NVIC_MPU_ATTR_AP_RW_RW; 
    // MPU region 2 enabled
    NVIC_MPU_ATTR_R    |= NVIC_MPU_ATTR_ENABLE;

    /*****************************************************/
    // second region of SRAM - MPU region 3 (4KiB)
    // 0x2000.2000-0x2000.2FFF
    /*****************************************************/
    NVIC_MPU_NUMBER_R  = 0x3;       //select region 3
    NVIC_MPU_BASE_R    = 0x20002000;//addr=0x20002000,  valid=0, region already set in NUMBER register
    
    // for internal SRAM S=1, C=1, B=0, size=11(1011) for 4KiB, XN=1(instruction fetch disabled), TEX=000
    NVIC_MPU_ATTR_R    = NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_CACHEABLE | ~NVIC_MPU_ATTR_BUFFRABLE | NVIC_MPU_ATTR_XN |
                         ~NVIC_MPU_ATTR_TEX_M | (NVIC_MPU_ATTR_SIZE_4KiB << 1) | NVIC_MPU_ATTR_AP_RW_RW; 
    // MPU region 3 enabled
    NVIC_MPU_ATTR_R    |= NVIC_MPU_ATTR_ENABLE;

    /*****************************************************/
    // third region of SRAM - MPU region 4 (4KiB)
    // 0x2000.3000-0x2000.3FFF
    /*****************************************************/
    NVIC_MPU_NUMBER_R  = 0x4;       //select region 4
    NVIC_MPU_BASE_R    = 0x20003000;//addr=0x20004000,  valid=0, region already set in NUMBER register
    
    // for internal SRAM S=1, C=1, B=0, size=11(1011) for 4KiB, XN=1(instruction fetch disabled), TEX=000
    NVIC_MPU_ATTR_R    = NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_CACHEABLE | ~NVIC_MPU_ATTR_BUFFRABLE | NVIC_MPU_ATTR_XN |
                         ~NVIC_MPU_ATTR_TEX_M | (NVIC_MPU_ATTR_SIZE_4KiB << 1) | NVIC_MPU_ATTR_AP_RW_RW; 
    // MPU region 4 enabled
    NVIC_MPU_ATTR_R    |= NVIC_MPU_ATTR_ENABLE;

    /*****************************************************/
    // fourth region of SRAM - MPU region 5 (4KiB)
    // 0x2000.4000-0x2000.4FFF
    /*****************************************************/
    NVIC_MPU_NUMBER_R  = 0x5;       //select region 5
    NVIC_MPU_BASE_R    = 0x20004000;//addr=0x20004000,  valid=0, region already set in NUMBER register
    
    // for internal SRAM S=1, C=1, B=0, size=11(1011) for 4KiB, XN=1(instruction fetch disabled), TEX=000
    NVIC_MPU_ATTR_R    = NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_CACHEABLE | ~NVIC_MPU_ATTR_BUFFRABLE | NVIC_MPU_ATTR_XN |
                         ~NVIC_MPU_ATTR_TEX_M | (NVIC_MPU_ATTR_SIZE_4KiB << 1) | NVIC_MPU_ATTR_AP_RW_RW; 
    // MPU region 5 enabled
    NVIC_MPU_ATTR_R    |= NVIC_MPU_ATTR_ENABLE;

    /*****************************************************/
    // fifth region of SRAM - MPU region 6 (4KiB)
    // 0x2000.5000-0x2000.5FFF
    /*****************************************************/
    NVIC_MPU_NUMBER_R  = 0x6;       //select region 6
    NVIC_MPU_BASE_R    = 0x20005000;//addr=0x20005000,  valid=0, region already set in NUMBER register
    
    // for internal SRAM S=1, C=1, B=0, size=11(1011) for 4KiB, XN=1(instruction fetch disabled), TEX=000
    NVIC_MPU_ATTR_R    = NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_CACHEABLE | ~NVIC_MPU_ATTR_BUFFRABLE | NVIC_MPU_ATTR_XN |
                         ~NVIC_MPU_ATTR_TEX_M | (NVIC_MPU_ATTR_SIZE_4KiB << 1) | NVIC_MPU_ATTR_AP_RW_RW; 
    // MPU region 6 enabled
    NVIC_MPU_ATTR_R    |= NVIC_MPU_ATTR_ENABLE;

    /*****************************************************/
    // sixth region of SRAM - MPU region 7 (8KiB)
    // 0x2000.6000-0x2000.7FFF
    /*****************************************************/
    NVIC_MPU_NUMBER_R  = 0x7;       //select region 7
    NVIC_MPU_BASE_R    = 0x20006000;//addr=0x20006000,  valid=0, region already set in NUMBER register
    
    // for internal SRAM S=1, C=1, B=0, size=12(1100) for 8KiB, XN=1(instruction fetch disabled), TEX=000
    NVIC_MPU_ATTR_R    = NVIC_MPU_ATTR_SHAREABLE | NVIC_MPU_ATTR_CACHEABLE | ~NVIC_MPU_ATTR_BUFFRABLE | NVIC_MPU_ATTR_XN |
                         ~NVIC_MPU_ATTR_TEX_M | (NVIC_MPU_ATTR_SIZE_8KiB << 1) | NVIC_MPU_ATTR_AP_RW_RW; 
    // MPU region 7 enabled
    NVIC_MPU_ATTR_R    |= NVIC_MPU_ATTR_ENABLE;

}

uint64_t createNoSramAcccessMask(void)
{
    return 1;
}

void applySramAccessMask(uint64_t srdBitMask)      // called only in privilege mode
{

}

void addSramAccessWindow(uint64_t *srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes)
{

}

/*
* Function: initMPU()
* Wrapper for three function calls to enable MPU regions 0-7, and enable the MPU
*/
void initMPU()
{
    setBackgroundRule();        // enable MPU region #0 - background rule for all 4GiB of memory, RW access for both privileged and unprivileged mode
    allowFlashAccess();         // enable MPU region #1 - flash memory region of 256KiB starting at 0x0000.0000
    setupSramAccess();          // enable MPU regions #2-#7 - 32KiB internal SRAM divided into 6 subregions of 2 8KiB and 4 4KiB regions, all subregions disabled for now

    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_ENABLE | NVIC_MPU_CTRL_PRIVDEFEN | NVIC_MPU_CTRL_HFNMIENA;       // MPU enable, default region enable, MPU enabled during hard faults
}