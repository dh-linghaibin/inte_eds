/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _L_OS_H_
#define _L_OS_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include <stdint.h>

#define MAX_TASK	32

/*
**Next marco you cannot change!
*/
#define IDLE_PID			(0)
#define LOWEST_PID			(1)
#define HIGHEST_PID			(MAX_TASK-1)

/*
**Function in switch.S
*/
extern void taskSwOn(void);
extern void taskSwOff(void);
extern void swTask(void);
extern void swTaskAtInterrupt(void);
extern int  cntHeadZero(unsigned int );

typedef void (*taskEntry)(void);
struct task 
{
	unsigned int *sp;
	/*for program easy,sp must the first element in struct*/
	unsigned int pid;
	taskEntry entry;
	
	char *taskName;
	/*save the base stack point,used when free memory*/
	unsigned int *spBase; 
	
	/*This value used for resume task,when osTicks == nextRdyTime,
	**at function updateNextTask .
	**if this value was setted as 0,then the task cann't resume
	**at function updateNextTask*/
	unsigned int nextRdyTime;
};

extern unsigned int curPid;
extern unsigned int nextPid;
extern struct task *curTaskPtr;
extern struct task *nextTaskPtr;

//get first ready task .
//set the first ready task's pid to nextPid,
//		 first ready task to nextTaskPtr
//return value is nextTaskPtr
struct task *updateNextTask(void);

#define TASK_ERR_NO_MEMORY				-1
#define TASK_ERR_PID_USED				-2
#define TASK_ERR_PID_ILLEGAL			-3
#define TASK_ERR_NO_SUCH_TASK			-4

int taskDeleteSelf(void);
int taskDelete(unsigned int pid);
int taskCreate(struct task *task,unsigned int pid,taskEntry entry,
							 unsigned int *spBase,unsigned int spLen,const char *name);

/*
**Change task's pid.If the task exist.
*/
int changeSelfPid(unsigned int newPid);
int changePid(unsigned int oldPid,unsigned int newPid);

/*
**suspend the task itself
*/
void suspend(void);
void resume(unsigned int pid);

/*
**block task 'n' ticks
*/
void sleep(int delayTicks);



#define QSys_Version_Main		((unsigned char)0x00)
#define QSys_Version_Sub1		((unsigned char)0x00)
#define QSys_Version_Sub2		((unsigned char)0x00)
#define QSys_Version_Rc			((unsigned char)0x02)

#define QSys_Version			( (QSys_Version_Main << 24) \
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

#ifdef __cplusplus
}
#endif

#endif
