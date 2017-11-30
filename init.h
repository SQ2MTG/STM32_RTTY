#include <stm32f10x.h>


#define RED_LED_ON GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define RED_LED_OFF GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define GREEN_LED_ON GPIO_ResetBits(GPIOB, GPIO_Pin_7)
#define GREEN_LED_OFF GPIO_SetBits(GPIOB, GPIO_Pin_7)



void debug(char* str);

__IO uint16_t ADCVal[2];

#ifdef __cplusplus
extern "C" {
#endif

void NVIC_Conf();

void RCC_Conf();

void init_port();


void init_timer_callback(uint32_t period, void (*callback)());
void stop_timer();

void conserve_power();


void init_usart_gps(const uint32_t speed, const uint8_t enable_irq);

void spi_init();

void spi_deinit();
#ifdef __cplusplus
}
#endif
