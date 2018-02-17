/*
 *
 *
 *
 *
 * Author: Feb 2018
 * Cassio Batista - cassio.batista.13
 * Federal University of Pará (UFPA). Belém, Brazil.
 */

#ifndef _PWM_CHIP_H_
#define _PWM_CHIP_H_

#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

int pwm_export(void);
int pwm_unexport(void);
void pwm_polarity(void);
int pwm_start(void);
int pwm_stop(void);
unsigned long int pwm_period(unsigned long int period);
unsigned long int pwm_duty(unsigned long int duty);

#endif /* _PWM_CHIP_H_ */
