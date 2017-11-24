/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#include "l_os.h"
#include <stdio.h>

static unsigned int idleSp[64];
static struct task idleTask;

static void idleEntry(void);
extern void firstStartTask(void); //function in switch.S

void QSysStart(void)
{
	/*Create a idle task if user can't create in main.c*/
	taskCreate(&idleTask,IDLE_PID,idleEntry,idleSp,64,"Idle");
	
	/*select the task to be run ,At first time,curTaskPtr is 
	nextTaskPtr*/
	updateNextTask();
	curPid = nextPid;
	curTaskPtr = nextTaskPtr;
	
	firstStartTask();
}


static void idleEntry(void)
{
	while(1)
	{
		//do nothing ...
	}
}


unsigned int curPid,nextPid;
static unsigned int taskRdyBit = 0x00;
struct task *curTaskPtr,*nextTaskPtr;
static struct task *taskList[MAX_TASK]={NULL};


#define taskListClean(pid)				(taskList[pid]=NULL)
#define taskListSet(pid,task)			(taskList[pid]=task)
#define headZero()								(cntHeadZero(taskRdyBit))

/*check the task specified by pid is ready or not .If ready,result is non-zero */
#define taskIsRdy(pid)						(taskRdyBit &		(1ul<<(pid)))
#define addPidToRdyBit(pid)				(taskRdyBit |=  (1ul<<(pid)))
#define cleanPidFromRdyBit(pid)		(taskRdyBit &= ~(1ul<<(pid)))
//////////////////////////////////////////////////////////////////

static volatile unsigned int ticks = 0;
static void taskInit(struct task *task);

static unsigned char run_flag = 0;
/////////////////////////////////////////////////////////////////
/*
**Set nextPid and nextTaskPtr,before switch a 
**task ,must call this function first. Call
**this function at systick_handler before switch
**task .
*/
struct task *updateNextTask(void)
{
	/*Updata block information*/
	struct task *task ;
	for(int pid=0;pid<MAX_TASK;pid++)
	{
		
			task = taskList[pid];
			if((task==NULL)||(task->nextRdyTime == 0)) {
				continue;
			}
			else //a task was block,check it's nextRdyTime
			{
				if(ticks == task->nextRdyTime){
					task->nextRdyTime = 0;
					addPidToRdyBit(pid);
				}
			}
		
	}
	/*Bit		31	30	29	28	27	26...
	**Zero	0		1		2		3		4		5 ...
	**Pid		31	30	29	28	27	26...
	**HighestPriority=============>>LowestPriority
	*/
	int zero = headZero();
	nextPid = HIGHEST_PID - zero;
	nextTaskPtr = taskList[nextPid];
	return nextTaskPtr;
}

/*
**create a task ,only need pid,name and entry point.
**stack memory and task created by using malloc.
*/
int taskCreate(struct task *task,unsigned int pid,taskEntry entry,
							 unsigned int *spBase,unsigned int spLen,const char *name)
{
	if( pid > HIGHEST_PID)
		return TASK_ERR_PID_ILLEGAL;
	if( taskList[pid] )
		return TASK_ERR_PID_USED;
	
	task->pid = pid;
	/*In order to delete task and free memory*/
	task->spBase = spBase;
	/*!!!stack top!!!*/
	task->sp = spBase+(sizeof(unsigned int)*spLen);
	task->entry = entry;
	task->taskName = (char *)name;
	task->nextRdyTime = 0;
	
	taskSwOff();
		addPidToRdyBit(pid);
		taskListSet(pid,task);
		taskInit(task);
	taskSwOn();
	run_flag = 1;

	return 0;
}

/*
**delete the task specified by pid from taskList
**And free this task's memory .
*/
int taskDelete(unsigned int pid)
{
	if( taskList[pid]==NULL ) //No such task 
		return TASK_ERR_NO_SUCH_TASK;
	if( pid == IDLE_PID)
		return TASK_ERR_PID_ILLEGAL; //cannot delete idle task 
	
	taskSwOff();
		cleanPidFromRdyBit(pid);
		taskListClean(pid);
	taskSwOn();
	
	if(pid == curPid) //delete self
	{
		updateNextTask();
		swTask();
	}
	
	return 0;
}

int taskDeleteSelf(void)
{
	unsigned int pid;
	taskSwOff();
		pid = curPid;
	taskSwOn();
	
	return taskDelete(pid);	
}

/*
**suspend current task,and switch task now 
*/
void suspend(void)
{
	unsigned int pid;
	
	taskSwOff();
		pid = curPid;
	taskSwOn();
	
	if(taskIsRdy(pid))//task is ready,need suspend
	{
		taskSwOff();
			cleanPidFromRdyBit(pid);
		taskSwOn();
		
		/*set nextRdyTime as zero,otherwise it can be resume
		when tick = nextRdyTime*/
		taskList[pid]->nextRdyTime = 0;
		
		updateNextTask();
		swTask();
	}
}

void resume(unsigned int pid)
{
	if(taskIsRdy(pid)||(taskList[pid]==NULL))
		return ;//task already resume,or task was deleted
	
	taskSwOff();
		addPidToRdyBit(pid);
	taskSwOn();
	
	/*task was resume,so doen't need resume 
	**after delay at function updateNextTask*/
	taskList[pid]->nextRdyTime = 0;
	
	updateNextTask();
	swTask();
}

/*
**
*/
int changePid(unsigned int oldPid,unsigned int newPid)
{
	struct task *task = taskList[oldPid];
	
	if((oldPid == IDLE_PID)||(oldPid > HIGHEST_PID))
		return TASK_ERR_PID_ILLEGAL; //Idle task priority can't change
	if(task==NULL) 
		return TASK_ERR_NO_SUCH_TASK;//No such task 
	if(oldPid == newPid)
		return 0; //change pid is done!
	
	taskSwOff();
		cleanPidFromRdyBit(oldPid); //clean oldPid bit from taskRdyBit
		taskListClean(oldPid);
		addPidToRdyBit(newPid);
		taskListSet(newPid,task);
	taskSwOn();
	
	updateNextTask();
	swTask();
	
	return 0;
}

int changeSelfPid(unsigned int newPid)
{
	unsigned int pid;
	taskSwOff();
		pid = curPid;
	taskSwOn();
	
	int rev = changePid(pid,newPid);
	
	return (rev?rev:0);
}

/*
**block current task about 'delayTicks' time
*/
void sleep(int delayTicks)
{
	unsigned int pid;
	
	taskSwOff();
		pid = curPid;
	taskSwOn();
	
	if((taskList[pid]==NULL)||(pid==IDLE_PID)||(delayTicks==0))
		return ; //Not a task,just return 
	
	taskList[pid]->nextRdyTime = ticks + delayTicks;
	cleanPidFromRdyBit(pid); //clean pid bit from ready bit.just block it .
	
	updateNextTask();
	swTask();
}

//////////////////////////////////////////////////////////////////

/*
**Initial task stack,include R0-R3,
**R12,LR,PC,xPSR,R4-R11 .
*/
static void taskInit(struct task *task)
{
	unsigned int *sp = task->sp;
	
	*(sp)   = (unsigned int)0x01000000;		//xPSR
	*(--sp) = (unsigned int)task->entry;	//PC
	*(--sp) = (unsigned int)0xFFFFFFFE;		//LR
	*(--sp) = (unsigned int)0x12121212;		//R12
	*(--sp) = (unsigned int)0x03030303;		//R3
	*(--sp) = (unsigned int)0x02020202;		//R2
	*(--sp) = (unsigned int)0x01010101;		//R1
	*(--sp)	= (unsigned int)0x00000000;		//R0
	
	*(--sp)	= (unsigned int)0x11111111;		//R11
	*(--sp)	= (unsigned int)0x10101010;		//R10
	*(--sp)	= (unsigned int)0x09090909;		//R9
	*(--sp)	= (unsigned int)0x08080808;		//R8
	*(--sp)	= (unsigned int)0x07070707;		//R7
	*(--sp)	= (unsigned int)0x06060606;		//R6
	*(--sp)	= (unsigned int)0x05050505;		//R5
	*(--sp)	= (unsigned int)0x04040404;		//R4
	
	task->sp = sp;
	
}

////////////////////////////////////////////////////////////////////////
//void SysTick_Handler(void) {
//	if(run_flag == 1) {
//		ticks ++;
//		updateNextTask();
//		swTask();
//	}
//}

