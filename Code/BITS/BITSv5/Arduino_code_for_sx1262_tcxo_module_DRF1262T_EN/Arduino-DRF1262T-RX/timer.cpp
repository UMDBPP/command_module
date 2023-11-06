#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer1.h"
unsigned int timer1_counter = 0;
uint32_t TickCounter = 0;

void timer1_init(void)
{
      // Initializing timer1
    cli();             //disable global interrupt
    TCCR1A = 0;        //set TCCR1A to 0
    TCCR1B = 0;

    timer1_counter = 64911;
    TCNT1 = timer1_counter;
    
    TCCR1B |= (1 << CS12); //256 frequency divider
 
    // enable overflow interrupt for timer
    TIMSK1 = (1 << TOIE1);
    //enable global interrupt
    sei();
}

ISR(TIMER1_OVF_vect)
{
    TCNT1 = timer1_counter;
    TickCounter++;
}