/*
 * HMI.c
 *
 *  Created on: Nov 4, 2023
 *      Author: Shorouk Shawky
 */
#include "uart.h"
#include "lcd.h"
#include "keypad.h"
#include "timer1.h"
#include "common_macros.h"
#include"util/delay.h"
#include"avr/interrupt.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define PASS_LENGTH  5
#define OPEN_TIME 15
#define HOLDING_TIME 3
#define CLOSE_TIME 15
#define DANGER_TIME 60
#define ENTER_BUTTON 13
#define NORMAL_DELAY 600
#define KEY_DELAY    400
#define UART_DELAY  50
#define MAX_ERROR_TRIALS 2


/****************************************************************
*                            Global Variables
****************************************************************/
uint8 Password_1[5];
uint8 Password_2[5];
uint8 flag;
uint8 i;
uint8 key;

uint8 errorTrial = 0;
volatile int TIMER1_g_ticks = 0;

/*******************************************************************************
*                      Functions prototypes                                   *
*******************************************************************************/
void createNewPass(void);
void checkPass(void);
void changePass(void);
void showOptions(void);
void openDoor(void);
void turnOnBuzzer(void);
void Callback(void);
void turnOnMotor(void);
/****************************************************************
*                            functions definitions
****************************************************************/

int main (void){
	sei();
	LCD_init();
	UART_ConfigType uart_configuration = {BIT_DATA_8, DISABLE_PARITY, ONE_STOP_BIT, 9600 };
	UART_init(&uart_configuration);

	Timer1_setCallBack(Callback);
	Timer1_ConfigType Timer1_configuration = { 0, 8000, F_CPU_CLOCK_1024, COMPARE_MODE };
	Timer1_init(&Timer1_configuration);

	createNewPass();
	while(1){
		showOptions();
	}
}

/*
 * Description:
 * Function responsible for creating new passwords based on user input and confirming the new password.
 */
void createNewPass(void) {
    LCD_clearScreen();
    LCD_displayString("Plz Enter Pass:");
    LCD_moveCursor(1, 0);

     i = key = 0;
    flag = 1;

    // Loop to receive the user's input for the new password
    for (i = 0; i < PASS_LENGTH; i++) {
        key = KEYPAD_getPressedKey();

        // Check if the key pressed is a valid numeric key (0-9)
        if (key >= 0 && key <= 9) {
            LCD_displayCharacter('*'); // Display an asterisk to mask the input
            Password_1[i] = key; // Store the entered digit in the password array
            _delay_ms(KEY_DELAY); // Delay for stability
        }
    }
    // Wait for the user to press the "Enter" button on the keypad
    while (KEYPAD_getPressedKey() != ENTER_BUTTON);

     // Send the first part of the new password via UART
    for (i = 0; i < PASS_LENGTH; i++) {
        UART_sendByte(Password_1[i]);
        _delay_ms(UART_DELAY);
    }


    // Clear the LCD and prompt the user to re-enter the password
    LCD_clearScreen();
    LCD_displayString("Plz reEnter Pass:");
    LCD_moveCursor(1, 0);

    // Loop to receive the user's confirmation of the new password
    for (i = 0; i < PASS_LENGTH; i++) {
        key = KEYPAD_getPressedKey();

        // Check if the key pressed is a valid numeric key (0-9)
        if (key >= 0 && key <= 9) {
            LCD_displayCharacter('*'); // Display an asterisk to mask the input
            Password_2[i] = key; // Store the entered digit in the confirmation password array
            _delay_ms(KEY_DELAY); // Delay for stability
        }
    }

    // Wait for the user to press the "Enter" button on the keypad
    while (KEYPAD_getPressedKey() != ENTER_BUTTON);

    // Send the confirmation part of the new password via UART
    for (i = 0; i < PASS_LENGTH; i++) {
        UART_sendByte(Password_2[i]);
        _delay_ms(UART_DELAY);
    }

    // Clear the LCD and receive a flag via UART to indicate if the passwords match
    LCD_clearScreen();
    flag = UART_recieveByte();

    // Check the flag to determine if the passwords match
    if (flag == 1) {
        LCD_displayString("Matching....");
        _delay_ms(NORMAL_DELAY);
    } else {
        LCD_displayString("Not Matching!");
        _delay_ms(NORMAL_DELAY);
        createNewPass(); // Retry password creation in case of non-matching passwords
    }
}



/*
 * Description:
 * Helper Function responsible for receiving and sending a password via UART and the keypad.
 * It allows the user to enter a password, masking the input with asterisks.
 */
void checkPass(void) {
    LCD_clearScreen();
    uint8 Password[PASS_LENGTH];
    LCD_displayString("Plz Enter Pass:");
    LCD_moveCursor(1, 0);

    i = key = 0;

    // Loop to receive the user's input as the entered password
    while (i < PASS_LENGTH) {
        key = KEYPAD_getPressedKey();

        // Check if the key pressed is a valid numeric key (0-9)
        if (key >= 0 && key <= 9) {
            LCD_displayCharacter('*'); // Display an asterisk to mask the input
            Password[i] = key; // Store the entered digit in the password array
            _delay_ms(KEY_DELAY); // Delay for stability
            i++;
        }
    }

    // Wait for the user to press the "Enter" button on the keypad
    while (KEYPAD_getPressedKey() != ENTER_BUTTON);

    // Send the entered password via UART
    for (i = 0; i < PASS_LENGTH; i++) {
        UART_sendByte(Password[i]);
        _delay_ms(UART_DELAY);
    }
}

/*
 * Description:
 * Function responsible for managing the process of changing the password.
 * It checks the entered password, handles errors, and initiates the creation of a new password.
 */
void changePass(void) {
    checkPass(); // Check the entered password and send it via UART
    flag = UART_recieveByte(); // Receive a flag via UART

    LCD_clearScreen();

    // Check the flag to determine the next actions
    if (flag == 1) {
        LCD_displayString("Correct pass");
        _delay_ms(NORMAL_DELAY);
        createNewPass(); // Proceed to create a new password
    } else if ((flag == 0 )&&( errorTrial < MAX_ERROR_TRIALS)) {
        errorTrial++;
        LCD_displayString("Not Correct!");
        _delay_ms(NORMAL_DELAY);
        changePass(); // Retry the password change with limited error trials
    } else if((errorTrial>= MAX_ERROR_TRIALS)&&(flag==0)) {
        turnOnBuzzer(); // Sound the buzzer in case of too many errors
        errorTrial = 0; // Reset the error trial count
    }
}

/*
 * Description:
 * Function responsible for displaying options to the user and handling their choice.
 * 1 - Open the door.
 * 2 - Change the password.
 */
void showOptions(void) {
    LCD_clearScreen();
    LCD_moveCursor(0, 0);
    LCD_displayString("+ : Open Door");
    LCD_moveCursor(1, 0);
    LCD_displayString("- : Change Pass");
    key = KEYPAD_getPressedKey();

    // Check if the user's choice is valid (+ or -)
    if (key == '-') {
        UART_sendByte(key); // Send the user's choice via UART
        changePass(); // Initiate the password change process
        }
    else if (key == '+') {
    	UART_sendByte(key); // Send the user's choice via UART
        openDoor(); // Initiate the process to open the door
        }
    else {
        LCD_clearScreen();
        LCD_displayString("Enter Valid Key");
        _delay_ms(NORMAL_DELAY);
        showOptions(); // Retry the option selection if an invalid key is pressed
    }
}


/*
 * Description:
 * Function responsible for managing the opening of the door.
 * It checks the entered password, processes the result, and controls the door.
 */
void openDoor(void) {
    checkPass(); // Check the entered password
    flag = UART_recieveByte(); // Receive a flag via UART

    LCD_clearScreen();

    // Check the flag to determine the next actions
    if (flag == 1 && errorTrial == 0) {
        turnOnMotor(); // Open the door if the entered password is correct
    } else if (flag == 0 && errorTrial < MAX_ERROR_TRIALS) {
        errorTrial++;
        LCD_displayString("Not Correct!");
        _delay_ms(NORMAL_DELAY);
        openDoor(); // Retry the door opening with limited error trials
    } else if(errorTrial >= MAX_ERROR_TRIALS){
        turnOnBuzzer(); // Sound the buzzer in case of too many errors
        errorTrial = 0; // Reset the error trial count
    }
}

/*
 * Description:
 * Function responsible for turning on the buzzer in case of errors.
 * It waits for a specified duration to indicate an error state.
 */
void turnOnBuzzer(void) {
    TIMER1_g_ticks = 0; // Reset the timer ticks to 0
    LCD_clearScreen();
    LCD_displayString("Error !!!");

    // Wait for a specified duration (DangerTime) while indicating an error state
    while (TIMER1_g_ticks < DANGER_TIME);
}

/*
 * Description:
 * Callback function for timer 1.
 * It increments the timer ticks for timekeeping purposes.
 */
void Callback(void) {
    TIMER1_g_ticks++; // Increment the timer ticks
}

/*
 * Description:
 * Function responsible for managing the state of the door.
 * It controls the door's actions, including unlocking, holding, and locking.
 */
void turnOnMotor(void) {
    TIMER1_g_ticks = 0; // Reset the timer ticks to 0
    LCD_clearScreen();

    LCD_displayString("Door Un-locking");

    // Wait for a specified duration (OpenTime) while unlocking the door
    while (TIMER1_g_ticks < OPEN_TIME);

    LCD_clearScreen();
    LCD_displayString("Holding");

    // Wait for a specified duration (HoldingTime) while holding the door open
    while (TIMER1_g_ticks < OPEN_TIME + HOLDING_TIME);

    LCD_clearScreen();
    LCD_displayString("Door Locking");

    // Wait for a specified duration (CloseTime) while locking the door
    while (TIMER1_g_ticks < OPEN_TIME + HOLDING_TIME+ CLOSE_TIME);
}


