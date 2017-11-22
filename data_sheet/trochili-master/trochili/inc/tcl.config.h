/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_CONFIG_H
#define _TCL_CONFIG_H

/* �ں�ʱ�ӽ������ã�Ӳ����ʱ��ÿ���жϴ��� */
#define TCLC_TIME_TICK_RATE             (100U)

/* �߳����ȼ� {0,1,2,30,31} ��5�����ȼ��������ں���,�������ȼ�������û��߳�ʹ�� */
/* �ں�֧�ֵ�������ȼ��� */
#define TCLC_PRIORITY_NUM               (32U)

/* �ں�����߳����ȼ� */
#define TCLC_LOWEST_PRIORITY            (TCLC_PRIORITY_NUM - 1U)

/* �û��߳����ȼ���Χ���� */
#define TCLC_USER_PRIORITY_LOW          (29U)
#define TCLC_USER_PRIORITY_HIGH         (3U)

/* �ں˶������Ƴ��� */
#define TCL_OBJ_NAME_LEN                (16U)

/* �ں˶��Թ������� */
#define TCLC_ASSERT_ENABLE              (1)

/* ջ�����鹦������ */
#define TCLC_THREAD_STACK_CHECK_ENABLE  (1)
#define TCLC_THREAD_STACK_BARRIER_VALUE (0x5A5A5A5A)
#define TCLC_THREAD_STACK_ALARM_RATIO   (90U)         /* n%, ջʹ������ֵ�ٷֱ�         */

/* ����IPC�������� */
#define TCLC_IPC_ENABLE                 (1)
#define TCLC_IPC_SEMAPHORE_ENABLE       (1)
#define TCLC_IPC_MUTEX_ENABLE           (1)
#define TCLC_IPC_MAILBOX_ENABLE         (1)
#define TCLC_IPC_MQUE_ENABLE            (1)
#define TCLC_IPC_FLAGS_ENABLE           (1)

/* ��ʱ���������� */
#define TCLC_TIMER_ENABLE               (1)
#define TCLC_TIMER_WHEEL_SIZE           (32U)

/* �жϹ������� */
#define TCLC_IRQ_ENABLE                 (1)           /* ʹ���жϹ�����               */
#define TCLC_IRQ_VECTOR_NUM             (32U)         /* �����ж������������Ŀ         */
#define TCLC_IRQ_DAEMON_ENABLE          (1)           /* ʹ���첽�жϴ����߳�           */

/* ��̬�ڴ�������� */
#define TCLC_MEMORY_ENABLE              (1)
#define TCLC_MEMORY_POOL_ENABLE         (1)
#define TCLC_MEMORY_POOL_PAGES          (256U)       /* �̶�ҳ���С���ڴ���ܹ��������ڴ�ҳ�� */
#define TCLC_MEMORY_BUDDY_ENABLE        (1)
#define TCLC_MEMORY_BUDDY_PAGES         (64)         /* ����ڴ��㷨�ܹ��������ڴ�ҳ��         */

/* �ں˶�ʱ���ػ��߳����ȼ���ʱ��Ƭ��ջ��С */
#define TCLC_TIMER_DAEMON_PRIORITY      (2U)
#define TCLC_TIMER_DAEMON_SLICE         (10U)
#define TCLC_TIMER_DAEMON_STACK_BYTES   (512U)

/* �ں��ж��ػ��߳����ȼ���ʱ��Ƭ */
#define TCLC_IRQ_DAEMON_PRIORITY        (1U)
#define TCLC_IRQ_DAEMON_SLICE           (10U)
#define TCLC_IRQ_DAEMON_STACK_BYTES     (512U)

/* �ں�ROOT�߳����ȼ���ʱ��Ƭ��ջ��С */
#define TCLC_ROOT_THREAD_PRIORITY       (TCLC_LOWEST_PRIORITY)
#define TCLC_ROOT_THREAD_SLICE          (10U)
#define TCLC_ROOT_THREAD_STACK_BYTES    (512U)

/* �������������� */
#define TCLC_CPU_MINIMAL_STACK          (256U)
#define TCLC_CPU_STACK_ALIGNED          (4U)
#define TCLC_CPU_IRQ_NUM                (73U)
#define TCLC_CPU_CLOCK_FREQ             (72U*1024U*1024U)

#endif /* _TCL_CONFIG_H */
