/**
 ******************************************************************************

 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

// Task prototypes
void task1_handler(void);
void task2_handler(void);
void task3_handler(void);
void task4_handler(void);


void init_systick_timer(uint32_t tick_hz);

#define SIZE_TASK_STACK			1024U // arbitrarily allocating 1kb space to each task
#define SIZE_SCHED_STACK		1024U

#define SRAM_START				0x20000000U
#define SIZE_SRAM				((128) * (1024))
#define SRAM_END				((SRAM_START) + (SIZE_SRAM))

#define T1_STACK_START			SRAM_END
#define T2_STACK_START			((SRAM_END) - (1 * SIZE_TASK_STACK))
#define T3_STACK_START			((SRAM_END) - (2 * SIZE_TASK_STACK))
#define T4_STACK_START			((SRAM_END) - (3 * SIZE_TASK_STACK))
#define SCHED_STACK_START		((SRAM_END) - (4 * SIZE_TASK_STACK))

#define TICK_HZ 1000U
#define HSI_CLOCK				16000000U
#define SYSTICK_TIM_CLK			HSI_CLOCK

int main(void)
{
    /* Loop forever */
	init_systick_timer(TICK_HZ);
	for(;;);
}

// Function body

void task1_handler(void){
	while(1){
		printf("This is task 1\n");
	}
}

void task2_handler(void){
	while(1){
		printf("This is task 2\n");
	}
}

void task3_handler(void){
	while(1){
		printf("This is task 3\n");
	}
}

void task4_handler(void){
	while(1){
		printf("This is task 4\n");
	}
}

void init_systick_timer(uint32_t tick_hz){

	uint32_t *pSRVR = (uint32_t*)0xE000E014; //SysTick reload value register, SVR
	uint32_t *pSCSR = (uint32_t*)0xE000E010; //SysTick Control and Status register


	uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz)-1; //1ms

	//clear the value of SVR
	*pSRVR &= ~(0x00FFFFFFFF);

	//load the value in to SVR
	*pSRVR |= count_value;

	//enable SysTick exception request
	*pSCSR |= (1<<1);

	//indicate processor clock source
	*pSCSR |= (1<<2);

	//enable systick counter
	*pSCSR |= (1<<0);

}

void SysTick_Handler(void)
{
	//context switching
}


