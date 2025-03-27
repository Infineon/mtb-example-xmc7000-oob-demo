/******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the XMC7000 MCU SAR ADC example 
* for ModusToolbox.
*
* Related Document: See README.md
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


/******************************************************************************
* Include header files
******************************************************************************/
#include <stdio.h>
#include "cy_pdl.h"
#include "cycfg.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "print_message.h"
#include "button.h"
#include "oob_demo.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Channel 0 input pin */
#define VPLUS_CHANNEL_0             CYBSP_POT


/*******************************************************************************
*       Enumerated Types
*******************************************************************************/


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/* Single channel initialization function*/
void adc_single_channel_init(void);

/* Function to read input voltage from channel 0 */
void adc_single_channel_process(void);

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* ADC Object */
cyhal_adc_t adc_obj;

/* ADC Channel 0 Object */
cyhal_adc_channel_t adc_chan_0_obj;

/* Default ADC configuration */
const cyhal_adc_config_t adc_config = {
        .resolution = 12u,
        .average_count = 1u,
        .average_mode_flags = 0u,
        .continuous_scanning = false,
        .vneg = CYHAL_ADC_VNEG_VSSA,
        .vref = CYHAL_ADC_REF_INTERNAL,
        .ext_vref = NC,
        .ext_vref_mv = 0,
        .is_bypassed = false,
        .bypass_pin = NC, };       /* No connection */

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:

*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main_sar_adc(void)
{
    /* Variable to capture return value of functions */
    cy_rslt_t result;

    /* Print message */
    printf("****************** Running SAR ADC basic demo ******************\r\n");
    printf("In this demo, the ADC is configured in single channel configuration. \r\n");
    printf("Rotate the potentiometer and observe the ADC input voltage change. \r\n");
    printf("\r\n");

    /* Initialize Channel 0 */
    adc_single_channel_init();

    /* Update ADC configuration */
    result = cyhal_adc_configure(&adc_obj, &adc_config);
    if(result != CY_RSLT_SUCCESS)
    {
        printf("ADC configuration update failed. Error: %ld\n", (long unsigned int)result);
        CY_ASSERT(0);
    }

    while(!evtSwitch)
    {
        /* Sample input voltage at channel 0 */
        adc_single_channel_process();

        /* 200ms delay between scans */
        cyhal_system_delay_ms(200);
    }
    cyhal_adc_free(&adc_obj);
    cyhal_gpio_free(CYBSP_POT);
    return 0;
}
/*******************************************************************************
 * Function Name: adc_single_channel_init
 *******************************************************************************
 *
 * Summary:
 *  ADC single channel initialization function. This function initializes and
 *  configures channel 0 of ADC.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void adc_single_channel_init(void)
{
    /* Variable to capture return value of functions */
    cy_rslt_t result;

    /* Initialize ADC. The ADC block which can connect to pin 6[6] is selected */
    result = cyhal_adc_init(&adc_obj, VPLUS_CHANNEL_0, NULL);
    if(result != CY_RSLT_SUCCESS)
    {
        printf("ADC initialization failed. Error: %ld\n", (long unsigned int)result);
        CY_ASSERT(0);
    }

    /* ADC channel configuration */
    const cyhal_adc_channel_config_t channel_config = {
            .enable_averaging = false,  /* Disable averaging for channel */
            .min_acquisition_ns = 1000, /* Minimum acquisition time set to 1us */
            .enabled = true };          /* Sample this channel when ADC performs a scan */

    /* Initialize a channel 0 and configure it to scan P6_6 in single ended mode. */
    result  = cyhal_adc_channel_init_diff(&adc_chan_0_obj, &adc_obj, VPLUS_CHANNEL_0,
                                          CYHAL_ADC_VNEG, &channel_config);
    if(result != CY_RSLT_SUCCESS)
    {
        printf("ADC single ended channel initialization failed. Error: %ld\n", (long unsigned int)result);
        CY_ASSERT(0);
    }

}

/*******************************************************************************
 * Function Name: adc_single_channel_process
 *******************************************************************************
 *
 * Summary:
 *  ADC single channel process function. This function reads the input voltage
 *  and prints the input voltage on UART.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void adc_single_channel_process(void)
{
    /* Variable to store ADC conversion result from channel 0 */
    int32_t adc_result_0 = 0;

    /* Read input voltage, convert it to millivolts and print input voltage */
    adc_result_0 = cyhal_adc_read_uv(&adc_chan_0_obj)/1000;
    printf("Channel 0 input: %4ldmV\r\n", (long int)adc_result_0);
}


/* [] END OF FILE */
