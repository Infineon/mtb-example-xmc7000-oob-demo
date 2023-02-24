/*******************************************************************************
* File Name:   demo_pwm_sq_wave.c
*
* Description: This is the source code for the PWM Square Wave code example
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2022-2023, Cypress Semiconductor Corporation (an Infineon company) or
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


#include "cybsp.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include "print_message.h"
#include "button.h"
#include "oob_demo.h"


/*******************************************************************************
* Macros
*******************************************************************************/
/* PWM Frequency = 1, 10, 100, 1K, 10K, 100K and 1MHz */
#define PWM_FREQUENCY_1Hz    (1u)
#define PWM_FREQUENCY_10Hz   (10u)
#define PWM_FREQUENCY_100Hz  (100u)
#define PWM_FREQUENCY_1KHz   (1000u)
#define PWM_FREQUENCY_10KHz  (10000u)
#define PWM_FREQUENCY_100KHz (100000u)
#define PWM_FREQUENCY_1MKHz  (1000000u)
/* PWM Duty-cycle = 50% */
#define PWM_DUTY_CYCLE (50.0f)


/*******************************************************************************
* Global Variables
*******************************************************************************/
/* PWM object */
static cyhal_pwm_t pwm_led_control;

static uint8_t button_counter = 0;

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for the CPU. It configures the PWM and puts the CPU
* in Sleep mode to save power.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main_pwm_square_wave(void)
{
    /* API return code */
    cy_rslt_t result;

    printf("****************** Running PWM square-wave output demo ******************\r\n");
    printf("In this demo, the PWM output of 50%% duty cycle at 1 Hz generated on pin P16_2\r\n");
    printf("(also connected to USER LED2). This pin is also available on J32.10 for measurement. \r\n");
    printf("Press the USER BTN1 or USER BTN2 button to switch the PWM frequency at 1 Hz, 10 Hz, 100 Hz, \r\n");
    printf("1 kHz, 10 kHz, 100 kHz, or 1 MHz. The USER LED2 will blink depending on the selected frequency. \r\n");
    printf("\r\n");
    /*Initialize USER_BTN1*/
    button1_initial();

    /*Initialize USER_BTN2*/
    button2_initial();

    /* Initialize the PWM */
    result = cyhal_pwm_init(&pwm_led_control, CYBSP_USER_LED2, NULL);
    if(CY_RSLT_SUCCESS != result)
    {
        printf("API cyhal_pwm_init failed with error code: %lu\r\n", (unsigned long) result);
        CY_ASSERT(false);
    }

    /* In this example, PWM output is routed to the user LED2 on the kit.
        See HAL API Reference document for API details. */

    /* Set the PWM output frequency and duty cycle */
    result = cyhal_pwm_set_duty_cycle(&pwm_led_control, PWM_DUTY_CYCLE, PWM_FREQUENCY_1Hz);
    if(CY_RSLT_SUCCESS != result)
    {
        printf("API cyhal_pwm_set_duty_cycle failed with error code: %lu\r\n", (unsigned long) result);
        CY_ASSERT(false);
    }

    /* Stop current pwm */
    result = cyhal_pwm_stop(&pwm_led_control);

    /* Start the PWM */
    result = cyhal_pwm_start(&pwm_led_control);
    if(CY_RSLT_SUCCESS != result)
    {
        printf("API cyhal_pwm_start failed with error code: %lu\r\n", (unsigned long) result);
        CY_ASSERT(false);
    }

    /* Loop infinitely */
    while(!evtSwitch)
    {
        if( (button1_intr_event == true) || (button2_intr_event == true))
        {
            if(button1_Hysfilter_flag == false)
            {
                /* Hysteresis filter */
                Cy_SysLib_Delay(10u);
                button1_Hysfilter_flag = true;
            }
            else if(button2_Hysfilter_flag == false)
            {
                /* Hysteresis filter */
                Cy_SysLib_Delay(10u);
                button2_Hysfilter_flag = true;
            }
            /* wait for button release*/
            if((cyhal_gpio_read(CYBSP_USER_BTN1) == true && cyhal_gpio_read(CYBSP_USER_BTN2) == true))
            {
                if(button1_intr_event == true)
                {
                    button1_intr_event = false;
                    button1_Hysfilter_flag = false;
                    /* Enable interrupt */
                    cyhal_gpio_register_callback(CYBSP_USER_BTN1, &btn1_callback_data);
                }
                if(button2_intr_event == true)
                {
                    button2_intr_event = false;
                    button2_Hysfilter_flag = false;
                    /* Enable interrupt */
                    cyhal_gpio_register_callback(CYBSP_USER_BTN2, &btn2_callback_data);
                }
                button_counter++;
                 if(button_counter > 6)
                 {
                     button_counter = 0;
                 }
                 switch(button_counter)
                 {
                     case 0:
                     result = cyhal_pwm_set_duty_cycle(&pwm_led_control, PWM_DUTY_CYCLE, PWM_FREQUENCY_1Hz);
                     printf("PWM 1 Hz frequency is running.\r\n");
                     if(CY_RSLT_SUCCESS != result)
                     {
                         printf("API cyhal_pwm_set_duty_cycle failed with error code: %lu\r\n", (unsigned long) result);
                         CY_ASSERT(false);
                     }
                     break;

                     case 1:
                     result = cyhal_pwm_set_duty_cycle(&pwm_led_control, PWM_DUTY_CYCLE, PWM_FREQUENCY_10Hz);
                     printf("PWM 10 Hz frequency is running.\r\n");
                     if(CY_RSLT_SUCCESS != result)
                     {
                         printf("API cyhal_pwm_set_duty_cycle failed with error code: %lu\r\n", (unsigned long) result);
                         CY_ASSERT(false);
                     }
                     break;

                     case 2:
                     result = cyhal_pwm_set_duty_cycle(&pwm_led_control, PWM_DUTY_CYCLE, PWM_FREQUENCY_100Hz);
                     printf("PWM 100 Hz frequency is running.\r\n");
                     if(CY_RSLT_SUCCESS != result)
                     {
                         printf("API cyhal_pwm_set_duty_cycle failed with error code: %lu\r\n", (unsigned long) result);
                         CY_ASSERT(false);
                     }
                     break;

                     case 3:
                     result = cyhal_pwm_set_duty_cycle(&pwm_led_control, PWM_DUTY_CYCLE, PWM_FREQUENCY_1KHz);
                     printf("PWM 1 kHz frequency is running.\r\n");
                     if(CY_RSLT_SUCCESS != result)
                     {
                         printf("API cyhal_pwm_set_duty_cycle failed with error code: %lu\r\n", (unsigned long) result);
                         CY_ASSERT(false);
                     }
                     break;

                     case 4:
                     result = cyhal_pwm_set_duty_cycle(&pwm_led_control, PWM_DUTY_CYCLE, PWM_FREQUENCY_10KHz);
                     printf("PWM 10 kHz frequency is running.\r\n");
                     if(CY_RSLT_SUCCESS != result)
                     {
                         printf("API cyhal_pwm_set_duty_cycle failed with error code: %lu\r\n", (unsigned long) result);
                         CY_ASSERT(false);
                     }
                     break;
                     case 5:
                     result = cyhal_pwm_set_duty_cycle(&pwm_led_control, PWM_DUTY_CYCLE, PWM_FREQUENCY_100KHz);
                     printf("PWM 100 kHz frequency is running.\r\n");
                     if(CY_RSLT_SUCCESS != result)
                     {
                         printf("API cyhal_pwm_set_duty_cycle failed with error code: %lu\r\n", (unsigned long) result);
                         CY_ASSERT(false);
                     }
                     break;
                     case 6:
                     result = cyhal_pwm_set_duty_cycle(&pwm_led_control, PWM_DUTY_CYCLE, PWM_FREQUENCY_1MKHz);
                     printf("PWM 1 MHz frequency is running.\r\n");
                     if(CY_RSLT_SUCCESS != result)
                     {
                         printf("API cyhal_pwm_set_duty_cycle failed with error code: %lu\r\n", (unsigned long) result);
                         CY_ASSERT(false);
                     }
                     break;
                     default:
                     break;
                 }

            }
        }
    }
    /* Stop the PWM before quit this demo */
    cyhal_pwm_stop(&pwm_led_control);
    /* Un-initialize the User buttons and PWM*/
    cyhal_pwm_free(&pwm_led_control);
    cyhal_gpio_free(CYBSP_USER_BTN1);
    cyhal_gpio_free(CYBSP_USER_BTN2);
    cyhal_gpio_free(CYBSP_USER_LED2);
    button_counter = 0;
    button1_intr_event = false;
    return 0;
}


/* [] END OF FILE */
