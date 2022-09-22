/*******************************************************************************
* File Name:   demo_gpio_interrupt.c
*
* Description: This code example demonstrates the use of GPIO configured as an
*              input pin to generate interrupts
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

#include "cy_retarget_io.h"
#include "cyhal.h"
#include "cybsp.h"
#include "print_message.h"
#include "oob_demo.h"


/******************************************************************************
 * Macros
 *****************************************************************************/
#define GPIO_INTERRUPT_PRIORITY   (7u)


/*******************************************************************************
* Function Prototypes
********************************************************************************/
static void gpio1_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);
static void gpio2_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);

/*******************************************************************************
* Global Variables
********************************************************************************/
volatile bool gpio1_intr_flag = false;               // Button 1 interrupt flag
volatile bool gpio1_Hysfilter_flag = false;          // Hysteresis filter 1 flag
cyhal_gpio_callback_data_t gpio1_btn_callback_data;  // user button1 call back data

volatile bool gpio2_intr_flag = false;               // Button 2 interrupt flag
volatile bool gpio2_Hysfilter_flag = false;          // Hysteresis filter 2 flag
cyhal_gpio_callback_data_t gpio2_btn_callback_data;  // user button2 call back data

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function configures and initializes the GPIO
*  interrupt, Press BTN1 for turn off LEDs and Press BTN2 for turn on LEDs.
*
* Return: int
*
*******************************************************************************/
int main_gpio_interrupt(void)
{
    cy_rslt_t result;


    printf("****************** Running GPIO interrupt demo ******************\r\n");
    printf("GPIO Interrupt demo started successfully. \r\n");
    printf("Press the USER BTN1 button to turn off USER LEDs and press the USER BTN2 button to turn on USER LEDs. \r\n");
    printf("\r\n");
    /* Initialize the User LED */
    cyhal_gpio_init(CYBSP_USER_LED1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    cyhal_gpio_init(CYBSP_USER_LED2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    cyhal_gpio_init(CYBSP_USER_LED3, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    /* Initialize the user button */
    result = cyhal_gpio_init(CYBSP_USER_BTN1, CYHAL_GPIO_DIR_INPUT,
                                    CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    result = cyhal_gpio_init(CYBSP_USER_BTN2, CYHAL_GPIO_DIR_INPUT,
                                    CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Configure GPIO1 interrupt */
    gpio1_btn_callback_data.callback = gpio1_interrupt_handler;
    cyhal_gpio_register_callback(CYBSP_USER_BTN1, &gpio1_btn_callback_data);
    cyhal_gpio_enable_event(CYBSP_USER_BTN1, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);
    /* Configure GPIO interrupt */
    gpio2_btn_callback_data.callback = gpio2_interrupt_handler;
    cyhal_gpio_register_callback(CYBSP_USER_BTN2, &gpio2_btn_callback_data);
    cyhal_gpio_enable_event(CYBSP_USER_BTN2, CYHAL_GPIO_IRQ_FALL, GPIO_INTERRUPT_PRIORITY, true);

    /* Turn on LEDs by system default status */
    gpio2_intr_flag = true;

    while(!evtSwitch)
    {
        /* Check the interrupt status */
        if (true == gpio1_intr_flag)
        {
            if(gpio1_Hysfilter_flag == false)
            {
                /* Hysteresis filter */
                Cy_SysLib_Delay(10u);
                gpio1_Hysfilter_flag = true;
            }
            /* wait for button release*/
            if(cyhal_gpio_read(CYBSP_USER_BTN1) == true)
            {

                gpio1_intr_flag = false;
                gpio1_Hysfilter_flag = false;
                /* Enable interrupt */
                cyhal_gpio_register_callback(CYBSP_USER_BTN1, &gpio1_btn_callback_data);
                cyhal_gpio_write(CYBSP_USER_LED1, LED_OFF);
                cyhal_gpio_write(CYBSP_USER_LED2, LED_OFF);
                cyhal_gpio_write(CYBSP_USER_LED3, LED_OFF);
                printf("All the three USER LEDs turned OFF\r\n");
            }
        }
        if (true == gpio2_intr_flag)
        {

            if(gpio2_Hysfilter_flag == false)
            {
                /* Hysteresis filter */
                Cy_SysLib_Delay(10u);
                gpio2_Hysfilter_flag = true;
            }
            /* wait for button release*/
            if(cyhal_gpio_read(CYBSP_USER_BTN2) == true)
            {
                gpio2_intr_flag = false;
                gpio2_Hysfilter_flag = true;
                /* Enable interrupt */
                cyhal_gpio_register_callback(CYBSP_USER_BTN2, &gpio2_btn_callback_data);
                cyhal_gpio_write(CYBSP_USER_LED1, LED_ON);
                cyhal_gpio_write(CYBSP_USER_LED2, LED_ON);
                cyhal_gpio_write(CYBSP_USER_LED3, LED_ON);
                printf("All the three USER LEDs turned ON\r\n");
            }

        }
    }

    cyhal_gpio_free(CYBSP_USER_BTN1);
    cyhal_gpio_free(CYBSP_USER_BTN2);
    cyhal_gpio_free(CYBSP_USER_LED1);
    cyhal_gpio_free(CYBSP_USER_LED2);
    cyhal_gpio_free(CYBSP_USER_LED3);

    return 0;
}


/*******************************************************************************
* Function Name: gpio1_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void gpio1_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
    /* Disable interrupt */
    cyhal_gpio_register_callback(CYBSP_USER_BTN1, NULL);
    gpio1_intr_flag = true;

}
/*******************************************************************************
* Function Name: gpio2_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void gpio2_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
    /* Disable interrupt */
    cyhal_gpio_register_callback(CYBSP_USER_BTN2, NULL);
    gpio2_intr_flag = true;

}
/* [] END OF FILE */
