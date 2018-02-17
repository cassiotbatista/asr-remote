/*
 *
 *
 *
 *
 * Author: Feb 2018
 * Cassio Batista - cassio.batista.13
 * Federal University of Pará (UFPA). Belém, Brazil.
 */

#include "pwm_chip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>

#define PWM_DIR "/sys/class/pwm/pwmchip0/"

int
pwm_export(void)
{
	int fd, len;
	char buf[MAX_BUF];
	fd = open(PWM_DIR "export", O_WRONLY);
	if(fd < 0){
		perror("pwmchip0/export");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", 0);
	write(fd, buf, len);
	close(fd);
	return 0;
}

int
pwm_unexport(void)
{
	int fd, len;
	char buf[MAX_BUF];
	fd = open(PWM_DIR "unexport", O_WRONLY);
	if(fd < 0){
		perror("pwmchip0/unexport");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", 0);
	write(fd, buf, len);
	close(fd);
	return 0;
}

void
pwm_polarity()
{
	int fd, len;
	char buf[MAX_BUF];
	fd = open(PWM_DIR "pwm0/polarity", O_WRONLY);
	if(fd < 0){
		perror("pwmchip0/pwm0/polarity");
	}
	
	len = snprintf(buf, sizeof(buf), "%s", "normal");
	write(fd, buf, len);
	close(fd);
}

int
pwm_start(void)
{
	int fd, len;
	char buf[MAX_BUF];
	fd = open(PWM_DIR "pwm0/enable", O_WRONLY);
	if(fd < 0){
		perror("pwmchip0/pwm0/enable");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", 1);
	write(fd, buf, len);
	close(fd);
	return 0;
}

int
pwm_stop(void)
{
	int fd, len;
	char buf[MAX_BUF];
	fd = open(PWM_DIR "pwm0/enable", O_WRONLY);
	if(fd < 0){
		perror("pwmchip0/pwm0/enable");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", 0);
	write(fd, buf, len);
	close(fd);
	return 0;
}

unsigned long int
pwm_period(unsigned long int period)
{
	int fd, len;
	char buf[MAX_BUF];
	fd = open(PWM_DIR "pwm0/period", O_WRONLY);
	if(fd < 0){
		perror("pwmchip0/pwm0/period");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%lu", period);
	write(fd, buf, len);
	close(fd);
	return 0;
}

unsigned long int 
pwm_duty(unsigned long int duty)
{
	int fd, len;
	char buf[MAX_BUF];
	fd = open(PWM_DIR "pwm0/duty_cycle", O_WRONLY);
	if(fd < 0){
		perror("pwmchip0/pwm0/duty_cycle");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%lu", duty);
	write(fd, buf, len);
	close(fd);
	return 0;
}
