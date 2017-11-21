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

#define simple_fsm(neme,...)	\
	Class(neme) {	\
		C_task task;	\
		__VA_ARGS__	\
	}l##neme;

#define fsm_going(name)	\
	if(l##name.task.run == 1) { \
		UpdateTimer(l##name); 	\
		RunTask(name, l##name);	\
	}	

/*任务参数初始化*/
#define fsm_task_init(name) l##name.task.run = 0;l##name.task.timer = 0,l##name.task.lc = 0
/*任务标志位 打开*/
#define fsm_task_on(name) l##name.task.run = 1 /*开启任务*/
/*任务标志位 关闭*/
#define fsm_task_off(name) l##name.task.run = 0

void runtasks(void);

/**********************************驱动框架***************************************/
struct device {
    char name[10];/*设备名称*/
    
};

// typedef struct C_task C_task; 
// struct C_task {
//   TimeDef run;
//   TimeDef timer;
//   LineDef lc;
// };

//	do { 
//		if((lLedTask2.task.timer!=0)&&(lLedTask2.task.timer!=END)) 
//			lLedTask2.task.timer--; 
//	} while(0);
//		
//	do { 
//		if(lLedTask2.task.timer==0) 
//			lLedTask2.task.timer=LedTask2(&(lLedTask2)); 
//	} while(0);

//不对不是常量
//#define b_WaitX(ticks)      do { me.task.lc=(LINE+me.task.pc++); return (ticks); case (LINE+me.task.pc):;} while(0)

//unsigned short LedTask2(C_LedTask2 *cp) {
//	switch(me.task.lc){
//		default:{
//			me.timelen=50;
//			while(1) {
//				//WaitX(me.timelen-me.timeon);  
//				do { me.task.lc=LINE; return (0); case LINE:;} while(0);
//				x = LINE;
//				do { me.task.lc=LINE; return (me.timelen-me.timeon); case LINE:;} while(0);
//				USARTSendDMA("C_LedTask2.\r\n");
//				GPIO_WriteBit(GPIOC,GPIO_Pin_7,(BitAction)!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7));
//				if(x == 20) {
//					me.pin--;
//				}
//				if(me.pin == 0) {
//					me.pin = 10;
//					GPIO_WriteBit(GPIOC,GPIO_Pin_7,(BitAction)1);
//					CallSub(LedTask, led1);
//				} else {
//					me.pin--;
//				}
//			}
//		}
//	} 
//	do { 
//		me.task.lc=0; 
//		return END; 
//	}while(0); 
//}

//typedef struct C_LedTask2 C_LedTask2; 
//struct C_LedTask2 {
//	C_task task;
//	unsigned char pin;
//	unsigned char timeon;
//	unsigned char timelen; 
//}led2;

//Class(LedTask)
//{
//  C_task task;
//  unsigned char pin;
//  unsigned char timeon;
//  unsigned char timelen;
//}led1;

//	do { 
//		if((lLedTask2.task.timer!=0)&&(lLedTask2.task.timer!=END)) 
//			lLedTask2.task.timer--; 
//	} while(0);
//		
//	do { 
//		if(lLedTask2.task.timer==0) 
//			lLedTask2.task.timer=LedTask2(&(lLedTask2)); 
//	} while(0);


//typedef char * va_list;     // TC中定义为void*
//#define _INTSIZEOF(n)    ((sizeof(n)+sizeof(int)-1)&~(sizeof(int) - 1) ) //为了满足需要内存对齐的系统
//#define va_start(ap,v)    ( ap = (va_list)&v + _INTSIZEOF(v) )     //ap指向第一个变参的位置，即将第一个变参的地址赋予ap
//#define va_arg(ap,t)       ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )   /*获取变参的具体内容，t为变参的类型，如有多个参数，则通过移动ap的指针来获得变参的地址，从而获得内容*/
//#define va_end(ap) ( ap = (va_list)0 )   //清空va_list，即结束变参的获取
///** 
// *  求不定个参数的和 
// * 
// *  @param count 不定参数的个数 
// *  @param ...   不定参数的列表 
// * 
// *  @return 返回不定参数列表的和 
// */  
//int sum(int v, ...) {  
//	int ReturnValue=0;
//	int i=v;
//	va_list ap ;
//	va_start(ap,v);
//	while(i>0)
//	{
//		ReturnValue+=va_arg(ap,int) ;
//		i--;
//	}
//	va_end(ap); 
//	return ReturnValue/=v;
//}  

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *new,
                          struct list_head *prev,
                          struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

#define __list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_entry(pos, head, member)                          \
    for (pos = list_entry((head)->next, typeof(*pos), member);      \
         prefetch(pos->member.next), &pos->member != (head);        \
         pos = list_entry(pos->member.next, typeof(*pos), member))



#endif
