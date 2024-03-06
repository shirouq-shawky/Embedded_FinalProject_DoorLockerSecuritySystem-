/*
 * buzzer.c
 *
 *  Created on: Nov 1, 2023
 *      Author: Shorouk Shawky
 */

#include"buzzer.h"
#include"gpio.h"
#include"std_types.h"

void Buzzer_init(void)
{
	GPIO_setupPinDirection(BUZZER_PORT_ID,BUZZER_PIN_ID,PIN_OUTPUT);
	Buzzer_off();

}

/*Description:
 * Function to enable the Buzzer through the GPIO
 */
void Buzzer_on(void){
	GPIO_writePin(BUZZER_PORT_ID, BUZZER_PIN_ID, LOGIC_HIGH);


}


/*Description:
  * Function to disable the Buzzer through the GPIO
  */
void Buzzer_off(void){
	GPIO_writePin(BUZZER_PORT_ID, BUZZER_PIN_ID, LOGIC_LOW);

}
