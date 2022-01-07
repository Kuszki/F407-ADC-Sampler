#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "crc.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "adc.h"
#include "dma.h"

#define ARM_MATH_CM4
#include "arm_math.h"

extern "C"
{
	void SystemClock_Config(void);
}

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

extern TIM_HandleTypeDef htim2;

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

volatile int doReq = 0;
volatile int isDone = 0;

uint8_t dummy = 0;

//const float32_t A[] =
//{
//	1, 2, 3, 4,
//	5, 6, 7, 8,
//	9, 1, 2, 3,
//	4, 5, 6, 7
//};
//
//const float32_t B[] =
//{
//	1, 2, 3, 4
//};
//
//float32_t W[4];

volatile arm_status status;

size_t adc_samples = 1024;
uint32_t* adc_value;

int main(void)
{
	/* FPU initialization */
	SCB->CPACR |= ((3 << 10*2) | (3 << 11*2));

	adc_value = new uint32_t[adc_samples];

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_TIM2_Init();
	MX_ADC1_Init();
	MX_CRC_Init();
	MX_USART2_UART_Init();

	HAL_UART_Receive_IT(&huart2, &dummy, 1);

//	arm_matrix_instance_f32 mat_A;
//	arm_mat_init_f32(&mat_A, 4, 4, (float32_t*) A);
//
//	arm_matrix_instance_f32 mat_B;
//	arm_mat_init_f32(&mat_B, 4, 1, (float32_t*) B);
//
//	arm_matrix_instance_f32 mat_W;
//	arm_mat_init_f32(&mat_W, 4, 1, (float32_t*) W);
//	status = arm_mat_mult_f32(&mat_A, &mat_B, &mat_W);

	HAL_ADC_Start_DMA(&hadc1, adc_value, adc_samples);

	while (1) if (doReq)
	{
		isDone = 0;
		doReq = 0;

		HAL_TIM_Base_Start_IT(&htim2);

		while (!isDone);

		HAL_UART_Transmit(&huart2, (uint8_t*) adc_value,
				adc_samples*sizeof(uint32_t), 10000);

		isDone = 0;
		doReq = 0;
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	HAL_GPIO_TogglePin(GPIOD, LD5_Pin);
	HAL_TIM_Base_Stop_IT(&htim2);

	isDone = 1;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(GPIOD, LD6_Pin); // funguje
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_GPIO_TogglePin(GPIOD, LD4_Pin);

	HAL_UART_Receive_IT(huart, &dummy, 1);

	if (!doReq) doReq = true;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_GPIO_TogglePin(GPIOD, LD4_Pin);
}
