#include "derivative.h"
#include "SD.h"
#include "INCLUDES.h"

static void delay1ms(unsigned int n) 
{
    OSTimeDly(1);
}
/*************************************************************/
/*                      ��ʼ��SPIģ��                        */
/*************************************************************/
void SPI_Init(void) 
{
  DDRS    = 0xE0; /*��3λΪ1,1 Associated pin is configured as output.*/    
  SPI0CR2 = 0x10;	/*SS port pin with MODF feature*/
  SPI0CR1 = 0x5e;	/* 0b0101 1110,SPI is in master mode*/
  SPI0BR  = 0x45; //���ò�����Ϊ100k                  
}

/*************************************************************/
/*                    ����SPIʱ��Ϊ4MHz                      */
/*************************************************************/
void SPI_4M(void) 
{ 
  SPI0BR  = 0x11; //���ò�����Ϊ4M                
}

/*************************************************************/
/*                        ��ʼ��SD��                         */
/*************************************************************/
void SD_Init(void)
{
	SPI_Init();
	SD_deselect();
	CD_dir=0;
	WP_dir=0;
}

/*************************************************************/
/*                        ��ջ�����                         */
/*************************************************************/
void clear_buffer(byte buffer[])
{
    int i;     
    for(i=0;i<512;i++)	
		*(buffer+i)=0;
}
 
/*************************************************************/
/*                      SPI��дһ���ֽ�                      */
/*************************************************************/
byte SPI_Byte(byte value)
{
	while (!SPI0SR_SPTEF); /* 1 SPI data register empty*/
	SPI0DR = value;
	while(!(SPI0SR_SPIF)); /*1 New data copied to SPIDR.*/
	return SPI0DR;
}

/*************************************************************/
/*                       ��SD��д������                      */
/*************************************************************/
byte SD_send_command(byte cmd, long arg)
{
	byte a;
	byte retry=0;
	
	SPI_Byte(0xff);
	SD_select();
	
	SPI_Byte(cmd | 0x40);//�ֱ�д������
	SPI_Byte(arg>>24);
	SPI_Byte(arg>>16);
	SPI_Byte(arg>>8);
	SPI_Byte(arg);
	SPI_Byte(0x95);
	
	while((a = SPI_Byte(0xff)) == 0xff)//�ȴ���Ӧ��
		if(retry++ > 10) break;//��ʱ�˳�

	SD_deselect();

	return a;//����״ֵ̬
}

/*************************************************************/
/*                       ��SD��д������                      */
/*************************************************************/
byte SD_Reset(void)
{
	unsigned char i;
	unsigned char retry;
	unsigned char a=0;
	retry = 0;
	do
	{
		for(i=0;i<10;i++) SPI_Byte(0xff);
		a = SD_send_command(0,0);  //����������
		delay1ms(10);
		retry++;
		if(retry>10) return 1;      //��ʱ�˳�
	} while(a != 0x01);


	retry = 0;
	do
	{
		a = SD_send_command(1, 0);  //����������
		delay1ms(10);
		retry++;
		if(retry>100) return 1;      //��ʱ�˳�
	} while(a);
	a = SD_send_command(59, 0);   

	a = SD_send_command(16, 512);//��������С512

	return 0;//��������
}

/*************************************************************/
/*                     ��SD����ȡһ������                    */
/*************************************************************/
byte read_block(long sector, byte* buffer)
{
	byte a;          
	word i;
	a = SD_send_command(17, sector<<9);  //������ 	
	if(a != 0x00) 		return a;

	SD_select();
	//�����ݵĿ�ʼ
	while(SPI_Byte(0xff) != 0xfe);

	for(i=0; i<512; i++)              //��512������
	{
		*buffer++ = SPI_Byte(0xff);
	}

	SPI_Byte(0xff);              
	SPI_Byte(0xff);  	
	SD_deselect();
  	SPI_Byte(0xff);              
	return 0;
} 

/*************************************************************/
/*                     ��SD��д��һ������                    */
/*************************************************************/
byte write_block(long sector, byte* buffer)
{
	byte a;
	word i;
  if(sector<1) return 0xff;     //Ϊ�˱���SD������������������
	a = SD_send_command(24, sector<<9);//д����
	if(a != 0x00) return a;

	SD_select();
	
	SPI_Byte(0xff);
	SPI_Byte(0xff);
	SPI_Byte(0xff);

	SPI_Byte(0xfe);//����ʼ��
	
	for(i=0; i<512; i++)//��512�ֽ�����
	{
		SPI_Byte(*buffer++);
	}
	
	SPI_Byte(0xff);
	SPI_Byte(0xff);
	
	a = SPI_Byte(0xff); 	
	if( (a&0x1f) != 0x05)
	{
	  SD_deselect();
		return a;
	}
	//�ȴ�������
	while(!SPI_Byte(0xff));

  SD_deselect();

	return 0;
} 
