/*
 *
 *
 *
 *
 * Author: Feb 2018
 * Cassio Batista - cassio.batista.13
 * Federal University of Pará (UFPA). Belém, Brazil.
 */

#include "samsung.h"

void
mark(int us) 
{
	pwm_duty(PERIOD/3); /* set duty period to 1/3 of the PWM period <-> 33% */
	usleep(us-150);
}

void
space(int us) 
{
	pwm_duty(PERIOD); /* set duty period to 1/1 of the PWM period <-> 100%  */
	usleep(us-150);
}

void
samsung_create()
{
	pwm_export();   /* */
	pwm_polarity(); /* */
	pwm_stop();     /* force PWM to be turned off before start */

	pwm_period(SAMSUNG_PERIOD);
}

void
samsung_destroy()
{
	pwm_stop();
	pwm_unexport(); /* TODO: necessary?? */
}

int
samsung_send(const char* stream)
{
	int i, j;

	pwm_duty(0);
	pwm_start(); /* Turn PWM on */

	for(j=0; j<1; j++) {
		mark(4500);
		space(4500);
		for(i=1; i<34; i++) {
			mark(560);            /* a bit always start with a high pulse */
			if(stream[i]=='1') {
				space(1690);
			} else {
				space(560);
			}
		}
		usleep(200000); /* 200 ms, since signal lasts 108 ms */
	}

	sleep(0.5);
	pwm_duty(0);
	pwm_stop();

	return 0;
}
