/*
 * onboard_leds.h
 *
 *  Created on: Aug 30, 2025
 *      Author: Abhishek Dhital
 */

#ifndef ONBOARD_LEDS_H_
#define ONBOARD_LEDS_H_

#define RED_LED_MASK 2
#define BLUE_LED_MASK 4
#define GREEN_LED_MASK 8

#define RED_LED *((volatile uint32_t*) (0x42000000 + (0x400253FC - 0x40000000) * 32 + 1 * 4))
#define GREEN_LED *((volatile uint32_t*) (0x42000000 + (0x400253FC - 0x40000000) * 32 + 3 * 4))
#define BLUE_LED *((volatile uint32_t*) (0x42000000 + (0x400253FC - 0x40000000) * 32 + 2 * 4))

typedef enum _led_color_{RED, BLUE, GREEN} ledColor;

void initOnboardLeds();
void ledState(ledColor, uint8_t);


#endif /* ONBOARD_LEDS_H_ */
