///*
// * pwm_timer0.c
// *
// *  Created on: Oct 14, 2023
// *      Author: Shorouk Shawky
// */
//
#include "timer0.h"
#include "timer0.h"

#include"gpio.h"
#include"common_macros.h"
#include"avr/io.h"

void Timer0_init(const Timer0_Config *Config_Ptr)
{
	TCNT0 = 0;

	GPIO_setupPinDirection(PORTB_ID, PIN3_ID, PIN_OUTPUT);

	switch (Config_Ptr->timer_mode) {
		case (NORMAL0_MODE):
			CLEAR_BIT(TCCR0, WGM00);
			CLEAR_BIT(TCCR0, WGM01);
			break;
		case COMPARE0_MODE:
			CLEAR_BIT(TCCR0, WGM00);
			SET_BIT(TCCR0, WGM01);
			break;
		case FAST_PWM_MODE:
			SET_BIT(TCCR0, WGM00);
			SET_BIT(TCCR0, WGM01);
			break;
		}
	TCCR0 = (TCCR0 & 0xf8) | (Config_Ptr->clock);
	TCCR0 = (TCCR0 & 0xcf) | (Config_Ptr->pwm_mode << 4);
}
/*
 * Description :
 *  Setup the compare value based on the required input duty cycle.
 */
void Timer0_PWM_Start(uint8 duty_cycle_percentage) {
	uint8 top = (float) (duty_cycle_percentage / 100.0) * TIMER0_MAX_VALUE;
	OCR0 = top;
}
/*
 * Description :
 * Function responsible for De_initialize the TIMER_0 driver.
 */
void TIMER0_deinit() {
	TCNT0 = OCR0 = TCCR0 = 0;
}
