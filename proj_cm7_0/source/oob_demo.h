/******************************************************************************
* File Name:   oob_demo.h
*
* Description: Oob_demo function for management all of demonstrates.
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

#ifndef _OOB_DEMO_H_
#define _OOB_DEMO_H_

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* demo project number */
#define     DEMONUM              7
/* default command */
#define     CMD_DEFAULT          0xFF
#define     DEM_HELLO_WORD       0x31
#define     DEM_PWM_WAVE         0x32
#define     DEM_IO_INTR          0x33
#define     DEM_IO_ADC           0x34
#define     DEM_IO_POWER         0x35
#define     DEM_IO_QSPI          0x36
#define     DEM_IO_CANFD         0x37

/* LED states */
#define LED_ON                            (0)
#define LED_OFF                           (1)

/*******************************************************************************
* External Functions
*******************************************************************************/
extern int main_hellowrold(void);
extern int main_pwm_square_wave(void);
extern int main_gpio_interrupt(void);
extern int main_sar_adc(void);
extern int main_powermode(void);
extern int main_qspi_memory(void);
extern int main_canfd(void);
extern void handle_error(void);

/* Array of demo projects */
extern int (*demoProject[DEMONUM])(void);
/*******************************************************************************
* External Variables
*******************************************************************************/
/* demo switch flag */
extern bool evtSwitch;
/* demo project index */
extern uint8_t demoIndex;
/* Hibernate reset status */
extern bool Hibresetstatus;
#endif
