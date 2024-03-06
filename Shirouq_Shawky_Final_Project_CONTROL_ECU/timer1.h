/*
 * timer.h
 *
 *  Created on: Nov 1, 2023
 *      Author: Shorouk Shawky
 */

#ifndef TIMER1_H_
#define TIMER1_H_

#include "std_types.h"

/****************************************************************
*                        Definitions                            *
****************************************************************/
//#define TIMER1_Compare_Mode

/*******************************************************************************
 *                      Data types  Declaration                                 *
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

//#ifndef TIMER1_Compare_Mode

//typedef struct{
//	uint16 Initial_value;
//	Timer1_Prescaler Prescaler;
//	Timer1_Mode mode;
//}Timer1_ConfigType;

//#else
typedef struct {
 uint16 initial_value;
 uint16 compare_value;
 Timer1_Prescaler prescaler;
 Timer1_Mode mode;
} Timer1_ConfigType;
//#endif

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

//#ifndef TIMER1_Compare_Mode
///*
// * Description :
// * Function responsible for initialize the TIMER_1 driver in normal mode.
// */
//void TIMER1_init_Normal(Timer1_ConfigType* Config_ptr);
//#else

/*
 * Description :
 * Function to initialize the Timer1 driver  in compare mode.
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
