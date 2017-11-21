#include "dev_obj.h"

/*头结点*/
static dev_obj head_obj;

static dev_obj* init_dev_obj(void) {
	dev_obj* list;
	list = (dev_obj *)SramMalloc(sizeof(dev_obj));
	if(list == NULL) {
		return NULL;
	}
	list->next = NULL;
	list->down = NULL;
	return list;
}

//static void free_doublelist(dev_obj** list) {
//	dev_obj* head;
//	dev_obj* now;
//	head = *list;
//	while(head->next) {
//		now = head->next;
//		head = head->next;
//		if(now) {
//			SramFree((void *)now);
//			now = NULL;
//		}
//	}
//	return ;
//}

int register_dev_obj(const char *name,void *dev) {
	dev_obj *head = &head_obj;
	dev_obj *downhead = &head_obj;	
	dev_obj* point = NULL;
	point = init_dev_obj();
	if(point == NULL) {
		return -1;
	}
	strcpy(point->name,name);
    point->dev = dev;
	//lhb *l = (lhb *)SramMalloc(sizeof(lhb));
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

int uregister_dev_obj(const char * name) {
	dev_obj* head = &head_obj;
	dev_obj* now = NULL;
	dev_obj* last = NULL;
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
		now = &head_obj;  
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
		now->down = last;
		last->next = now;
        if(head) {
            SramFree((void *)head);
            head = NULL;
        }
	}
	return 0;
}

void * get_device(const char *name) {
	dev_obj *head = &head_obj;
	while(head->next) {
		if(strcmp(head->next->name,name) == 0){
			return head->next->dev;
		}
		head = head->next;
	}
	return 0;
}
