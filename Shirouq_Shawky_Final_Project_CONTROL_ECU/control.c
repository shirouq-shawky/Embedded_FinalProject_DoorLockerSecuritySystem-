/*
 * control.c
 *
 *  Created on: Nov 4, 2023
 *      Author: Shorouk Shawky
 */



#include "external_eeprom.h"
#include "uart.h"
#include "timer1.h"
#include "timer0.h"
#include "dc_motor.h"
#include "twi.h"
#include "buzzer.h"
#include "gpio.h"
#include"util/delay.h"
#include"avr/interrupt.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define OPEN_TIME 15
#define HOLDING_TIME 3
#define CLOSE_TIME 15
#define DANGER_TIME 60
#define UART_DELAY 50
#define PASS_LENGTH 5
#define EEPROM_DELAY 10
#define MAX_ERROR_TRIALS 2

/*******************************************************************************
 *                                global variables                                  *
 *******************************************************************************/
uint8 errorTrial = 0;
volatile int TIMER1_g_ticks = 0;

uint8 Password_1[5];
uint8 Password_2[5];
uint8 flag;
uint8 OptionChoosed;
uint16 eeprom_index;


/*******************************************************************************
*                      Functions prototypes                                   *
*******************************************************************************/
void createNewPass(void);
void saveNewPassEEPROM(void);
void TakeOptions(void) ;
void checkPass();
void openDoor(void) ;
void Callback(void) ;
void turnOnBuzzer(void);
void turnOnMotor(void);
void changePass();



/*******************************************************************************
*                      Functions Definitions                                   *
*******************************************************************************/

int main (void){
	sei();
	UART_ConfigType UART_configuration = {BIT_DATA_8, DISABLE_PARITY, ONE_STOP_BIT, 9600 };
	UART_init(&UART_configuration);

	TWI_BaudRate rate={TWI_F_CPU_CLOCK,2};
	TWI_ConfigType config={1,rate};
	TWI_init(&config);

	Timer0_Config Timer0_config = { FAST_PWM_MODE,NON_INVERTING_MODE, TIMER0_F_CPU_CLOCK_8 };
	Timer0_init(&Timer0_config);

	Timer1_setCallBack(Callback);
	Timer1_ConfigType Timer1_configuration ={ 0, 8000, F_CPU_CLOCK_1024, COMPARE_MODE };
	Timer1_init(&Timer1_configuration);



	DcMotor_Init();
	Buzzer_init();

	createNewPass();
	while (1) {
			TakeOptions();
		}
}



/*
 * Description :
 * Function responsible for creating new passwords
 * take 2 password from user and heck wheather they are the same or not  .
 * if the are the same, Call a function to save password in EEPROM.
 */
void createNewPass(void) {
	// get the first password
	for (int i = 0; i < PASS_LENGTH; i++) {
		Password_1[i] = UART_recieveByte();
		_delay_ms(UART_DELAY);
	}
	// get the second password
	for (int i = 0; i < PASS_LENGTH; i++) {
		Password_2[i] = UART_recieveByte();
		_delay_ms(UART_DELAY);
	}
	flag = 1;
// check the equality of the second and the first passwords
	for (int i = 0; i < PASS_LENGTH; i++) {
		if (Password_1[i] != Password_2[i]) {
			flag = 0;
		}
	}
	UART_sendByte(flag);
	if (flag == 1) {
		saveNewPassEEPROM();
	}
	else {
		createNewPass();
	}
}


/*
 * Description :
 * Function responsible for Saving the Password in the eeprom starting from location 0x0311
 * save password then increment index
 * call takeOptions function.
 */
void saveNewPassEEPROM(void) {
	eeprom_index = 0x0311;
	for (int i = 0; i < PASS_LENGTH; i++) {
		uint8 key = Password_1[i];
		EEPROM_writeByte(eeprom_index, key);
		_delay_ms(EEPROM_DELAY);
		eeprom_index++;
	}
	TakeOptions();
}
/*
 * Description :
 * Function responsible for executing the option of the user.
 * 1-open the door,call openDoor function.
 * 2-change the password, call changePass function.
 */
void TakeOptions(void) {
	OptionChoosed = UART_recieveByte();
	if (OptionChoosed == '+') {
		openDoor();
	} else if(OptionChoosed == '-') {
		changePass();
	}
}
/*
 * Description :
 * Helper Function responsible for checking the sent password to the given in the eeprom.
 */
void checkPass() {
    // to store the password entered by the user
    uint8 Password[PASS_LENGTH];

    // Variable to store the password read from EEPROM
    uint8 saved_password;

    // Initialize a flag to indicate whether the passwords match
    flag = 1;

    // Set the EEPROM index to the starting location of the password data
    eeprom_index = 0x0311;

    // Loop to receive the user's input as the entered password
    for (int i = 0; i < PASS_LENGTH; i++) {
        Password[i] = UART_recieveByte();
        _delay_ms(50);
    }

    // Loop to compare the entered password with the one stored in EEPROM
    for (int i = 0; i < PASS_LENGTH; i++) {

        EEPROM_readByte(eeprom_index, &saved_password);
        _delay_ms(EEPROM_DELAY);


        eeprom_index++;

        // Check if the saved_password doesn't match the corresponding character in the entered Password
        if (saved_password != Password[i]) {
            // If a mismatch is found, set the flag to 0 to indicate a non-matching password
            flag = 0;
            break;
        }
    }

    // Reset the error trial counter since the password was checked successfully
//    errorTrial = 0;
}

/*
 * Description:
 * Function responsible for door management. It checks the entered password, controls the door, and handles errors.
 */
void openDoor(void) {
    // Check the entered password and set the 'flag' variable to indicate the result
    checkPass();

    // Send the 'flag' value back via UART
    UART_sendByte(flag);

    // If the password matches and no previous errors, proceed to open the door
    if (flag == 1 && errorTrial == 0) {
        turnOnMotor(); // Open the door
    }
    // If the password does not match and the error trial count is less than the maximum allowed, retry
    else if (flag == 0 && errorTrial < MAX_ERROR_TRIALS) {
        errorTrial++; // Increase the error trial count
        openDoor(); // Recursive call to retry the operation
    }
    // If the error trial count exceeds the maximum allowed, sound the buzzer and reset the count
    else if ((errorTrial >= MAX_ERROR_TRIALS)&&(flag==0)) {
    	turnOnBuzzer(); // Sound the buzzer to indicate multiple errors
        errorTrial = 0; // Reset the error trial count
    }
}


/*
 * Description :
 * Function responsible for call back for timer 1 .
 */
void Callback(void) {
	TIMER1_g_ticks++;
}
/*
 * Description:
 * Function responsible for turning on the buzzer to indicate errors.
 */
void turnOnBuzzer(void) {
    TIMER1_g_ticks = 0; // Reset the timer ticks to 0

    Buzzer_on(); // Turn on the buzzer to produce sound

    // Wait for a specified duration (DANGER_TIME) while the buzzer is on
    while (TIMER1_g_ticks < DANGER_TIME); // Wait until the timer reaches the specified danger time

    Buzzer_off(); // Turn off the buzzer after the specified duration
}

/*
 * Description:
 * Function responsible for controlling the motor to perform door operations (OPEN - HOLD - CLOSE).
 */
void turnOnMotor(void) {
    TIMER1_g_ticks = 0; // Reset the timer ticks to 0

    DcMotor_Rotate(CW); // Rotate the motor in the clockwise direction (OPEN)

    // Wait for the specified duration (OPEN_TIME) while the motor is rotating in the OPEN direction
    while (TIMER1_g_ticks < OPEN_TIME);

    DcMotor_Rotate(STOP); // Stop the motor (HOLD)

    // Wait for the specified duration (HOLDING_TIME) while the door is held in place
    while (TIMER1_g_ticks < OPEN_TIME + HOLDING_TIME);

    DcMotor_Rotate(A_CW); // Rotate the motor in the anti-clockwise direction (CLOSE)

    // Wait for the specified duration (CLOSE_TIME) while the motor is rotating in the CLOSE direction
    while (TIMER1_g_ticks < OPEN_TIME + HOLDING_TIME + CLOSE_TIME);

    DcMotor_Rotate(STOP); // Stop the motor (Door is now closed)
}

/*
 * Description:
 * Function responsible for changing the password based on user input and handling error conditions.
 */
void changePass(void) {
    // Check the entered password and set the 'flag' variable to indicate the result
    checkPass();

    // Send the 'flag' value back via UART
    UART_sendByte(flag);

    // If the error trial count has reached the maximum allowed, sound the buzzer and reset the count
    if (errorTrial >= MAX_ERROR_TRIALS) {
        turnOnBuzzer(); // Sound the buzzer to indicate multiple errors
        errorTrial = 0; // Reset the error trial count
    }
    // If the error trial count is within the limit and the entered password is correct, allow password change
    else if (errorTrial ==0 && flag == 1) {
        createNewPass(); // Initiate the process to create a new password
    }
    // If the error trial count is within the limit, the entered password is incorrect, and not exceeding the limit, retry
    else if (flag == 0 && errorTrial < MAX_ERROR_TRIALS) {
        errorTrial++; // Increase the error trial count
        changePass(); // Recursive call to retry the password change
    }
}



