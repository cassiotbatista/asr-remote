/*
 *
 *
 *
 *
 * Author: Feb 2018
 * Cassio Batista - cassio.batista.13
 * Federal University of Pará (UFPA). Belém, Brazil.
 */

#ifndef _SAMSUNG_H_
#define _SAMSUNG_H_

#include "pwm.h"
#include <signal.h>

#define SAMSUNG_PERIOD 26385 /* 26.3852247 us = 1/37.9 kHz (time must be in ns) */

#define SAMSUNG_INC_VOL "1111000001110000011100000000111110" /* increase volume */ 
#define SAMSUNG_DEC_VOL "1111000001110000011010000001011110" /* decrease volume */ 
#define SAMSUNG_NEXT_CH "1111000001110000001001000101101110" /* next channel */ 
#define SAMSUNG_PREV_CH "1111000001110000000001000111101110" /* previous channel */ 
#define SAMSUNG_ON_OFF  "1111000001110000001000000101111110" /* turn on/off */ 

void mark(int us);
void space(int us);
void samsung_create();
void samsung_destroy();
int samsung_send(const char* stream);
#endif /* _SAMSUNG_H_ */
