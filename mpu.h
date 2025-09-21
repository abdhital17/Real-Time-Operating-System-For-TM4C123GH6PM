#ifndef MPU_H_
#define MPU_H_

#include <stdint.h>

/* MPU Register bitfield definitions */
#define NVIC_MPU_ATTR_SIZE_4GiB                 ((0x1F) >> 1)       // SIZE field = 0b11111 for all 4GiB memory
#define NVIC_MPU_ATTR_SIZE_256KiB               ((0x11) >> 1)       // SIZE field = 0b10001 for 256KiB of flash memory
#define NVIC_MPU_ATTR_SIZE_8KiB                 ((0xC) >> 1)        // SIZE field = 0b1100 for 8KiB of SRAM
#define NVIC_MPU_ATTR_SIZE_4KiB                 ((0xB) >> 1)        // SIZE field = 0b1011 for 4KiB of SRAM
#define NVIC_MPU_ATTR_AP_RW_RW                  0x03000000          // AP = 011 for RW access in both privileged and unprivileged mode
                                                                    // execute(X) access determined by XN (bit 28) in the ATTR register
#define NVIC_MPU_ATTR_AP_RW_NONE                0x01000000          // AP = 001 for RW access in only privileged mode
                                                                    // execute(X) access determined by XN (bit 28) in the ATTR register


void initMPU();
void setBackgroundRule(void);
void allowFlashAccess(void);
void allowPeripheralAccess(void);
void setupSramAccess(void);
uint64_t createNoSramAcccessMask(void);
void applySramAccessMask(uint64_t);
void addSramAccessWindow(uint64_t*, uint32_t*, uint32_t);


#endif /* MPU_H_ */