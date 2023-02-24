/******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the XMC7000 MCU QSPI memory example 
* for ModusToolbox.
*
* Related Document: See README.md
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
#include <inttypes.h>
     

/*******************************************************************************
* Macros
********************************************************************************/
#define PACKET_SIZE             (64u)     /* Memory Read/Write size */

/* Used when an array of data is printed on the console */
#define NUM_BYTES_PER_LINE      (16u)
#define LED_TOGGLE_DELAY_MSEC   (1000u)   /* LED blink delay */
#define MEM_SLOT_NUM            (0u)      /* Slot number of the memory to use */
#define QSPI_BUS_FREQUENCY_HZ   (50000000lu)
#define FLASH_DATA_AFTER_ERASE  (0xFFu)   /* Flash data after erase */
#define ADDRESS                 (0u)

#define DEVICE_SPECIFIC_RDSR1_COMMAND                  (0x05)
#define DEVICE_SPECIFIC_WREN_COMMAND                   (0x06)
#define DEVICE_SPECIFIC_4SE_COMMAND                    (0xdc)
#define DEVICE_SPECIFIC_4QPP_COMMAND                   (0x34)
#define DEVICE_SPECIFIC_4QIOR_COMMAND                  (0xEC)

/* Size of the message block that can be processed by Crypto hardware for
 * AES encryption.
 */
#define AES128_ENCRYPTION_LENGTH             (uint32_t)(16u)
#define AES128_KEY_LENGTH                    (uint32_t)(16u)

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Variables to hold the user message and the corresponding encrypted message */

cy_stc_crypto_aes_state_t aes_state;


uint8_t qspi_tx_buf[PACKET_SIZE];
uint8_t qspi_rx_buf[PACKET_SIZE];
CY_ALIGN(4) uint8_t encrypted_msg[PACKET_SIZE];
CY_ALIGN(4) uint8_t decrypted_msg[PACKET_SIZE];

static cyhal_qspi_t qspi_obj;
static uint8_t qspi_mem_sta;

/* Key used for AES encryption*/
CY_ALIGN(4) uint8_t aes_key[PACKET_SIZE] = {0xAA, 0xBB, 0xCC, 0xDD,
                                            0xEE, 0xFF, 0xFF, 0xEE,
                                            0xDD, 0xCC, 0xBB, 0xAA,
                                            0xAA, 0xBB, 0xCC, 0xDD,
                                            0xAA, 0xBB, 0xCC, 0xDD,
                                            0xEE, 0xFF, 0xFF, 0xEE,
                                            0xDD, 0xCC, 0xBB, 0xAA,
                                            0xAA, 0xBB, 0xCC, 0xDD,
                                            0xAA, 0xBB, 0xCC, 0xDD,
                                            0xEE, 0xFF, 0xFF, 0xEE,
                                            0xDD, 0xCC, 0xBB, 0xAA,
                                            0xAA, 0xBB, 0xCC, 0xDD,
                                            0xAA, 0xBB, 0xCC, 0xDD,
                                            0xEE, 0xFF, 0xFF, 0xEE,
                                            0xDD, 0xCC, 0xBB, 0xAA,
                                            0xAA, 0xBB, 0xCC, 0xDD,
};


/* read status command structure */
cyhal_qspi_command_t device_specific_rdsr1_command =
{
    .instruction.bus_width      = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for the instruction */
    .instruction.data_rate      = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for instruction (SDR/DDR) */
    .instruction.two_byte_cmd   = false,                        /* command is 1-byte value */
    .instruction.value          = DEVICE_SPECIFIC_RDSR1_COMMAND,/* Instruction value */
    .instruction.disabled       = false,                        /* Instruction phase skipped if disabled, set to false */                                                               
    
    .address.bus_width          = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for the address */
    .address.data_rate          = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for address (SDR/DDR) */
    .address.size               = CYHAL_QSPI_CFG_SIZE_8,        /* Address size in bits */
    .address.disabled           = true,                         /* Address phase skipped if disabled, set to true */

    .mode_bits.disabled         = true,                         /* Mode bits phase skipped if disabled, set to true */
    .dummy_cycles.dummy_count   = 0u,                           /* Dummy cycles count */
    
    .data.bus_width             = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for data */
    .data.data_rate             = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for data (SDR/DDR) */
};

/* Write enable command structure */
cyhal_qspi_command_t device_specific_wren_command =
{
    .instruction.bus_width      = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for the instruction */
    .instruction.data_rate      = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for instruction (SDR/DDR) */
    .instruction.two_byte_cmd   = false,                        /* command is 1-byte value */
    .instruction.value          = DEVICE_SPECIFIC_WREN_COMMAND, /* Instruction value */
    .instruction.disabled       = false,                        /* Instruction phase skipped if disabled, set to false */
    
    .address.bus_width          = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for the address */
    .address.data_rate          = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for address (SDR/DDR) */
    .address.size               = CYHAL_QSPI_CFG_SIZE_8,        /* Address size in bits */
    .address.disabled           = true,                         /* Address phase skipped if disabled, set to true */
    
    .mode_bits.disabled         = true,                         /* Mode bits phase skipped if disabled, set to true */
    .dummy_cycles.dummy_count   = 0u,                           /* Dummy cycles count */
    
    .data.bus_width             = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for data */
    .data.data_rate             = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for data (SDR/DDR) */
};

/* sector erase command structure */
cyhal_qspi_command_t device_specific_4se_command =
{
    .instruction.bus_width      = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for the instruction */
    .instruction.data_rate      = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for instruction (SDR/DDR) */
    .instruction.two_byte_cmd   = false,                        /* command is 1-byte value */
    .instruction.value          = DEVICE_SPECIFIC_4SE_COMMAND,  /* Instruction value */
    .instruction.disabled       = false,                        /* Instruction phase skipped if disabled, set to true */
   
    .address.bus_width          = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for the address */
    .address.data_rate          = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for address (SDR/DDR) */
    .address.size               = CYHAL_QSPI_CFG_SIZE_32,       /* Address size in bits */
    .address.disabled           = false,                        /* Address phase skipped if disabled, set to false */
    
    .mode_bits.disabled         = true,                         /* Mode bits phase skipped if disabled, set to true */
    .dummy_cycles.dummy_count   = 0u,                           /* Dummy cycles count */
    
    .data.bus_width             = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for data */
    .data.data_rate             = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for data (SDR/DDR) */
};

/* Quad Page Program command structure */
cyhal_qspi_command_t device_specific_4qpp_command =
{
    .instruction.bus_width      = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for the instruction */
    .instruction.data_rate      = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for instruction (SDR/DDR) */
    .instruction.two_byte_cmd   = false,                        /* command is 1-byte value */ 
    .instruction.value          = DEVICE_SPECIFIC_4QPP_COMMAND, /* Instruction value */
    .instruction.disabled       = false,                        /* Instruction phase skipped if disabled, set to false */

    .address.bus_width          = CYHAL_QSPI_CFG_BUS_SINGLE,    /* Bus width for the address */
    .address.data_rate          = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for address (SDR/DDR) */
    .address.size               = CYHAL_QSPI_CFG_SIZE_32,       /* Address size in bits */
    .address.disabled           = false,                        /* Address phase skipped if disabled, set to true */

    .mode_bits.disabled         = true,                         /* Mode bits phase skipped if disabled, set to true */
    .dummy_cycles.dummy_count   = 0u,                           /* Dummy cycles count */
    
    .data.bus_width             = CYHAL_QSPI_CFG_BUS_QUAD,      /* Bus width for data */
    .data.data_rate             = CYHAL_QSPI_DATARATE_SDR,      /* Data rate for data (SDR/DDR) */
};

/* Quad I/O Read command structure */
cyhal_qspi_command_t device_specific_4qior_command =
{
    .instruction.bus_width      = CYHAL_QSPI_CFG_BUS_SINGLE,     /* Bus width for the instruction */
    .instruction.data_rate      = CYHAL_QSPI_DATARATE_SDR,       /* Data rate for instruction (SDR/DDR) */
    .instruction.two_byte_cmd   = false,                         /* command is 1-byte value */
    .instruction.value          = DEVICE_SPECIFIC_4QIOR_COMMAND, /* Instruction value */
    .instruction.disabled       = false,                         /* Instruction phase skipped if disabled, set to false */
    
    .address.bus_width          = CYHAL_QSPI_CFG_BUS_QUAD,       /* Bus width for the address */
    .address.data_rate          = CYHAL_QSPI_DATARATE_SDR,       /* Data rate for address (SDR/DDR) */
    .address.size               = CYHAL_QSPI_CFG_SIZE_32,        /* Address size in bits */
    .address.disabled           = false,                         /* Address phase skipped if disabled, set to false */
   
    .mode_bits.data_rate        = CYHAL_QSPI_DATARATE_SDR,
    .mode_bits.size             = CYHAL_QSPI_CFG_SIZE_8,
    .mode_bits.disabled         = false,                          /* Mode bits phase skipped if disabled, set to false */

    /* The 8-bit mode byte. This value is 0xFFFFFFFF when there is no mode present. */
    .mode_bits.value            = 0xFFFFFFFFU,
    .mode_bits.bus_width        = CYHAL_QSPI_CFG_BUS_QUAD,

    .dummy_cycles.dummy_count   = 4u,                             /* Dummy cycles count */
    .dummy_cycles.bus_width     = CYHAL_QSPI_CFG_BUS_SINGLE,

    .data.bus_width             = CYHAL_QSPI_CFG_BUS_QUAD,        /* Bus width for data */
    .data.data_rate             = CYHAL_QSPI_DATARATE_SDR,        /* Data rate for data (SDR/DDR) */
};


/*******************************************************************************
* Function Prototypes
********************************************************************************/
static void qspi_wait_mem_free(void);
void encrypt_message(uint8_t* message, uint8_t size);
void decrypt_message(uint8_t* message, uint8_t size);

/*******************************************************************************
* Function Name: check_status
****************************************************************************//**
* Summary:
*  Prints the message, indicates the non-zero status by turning the LED on, and
*  asserts the non-zero status.
*
* Parameters:
*  message - message to print if status is non-zero.
*  status - status for evaluation.
*
*******************************************************************************/
void check_status(char *message, uint32_t status)
{
    if (0u != status)
    {
        printf("\r\n================================================================================\r\n");
        printf("\nFAIL: %s\r\n", message);
        printf("Error Code: 0x%08"PRIX32"\n", status);
        printf("\r\n================================================================================\r\n");

        /* On failure, turn the LED ON */
        cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
        CY_ASSERT(false);
    }
}

/*******************************************************************************
* Function Name: print_array
****************************************************************************//**
* Summary:
*  Prints the content of the buffer to the UART console.
*
* Parameters:
*  message - message to print before array output
*  buf - buffer to print on the console.
*  size - size of the buffer.
*
*******************************************************************************/
void print_array(char *message, uint8_t *buf, uint32_t size)
{
    printf("\r\n%s (%"PRIu32" bytes):\r\n", message, size);
    printf("-------------------------\r\n");

    for (uint32_t index = 0; index < size; index++)
    {
        printf("0x%02X ", buf[index]);

        if (0u == ((index + 1) % NUM_BYTES_PER_LINE))
        {
            printf("\r\n");
        }
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  This is the main function for XMC7200 CPU. It does...
*  1. Initializes UART for console output and SMIF for interfacing a QSPI flash.
*  2. Performs erase followed by write and verifies the written data by
*     reading it back.
*
* Parameters:
*  None
*
* Return:
*  int
*
*******************************************************************************/
int main_qspi_memory(void)
{
    cy_rslt_t result;
    uint8_t tx_buf[PACKET_SIZE];
    uint8_t rx_buf[PACKET_SIZE];
    size_t size = PACKET_SIZE;

    /* Enable the Crypto block */
    Cy_Crypto_Core_Enable(CRYPTO);
     
    SCB_DisableDCache();
     
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("****************** Running QSPI memory read/write demo ******************\r\n");
    printf("In this demo, the erase, write and read operation are automatically \r\n");
    printf("executed on the QSPI memory with AES encryption and decryption. \r\n");
    printf("Observe the USER LED1 to determine the status of the read write operation. \r\n");
    printf("USER LED1 is blinking: Successful operation.  \r\n");
    printf("USER LED1 is always ON: Failed operation. \r\n");
    printf("\r\n");



    /* Initialize the User LED */
     result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
               CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

     check_status("User LED initialization failed", result);

     /* QSPI parameter initialize */
     cyhal_qspi_slave_pin_config_t memory_pin_set =
     {
         .io   = { CYBSP_QSPI_D0,
                 CYBSP_QSPI_D1, CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC, },
         .ssel = CYBSP_QSPI_SS
     };
     /* QSPI device initialize */
     result = cyhal_qspi_init(&qspi_obj, CYBSP_QSPI_SCK, &memory_pin_set, QSPI_BUS_FREQUENCY_HZ, 0, NULL);
     check_status("Serial Flash initialization failed", result);
     /* Write enable */
     result = cyhal_qspi_transfer(&qspi_obj, &device_specific_wren_command, 0, NULL, 0, \
                                       NULL, 0);
     check_status("Writing to memory failed", result);
     /* sector erase */
     printf("\r\n");
     printf("1. Erasing \r\n");
     result = cyhal_qspi_transfer(&qspi_obj, &device_specific_4se_command, ADDRESS, NULL, 0, \
                                      NULL, 0);
     /* Wait memory free */
     qspi_wait_mem_free();

     /* sector read */
     printf("\r\n");
     printf("2. Reading after Erase and verifying that each byte is 0xFF\r\n");
     result = cyhal_qspi_read(&qspi_obj, &device_specific_4qior_command, ADDRESS, rx_buf, &size);
     check_status("Writing to memory failed", result);
     print_array("Received Data", rx_buf, PACKET_SIZE);
     memset(tx_buf, FLASH_DATA_AFTER_ERASE, PACKET_SIZE);
     check_status("Flash contains data other than 0xFF after erase",
                  memcmp(tx_buf, rx_buf, PACKET_SIZE));

     /* Prepare the TX buffer */
     for (uint32_t index = 0; index < PACKET_SIZE; index++)
     {
         encrypted_msg[index] = (uint8_t)index;
     }

     /* Write enable */
     result = cyhal_qspi_transfer(&qspi_obj, &device_specific_wren_command, 0, NULL, 0, \
                                      NULL, 0);
     check_status("Writing to memory failed", result);

     /* Write the content of the TX buffer to the memory */
     printf("\r\n");
     printf("3. Writing data to memory\r\n");
     print_array("Write Data", encrypted_msg, PACKET_SIZE);
     encrypt_message(tx_buf, PACKET_SIZE);
     /* Write page */
     result = cyhal_qspi_write(&qspi_obj, &device_specific_4qpp_command, ADDRESS, tx_buf, &size);
     check_status("Writing to memory failed", result);

     /* Wait memory free */
     qspi_wait_mem_free();
     /* Read back after Write for verification */

     printf("\r\n");
     printf("4. Reading back for verification\r\n");
     /* read page */
     result = cyhal_qspi_read(&qspi_obj, &device_specific_4qior_command, ADDRESS, rx_buf, &size);
     check_status("Writing to memory failed", result);
     decrypt_message(rx_buf, PACKET_SIZE);
     print_array("Received Data", decrypted_msg, PACKET_SIZE);

     check_status("Read data does not match with written data. Read/Write operation failed.",
            memcmp(encrypted_msg, decrypted_msg, PACKET_SIZE));

     printf("\r\n");
     printf("================================================================================\r\n");
     printf("\r\n");
     printf("SUCCESS: Read data matches with written data!\r\n");
     printf("\r\n");
     printf("================================================================================\r\n");

    while(!evtSwitch)
    {
        cyhal_gpio_toggle(CYBSP_USER_LED);
        cyhal_system_delay_ms(LED_TOGGLE_DELAY_MSEC);
    }
    Cy_Crypto_Core_Disable(CRYPTO);
    cyhal_gpio_free(CYBSP_USER_LED);
    cyhal_qspi_free(&qspi_obj);
    return 0;
}


/*******************************************************************************
* Function Name: encrypt_message
********************************************************************************
* Summary: Function used to encrypt the message.
*
* Parameters:
*  char * message - pointer to the message to be encrypted
*  uint8_t size   - size of message to be encrypted.
*
* Return:
*  void
*
*******************************************************************************/
void encrypt_message(uint8_t* message, uint8_t size)
{
    uint8_t aes_block_count = 0;

    aes_block_count =  (size % AES128_ENCRYPTION_LENGTH == 0) ?
                       (size / AES128_ENCRYPTION_LENGTH)
                       : (1 + size / AES128_ENCRYPTION_LENGTH);

    /* Initializes the AES operation by setting key and key length */
    Cy_Crypto_Core_Aes_Init(CRYPTO, aes_key, CY_CRYPTO_KEY_AES_128, &aes_state);

    for (int i = 0; i < aes_block_count ; i++)
    {
        /* Perform AES ECB Encryption mode of operation */
        Cy_Crypto_Core_Aes_Ecb(CRYPTO, CY_CRYPTO_ENCRYPT,
                               (message + AES128_ENCRYPTION_LENGTH * i),
                               (encrypted_msg + AES128_ENCRYPTION_LENGTH * i),
                                &aes_state);
        /* Wait for Crypto Block to be available */
        Cy_Crypto_Core_WaitForReady(CRYPTO);
    }
    Cy_Crypto_Core_Aes_Free(CRYPTO, &aes_state);
}

/*******************************************************************************
* Function Name: decrypt_message
********************************************************************************
* Summary: Function used to decrypt the message.
*
* Parameters:
*  char * message - pointer to the message to be decrypted
*  uint8_t size   - size of message to be decrypted.
*
* Return:
*  void
*
*******************************************************************************/
void decrypt_message(uint8_t* message, uint8_t size)
{
    uint8_t aes_block_count = 0;

    aes_block_count =  (size % AES128_ENCRYPTION_LENGTH == 0) ?
                       (size / AES128_ENCRYPTION_LENGTH)
                       : (1 + size / AES128_ENCRYPTION_LENGTH);

    /* Initializes the AES operation by setting key and key length */
    Cy_Crypto_Core_Aes_Init(CRYPTO, aes_key, CY_CRYPTO_KEY_AES_128, &aes_state);

    /* Start decryption operation*/
    for (int i = 0; i < aes_block_count ; i++)
    {
        /* Perform AES ECB Decryption mode of operation */
        Cy_Crypto_Core_Aes_Ecb(CRYPTO, CY_CRYPTO_DECRYPT,
                               (decrypted_msg + AES128_ENCRYPTION_LENGTH * i),
                               (message + AES128_ENCRYPTION_LENGTH * i),
                               &aes_state);
        /* Wait for Crypto Block to be available */
        Cy_Crypto_Core_WaitForReady(CRYPTO);
    }

    Cy_Crypto_Core_Aes_Free(CRYPTO, &aes_state);
}

/*******************************************************************************
* Function Name: qspi_wait_mem_free
****************************************************************************//**
* Summary:
*  waiting for qspi flash free.
*
* Parameters:
*  None
*
* Return:
*  none
*
*******************************************************************************/
static void qspi_wait_mem_free(void)
{
    cy_rslt_t result;
    do
    {
        result = cyhal_qspi_transfer(&qspi_obj, &device_specific_rdsr1_command, 0, NULL, 0, \
                                     &qspi_mem_sta, 1u);
        check_status("read status failed", result);
    }
    while(qspi_mem_sta!=0);
}
