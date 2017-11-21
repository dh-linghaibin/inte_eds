
#include "rtc.h"

static void (*rtc_callback)(void); 

/*!
    \brief      configure the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void nvic_configuration(void) {
	/* init the exti interrupt */
	exti_init(EXTI_17,EXTI_INTERRUPT,EXTI_TRIG_RISING);
	/* nvic grout set */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE3_SUB1);
	/* eaable atc_alarm interrput */
    nvic_irq_enable(RTC_Alarm_IRQn,0U, 0U);
}

/*!
    \brief      configure the RTC
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void rtc_configuration(void) {
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable LXTAL */
    rcu_osci_on(RCU_IRC40K);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_IRC40K);
    
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC alarm interrupt*/
    rtc_interrupt_enable(RTC_INT_ALARM);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
	
	/* set RTC prescaler: set RTC period to 1s */
	rtc_counter_set(0xfffffff0);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

	/* set RTC prescaler: set RTC period to 1s */
	rtc_alarm_config(rtc_counter_get() + 15);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait(); 
}


static void rtc_init(struct _rtc_obj* rtc) {
	nvic_configuration();
	rtc_configuration();
	//bkp_write_data(BKP_DATA_0, 0xA5A5);
	/* clear reset flags */
    rcu_all_reset_flag_clear();
}

void rtc_register(void) {
	struct _rtc_obj *rtc = GET_DAV(struct _rtc_obj);

	rtc->init = &rtc_init;

    register_dev_obj("rtc",rtc);
}


/*!
    \brief      this function handles RTC global interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void RTC_IRQHandler(void)
{
    if (rtc_flag_get(RTC_FLAG_SECOND) != RESET){
        /* clear the RTC second interrupt flag*/
        rtc_flag_clear(RTC_FLAG_SECOND);

		//rtc_prescaler_set(65535);
		printf("rtc \r\n");
		if(rtc_callback != 0) {
			rtc_callback();
		}
    } 
	
	if (rtc_flag_get(RTC_FLAG_ALARM) != RESET) {
		/* clear the RTC alarm interrupt flag*/
        rtc_flag_clear(RTC_FLAG_ALARM);

		/* set RTC prescaler: set RTC period to 1s */
		rtc_alarm_config(rtc_counter_get() + 10);

		/* wait until last write operation on RTC registers has finished */
		rtc_lwoff_wait(); 
	}
}

void RTC_Alarm_IRQHandler(void) {
	if (rtc_flag_get(RTC_FLAG_ALARM) != RESET) {
		/* clear the RTC alarm interrupt flag*/
        rtc_flag_clear(RTC_FLAG_ALARM);
		/* clear the extu_17 interupt flag */
		exti_interrupt_flag_clear(EXTI_17);

		/* set RTC prescaler: set RTC period to 1s */
		rtc_alarm_config(rtc_counter_get() + 15);
		/* wait until last write operation on RTC registers has finished */
		rtc_lwoff_wait(); 
	}
}

