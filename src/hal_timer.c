/*******************************************************************************
 * user include
 ******************************************************************************/

#include "Pit_Ip.h"
#include "IntCtrl_Ip.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "hal_timer.h"

#define PIT_INST 0U

void PIT_0_ISR(void);

void lptmr_isr(uint8_t channel)
{
    //lptmr_drv_clearcompareflag(inst_lptmr1);
    hal_timer_1ms_period();
}

/*******************************************************************************
 * variables
 ******************************************************************************/
static uint16_t gs_1ms_cnt = 0u;
static uint16_t gs_100ms_cnt = 0u;

/*function**********************************************************************
 *
 * function name : hal_timer_init
 * description   : this function initializes the timer module.
 *
 * implements : hal_timer_init_activity
 *end**************************************************************************/
void hal_timer_init(void)
{
    Pit_Ip_Init(PIT_INST, &PIT_0_InitConfig_PB);       /* initialize the PIT0 module */
	Pit_Ip_InitChannel(PIT_INST, PIT_0_CH_0);        /* initialize PIT channel 0 */
	IntCtrl_Ip_InstallHandler(PIT0_IRQn,PIT_0_ISR,NULL_PTR);
	Pit_Ip_EnableChannelInterrupt(PIT_INST, 0U);     /* enable the PIT channel 0 interrupt */
	Pit_Ip_StartChannel(PIT_INST, 0U, 40000);
}

void hal_timer_1ms_period(void)
{
    uint16_t cnt_tmp = 0u;

    cnt_tmp = gs_1ms_cnt + 1u;
    if (0u != cnt_tmp) {
        gs_1ms_cnt++;
    }

    cnt_tmp = gs_100ms_cnt + 1u;
    if (0u != cnt_tmp) {
        gs_100ms_cnt++;
    }
}

bool hal_timer_is_1ms_tick_timeout(void)
{
    bool result = false;

    if (gs_1ms_cnt)  {
        result = true;
        gs_1ms_cnt--;
    }

    return result;
}


bool hal_timer_is_100ms_tick_timeout(void)
{
    bool result = false;

    if (gs_100ms_cnt >= 100u) {
        result = true;
        gs_100ms_cnt -= 100u;
    }

    return result;
}

uint32_t hal_timer_get_timer_tick_cnt(void)
{
    uint32_t hardware_timer_tick_cnt;
    uint32_t timer_tick_cnt;

#if 0
    /*for s32k1xx get timer counter(lptimer), get timer count will trigger the period incorrect.*/
    hardware_timer_tick_cnt = lptmr_drv_getcountervaluebycount(inst_lptmr1);
#endif

    timer_tick_cnt = (hardware_timer_tick_cnt & 0xffffu) | (timer_tick_cnt << 16u);

    return timer_tick_cnt;
}

void hal_timer_free(void)
{

}

