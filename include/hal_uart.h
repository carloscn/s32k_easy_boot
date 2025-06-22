/**
 * @file hal_uart.h
 * @brief Hardware Abstraction Layer (HAL) for UART operations
 *
 * @version 1.0.0
 * @date 2024-12-23
 *
 * @details
 * This header provides an abstraction layer for UART communications, including
 * initialization, deinitialization, data transmission, and reception with both
 * blocking and interrupt-based methods. It also includes callback handlers for
 * transmission and reception completion.
 *
 * @history
 *  - v1.0.0, 2024-12-23, Initial UART HAL interface definitions
 *  - v1.0.1, [future date], [description of updates]
 *  - ...
 */

#ifndef _HAL_UART_H_
#define _HAL_UART_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HAL_UART_NUM_T{
	HAL_UART_0 = 0,
    HAL_UART_1,
    HAL_UART_2,
    HAL_UART_3,
    HAL_UART_4,
    HAL_UART_5,
	HAL_UART_6,
    HAL_UART_NUM_MAX
} HAL_UART_NUM;

typedef struct hal_uart_t {
    HAL_UART_NUM num;
    uint8_t irq;
} HAL_UART;

/**
 * @brief Initialize the specified UART peripheral.
 *
 * This function initializes the UART peripheral with the given settings.
 *
 * @param huart Pointer to the HAL_UART structure containing the UART configuration.
 * @return int32_t HAL status: HAL_OK if successful, otherwise an error code.
 */
int32_t hal_uart_init(HAL_UART *huart);

/**
 * @brief Deinitialize the specified UART peripheral.
 *
 * This function deinitializes the UART peripheral and releases any resources associated with it.
 *
 * @param huart Pointer to the HAL_UART structure.
 * @return None
 */
void hal_uart_deinit(HAL_UART *huart);

/**
 * @brief Transmit data over UART.
 *
 * This function sends data over the specified UART interface.
 *
 * @param huart Pointer to the HAL_UART structure.
 * @param data Pointer to the data to transmit.
 * @param size Size of the data to transmit.
 * @param timeout_ms Timeout duration for transmission.
 * @return int32_t HAL status: HAL_OK if successful, otherwise an error code.
 */
int32_t hal_uart_transmit(HAL_UART *huart, uint8_t *data, size_t size, size_t timeout_ms);

/**
 * @brief Receive data over UART.
 *
 * This function receives data from the specified UART interface.
 *
 * @param huart Pointer to the HAL_UART structure.
 * @param data Pointer to the buffer where received data will be stored.
 * @param size Size of the data to receive.
 * @param timeout_ms Timeout duration for reception.
 * @return int32_t HAL status: HAL_OK if successful, otherwise an error code.
 */
int32_t hal_uart_receive(HAL_UART *huart, uint8_t *data, size_t size, size_t timeout_ms);

/**
 * @brief Transmit data over UART using interrupt.
 *
 * This function sends data over the specified UART interface using interrupt mode.
 *
 * @param huart Pointer to the HAL_UART structure.
 * @param data Pointer to the data to transmit.
 * @param size Size of the data to transmit.
 * @return int32_t HAL status: HAL_OK if successful, otherwise an error code.
 */
int32_t hal_uart_transmit_it(HAL_UART *huart, uint8_t *data, size_t size);

/**
 * @brief Receive data over UART using interrupt.
 *
 * This function receives data from the specified UART interface using interrupt mode.
 *
 * @param huart Pointer to the HAL_UART structure.
 * @param data Pointer to the buffer where received data will be stored.
 * @param size Size of the data to receive.
 * @return int32_t HAL status: HAL_OK if successful, otherwise an error code.
 */
int32_t hal_uart_receive_it(HAL_UART *huart, uint8_t *data, size_t size);

/**
 * @brief UART transmission complete callback.
 *
 * This function is called when UART transmission is complete.
 * It can be overridden by the user to implement custom transmission complete handling.
 *
 * @param huart Pointer to the HAL_UART structure.
 */
void hal_uart_tx_cplt_callback(HAL_UART *huart);

/**
 * @brief UART reception complete callback.
 *
 * This function is called when UART reception is complete.
 * It can be overridden by the user to implement custom reception complete handling.
 *
 * @param huart Pointer to the HAL_UART structure.
 */
void hal_uart_rx_cplt_callback(HAL_UART *huart);

#if HAL_UNIT_TEST
void hal_uart_unit_test(void);
#endif /* HAL_UNIT_TEST */

#ifdef __cplusplus
}
#endif

#endif /* _HAL_UART_H_ */
