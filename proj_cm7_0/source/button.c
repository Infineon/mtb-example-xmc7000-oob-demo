/*******************************************************************************
* File Name:   button.c
*
* Description: Detect the keys state by GPIO interrupt mode.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2022-2025, Cypress Semiconductor Corporation (an Infineon company) or
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

#include "button.h"
#include "oob_demo.h"

/******************************************************************************
 * Macros
 *****************************************************************************/
#define GPIO_INTERRUPT_PRIORITY (7u)
/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool button1_intr_event = false;
volatile bool button1_Hysfilter_flag = false;          // Hysteresis filter 1 flag
/* user button1 call back data */
cyhal_gpio_callback_data_t btn1_callback_data;
volatile bool button2_intr_event = false;
volatile bool button2_Hysfilter_flag = false;          // Hysteresis filter 1 flag
/* user button2 call back data */
cyhal_gpio_callback_data_t btn2_callback_data;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void button1_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);
static void button2_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);

/*******************************************************************************
* Function Name: button_initial
********************************************************************************
* Summary:
* Initial button port and callback
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void button1_initial()
{
    cy_rslt_t result;
    result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
                                CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    /* Configure GPIO interrupt */
    btn1_callback_data.callback = button1_interrupt_handler;
    cyhal_gpio_register_callback(CYBSP_USER_BTN1, &btn1_callback_data);
    cyhal_gpio_enable_event(CYBSP_USER_BTN1, CYHAL_GPIO_IRQ_FALL,
                                GPIO_INTERRUPT_PRIORITY, true);
}


/*******************************************************************************
* Function Name: button_interrupt_handler
********************************************************************************
* Summary:
*   button interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void button1_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
    /* Disable interrupt */
    cyhal_gpio_register_callback(CYBSP_USER_BTN1, NULL);
    button1_intr_event = true;
}

/*******************************************************************************
* Function Name: button_initial
********************************************************************************
* Summary:
* Initial button port and callback
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void button2_initial()
{
    cy_rslt_t result;
    result = cyhal_gpio_init(CYBSP_USER_BTN2, CYHAL_GPIO_DIR_INPUT,
                                CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    /* Configure GPIO interrupt */
    btn2_callback_data.callback = button2_interrupt_handler;
    cyhal_gpio_register_callback(CYBSP_USER_BTN2,
                                    &btn2_callback_data);
    cyhal_gpio_enable_event(CYBSP_USER_BTN2, CYHAL_GPIO_IRQ_FALL,
                                    GPIO_INTERRUPT_PRIORITY, true);
}


/*******************************************************************************
* Function Name: button2_interrupt_handler
********************************************************************************
* Summary:
*   button2 interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void button2_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
    /* Disable interrupt */
    cyhal_gpio_register_callback(CYBSP_USER_BTN2, NULL);
    button2_intr_event = true;
}
