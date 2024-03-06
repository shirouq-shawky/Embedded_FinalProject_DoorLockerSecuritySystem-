/*
 * timer.c
 *
 *  Created on: Nov 1, 2023
 *      Author: Shorouk Shawky
 */

#include <avr/io.h>
#include "timer1.h"
#include <avr/interrupt.h>

/********************************************************************************
 *                       Global variables                                       *
* ******************************************************************************/

/* Global variable to Point to address of callBack function in Application*/
static volatile  void(*callBack_ptr)(void) = NULL_PTR;


/* Interrupt Service Routine for timer1 compare mode */
ISR(TIMER1_COMPA_vect)
{
	if(callBack_ptr != NULL_PTR){
			(*callBack_ptr)();
		}
}

/* Interrupt Service Routine for timer1 normal mode */
ISR(TIMER1_OVF_vect)
{
	if(callBack_ptr != NULL_PTR){
				(*callBack_ptr)();
			}
}




/*
 * Description :
 * Function to initialize the Timer driver
 */
void Timer1_init(const Timer1_ConfigType * Config_Ptr){
	TCNT1=Config_Ptr->initial_value;           /* Set timer1 initial count */
	OCR1A=Config_Ptr->compare_value;           /* Set the Compare value*/

	if((Config_Ptr->mode)==0){
					/*normal mode is selected*/
			TIMSK |= (1<<TOIE1); /* Enable Timer1 normal-mode Interrupt */
						}
			else if((Config_Ptr->mode)==4) {
				/*comare mode is selected*/
				TIMSK |= (1 << OCIE1A);  /* Enable Timer1 compare-mode Interrupt */
			}

	/*
	 * FOC1A : It will be set when Timer1 Operate in non PWM Mode
	 * Set First 2-bits From Mode Member to Specify which Mode Timer1 Will Operate
	*/
	TCCR1A = (1<<FOC1A) | (1<<FOC1B) | ((TCCR1A & 0xFC) | (Config_Ptr->mode & 0x03));


	/*
		 * insert the required clock value in the first three bits (CS10, CS11 and CS12) of TCCR1B Register
		 * Set Last 2-bits From Mode Member to Specify Which Mode Timer1 Will Operate
	 */
	TCCR1B = (TCCR1B & 0xF8) | ((Config_Ptr->prescaler)& 0x07) | ((TCCR1B & 0xE7) | ((Config_Ptr->mode & 0x0C)<<1));




}

/*
 * Description :
 *  Function to disable the Timer1.
 */
void Timer1_deInit(void){
	/* Clear Timer Register */
	TCNT1 = OCR1A =TCCR1A= TCCR1B = 0;
	/* Disable Interrupt */
	TIMSK &= ~(1 << OCIE1A) &(~(1<<TOIE1));
}
/*
 * Description :
 *  Function to set the Call Back function address.
 */
void Timer1_setCallBack(void(*a_ptr)(void)){
	/* Make global variable points to same function to be called in ISR when an detection occur*/
		callBack_ptr = a_ptr;
}
