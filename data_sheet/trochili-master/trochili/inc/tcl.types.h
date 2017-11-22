/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2016 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_TYPES_H
#define _TCL_TYPES_H

/* �����������Ͷ��壬�ں���ֲʱ����ȷ�� */
typedef unsigned char      TByte;
typedef char               TChar;
typedef unsigned int       TBase32;
typedef unsigned int       TAddr32;
typedef unsigned int       TReg32;
typedef unsigned int       TIndex;
typedef unsigned int       TPriority;
typedef unsigned int       TBitMask;
typedef unsigned int       TOption;
typedef unsigned int       TProperty;
typedef unsigned long long TTimeTick;
typedef unsigned int       TError;
typedef unsigned int       TArgument;

/* �������Ͷ���                    */
typedef enum
{
    eFalse = 0U,
    eTrue  = 1U
} TBool;

/* API�����������ֵ��ͳһ����  */
typedef enum
{
    eFailure,                            /* ��������ʧ��         */
    eSuccess,                            /* �������óɹ�         */
    eError,                              /* �������÷�������     */
} TState;

#define TCLM_OFFSET_OF(TYPE, MEMBER) ((TBase32)(&(((TYPE *)0)->MEMBER)))


/* �Ĵ�����д�궨�� */
#define TCLM_GET_REG32(r)   (*((volatile unsigned int*)(r)))
#define TCLM_SET_REG32(r,v) (*((volatile unsigned int*)(r)) = ((unsigned int)(v)))

/* ��ֵ���޶��� */
#define TCLM_MAX_VALUE08    (0xff)
#define TCLM_MAX_VALUE16    (0xffff)
#define TCLM_MAX_VALUE32    (0xffffffff)
#define TCLM_MAX_VALUE64    (0xffffffffffffffff)

#endif /* _TCL_TYPES_H */

