#include "common.h"

void init_timer ()
{
    /* Timer0 to 64us
     * 16 MHz / 1024 = 64us
     */
    TCCR0A = 0;
    TCCR0B = (1<<CS02) | (1<<CS00); // Prescaler 1024
    TCNT0 = 0;

    /* Timer1 to 64us
     * 16 MHz / 1024 = 64us
     */
    TCCR1A = 0;
    TCCR1B = (1<<CS12) | (1<<CS10);
    TCNT1H = 0;
    TCNT1L = 0;
}

void stop_timer ()
{
    TCCR0B = 0;
    TCCR1B = 0;
}
