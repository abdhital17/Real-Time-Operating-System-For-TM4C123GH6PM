#include <stdint.h>
#include <stdbool.h>
#include "uart0.h"
#include "onboard_leds.h"
#include "terminal.h"

int main()
{
    // initialize the UART0 module
    initUart0();
    // initialize the onboard LEDs
    initOnboardLeds();

    // start the shell
    startShell();

    return 0;
}
