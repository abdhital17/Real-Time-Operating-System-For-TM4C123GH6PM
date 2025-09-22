/*
 * onboard_leds.c
 *
 *  Created on: Aug 30, 2025
 *      Author: Abhishek Dhital
 */

#include <stdint.h>
#include "onboard_leds.h"
#include "tm4c123gh6pm.h"

void initOnboardLeds()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
    // Enable clock to GPIO PortF
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    // Use APB
    SYSCTL_GPIOHBCTL_R &= ~SYSCTL_GPIOHBCTL_PORTF;
    _delay_cycles(3);

    // Configure PF1,PF2,PF3 as outputs
    GPIO_PORTF_DIR_R |= RED_LED_MASK | BLUE_LED_MASK | GREEN_LED_MASK;
    // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DR2R_R |= RED_LED_MASK | BLUE_LED_MASK | GREEN_LED_MASK;;
    // Digital enable on those pins
    GPIO_PORTF_DEN_R |= RED_LED_MASK | BLUE_LED_MASK | GREEN_LED_MASK;
}

void setLED(ledColor color, ledState state)
{
    switch(color)
    {
        case RED:
            RED_LED = state == OFF ? 0:1;
            break;
        case BLUE:
            BLUE_LED = state == OFF ? 0:1;
            break;
        case GREEN:
            GREEN_LED = state == OFF ? 0:1;
            break;
        default:
            RED_LED = BLUE_LED = GREEN_LED = 0;
            break;
    }
}
