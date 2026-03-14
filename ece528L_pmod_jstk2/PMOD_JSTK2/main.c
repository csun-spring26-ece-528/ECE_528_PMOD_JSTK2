/**
 * @file PMOD_JSTK2_main.c
 *
 * @brief Main source code for the PMOD_JSTK2 program.
 *
 * This file contains the main entry point for the PMOD_JSTK2 program,
 * which is used to demonstrate the PMOD_JSTK2 driver.
 *
 * The PMOD JSTK2 module uses the following SPI configuration:
 *  - SPI Mode 0
 *  - Chip Select Active Low
 *  - SCLK Frequency: 1 MHz
 *  - MSB First
 *
 * @author Aaron Nanas
 *
 */

#include <stdint.h>
#include <math.h>
#include "msp.h"
#include "inc/Clock.h"
#include "inc/CortexM.h"
#include "inc/GPIO.h"
#include "inc/EUSCI_A0_UART.h"
#include "inc/PMOD_JSTK2.h"

#define CENTER_OR_TRIGGER_BTN_INACTIVE      0x00
#define CENTER_BTN_ACTIVE                   0x01
#define TRIGGER_BTN_ACTIVE                  0x02
#define CENTER_AND_TRIGGER_BTN_ACTIVE       0x03

//#define XY_POSITION_10_BIT  1
#define XY_POSITION_8_BIT   1

/**
 * @brief This function selects the RGB LED color based on the status of the PMOD JSTK2 buttons.
 *
 * This function checks the status of the PMOD JSTK2 buttons and
 * sets the RGB LED color accordingly using the PMOD_JSTK2_Set_RGB_LED function.
 * - CENTER_OR_TRIGGER_BTN_INACTIVE:    RGB LED is off
 * - CENTER_BTN_ACTIVE:                 RGB LED is red
 * - TRIGGER_BTN_ACTIVE:                RGB LED is green
 * - CENTER_AND_TRIGGER_BTN_ACTIVE:     RGB LED is blue
 *
 * @param Center_Button_Status - Status of the Center Button (0x00 for not pressed, 0x01 for pressed).
 * @param Trigger_Button_Status - Status of the Trigger Button (0x00 for inactive, 0x02 for pressed).
 *
 * @return None
 */
void Select_RGB_LED(uint8_t Center_Button_Status, uint8_t Trigger_Button_Status)
{
    switch(Center_Button_Status | Trigger_Button_Status)
    {
        case CENTER_OR_TRIGGER_BTN_INACTIVE:
        {
            PMOD_JSTK2_Set_RGB_LED(0x00, 0x00, 0x00);
            break;
        }

        case CENTER_BTN_ACTIVE:
        {
            PMOD_JSTK2_Set_RGB_LED(0xFF, 0x00, 0x00);
            break;
        }

        case TRIGGER_BTN_ACTIVE:
        {
            PMOD_JSTK2_Set_RGB_LED(0x00, 0xFF, 0x00);
            break;
        }

        case CENTER_AND_TRIGGER_BTN_ACTIVE:
        {
            PMOD_JSTK2_Set_RGB_LED(0x00, 0x00, 0xFF);
            break;
        }
    }
}

int main(void)
{
    // Ensure that interrupts are disabled during initialization
    DisableInterrupts();

    // Initialize the 48 MHz Clock
    Clock_Init48MHz();

    // Initialize the built-in red LED
    LED1_Init();
    LED2_Init();

    // Initialize the buttons
    Buttons_Init();

    // Initialize EUSCI_A0_UART
    EUSCI_A0_UART_Init_Printf();

    // Initialize the PMOD JSTK2 module using EUSCI_A3_SPI
    PMOD_JSTK2_Init();

    // Enable the interrupts used by the modules
    EnableInterrupts();

    while(1)
    {
#if defined XY_POSITION_10_BIT

        PMOD_JSTK2_Get_Basic_Packet();

        X_Position_10_Bit = (rx_buffer[1] << 8) | rx_buffer[0];
        Y_Position_10_Bit = (rx_buffer[3] << 8) | rx_buffer[2];

        Center_Button_Status = (rx_buffer[4] & 0x01);
        Trigger_Button_Status = (rx_buffer[4] & 0x02);

        printf("X: %d    Y: %d    Center Button: 0x%02X    Trigger Button: 0x%02X\n", X_Position_10_Bit, Y_Position_10_Bit, Center_Button_Status, Trigger_Button_Status);

        Select_RGB_LED(Center_Button_Status, Trigger_Button_Status);

#elif defined XY_POSITION_8_BIT
    #if defined XY_POSITION_10_BIT
        #error "Only XY_POSITION_10_BIT or XY_POSITION_8_BIT can be active."
    #endif

        PMOD_JSTK2_Get_XY_Position();

        Center_Button_Status = (rx_buffer[4] & 0x01);
        Trigger_Button_Status = (rx_buffer[4] & 0x02);

        printf("X: %d    Y: %d    Center Button: 0x%02X    Trigger Button: 0x%02X\n", X_Position_8_Bit, Y_Position_8_Bit, Center_Button_Status, Trigger_Button_Status);
        PMOD_JSTK2_Set_RGB_LED(X_Position_8_Bit, 0x00, Y_Position_8_Bit);

#else
    #error "Define either one of the options: XY_POSITION_10_BIT or XY_POSITION_8_BIT"
#endif
    }
}
