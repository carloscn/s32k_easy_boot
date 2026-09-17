#include "osal_log.h"
#include "hal_uart.h"
#include <string.h>

extern HAL_UART lpuart6;

static int s_log_enabled = 1;

void osal_log_set_enabled(int on)
{
    s_log_enabled = (0 != on) ? 1 : 0;
}

void osal_log_info(const char *msg)
{
	if (0 == s_log_enabled) {
		return;
	}
	if (NULL == msg || msg[0] == '\0') {
		return;
	}
	hal_uart_transmit_it(&lpuart6, msg, strlen(msg));
}
