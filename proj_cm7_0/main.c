/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the XMC7000 Out of Box demo Example
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2022, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "oob_demo.h"
#include "print_message.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
********************************************************************************/

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void startup_message(void);

/*******************************************************************************
* Global Variables
********************************************************************************/
/* Demo switch flag */
bool evtSwitch = true;
/* Demo project index */
uint8_t demoIndex = 1u;
/* Hibernate reset status */
bool Hibresetstatus = false;
/* Array of demo projects */
int (*demoProject[DEMONUM])(void) = {
    main_hellowrold,
    main_pwm_square_wave,
    main_gpio_interrupt,
    main_sar_adc,
    main_powermode,
    main_qspi_memory,
    main_canfd
};


/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(void)
{
    /* Disable all interrupts. */
    __disable_irq();
    while (1)
    {
    }
}

/*******************************************************************************
* Function Name: startup_message
********************************************************************************
* Summary:
* prints demo options message on the UART debug port
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void startup_message(void)
{
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("*******************************************************************\r\n");
    printf("** KIT_XMC72_EVK - Running the out-of-the-box (OOB) demo project **\r\n");
    printf("*******************************************************************\r\n");
    printf("Enter an option from 1 - 7 to run the selected demo:\r\n");
    printf("\r\n");
    printf("\r\n");
    printf("1. Hello world\r\n");
    printf("2. PWM square-wave output\r\n");
    printf("3. GPIO interrupt\r\n");
    printf("4. SAR ADC basic\r\n");
    printf("5. XMC(TM) MCU power modes\r\n");
    printf("6. QSPI memory read/write\r\n");
    printf("7. CAN FD loopback\r\n");
    printf("For more projects visit our code examples repositories:\r\n\n");
    printf("https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software\r\n");
    printf("For detailed steps refer to the README document\r\n\n");
    printf("\r\n");
    printf("\r\n");
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* The main function performs the following actions:
*  1. Initial UART component.
*  2. Show 7 demos navigation interfaces on the UART serial terminal.
*  3. Enter the "Hello world" demo (default demo) automatically
*  4. You can enter 1~7 key for change the demos
*
* Parameters:
*  None
*
* Return:
*  int
*
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize UART port */
    uart_port_initial();

    /* Check the reset reason */
    if(CY_SYSLIB_RESET_HIB_WAKEUP == (Cy_SysLib_GetResetReason() & CY_SYSLIB_RESET_HIB_WAKEUP))
    {
        /*If system reset from Hibernate status, it will return back to power modes demo */
        demoIndex = 5u;
        Hibresetstatus = true;
    }



    for (;;)
    {
        if(evtSwitch)
        {
            evtSwitch = false;
            startup_message();
            (*demoProject[demoIndex-1])();
        }
        else
        {
            /* Will not run here */
        }
    }
}

/* [] END OF FILE */
