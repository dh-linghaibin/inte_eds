/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "iwdg.h"


void iwdg_init(struct _iwdg_obj *iwdg) {
	 /* enable IRC40K */
    rcu_osci_on(RCU_IRC40K);
  
    /* wait till IRC40K is ready */
    while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K)){}  
    /* confiure FWDGT counter clock: 40KHz(IRC40K) / 64 = 0.625 KHz */
    fwdgt_config(2*500, FWDGT_PSC_DIV256);  
    /* after 1.6 seconds to generate a reset */
    fwdgt_enable();

    /* check if the system has resumed from FWDGT reset */
    if(RESET != rcu_flag_get(RCU_FLAG_FWDGTRST)){
        /* clear the FWDGT reset flag */
        rcu_all_reset_flag_clear();
    } else {
		/* not fwdog reset */
    }
}

void iwdg_reload(struct _iwdg_obj *iwdg) {
	fwdgt_counter_reload();
}


void iwdg_register(void) {
	struct _iwdg_obj *iwdg = GET_DAV(struct _iwdg_obj);

	iwdg->init 	 = &iwdg_init;
	iwdg->reload = &iwdg_reload;

    register_dev_obj("iwdg",iwdg);
}
