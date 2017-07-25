#include "ATD.h"
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

void atd_init(void)
{
	ATD0CTL2_AFFC=1;	//����A/Dת��,��������,��ֹ�ж�,Changes all ATD conversion complete flags to a fast clear sequence
	ATD0CTL1_SRES=2;  //ѡ��12λģ��ת��
	ATD0CTL3 = 0x88;   //ÿ��ת��1��ͨ��
	ATD0CTL4 = 0x07;   //,ADģ��ʱ��Ƶ��Ϊ2MHz
}

uint16_t atd_capture(uint8_t channel) 
{
	 uint16_t AtdResult;
	 if(channel==0)
	    ATD0CTL5 = 0x00;   //select the analog input channel AN0
	 if(channel==1)
	    ATD0CTL5 = 0x01;   //select the analog input channel AN1
	 while(!ATD0STAT2_CCF0);	//A conversion complete flag is set at the end of each conversion in a sequence
	 AtdResult = ATD0DR0;	//ATD Conversion Result Registers,The A/D conversion results are stored in 16 result registers
	 return(AtdResult);
}