/******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the XMC7000 MCU CANFD example 
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


/* CAN channel number */
# if defined(KIT_XMC71_V1) || defined(KIT_XMC71_V2)
#define CAN_HW_CHANNEL          0
#else
#define CAN_HW_CHANNEL          1
#endif
#define CAN_BUFFER_INDEX        0
/* CAN address*/
#define CAN_ID                  1
/* CAN data length code, frame has 8 data bytes in this example */
#define CAN_DLC                 8


/*******************************************************************************
* Function Prototypes+++++++++++++++
*******************************************************************************/
/* canfd interrupt handler */
void isr_canfd (void);


/*******************************************************************************
* Global Variables
*******************************************************************************/
/* This structure initializes the CANFD interrupt for the NVIC */
cy_stc_sysint_t canfd_irq_cfg =
{
    .intrSrc  = (NvicMux2_IRQn << 16) | CANFD_IRQ_0, /* Source of interrupt signal */
    .intrPriority = 1U, /* Interrupt priority */
};

/* This is a shared context structure, unique for each canfd channel */
cy_stc_canfd_context_t canfd_context; 

/* Variable CANFD receive interrupt status */
bool canfdRevIntrFlag = false;


/* Array to hold the data bytes of the CANFD frame */
uint8_t canfd_data_buffer[CY_CANFD_DATA_ELEMENTS_MAX];

const uint32_t CANFD_OriginalData[] =
{
    [CANFD_DATA_0] = 0x04030201U,
    [CANFD_DATA_1] = 0x08070605U,
};

/* Variable to hold the data length code of the CANFD frame */
int canfd_dlc;
/* Variable to hold the Identifier of the CANFD frame */
int canfd_id;
/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function. It initializes the CANFD channel 
* and interrupt. User button and User LED are also initialized. The main loop
* checks for the button pressed interrupt flag and when it is set, a CANFD frame
* is sent. Whenever a CANFD frame is received from other CANFD device, the user LED 
* toggles and the received data is logged over serial terminal from other CANFD device.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main_canfd(void)
{
    cy_en_canfd_status_t status;

    printf("****************** Running CAN FD loopback demo ******************\r\n");
    printf("In this demo, to send 8-bytes CAN FD frame, connect the CAN FD analyzer \r\n");
    printf("or another kit and press the USER BTN1 button. Also, it can \r\n");
    printf("receive CAN FD data and print the received data over UART serial terminal. \r\n");
    printf("\r\n");
    printf("Note 1: The length of received data can only be less than or equal to 8-bytes. \r\n");
    printf("\r\n");
    printf("Note 2: Press the reset button on both the kits to stop the communication. \r\n");
    printf("\r\n");

    /* Setting device Identifier and send buffer*/
    CANFD_T0RegisterBuffer_0.id = CAN_ID;
    CANFD_txBuffer_0.data_area_f = (uint32_t *)canfd_data_buffer;
    /*Initialize USER_BTN1*/
    button1_initial();
    cyhal_gpio_init(CYBSP_USER_LED1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    cyhal_gpio_init(CYBSP_CANFD_STB, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_ON);

    /* Hook the interrupt service routine and enable the interrupt */
    (void) Cy_SysInt_Init(&canfd_irq_cfg, &isr_canfd);
    NVIC_EnableIRQ(NvicMux2_IRQn);

    status = Cy_CANFD_Init(CANFD_HW, CAN_HW_CHANNEL, &CANFD_config,
                           &canfd_context);
    if (status != CY_CANFD_SUCCESS)
    {
        CY_ASSERT(0);
    }

    while(!evtSwitch)
    {
        if(button1_intr_event == true)
         {
             if(button1_Hysfilter_flag == false)
             {
                 /* Hysteresis filter */
                 Cy_SysLib_Delay(10u);
                 button1_Hysfilter_flag = true;
             }
             /* wait for button release*/
             if(cyhal_gpio_read(CYBSP_USER_BTN1) == true)
             {
                 /* Assign the user defined data buffer to CANFD data area */
                 memcpy(canfd_data_buffer, CANFD_OriginalData, sizeof(CANFD_OriginalData));
                 CANFD_txBuffer_0.t1_f->dlc = CAN_DLC;
                 if(button1_intr_event == true)
                 {
                     button1_intr_event = false;
                     button1_Hysfilter_flag = false;
                     /* Enable interrupt */
                     cyhal_gpio_register_callback(CYBSP_USER_BTN1, &btn1_callback_data);
                 }

                /* Sending CANFD frame to other node */
                status = Cy_CANFD_UpdateAndTransmitMsgBuffer(CANFD_HW,
                                                        CAN_HW_CHANNEL,
                                                        &CANFD_txBuffer_0,
                                                        CAN_BUFFER_INDEX,
                                                        &canfd_context);
                printf("CAN FD frame sent\r\n\r\n");
             }
        }
        if(canfdRevIntrFlag == true)
        {
            canfdRevIntrFlag = false;
            printf("%d bytes received  with identifier %d\r\n\r\n",
                                                        canfd_dlc,
                                                        canfd_id);

            printf("Rx Data : ");
            for (uint8_t msg_idx = 0; msg_idx < canfd_dlc; msg_idx++)
            {
                printf(" 0x%x ", canfd_data_buffer[msg_idx]);
                canfd_data_buffer[msg_idx]+=1;
            }
            printf("\r\n\r\n");
            CANFD_txBuffer_0.t1_f->dlc = canfd_dlc;
            /* Sending CANFD frame to other node */
            status = Cy_CANFD_UpdateAndTransmitMsgBuffer(CANFD_HW,
                                                    CAN_HW_CHANNEL,
                                                    &CANFD_txBuffer_0,
                                                    CAN_BUFFER_INDEX,
                                                    &canfd_context);
        }
    }
    Cy_CANFD_DeInit(CANFD_HW, CAN_HW_CHANNEL, &canfd_context);
    cyhal_gpio_free(CYBSP_USER_BTN1);
    cyhal_gpio_free(CYBSP_USER_LED1);
    cyhal_gpio_free(CYBSP_CANFD_STB);
    return 0;
}

/*******************************************************************************
* Function Name: isr_canfd
********************************************************************************
* Summary:
* This is the interrupt handler function for the canfd interrupt.
*
* Parameters:
*  none
*    
*
*******************************************************************************/
void isr_canfd(void)
{
    /* Just call the IRQ handler with the current channel number and context */
    Cy_CANFD_IrqHandler(CANFD_HW, CAN_HW_CHANNEL, &canfd_context);
}

/*******************************************************************************
* Function Name: canfd_rx_callback
********************************************************************************
* Summary:
* This is the callback function for canfd reception
*
* Parameters:
*    rxFIFOMsg                      Message was received in Rx FIFO (0/1)
*    msgBufOrRxFIFONum              RxFIFO number of the received message
*    basemsg                        Message buffer
*
*******************************************************************************/
void canfd_rx_callback (bool                        rxFIFOMsg, 
                        uint8_t                     msgBufOrRxFIFONum, 
                        cy_stc_canfd_rx_buffer_t*   basemsg)
{


    /* Message was received in Rx FIFO */
    if (rxFIFOMsg == true)
    {
        /* Checking whether the frame received is a data frame */
        if(CY_CANFD_RTR_DATA_FRAME == basemsg->r0_f->rtr) 
        {
            /* Toggle the user LED */
            cyhal_gpio_toggle(CYBSP_USER_LED1);
            /* Get the CAN DLC and ID from received message */
            canfd_dlc = basemsg->r1_f->dlc;
            canfd_id  = basemsg->r0_f->id;
            memcpy(canfd_data_buffer, basemsg->data_area_f, canfd_dlc);
            /* Print the received message by UART */
            canfdRevIntrFlag = true;
        }
    }
    /* These parameters are not used in this snippet */
    (void)msgBufOrRxFIFONum;
}

/* [] END OF FILE */
