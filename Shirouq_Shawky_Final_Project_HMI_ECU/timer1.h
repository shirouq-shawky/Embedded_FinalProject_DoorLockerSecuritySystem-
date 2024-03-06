/*
 * timer.h
 *
 *  Created on: Nov 1, 2023
 *      Author: Shorouk Shawky
 */

#ifndef TIMER1_H_
#define TIMER1_H_

#include "std_types.h"

/*******************************************************************************
 *                      Data types                                  *
 *******************************************************************************/
typedef enum {
	NORMAL_MODE, COMPARE_MODE=4
} Timer1_Mode;

typedef enum {
	NO_CLOCK,
	F_CPU_CLOCK,
	F_CPU_CLOCK_8,
	F_CPU_CLOCK_64,
	F_CPU_CLOCK_256,
	F_CPU_CLOCK_1024
} Timer1_Prescaler;

typedef struct {
 uint16 initial_value;
 uint16 compare_value; // it will be used in compare mode only.
 Timer1_Prescaler prescaler;
 Timer1_Mode mode;
} Timer1_ConfigType;


/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Function to initialize the Timer driver
 */
void Timer1_init(const Timer1_ConfigType * Config_Ptr);


/*
 * Description :
 *  Function to disable the Timer1.
 */
void Timer1_deInit(void);


/*
 * Description :
 *  Function to set the Call Back function address.
 */
void Timer1_setCallBack(void(*a_ptr)(void));


#endif /* TIMER1_H_ */
