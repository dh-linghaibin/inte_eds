#include "QSys.h"

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


