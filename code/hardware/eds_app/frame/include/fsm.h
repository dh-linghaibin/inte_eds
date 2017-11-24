/*
 * This file is part of the 
 *
 * Copyright (c) 2016-2017 linghaibin
 *
 */

#ifndef _FSM_H_
#define _FSM_H_

// 3，得到指定地址上的一个字节或字
#define MEM_B( x ) ( *( (byte *) (x) ) )
#define MEM_W( x ) ( *( (word *) (x) ) )
// 4，求最大值和最小值
#define MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#define MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
// 5. 得到一个 field 在结构体(struct)中的偏移量
#define FPOS( type, field ) \
( (dword) &(( type *) 0)-> field )
// 6. 得到一个结构体中 field 所占用的字节数
#define FSIZ( type, field ) sizeof( ((type *) 0)->field )

// 返回数组元素的个数
#define ARR_SIZE( a ) ( sizeof( (a) ) / sizeof( (a[0]) ) )


/****small*********************************************/
#define TimeDef         unsigned short
#define LineDef         unsigned int//unsigned char
#define END             ((TimeDef)-1)
#define LINE            ((__LINE__%(LineDef)-1)+1)  

#define me  (*cp)
#define TaskFun(TaskName)     TimeDef TaskName(C_##TaskName *cp) {switch(me.task.lc){default:
#define Exit            do { me.task.lc=0; return END; }  while(0)
#define Restart           do { me.task.lc=0; return 0; }  while(0)
#define EndFun            } Exit; }

#define WaitX(ticks)      do { me.task.lc=LINE; return (ticks); case LINE:;} while(0)
#define l_WaitX(number,ticks)      do { me.task.lc=(LINE+number); return (ticks); case (LINE+number):;} while(0)
#define WaitUntil(A)      do { while(!(A)) WaitX(0);} while(0)
#define l_WaitUntil(number,A)      do { while(!(A)) l_WaitX(number,0);} while(0)

#define UpdateTimer(TaskVar)    do { if((TaskVar.task.timer!=0)&&(TaskVar.task.timer!=END)) TaskVar.task.timer--; }  while(0)
#define RunTask(TaskName,TaskVar) do { if(TaskVar.task.timer==0) TaskVar.task.timer=TaskName(&(TaskVar)); }  while(0)

#define CallSub(SubTaskName,SubTaskVar)    do { WaitX(0);SubTaskVar.task.timer=SubTaskName(&(SubTaskVar));      \
                  if(SubTaskVar.task.timer!=END) return SubTaskVar.task.timer;} while(0)

#define l_CallSub(number,SubTaskName)    do { l_WaitX(number,0);l##SubTaskName.task.timer=SubTaskName(&(l##SubTaskName));      \
                  if(l##SubTaskName.task.timer!=END) return l##SubTaskName.task.timer;} while(0)
/****************************************************************/
#define Class(type)         typedef struct C_##type C_##type; struct C_##type

Class(task) {
	TimeDef run;
	TimeDef timer;
	LineDef lc;
};

//#define cortex(name,...)	unsigned short name(C_##name *cp) {	switch(me.task.lc){	default:{  __VA_ARGS__	}}do {me.task.lc=0;return END;}while(0);}
#define fsm_initialiser(name,...)	unsigned short name(C_##name *cp) {	switch(me.task.lc){	default:{  __VA_ARGS__	}}do {me.task.lc=0;return END;}while(0);}
#define fsm_init_name(name) 		unsigned short name(C_##name *cp) {	switch(me.task.lc){	default:{
#define fsm_end						}}do {me.task.lc=0;return END;}while(0);}

#define simple_fsm(name,...)	\
	Class(name) {	\
		C_task task;	\
		__VA_ARGS__	\
	}l##name; \
	unsigned short name(C_##name *cp);

#define fsm_going(name)	\
	if(l##name.task.run == 1) { \
		UpdateTimer(l##name); 	\
		RunTask(name, l##name);	\
	}	

/*声明*/
//#define extern_fsm_initialiser(name) typedef struct C_##name C_##name; C_##name l##neme;unsigned short name(C_##name *cp) 
/*任务参数初始化*/
#define fsm_task_init(name) l##name.task.run = 0;l##name.task.timer = 0,l##name.task.lc = 0
/*任务标志位 打开*/
#define fsm_task_on(name) l##name.task.run = 1 /*开启任务*/
/*任务标志位 关闭*/
#define fsm_task_off(name) l##name.task.run = 0

#endif
