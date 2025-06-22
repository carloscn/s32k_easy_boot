#include "osal_log.h"
#include "hal_uart.h"
#include <string.h>

extern HAL_UART lpuart6;
void osal_log_info(const char *msg)
{
	if (NULL == msg || msg[0] == '\0') {
		return;
	}
	hal_uart_transmit_it(&lpuart6, msg, strlen(msg));
}
