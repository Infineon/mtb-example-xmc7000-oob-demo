/*******************************************************************************
* File Name:   demo_helloworld.c
*
* Description: This is the source code for the XMC7000 MCU: Hello World Example
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "button.h"
#include "oob_demo.h"
#include "print_message.h"


/*******************************************************************************
* Macros
*******************************************************************************/
/* LED blink timer clock value in Hz  */
#define LED_BLINK_TIMER_CLOCK_HZ          (10000)

/* LED blink timer period value */
#define LED_BLINK_TIMER_PERIOD            (9999)


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void timer_init(void);
static void isr_timer(void *callback_arg, cyhal_timer_event_t event);
static void led_statemanagement(uint8_t sta);

/*******************************************************************************
* Global Variables
*******************************************************************************/
bool timer_interrupt_flag = false;

bool led_blink_active_flag = true;

/* Variable for storing character read from terminal */
uint8_t uart_read_value;

/* Timer object used for blinking the LED */
cyhal_timer_t led_blink_timer;

/* counter object used for blinking the LED state */
uint8_t led_statecounter = 0;

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM4 CPU. It sets up a timer to trigger a 
* periodic interrupt. The main while loop checks for the status of a flag set 
* by the interrupt and toggles an LED at 1Hz to create an LED blinky. The 
* while loop also checks whether the 'BTN1' key was pressed and
* stops/restarts LED blinking.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main_hellowrold(void)
{

    printf("****************** Running Hello world demo ******************\r\n");
    printf("Hello World!!!\r\n");
    printf("Press the Enter key to pause or resume blinking the user LED. Alternatively, \r\n");
    printf("use either USER BTN1 or USER BTN2 to pause or resume the blinking.\r\n");
    printf("\r\n");
    /* Initialize the User LED */
    cyhal_gpio_init(CYBSP_USER_LED1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    cyhal_gpio_init(CYBSP_USER_LED2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
#if defined(KIT_XMC72) || defined(KIT_T2GBH)
    cyhal_gpio_init(CYBSP_USER_LED3, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
#endif

    /*Initialize GPIO connected to USER BTN1*/
    button1_initial();

    /*Initialize GPIO connected to USER BTN2*/
    button2_initial();

    /* Initialize timer to toggle the LED */
    timer_init();

    while(!evtSwitch)
    {
        /* Check if either BTN1 or BTN2 key pressed or receive 'Enter key' */
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
                /* Pause LED blinking by stopping the timer */
                if (led_blink_active_flag)
                {
                    led_blink_active_flag = false;
                    cyhal_timer_stop(&led_blink_timer);
                    printf("LED blinking paused \r\n");
                }
                else /* Resume LED blinking by starting the timer */
                {
                    led_blink_active_flag = true;
                    cyhal_timer_start(&led_blink_timer);
                    printf("LED blinking resumed \r\n");
                }

            }
        }
        else if(recCmd == 0x0D)
        {
            recCmd = 0xff;
            /* Pause LED blinking by stopping the timer */
            if (led_blink_active_flag)
            {
                led_blink_active_flag = false;
                cyhal_timer_stop(&led_blink_timer);
                printf("LED blinking paused \r\n");
            }
            else /* Resume LED blinking by starting the timer */
            {
                led_blink_active_flag = true;
                cyhal_timer_start(&led_blink_timer);
                printf("LED blinking resumed \r\n");
            }
        }
        /* Check if timer elapsed (interrupt fired) and toggle the LED */
        if (timer_interrupt_flag)
        {
            /* Clear the flag */
            timer_interrupt_flag = false;
            led_statecounter++;
            #if defined(KIT_XMC72) || defined(KIT_T2GBH)
            if(led_statecounter>4)
            {
                led_statecounter = 0;
            }
            #else
            if(led_statecounter>3)
            {
                led_statecounter = 0;
            }
            #endif
            /* switch the USER LED state */
            led_statemanagement(led_statecounter);
        }
    }
    /* Un-initialize the User LEDs, timer and button */
    cyhal_timer_free(&led_blink_timer);
    cyhal_gpio_free(CYBSP_USER_BTN1);
    cyhal_gpio_free(CYBSP_USER_BTN2);
    cyhal_gpio_free(CYBSP_USER_LED1);
    cyhal_gpio_free(CYBSP_USER_LED2);
#if defined(KIT_XMC72) || defined(KIT_T2GBH)
    cyhal_gpio_free(CYBSP_USER_LED3);
# endif
    led_statecounter = 0;

    return 0;
}


/*******************************************************************************
* Function Name: timer_init
********************************************************************************
* Summary:
* This function creates and configures a Timer object. The timer ticks 
* continuously and produces a periodic interrupt on every terminal count 
* event. The period is defined by the 'period' and 'compare_value' of the 
* timer configuration structure 'led_blink_timer_cfg'. Without any changes, 
* this application is designed to produce an interrupt every 1 second.
*
* Parameters:
*  none
*
*******************************************************************************/
void timer_init(void)
{
    cy_rslt_t result;

    const cyhal_timer_cfg_t led_blink_timer_cfg = 
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = LED_BLINK_TIMER_PERIOD,   /* Defines the timer period */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = true,              /* Run timer indefinitely */
        .value = 0                          /* Initial value of counter */
    };

    /* Initialize the timer object. Does not use input pin ('pin' is NC) and
     * does not use a pre-configured clock source ('clk' is NULL). */
    result = cyhal_timer_init(&led_blink_timer, NC, NULL);

    /* timer init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Configure timer period and operation mode such as count direction, 
       duration */
    cyhal_timer_configure(&led_blink_timer, &led_blink_timer_cfg);

    /* Set the frequency of timer's clock source */
    cyhal_timer_set_frequency(&led_blink_timer, LED_BLINK_TIMER_CLOCK_HZ);

    /* Assign the ISR to execute on timer interrupt */
    cyhal_timer_register_callback(&led_blink_timer, isr_timer, NULL);

    /* Set the event on which timer interrupt occurs and enable it */
    cyhal_timer_enable_event(&led_blink_timer, CYHAL_TIMER_IRQ_TERMINAL_COUNT,
                              7, true);

    /* Start the timer with the configured settings */
    cyhal_timer_start(&led_blink_timer);
 }


/*******************************************************************************
* Function Name: isr_timer
********************************************************************************
* Summary:
* This is the interrupt handler function for the timer interrupt.
*
* Parameters:
*    callback_arg    Arguments passed to the interrupt callback
*    event            Timer/counter interrupt triggers
*
*******************************************************************************/
static void isr_timer(void *callback_arg, cyhal_timer_event_t event)
{
    (void) callback_arg;
    (void) event;

    /* Set the interrupt flag and process it from the main while(1) loop */
    timer_interrupt_flag = true;
}

/*******************************************************************************
* Function Name: led_statemanagement
********************************************************************************
* Summary:
* This function is for switching LEDs states.
*
* Parameters:
*  sta: Next states of LEDs
*
* Return:
*  none
*
*******************************************************************************/
static void led_statemanagement(uint8_t sta)
{
#if defined(KIT_XMC72) || defined(KIT_T2GBH)
    /* array contains three LEDs display status 000 - 001 - 010 - 100 - 111 */
    uint8_t array[] = {0,1,2,4,7};
# else
    /* array contains two LEDs display status 000 - 001 - 010 - 011 */
    uint8_t array[] = {0,1,2,3};
# endif
    /* Turn on/off the LED depending upon the bit set/cleared*/
    cyhal_gpio_write(CYBSP_USER_LED1, (array[sta]&0x01) ? LED_ON : LED_OFF);
    cyhal_gpio_write(CYBSP_USER_LED2, (array[sta]&0x02) ? LED_ON : LED_OFF);
#if defined(KIT_XMC72) || defined(KIT_T2GBH)
    cyhal_gpio_write(CYBSP_USER_LED3, (array[sta]&0x04) ? LED_ON : LED_OFF);
#endif

}
/* [] END OF FILE */
