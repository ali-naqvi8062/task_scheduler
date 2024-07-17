/**
 ******************************************************************************

 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include "main.h"



#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

// Task prototypes
void task1_handler(void);
void task2_handler(void);
void task3_handler(void);
void task4_handler(void);


void init_systick_timer(uint32_t tick_hz);
__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack);
void init_tasks_stack(void);
void enable_processor_faults(void);
__attribute__((naked)) void switch_sp_to_psp(void);
uint32_t get_psp_value(void);

//global variables: PSP tracker and LR addresses for tasks

uint32_t psp_of_tasks[MAX_TASKS] = {T1_STACK_START, T2_STACK_START, T3_STACK_START, T4_STACK_START};
uint32_t task_handlers[MAX_TASKS];

uint8_t current_task = 0; //denotes current task running on CPU



int main(void)
{
	enable_processor_faults();

	init_scheduler_stack(SCHED_STACK_START);

	task_handlers[0] = (uint32_t)task1_handler;
	task_handlers[1] = (uint32_t)task2_handler;
	task_handlers[2] = (uint32_t)task3_handler;
	task_handlers[3] = (uint32_t)task4_handler;

	init_tasks_stack();

	init_systick_timer(TICK_HZ);

	switch_sp_to_psp();

	task1_handler();

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

__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack){
	__asm volatile("MSR MSP,%0": :	"r" (sched_top_of_stack)	:	);
	__asm volatile("BX LR"); // go back to main
}


void init_tasks_stack(void){

	uint32_t *pPSP;

	for(int i = 0; i < MAX_TASKS;i++){
		pPSP = (uint32_t*)psp_of_tasks[i];

		pPSP--;
		*pPSP = DUMMY_XPSR; //0x01000000

		pPSP--; //PC
		*pPSP = task_handlers[i];

		pPSP--; //LR
		*pPSP = 0xFFFFFFFD;

		for(int j = 0; j < 13; j++){
			pPSP--;
			*pPSP = 0;
		}

		psp_of_tasks[i] = (uint32_t)pPSP;
	}
}

void enable_processor_faults(void){

	uint32_t *pSHCSR = (uint32_t*)0xE000ED24;

	*pSHCSR |= (1<<16); // mem manage
	*pSHCSR |= (1<<17); // bus fault
	*pSHCSR |= (1<<18); // usage fault


}

uint32_t get_psp_value(void){
	return psp_of_tasks[current_task];
}

__attribute__((naked)) void switch_sp_to_psp(void){

	//1. initialize PSP first, with Task 1 stack start since it will be launched first
	// start by saving the link register (LR)

	__asm volatile ("PUSH {LR}"); //save LR

	__asm volatile ("BL get_psp_value"); //branch with link to come back
	// value is stored in R0

	__asm volatile ("MSR PSP,R0"); // we have configured PSP

	__asm volatile ("POP {LR}"); // LR is preserved, it is our connection to main()

	//2. Change SP to PSP using CONTROL register

	__asm volatile ("MOV R0,#0x02"); //set bit 2 to make sp -> psp
	__asm volatile ("MSR CONTROL,R0");
	__asm volatile ("BX LR"); // return to main
}

void SysTick_Handler(void){
	//context switching
}

//fault handlers

void HardFault_Handler(void){
	printf("Exception : Hardfault\n");
	while(1);
}

void MemManage_Handler(void){
	printf("Exception : MemManage\n");
	while(1);
}

void BusFault_Handler(void){
	printf("Exception : BusFault\n");
	while(1);
}


