/*
 * dc_motor.h
 *
 *  Created on: Oct 13, 2023
 *      Author: Shorouk Shawky
 */

#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_

#include"std_types.h"
#include "gpio.h"

/* Definitions*/
#define DC_MOTOR_PIN1_PORT_ID       PORTB_ID
#define DC_MOTOR_PIN1_ID            PIN0_ID
#define DC_MOTOR_PIN2_PORT_ID       PORTB_ID
#define DC_MOTOR_PIN2_ID            PIN1_ID

/*Types Declaration*/
typedef enum {
	STOP, CW, A_CW
}DcMotor_State;

/* Functions Prototypes     */
/*
 * Description :
 * The Function responsible for setup the direction for the two motor pins through the GPIO driver.
 * Stop at the DC-Motor at the beginning through the GPIO driver.
 *
 */

void DcMotor_Init(void);

/*
 * Description :
 * The function responsible for rotate the DC Motor CW/ or A-CW or stop the motor based on the state input state value.
 * Send the required duty cycle to the PWM driver based on the required speed value.
 */
void DcMotor_Rotate(DcMotor_State state);

#endif /* DC_MOTOR_H_ */
