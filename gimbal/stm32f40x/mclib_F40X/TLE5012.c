/******************************************************************************/
/*                                                                            */
/*                                 TLE5012.c                                  */
/*                     Dreamer Rongfei.Deng 2015.9.24                         */
/*                                                                            */
/******************************************************************************/


/*============================================================================*/
/*                               Header include                               */
/*============================================================================*/
#include <board.h>
#ifdef RT_USING_FINSH
#include "finsh.h"
#endif
#include "app_config.h"
#include "TLE5012.h"
/*============================================================================*/
/*                              Macro Definition                              */
/*============================================================================*/
#define SPI_Module_CS2_LOW()      GPIO_ResetBits(GPIOA, GPIO_Pin_4) //3
#define SPI_Module_CS2_HIGH()     GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define SPI_READ_WRITE_B          0x4000
#define SPI_DUMMY_BYTE            0xffff

/*tle5012b�ļĴ�����ַ*/
#define STAT_REGISTER    0x8001
#define AVAL_REGISTER    0x8021
#define ASPD_REGISTER    0x8032
/*============================================================================*/
/*                              Global variables                              */
/*============================================================================*/
static SPI_InitTypeDef SPI_InitStructure; 
static GPIO_InitTypeDef GPIO_InitStructure;

/*============================================================================*/
/*                             Function definition                            */
/*============================================================================*/
static void spi_enc(void)
{

//	SPI_InitTypeDef SPI_InitStructure;
	
  
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1  , ENABLE);//RCC_APB2Periph_AFIO

	// spi clk , mosi , miso pin config
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);
	
	// spi cs pin config
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	SPI_Module_CS2_HIGH(); 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//SPI_Direction_1Line_Rx  SPI_Direction_1Line_Tx
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;//SPI_DataSize_8b
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; 
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//SPI_BaudRatePrescaler_256 SPI_BaudRatePrescaler_128  SPI_BaudRatePrescaler_64
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;// SPI_FirstBit_LSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE);
}


static rt_uint16_t tle5012b_output_dat;
static rt_uint16_t tle5012b_spi_SendByte(rt_uint16_t byte)
{	
	SPI_Module_CS2_LOW(); //����CS�ܽŵ�ƽ��ʹ��SPI����ȡtle5012����
	
  /* Loop while DR register in not emplty */
	/*����ͼд���ͻ�����֮ǰ����ȷ��TXE��־Ϊ��1���������ͻ�����Ϊ��*/
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){};//TXE��־��Ϊ1ʱ���ȴ�
  SPI_I2S_SendData(SPI1, byte);

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){};
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET){};
	/*
		��ΪӲ����SPI_MOSI��SPI_MISO�������ܽ�������һ��( tle5012ֻ��һ��data�ܽ� ) ��
		���Ե��������ַ��Ҫ����tle5012�����������ݵ�ʱ�򣬰�SPI_MOSI�ܽ�����Ϊ��©������ҷ���0xFF
		��Ϊһֱ����0xFF�������൱��SPI_MOSI�ܽ�һֱ���ڸ���̬���ͷ�SPI����
	*/
	GPIOA->MODER	=	GPIOA->MODER & 0xFFFF3FFF;
	GPIOA->MODER	=	GPIOA->MODER | 0x00008000;
	GPIOA->OTYPER &= 0xFFFFFF7F;		
	GPIOA->OTYPER |= 0x00000080;		
  SPI_I2S_SendData(SPI1, 0xFFFF);
	
	/* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){};
		
	tle5012b_output_dat = SPI_I2S_ReceiveData(SPI1);	
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET){};
	
	/*��SPI_MOSI�ܽ���Ϊ����ģʽ�������ƿ�SPI����*/
	GPIOA->OTYPER &= (rt_uint32_t)0xFFFFFF7F;		
	
	SPI_Module_CS2_HIGH();//����CS�ܽŵ�ƽ����ֹSPI

  /* Return the byte read from the SPI bus */
  return tle5012b_output_dat;
}


static rt_uint16_t Temp;
rt_uint16_t tle5012b_read_reg(rt_uint16_t regID)
{
	Temp = tle5012b_spi_SendByte(regID);
  return Temp;
}


static rt_int16_t Read_tle5012_reg=0;
rt_int16_t TLE5012_Read(void)
{
	/*������Ҫ�����������֣�����ʱ����һ�㣬tle5012�з���ѡ��ļĴ������ã�����һ�²�Ҫ���ò���*/
//	Read_tle5012_reg = 0x7fff - (tle5012b_read_reg(AVAL_REGISTER))&0x7fff;
	
	Read_tle5012_reg = (tle5012b_read_reg(AVAL_REGISTER))&0x7fff;
	return Read_tle5012_reg;
}

static rt_bool_t isInit = RT_FALSE;
void TLE5012_init(void)
{
	if(isInit) return;
	
	spi_enc();
	
	isInit = RT_TRUE;
}



static volatile rt_uint16_t Encoder_pos = 0;
void TLE5012_ResetEncoder(void)
{
//	configs.encoder_align_pos = 0x7fff -  (tle5012b_read_reg(AVAL_REGISTER))&0x7fff;
	configs.encoder_align_pos = (tle5012b_read_reg(AVAL_REGISTER))&0x7fff;
}

rt_int16_t TLE5012_Aligned(void)
{
//	Encoder_pos = 0x7fff - (tle5012b_read_reg(AVAL_REGISTER))&0x7fff; //ANGLECOM_REGISTER
	Encoder_pos = (tle5012b_read_reg(AVAL_REGISTER))&0x7fff; //ANGLECOM_REGISTER
	Encoder_pos = (Encoder_pos - configs.encoder_align_pos)&0x7fff;
	
	return Encoder_pos;
}


rt_int16_t TLE5012_Postion(void)
{
	return  Encoder_pos;
}



