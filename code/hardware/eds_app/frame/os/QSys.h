#ifndef QSys_h
#define QSys_h

#include "task.h"

#define QSys_Version_Main					((unsigned char)0x00)
#define QSys_Version_Sub1					((unsigned char)0x00)
#define QSys_Version_Sub2					((unsigned char)0x00)
#define QSys_Version_Rc						((unsigned char)0x02)

#define QSys_Version							( (QSys_Version_Main << 24) \
																	 |(QSys_Version_Sub1 << 16) \
																	 |(QSys_Version_Sub2 <<  8) \
																	 |(QSys_Version_Rc				) )

/*
**this function will nerver return .It's a dead loop
**It will create a task named "idle" ,do something when 
**there is no task to run .
**And Start the QSys for task switch .Also do something 
**for init system .
*/
void QSysStart(void);

#endif

