/*******************************************************************************
* File Name:   print_message.c
*
* Description: Print message function for communication with terminal.
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

#include "print_message.h"
#include "oob_demo.h"
#include "cy_retarget_io.h"


/*******************************************************************************
* Global Variables
*******************************************************************************/
cyhal_uart_t    uart_obj;
uint8_t         tx_buf[TX_BUF_SIZE];
size_t          tx_length = TX_BUF_SIZE;
/* UART received command. */
uint8_t         recCmd = 0;

/* Initialize the UART configuration structure. */
const cyhal_uart_cfg_t uart_config =
{
    .data_bits      = DATA_BITS_8,
    .stop_bits      = STOP_BITS_1,
    .parity         = CYHAL_UART_PARITY_NONE,
    .rx_buffer      = NULL,
    .rx_buffer_size = 0
};


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void uart_port_initial(void);
void uart_event_handler(void* handler_arg, cyhal_uart_event_t event);

/*******************************************************************************
* Function Name: uart_port_initial
********************************************************************************
* Summary:
* Initial uart port and register interrupt callback
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void uart_port_initial()
{
    cy_rslt_t    rslt;

    /* Initialize retarget-io to use the debug UART port */
    rslt = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);

    if(CY_RSLT_SUCCESS != rslt)
    {
        /* Disable all interrupts. */
        __disable_irq();

        CY_ASSERT(false);
    }
    /* The UART callback handler registration. */
    cyhal_uart_register_callback(&cy_retarget_io_uart_obj, uart_event_handler, NULL);

   /* Enable required UART events. */
    cyhal_uart_enable_event(&cy_retarget_io_uart_obj,(cyhal_uart_event_t)(CYHAL_UART_IRQ_RX_DONE | CYHAL_UART_IRQ_RX_NOT_EMPTY), INT_PRIORITY, true);

}

/*******************************************************************************
* Function Name: uart_event_handler
********************************************************************************
* Summary:
* Uart interrupt event handler callback function
*
* Parameters:
*  handler_arg: user defined argument
*  event: uart interrupt event source
*
* Return:
*  none
*
*******************************************************************************/
void uart_event_handler(void* handler_arg, cyhal_uart_event_t event)
{
    (void)handler_arg;

    if ((event & CYHAL_UART_IRQ_RX_DONE) == CYHAL_UART_IRQ_RX_DONE)
    {
        handle_error();
        /* All Rx data has been received */
        /* Insert application code to handle Rx done */
    }
    else if ((event & CYHAL_UART_IRQ_RX_NOT_EMPTY) == CYHAL_UART_IRQ_RX_NOT_EMPTY)
    {
        /* Get input command */
        cyhal_uart_getc(&cy_retarget_io_uart_obj, &recCmd, 1);

        /* Distinguish command */
        switch(recCmd)
        {
            case DEM_HELLO_WORD:
                if(demoIndex != (DEM_HELLO_WORD & 0x0F))
                {
                    /* Set demoIndex */
                    demoIndex = DEM_HELLO_WORD & 0x0F;
                    /* Change demo switch flag */
                    evtSwitch = true;
                }
                break;
            case DEM_PWM_WAVE:
                if(demoIndex != (DEM_PWM_WAVE & 0x0F))
                {
                    /* Set demoIndex */
                    demoIndex = DEM_PWM_WAVE & 0x0F;
                    /* Change demo switch flag */
                    evtSwitch = true;
                }
                break;
            case DEM_IO_INTR:
                if(demoIndex != (DEM_IO_INTR & 0x0F))
                {
                    /* Set demoIndex */
                    demoIndex = DEM_IO_INTR & 0x0F;
                    /* Change demo switch flag */
                    evtSwitch = true;
                }
                break;
            case DEM_IO_ADC:
                if(demoIndex != (DEM_IO_ADC & 0x0F))
                {
                    /* Set demoIndex */
                    demoIndex = DEM_IO_ADC & 0x0F;
                    /* Change demo switch flag */
                    evtSwitch = true;
                }
                break;
            case DEM_IO_POWER:
                if(demoIndex != (DEM_IO_POWER & 0x0F))
                {
                    /* Set demoIndex */
                    demoIndex = DEM_IO_POWER & 0x0F;
                    /* Change demo switch flag */
                    evtSwitch = true;
                }
                break;
            case DEM_IO_QSPI:
                if(demoIndex != (DEM_IO_QSPI & 0x0F))
                {
                    /* Set demoIndex */
                    demoIndex = DEM_IO_QSPI & 0x0F;
                    /* Change demo switch flag */
                    evtSwitch = true;
                }
                break;
            case DEM_IO_CANFD:
                if(demoIndex != (DEM_IO_CANFD & 0x0F))
                {
                    /* Set demoIndex */
                    demoIndex = DEM_IO_CANFD & 0x0F;
                    /* Change demo switch flag */
                    evtSwitch = true;
                }
                break;
            default:
                break;
        }
    }
}
