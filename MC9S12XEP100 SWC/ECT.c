#include "ECT.h"
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

void ECT_init(void)
{
  ECT_TSCR1_TFFCA = 1;  // ��ʱ����־λ�������
  ECT_TSCR1_TEN = 1;    // ��ʱ��ʹ��λ. 1=����ʱ����������; 0=ʹ����ʱ����������(����������)
  ECT_TIOS  = 0xff;      //ָ������ͨ��Ϊ����ȽϷ�ʽ
  ECT_TCTL1 = 0x00;	    // ���ĸ�ͨ������Ϊ��ʱ����������ŶϿ�
  ECT_TCTL2 = 0x00;     // ǰ�ĸ�ͨ������Ϊ��ʱ����������ŶϿ�
  ECT_DLYCT = 0x00;	    // �ӳٿ��ƹ��ܽ�ֹ
  ECT_ICOVW = 0x00;	    // ��Ӧ�ļĴ�����������;  NOVWx = 1, ��Ӧ�ļĴ�����������
  ECT_ICSYS = 0x00;	    // ��ֹIC��PAC�ı��ּĴ���
  ECT_TIE   = 0x01;     // ����ͨ��0��ʱ�ж�
  ECT_TSCR2 = 0x07;	    // Ԥ��Ƶϵ��pr2-pr0:111,,ʱ������Ϊ4us,
  ECT_TFLG1 = 0xff;	    // �����IC/OC�жϱ�־λ
  ECT_TFLG2 = 0xff;     // ������ɶ�ʱ���жϱ�־λ
  
  ECT_TFLG1_C0F = 1;
  ECT_TC0 = ECT_TCNT + 1250;         //��������Ƚ�ʱ��Ϊ5ms
}

#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt void scan(void)	//VECTOR ADDRESS 0xffee scan
{
  if(ECT_TFLG1_C0F == 1)
  {
    ECT_TFLG1_C0F = 1;
    ECT_TC0 = ECT_TCNT + 1250;         //��������Ƚ�ʱ��Ϊ5ms
  }
  ShuMaGuan_ScanDisplayNumber();
}

#pragma CODE_SEG DEFAULT