#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sram.h"
#include "list.h"

static doublelist* init_doublelist(void) {
	doublelist* list;
	list = (doublelist *)SramMalloc(sizeof(double));
	if(list == NULL) {
		return NULL;
	}
	list->next = NULL;
	list->down = NULL;
	return list;
}

static int count_doublelist(doublelist* head) {
	int len;
	while(head->next) {
		len++;
		head = head->next;
	}
	return len;
}

static void print_doublelist(doublelist* list) {
	doublelist* head = list; 
	while(head->next) {
		head = head->next;	
	}
	head = list;
	while(head->down){
		head = head->down;
	}
	return;
} 

static void free_doublelist(doublelist** list){
	doublelist* head;
	doublelist* now;
	head = *list;
	while(head->next) {
		now = head->next;
		head = head->next;
		if(now) {
			SramFree((void *)now);
			now = NULL;
		}
	}
	return ;
}

void sss(void) {
	//printf("get fount\n");
}

static int insert_doublelist(doublelist ** list,const char *name,device *dev) {
	doublelist *head = *list;
	doublelist *downhead = *list;
	if(list == NULL) {
		return -1;	
	}	
	doublelist* point = NULL;
	point = init_doublelist();
	if(point == NULL) {
		return -1;
	}
	strcpy(point->name,name);
	point->dev = dev;
	/*双向链表中还没有节点*/
	if(head->next == NULL || head->down == NULL) {
		head->next = point;
		head->down = point;
		return 0;
	}	
	while(head->next) {
		/*判断有没有这个名字注册*/
		if(!strcmp(name,head->name)) {
			return -2;
		}
		head = head->next;
	}
	//插入到最后
	if(head->next == NULL) {
		head->next = point;
		point->down = head;
		downhead->down = point;
	} else {
		point->next = head->next;	
		point->down = head;
		head->next->down = point;
		head->next = point; 
	}
	return 0;
}

static int delete_doublelist(doublelist **list,const char * name) {
	doublelist* head = *list;
	doublelist* now = NULL;
	doublelist* last = NULL;
	if(head == NULL) {
		return -1;
    }
	if(strcmp(name,head->next->name) == 0) {
		now = head->next;
		head->next = now->next;
		head->next->down = NULL;
		if(now) {
			SramFree((void*) head);
			now = NULL;
		}
		return 0;
	}
	while(head->next && strcmp(name,head->name)) {
		head = head->next;
	}
	//delet last head
	if(head->next == NULL) {
		now = *list;  
        head->down->next = NULL;
        now->down = head->down;
        if(head) {
            SramFree((void *)head);
            head = NULL;
        }
	} else {
		//delet center head
		now = head->next;
        last = head->down;
        // now->down = head->down;
        // last->next = head->next;
		now->down = last;
		last->next = now;
        if(head) {
            SramFree((void *)head);
            head = NULL;
        }
	}
	return 0;
}

static doublelist* find_doublelist(doublelist *head,const char *name) {
	while(head->next) {
		if(strcmp(head->name,name) == 0){
			return head;
		}
		head = head->next;
	}
	return 0;
}

static device* get_device(doublelist *head,const char *name) {
	while(head->next) {
		if(strcmp(head->name,name) == 0){
			return head->dev;
		}
		head = head->next;
	}
	return 0;
}

void led_write(unsigned char type,...) {
	USARTSendDMA("led writer.\r\n");
}

int list_test(void){
	doublelist *head = init_doublelist();
	if(head == NULL) {
		return -1;
	}
	device *led;/*设备*/
	led = (device *)SramMalloc(sizeof(device));
	led->write = &led_write;
	insert_doublelist(&head,"l",led);
	insert_doublelist(&head,"g",led);
	insert_doublelist(&head,"r",led);
	insert_doublelist(&head,"y",led);
	insert_doublelist(&head,"b",led);
	insert_doublelist(&head,"d",led);
	insert_doublelist(&head,"ss",led);
	insert_doublelist(&head,"dd",led);
	print_doublelist(head);
	//delete_doublelist(&head,"g");
	//print_doublelist(head);
	//led.write(1);
	device * led2 = get_device(head,"x");
	if(led2 != NULL) {
		led2->write(2);
	}
	free_doublelist(&head);
	return 0;
}





