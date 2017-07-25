#include "SCI.h"
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "PLL.h"


void sci_init(void)
{
	SCI0BD = BUS_CLOCK/16/115200;   //����SCI0������Ϊ115200
	SCI0CR1 = 0x00;       //����SCI0Ϊ����ģʽ����λ����λ������żУ��
	SCI0CR2 = 0b00101100;       //���������ݺͽ�������,enable interrupts 
}

void sci_send(uint8_t data) 
{
  while(!SCI0SR1_TDRE);       //Transmit Data Register Empty Flag
  SCI0DRL = data;
}

void sci_send_string(uint8_t *putchar) 
{
  while(*putchar!=0x00)       //�ж��ַ����Ƿ������
  {
   SCI_send(*putchar++);  
  }
}
uint8_t sci_receive(void) 
{
  while(!SCI0SR1_RDRF);          //Receive Data Register Full Flag
  return(SCI0DRL);
}
