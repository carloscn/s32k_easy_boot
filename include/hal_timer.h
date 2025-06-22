#ifndef hal_timer_h
#define hal_timer_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*******************************************************************************
 * user include
 ******************************************************************************/

/*!
 * @file: hal_timer.h
 *
 * @brief: hardware abstraction layer for timer module, providing functions to initialize, manage, and deinitialize timer operations.
 *
 * @page misra_violations misra-c:2012 violations
 *
 * @section rule_x-y rule: x.y (required)
 * violates misra 2012 required rule x.y, rule description here.
 *
 * @par version history
<pre><b>
version:   author:       date&&time:      revision log: </b>
 v1.0.0  tomlin tang  2019-01-17 15:30:01  first create
 v1.0.1  [your name]  2025-06-22 13:33:00  renamed to hal_timer.h, updated to lowercase, used stdint.h types, hal_timer_xxx naming
</pre>
 */

/*!
 * @addtogroup hal_timer
 * @{
 */

/*******************************************************************************
 * definitions
 ******************************************************************************/
/*!
 * @brief timer hal module for managing periodic interrupts and timeouts.
 * \n <b>hardware timer configuration</b>
 * \n <i>supports 1ms and 100ms tick tracking</i>
 *  - provides timer initialization and deinitialization
 *  - tracks 1ms and 100ms timeouts
 *  - generates random seed from timer ticks
 *
 * implements : hal_timer_instance_t_class
 */

#if defined (__cplusplus)
extern "c" {
#endif

/*******************************************************************************
 * api
 ******************************************************************************/

/*! @}*/
/*!
 * @name initialize
 * initializes the timer module.
 *
 * to disable this module, call the deinitialization function (see \ref uninitialize "disable module")
 */
/*! @{*/

/*!
 * @brief initializes the timer module.
 *
 * configures the hardware timer and enables interrupts.
 *
 * @param[in] instance instance number
 * @return void
 */
void hal_timer_init(void);

/*! @}*/
/*!
 * @name timeout check
 * checks for timer timeouts.
 *
 * to disable this module, call the deinitialization function (see \ref uninitialize "disable module")
 */
/*! @{*/

/*!
 * @brief checks if a 1ms tick has timed out.
 *
 * returns true if a 1ms tick has occurred, else false.
 *
 * @param[in] instance instance number
 * @return bool true if timeout, false otherwise
 */
bool hal_timer_is_1ms_tick_timeout(void);

/* timer 1ms period callback */
void hal_timer_1ms_period(void);

/* checks if a 100ms tick has timed out */
bool hal_timer_is_100ms_tick_timeout(void);

/* gets timer tick count for random seed generation */
uint32_t hal_timer_get_timer_tick_cnt(void);

/*!
 * @brief deinitializes the timer module.
 *
 * disables the timer and clears configurations.
 *
 * @param[in] instance instance number
 * @return void
 */
void hal_timer_free(void);

#if defined (__cplusplus)
}
#endif

/*! @}*/

#endif /* hal_timer_h */
