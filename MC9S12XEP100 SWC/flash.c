#include "flash.h"
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <stdint.h>

#define readWord(address) ((uint16_t)(*(volatile uint16_t *near)(address)))
#define DFLASH_LOWEST_START_PAGE        0x00        //����data flash����ʼҳ
#define DFLASH_START                    0x100000  //����data flash����ʼ��ַ
#define DFLASH_PAGE_SIZE                0x0400      //����data flash�Ĵ�СΪ1K.
#define DFLASH_PAGE_WINDOW_START        0x0800      //����data flashҳ�洰�ڵ���ʼ��ַ
#define DFLASH_ERASE_SECTOR_SIZE 256
#define DFLASH_END 0x1080c00

#define FlashInitOk 0xA5
#define FlashInitFailed 0x5A
#define DFLASH_IS_ERASE_ADDRESS_ALIGNED(a)  ( 0 == ((DFLASH_ERASE_SECTOR_SIZE-1)&(a)) )
#define DFLASH_IS_ERASE_ADDRESS_OK(a)  ( ((a) < DFLASH_END ) && ((a) >= DFLASH_START ))
static unsigned char FlashInitStatus=FlashInitFailed;

void DFlashInit(tFlashParam* FlashParam)
{
	if((FLASH_DRIVER_VERSION_MAJOR == FlashParam->majornumber) ||
		(FLASH_DRIVER_VERSION_MINOR == FlashParam->minornumber) ||
		(FLASH_DRIVER_VERSION_PATCH == FlashParam->patchlevel))
	{
		while(FSTAT_CCIF==0);            /*�ȴ����ڴ����FLASH������� */
		FCLKDIV=0x0F;                    /*�ⲿ����Ϊ16M.FLASHʱ�Ӳ�����1M����������ֲ� ,Flash Clock Divider Register  */
		FCNFG=0x00;                     /*��ֹ�ж�*/
		while(FCLKDIV_FDIVLD==0);        /*�ȴ�ʱ�����óɹ�*/
		FlashParam->errorcode = kFlashOk;
		FlashInitStatus=FlashInitOk;
	}
	else
	{
		FlashParam->errorcode = kFlashFailed;
		FlashInitStatus=FlashInitFailed;
	}   
}
void DFlashDeinit(tFlashParam* FlashParam)
{
	if(FlashInitStatus == FlashInitOk)
	{
		while(FSTAT_CCIF==0);            /*�ȴ����ڴ����FLASH������� */
		FCLKDIV=0x00;                    /*�ⲿ����Ϊ16M.FLASHʱ�Ӳ�����1M����������ֲ� ,Flash Clock Divider Register  */
		FCNFG=0x00;                     /*��ֹ�ж�*/
		while(FCLKDIV_FDIVLD==0);        /*�ȴ�ʱ�����óɹ�*/
		FlashParam->errorcode = kFlashOk;
	}
	else
	{
		FlashParam->errorcode = kFlashFailed;
	}
}
void DFlashErase(tFlashParam* FlashParam)	/*����DFLASH��һ������*/
{
	tAddress address;
	tLength length;
	int i;
	uint16_t ReadData;
	uint8_t lastepage;          //���ڴ洢EPAGE��ֵ
	uint8_t epage;              //���ڼ���EPAGE��ֵ
    lastepage = EPAGE;   //����EPAGE��ֵ
	address=FlashParam->address;
	length=FlashParam->length;
	if(FlashInitStatus == FlashInitOk)
	{
		if((FALSE == DFLASH_IS_ERASE_ADDRESS_ALIGNED(address)) ||
			(FALSE == DFLASH_IS_ERASE_ADDRESS_OK(address)))
		{
			FlashParam->errorcode=kFlashInvalidAddress;
		}
		else if((FALSE == DFLASH_IS_ERASE_ADDRESS_ALIGNED(length)) ||
			(FALSE == DFLASH_IS_ERASE_ADDRESS_OK(address+length)))
		{
			FlashParam->errorcode=kFlashInvalidSize;
		}
		else
		{
			while(length > 0)
			{
				while(FSTAT_CCIF==0);
				if(FSTAT_ACCERR)           //�жϲ������־λ��
				  FSTAT_ACCERR=1;
				if(FSTAT_FPVIOL)           //�жϲ������־λ��
				  FSTAT_FPVIOL=1;
				FCCOBIX_CCOBIX=0x00;
				FCCOB=0x1200 | ((address & 0x00FF0000) >>16);           //д���������͸�λ��ַ
				FCCOBIX_CCOBIX=0x01;
				FCCOB=address & 0x0000FFFF;           //д���16λ�ĵ�ַ
				FSTAT_CCIF=1;           //����ִ������
				while(FSTAT_CCIF==0);   //�ȴ�ִ�����
				length -= DFLASH_ERASE_SECTOR_SIZE;
				address += DFLASH_ERASE_SECTOR_SIZE;
			}
			address=FlashParam->address;
			length=FlashParam->length;
			for(i=0;i<(length/2);i++)
			{
				epage = (byte)((DFLASH_LOWEST_START_PAGE)+(address >>10));   //����EPAGE
				EPAGE=epage;                                                     //��EPAGE��ֵ
				ReadData = readWord((address & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);  //��ȡҳ�洰���е�����
				if( 0xffff != ReadData)
				{
					FlashParam->errorAddress=address;
					FlashParam->errorcode = kFlashFailed;
					break;
				}
				address+=2;
			}
			EPAGE= lastepage;       //�ָ�EPAGE��ֵ
			FlashParam->errorcode = kFlashOk;
		}
	}
	else
	{
		FlashParam->errorcode = kFlashFailed;
	}
}

void DFlashWrite(tFlashParam* FlashParam)	/*��DFLASHд������ */
{
	uint8_t doCount=0;
	uint8_t index=0,i;
	tAddress address;
	tLength length;
	tData *data;
	uint16_t ReadData;
	uint8_t lastepage;          //���ڴ洢EPAGE��ֵ
	uint8_t epage;              //���ڼ���EPAGE��ֵ
	lastepage = EPAGE;   //����EPAGE��ֵ
	address=FlashParam->address;
	length=FlashParam->length;
	data=FlashParam->data;
	if(FlashInitStatus == FlashInitOk)
	{
		if((FALSE == DFLASH_IS_ERASE_ADDRESS_ALIGNED(address)) ||
			(FALSE == DFLASH_IS_ERASE_ADDRESS_OK(address)))
		{
			FlashParam->errorcode=kFlashInvalidAddress;
		}
		else if((0 != (length % 2)) ||
			(FALSE == DFLASH_IS_ERASE_ADDRESS_OK(address+length)))
		{
			FlashParam->errorcode=kFlashInvalidSize;
		}
		else if(FALSE == data)
		{
			FlashParam->errorcode=kFlashInvalidData;
		}
		else
		{
			while(length >0)
			{
				doCount=(length>8 ? 8 : length)/2;
				while(FSTAT_CCIF==0);
				if(FSTAT_ACCERR)           //�жϲ������־λ��
					FSTAT_ACCERR=1;
				if(FSTAT_FPVIOL)           //�жϲ������־λ��
					FSTAT_FPVIOL=1;
				FCCOBIX_CCOBIX=0x00;
				FCCOB=0x1100 | ((address & 0x00FF0000) >>16);         //д������͸�λ��ַ
				FCCOBIX_CCOBIX=0x01;
				FCCOB=address & 0x0000FFFF;         //д���16λ��ַ
				for(i=0;i<doCount;i++)
				{
					FCCOBIX_CCOBIX=0x02+i;  //д���i������
					FCCOB=(uint16_t)(*data << 8); /* ���浽��8λ */
					data++;
					FCCOB |= (uint16_t)(*data); /* ���浽��8λ */
					data++;
				}
				length-=doCount*2;
				address+=doCount*2;
				FSTAT_CCIF=1;         //����ִ������
				while(FSTAT_CCIF==0); //�ȴ�ִ�����
			}
			address=FlashParam->address;
			length=FlashParam->length;
			data=FlashParam->data;
			for(i=0;i<(length/2);i++)
			{
				epage = (byte)((DFLASH_LOWEST_START_PAGE)+(address >>10));   //����EPAGE
				EPAGE=epage;                                                     //��EPAGE��ֵ
				ReadData = readWord((address & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);  //��ȡҳ�洰���е�����
				if(((ReadData >> 8) != *data)) 
				{
					FlashParam->errorAddress=address;
					FlashParam->errorcode = kFlashFailed;
					break;
				}
			    if(((ReadData << 8) != *(++data)))
				{
					FlashParam->errorAddress=address;
					FlashParam->errorcode = kFlashFailed;
					break;
				}
				address+=2;
				data++;
			}
			EPAGE= lastepage;       //�ָ�EPAGE��ֵ
			FlashParam->errorcode = kFlashOk;
		}
	}
	else
	{
		FlashParam->errorcode = kFlashFailed;
	}
}
void DFlashRead(tFlashParam* FlashParam)
{
	uint8_t lastepage;          //���ڴ洢EPAGE��ֵ
	uint8_t epage;              //���ڼ���EPAGE��ֵ
	uint8_t i;
	tAddress address;
	tLength length;
	tData *data;
	uint16_t ReadData;
	lastepage = EPAGE;   //����EPAGE��ֵ
	address=FlashParam->address;
	length=FlashParam->length;
	data=FlashParam->data;
	if(FlashInitStatus == FlashInitOk)
	{
		if((FALSE == DFLASH_IS_ERASE_ADDRESS_ALIGNED(address)) ||
			(FALSE == DFLASH_IS_ERASE_ADDRESS_OK(address)))
		{
			FlashParam->errorcode=kFlashInvalidAddress;
		}
		else if((0 != (length % 2)) ||
			(FALSE == DFLASH_IS_ERASE_ADDRESS_OK(address+length)))
		{
			FlashParam->errorcode=kFlashInvalidSize;
		}
		else if(FALSE == data)
		{
			FlashParam->errorcode=kFlashInvalidData;
		}
		else
		{
			for(i=0;i<length;i++)
			{
				epage = (byte)((DFLASH_LOWEST_START_PAGE)+(address >>10));   //����EPAGE
				EPAGE=epage;                                                     //��EPAGE��ֵ
				ReadData= readWord((address & (DFLASH_PAGE_SIZE - 1)) + DFLASH_PAGE_WINDOW_START);  //��ȡҳ�洰���е�����
				address+=2;
				*data=ReadData >> 8;
				data++;
				*data=ReadData << 8;
				data++;
			}
			EPAGE= lastepage;       //�ָ�EPAGE��ֵ
			FlashParam->errorcode = kFlashOk;
		}
	}
	else
	{
		FlashParam->errorcode = kFlashFailed;
	}
}
