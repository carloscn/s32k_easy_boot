/**
 * @file test_flash.c
 * @brief Test program for the HAL flash memory interface
 *
 * @version 1.0.1
 * @date 2025-06-21
 *
 * @details
 * This test program verifies the functionality of the flash HAL interface defined
 * in hal_flash.h for the NXP S32K312 platform. It tests initialization, sector erase,
 * data write, data read, and resource deallocation, using the C40_Ip driver. Test
 * progress and results are logged using osal_log_info. Error handling is based on
 * hal_err.h codes. The program halts on failure and reports success via Exit_Example.
 *
 * @history
 *  - v1.0.0, 2025-06-21, Initial test implementation
 *  - v1.0.1, 2025-06-21, Added osal_log_info for test status output
 */

#include "hal_flash.h"
#include "C40_Ip.h"
#include "hal_error.h"
#include "osal_log.h"
#include <string.h>
#include <stdio.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TEST_SECTOR_ADDR    (C40_DATA_ARRAY_0_BLOCK_2_S000) /* Valid flash address for testing */
#define FLASH_START_ADDR	(C40_IP_DATA_BLOCK_BASE_ADDR + 0x4000)
#define TEST_BUFFER_SIZE    (128U)        /* Size of test data buffer */
#define TEST_NUM_SECTORS    (1U)          /* Number of sectors to erase */
#define LOG_BUFFER_SIZE     (128U)        /* Size of log message buffer */

/*******************************************************************************
 * Local Variables
 ******************************************************************************/

static uint8_t tx_buffer[TEST_BUFFER_SIZE]; /* Transmit buffer for write */
static uint8_t rx_buffer[TEST_BUFFER_SIZE]; /* Receive buffer for read */
static char log_buffer[LOG_BUFFER_SIZE];    /* Buffer for log messages */

/*******************************************************************************
 * Local Functions
 ******************************************************************************/

/**
 * @brief Converts a HAL error code to a string
 *
 * @param error_code The HAL error code
 * @return const char* Pointer to the error code string
 */
static const char* test_error_to_string(hal_err_t error_code)
{
    switch (error_code) {
        case HAL_ERR_SUCCESS: return "HAL_ERR_SUCCESS";
        case HAL_ERR_INVALID_PARAM: return "HAL_ERR_INVALID_PARAM";
        case HAL_ERR_NOT_INITIALIZED: return "HAL_ERR_NOT_INITIALIZED";
        case HAL_ERR_FLASH_INIT_FAILED: return "HAL_ERR_FLASH_INIT_FAILED";
        case HAL_ERR_FLASH_WRITE_FAILED: return "HAL_ERR_FLASH_WRITE_FAILED";
        case HAL_ERR_FLASH_READ_FAILED: return "HAL_ERR_FLASH_READ_FAILED";
        case HAL_ERR_FLASH_ERASE_FAILED: return "HAL_ERR_FLASH_ERASE_FAILED";
        case HAL_ERR_FLASH_INVALID_ADDR: return "HAL_ERR_FLASH_INVALID_ADDR";
        case HAL_ERR_FLASH_SECTOR_PROTECTED: return "HAL_ERR_FLASH_SECTOR_PROTECTED";
        default: return "UNKNOWN_ERROR";
    }
}

/**
 * @brief Initializes test buffers with a pattern
 *
 * @param length The size of the buffers to initialize
 */
static void test_init_buffers(uint32_t length)
{
    uint32_t i;
    for (i = 0U; i < length; i++) {
        tx_buffer[i] = (uint8_t)(i & 0xFF); /* Simple pattern: 0, 1, 2, ..., 255 */
        rx_buffer[i] = 0U;
    }
}

/**
 * @brief Verifies that transmit and receive buffers match
 *
 * @return boolean TRUE if buffers match, FALSE otherwise
 */
static boolean test_verify_buffers(void)
{
    uint32_t i;
    for (i = 0U; i < TEST_BUFFER_SIZE; i++) {
        if (rx_buffer[i] != tx_buffer[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

/**
 * @brief Asserts a condition, logging failure and halting
 *
 * @param condition The condition to check
 * @param error_code The HAL error code to report on failure
 */
static void test_assert(boolean condition, hal_err_t error_code)
{
    if (!condition) {
        snprintf(log_buffer, LOG_BUFFER_SIZE, "Test failed with error: %s (0x%X)\r\n",
                 test_error_to_string(error_code), error_code);
        osal_log_info(log_buffer);
        while (TRUE) {
            /* Halt on failure */
        }
    }
}

/**
 * @brief Logs the start of a test case
 *
 * @param test_number The test case number
 * @param description The test description
 */
static void test_log_start(uint32_t test_number, const char *description)
{
    snprintf(log_buffer, LOG_BUFFER_SIZE, "Starting Test %lu: %s\r\n", test_number, description);
    osal_log_info(log_buffer);
}

/**
 * @brief Logs the successful completion of a test case
 *
 * @param test_number The test case number
 */
static void test_log_pass(uint32_t test_number)
{
    snprintf(log_buffer, LOG_BUFFER_SIZE, "Test %lu passed\r\n", test_number);
    osal_log_info(log_buffer);
}

/*******************************************************************************
 * Global Functions
 ******************************************************************************/

/**
 * @brief Test function for the flash HAL interface
 *
 * @return int Returns 0 on success, non-zero on failure
 */
int test_flash(void)
{
    hal_err_t status;

    /* Test 1: Initialize flash module */
    test_log_start(1, "Initialize flash module\r\n");
    status = hal_flash_init();
    test_assert(status == HAL_ERR_SUCCESS, HAL_ERR_FLASH_INIT_FAILED);
    test_log_pass(1);

    /* Test 2: Initialize buffers */
    test_log_start(2, "Initialize buffers\r\n");
    test_init_buffers(TEST_BUFFER_SIZE);
    test_log_pass(2);

    /* Test 3: Erase sector */
    test_log_start(3, "Erase sector\r\n");
    status = hal_flash_erase_sector(FLASH_START_ADDR, TEST_NUM_SECTORS);
    test_assert(status == HAL_ERR_SUCCESS, HAL_ERR_FLASH_ERASE_FAILED);
    test_log_pass(3);

    /* Test 4: Write data to flash */
    test_log_start(4, "Write data to flash\r\n");
    status = hal_flash_write(FLASH_START_ADDR, tx_buffer, TEST_BUFFER_SIZE);
    test_assert(status == HAL_ERR_SUCCESS, HAL_ERR_FLASH_WRITE_FAILED);
    test_log_pass(4);

    /* Test 5: Read data from flash */
    test_log_start(5, "Read data from flash\r\n");
    status = hal_flash_read(FLASH_START_ADDR, rx_buffer, TEST_BUFFER_SIZE);
    test_assert(status == HAL_ERR_SUCCESS, HAL_ERR_FLASH_READ_FAILED);
    test_log_pass(5);

    /* Test 6: Verify read data matches written data */
    test_log_start(6, "Verify read data matches written data\r\n");
    test_assert(test_verify_buffers() == TRUE, HAL_ERR_FLASH_READ_FAILED);
    test_log_pass(6);

    /* Test 7: Error case - invalid address */
    test_log_start(7, "Error case - invalid address\r\n");
    status = hal_flash_write(0x0U, tx_buffer, TEST_BUFFER_SIZE);
    test_assert(status == HAL_ERR_INVALID_PARAM, HAL_ERR_SUCCESS);
    test_log_pass(7);

    /* Test 8: Error case - null pointer */
    test_log_start(8, "Error case - null pointer\r\n");
    status = hal_flash_read(FLASH_START_ADDR, NULL, TEST_BUFFER_SIZE);
    test_assert(status == HAL_ERR_INVALID_PARAM, HAL_ERR_SUCCESS);
    test_log_pass(8);

    /* Test 9: Error case - uninitialized module */
    test_log_start(9, "Error case - uninitialized module\r\n");
    hal_flash_free();
    status = hal_flash_write(FLASH_START_ADDR, tx_buffer, TEST_BUFFER_SIZE);
    test_assert(status == HAL_ERR_NOT_INITIALIZED, HAL_ERR_SUCCESS);
    test_log_pass(9);

    /* Reinitialize for cleanup */
    test_log_start(10, "Reinitialize for cleanup\r\n");
    status = hal_flash_init();
    test_assert(status == HAL_ERR_SUCCESS, HAL_ERR_FLASH_INIT_FAILED);
    test_log_pass(10);

    /* Test 11: Free resources */
    test_log_start(11, "Free resources\r\n");
    hal_flash_free();
    test_log_pass(11);

    /* All tests passed */
    osal_log_info("All tests passed\r\n");
    return 0;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
