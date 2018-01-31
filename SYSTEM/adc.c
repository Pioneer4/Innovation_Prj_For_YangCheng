/*************************************************************
*  Open source but not allowed to modify(All rights reserved)
*File name      : adc.c
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-10-10
*Description    : ���ڶ�adc�ӿڵĳ�ʼ����Դ������ԭ�ӣ�����Ӧ�޸ģ���
*Function List  : 
*************************************************************/
#include "myinclude.h"	


/*************************************************************
*Function Name  : AdcInit
*Auther         : ����
*Vertion        : v1.0
*Date           : 2017-10-10
*Description    : ��ʼ��ADC�ӿ�  PA5	
*Input          ��
*Output         ��
*Return         ��
*************************************************************/
void  AdcInit(void)
{    
	GPIO_InitTypeDef      GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //ʹ��ADC1ʱ��

	//�ȳ�ʼ��ADC1ͨ��5 IO��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;            //PA5 ͨ��5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;         //ģ������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;     //����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);               //��ʼ��  

	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);	 //ADC1��λ
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE); //��λ����	 


	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                    //����ģʽ
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//���������׶�֮����ӳ�5��ʱ��
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;     //DMAʧ��
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                 //Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz 
	ADC_CommonInit(&ADC_CommonInitStructure);                                   //��ʼ��

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      //12λģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;							    //��ɨ��ģʽ	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//�ر�����ת��
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //��ֹ������⣬ʹ���������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//�Ҷ���	
	ADC_InitStructure.ADC_NbrOfConversion = 1;									//1��ת���ڹ��������� Ҳ����ֻת����������1 
	ADC_Init(ADC1, &ADC_InitStructure);										    //ADC��ʼ��


	ADC_Cmd(ADC1, ENABLE);//����ADת����	
}


/*************************************************************
*Function Name  : GetAdc
*Auther         : ����
*Version        : v1.0
*Date           : 2017-10-10
*Description    : ���ADCֵ
*Input          ��ch ��ADC_Channel_0~ADC_Channel_16
				  @arg MOTOR_FORWARD  ���ǰ��
				  @arg MOTOR_BACKWARD �������
				  @arg MOTOR_STOP     ���ֹͣ
*Output         ��
*Return         ��ת�����
*************************************************************/
u16 GetAdc(u8 ch)   
{
	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_480Cycles );//ADC1,ADCͨ��,480������,��߲���ʱ�������߾�ȷ��			    

	ADC_SoftwareStartConv(ADC1);		           //ʹ��ָ����ADC1�����ת����������	

	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	       //�������һ��ADC1�������ת�����
}


/*************************************************************
*Function Name  : GetAdcAverage
*Auther         : ����
*Version        : v1.0
*Date           : 2017-10-10
*Description    : ��ȡͨ��ch��ת��ֵ��ȡtimes��,Ȼ��ƽ��
*Input          ��1. ch   ��ͨ�����
				  2. times����ȡ����
*Output         ��
*Return         ��ͨ��ch��times��ת�����ƽ��ֵ
*************************************************************/
u16 GetAdcAverage(u8 ch, u8 times)
{
	u32 temp_val = 0;
	u8 t;
	for (t=0; t<times; t++)
	{
		temp_val += GetAdc(ch);
		delay_ms(5);
	}
	return (temp_val / times);
} 
	 








