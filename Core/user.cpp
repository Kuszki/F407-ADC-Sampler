#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "crc.h"
#include "usart.h"
#include "gpio.h"
#include "dma.h"

#define ARM_MATH_CM4
#include "arm_math.h"

#define DATA_SIZE 64
#define INPUT_SIZE 512

extern "C"
{
	void SystemClock_Config(void);
}

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

volatile int doSend = 0;
volatile int doReq = 0;

const size_t isize = INPUT_SIZE;
const size_t osize = DATA_SIZE;

uint16_t* ibuffer = nullptr;
uint16_t* obuffer = nullptr;

uint8_t dummy = 0;

const float32_t A[] =
{
	1, 2, 3, 4,
	5, 6, 7, 8,
	9, 1, 2, 3,
	4, 5, 6, 7
};

const float32_t B[] =
{
	1, 2, 3, 4
};

float32_t W[4];

volatile arm_status status;

int main(void)
{
	/* FPU initialization */
	SCB->CPACR |= ((3 << 10*2) | (3 << 11*2));

	ibuffer = new uint16_t[isize];
	obuffer = new uint16_t[osize];

	for (size_t i = 0; i < osize; ++i) obuffer[i] = 0;
	for (size_t i = 0; i < isize; ++i) ibuffer[i] = 0;

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_CRC_Init();

	HAL_UART_Receive_IT(&huart2, &dummy, 1);

	arm_matrix_instance_f32 mat_A;
	arm_mat_init_f32(&mat_A, 4, 4, (float32_t*) A);

	arm_matrix_instance_f32 mat_B;
	arm_mat_init_f32(&mat_B, 4, 1, (float32_t*) B);

	arm_matrix_instance_f32 mat_W;
	arm_mat_init_f32(&mat_W, 4, 1, (float32_t*) W);
	status = arm_mat_mult_f32(&mat_A, &mat_B, &mat_W);

	while (1) if (doReq)
	{
		HAL_Delay(1000);

		doReq = 0;
		doSend = 4;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_GPIO_TogglePin(GPIOD, LD4_Pin);

	HAL_UART_Receive_IT(huart, &dummy, 1);

	if (!doSend) doReq = 1;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_GPIO_TogglePin(GPIOD, LD4_Pin);
}