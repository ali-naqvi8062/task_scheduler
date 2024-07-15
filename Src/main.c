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

int main(void)
{
    /* Loop forever */
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


