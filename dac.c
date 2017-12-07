#include <stm32f10x.h>
#include <stm32f10x_dac.h>
#include <stm32f10x_dma.h>

#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_rcc.h>



#define DAC_DHR12R1_Address      0x40007408

/* Init Structure definition */
DAC_InitTypeDef            DAC_init_struct;
DMA_InitTypeDef            DMA_init_struct;


const uint16_t Sine12bit[32] = {
                      2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909,
                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647};



void change_timer_period(uint32_t period){
	/*
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = period;
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
*/
	//NVIC_DisableIRQ(TIM2_IRQn);

	  TIM2->ARR=period;
	  TIM2->EGR = TIM_PSCReloadMode_Immediate;
	  //NVIC_EnableIRQ(TIM2_IRQn);
}

void change_timer_period2(uint32_t period){
	  TIM2->ARR=period;
}


void stop_DAC(){
	DMA_Cmd(DMA1_Channel3, DISABLE);
	DAC_Cmd(DAC_Channel_1, DISABLE);
	DMA_DeInit(DMA1_Channel3);


}

void init_DAC(){
	//TIM_Cmd(TIM2,DISABLE);

/*
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = 84;
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  // TIM2 TRGO selection
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
*/

  /* DAC channel1 Configuration */
  DAC_init_struct.DAC_Trigger = DAC_Trigger_T2_TRGO;
  DAC_init_struct.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_init_struct.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
  DAC_Init(DAC_Channel_1, &DAC_init_struct);

  DMA_DeInit(DMA1_Channel3);

  DMA_init_struct.DMA_PeripheralBaseAddr = DAC_DHR12R1_Address;
  DMA_init_struct.DMA_MemoryBaseAddr = (uint32_t)&Sine12bit;
  DMA_init_struct.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_init_struct.DMA_BufferSize = 32;
  DMA_init_struct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_init_struct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_init_struct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_init_struct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_init_struct.DMA_Mode = DMA_Mode_Circular;
  DMA_init_struct.DMA_Priority = DMA_Priority_High;
  DMA_init_struct.DMA_M2M = DMA_M2M_Disable;

  DMA_Init(DMA1_Channel3, &DMA_init_struct);
  DMA_Cmd(DMA1_Channel3, ENABLE);
  DAC_Cmd(DAC_Channel_1, ENABLE);
  DAC_DMACmd(DAC_Channel_1, ENABLE);
  //TIM_Cmd(TIM2,ENABLE);

}

