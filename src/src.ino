#include "application.h";

void setup() 
{
    new Application();
}

void loop() 
{
    aApplication->run();
}

ISR(TIMER2_COMPA_vect)
{
    aApplication->interrupt();
}

ISR (PCINT1_vect)
{
    aApplication->int2();
}

ISR (INT4_vect)
{
    aApplication->int1();
}

ISR (INT5_vect)
{
    aApplication->int1();
}
