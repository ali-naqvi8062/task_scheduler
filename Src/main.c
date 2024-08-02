/**
 ******************************************************************************

 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "led.h"



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
void update_next_task(void);
void save_psp_value(uint32_t current_psp_value);
void update_global_tick_count(void);
void task_delay(uint32_t tick_count);
void unblock_tasks(void);
void schedule(void);

//global variables:

uint8_t current_task = 1; //denotes current task running on CPU
uint32_t g_tick_count = 0;

typedef struct{
	uint32_t psp_value;
	uint32_t block_count;
	uint8_t current_state;
	void (*task_handler)(void);
}TCB_t; //task control block

TCB_t user_tasks[MAX_TASKS];

int main(void)
{
	enable_processor_faults();

	init_scheduler_stack(SCHED_STACK_START);

	init_tasks_stack();

	led_init_all();

	init_systick_timer(TICK_HZ);

	switch_sp_to_psp();

	task1_handler();

	for(;;);
}

void idle_task(void){
	while(1);
}

void task1_handler(void){
	while(1){
		led_on(LED_GREEN);
		delay(DELAY_COUNT_1S);
		led_off(LED_GREEN);
		delay(DELAY_COUNT_1S);
	}
}

void task2_handler(void){
	while(1){
		led_on(LED_ORANGE);
		delay(DELAY_COUNT_500MS);
		led_off(LED_ORANGE);
		delay(DELAY_COUNT_500MS);
	}
}

void task3_handler(void){
	while(1){
		led_on(LED_BLUE);
		delay(DELAY_COUNT_250MS);
		led_off(LED_BLUE);
		delay(DELAY_COUNT_250MS);
	}
}

void task4_handler(void){
	while(1){
		led_on(LED_RED);
		delay(DELAY_COUNT_125MS);
		led_off(LED_GREEN);
		delay(DELAY_COUNT_125MS);
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

	user_tasks[0].current_state = TASK_RUNNING_STATE;
	user_tasks[1].current_state = TASK_RUNNING_STATE;
	user_tasks[2].current_state = TASK_RUNNING_STATE;
	user_tasks[3].current_state = TASK_RUNNING_STATE;
	user_tasks[4].current_state = TASK_RUNNING_STATE;


	user_tasks[0].psp_value = IDLE_STACK_START;
	user_tasks[1].psp_value = T1_STACK_START;
	user_tasks[2].psp_value = T2_STACK_START;
	user_tasks[3].psp_value = T3_STACK_START;
	user_tasks[4].psp_value = T4_STACK_START;


	user_tasks[0].task_handler = idle_task;
	user_tasks[1].task_handler = task1_handler;
	user_tasks[2].task_handler = task2_handler;
	user_tasks[3].task_handler = task3_handler;
	user_tasks[4].task_handler = task4_handler;



	uint32_t *pPSP;

	for(int i = 0; i < MAX_TASKS;i++){
		pPSP = (uint32_t*)user_tasks[i].psp_value;

		pPSP--;
		*pPSP = DUMMY_XPSR; //0x01000000

		pPSP--; //PC
		*pPSP = (uint32_t)user_tasks[i].task_handler;

		pPSP--; //LR
		*pPSP = 0xFFFFFFFD;

		for(int j = 0; j < 13; j++){
			pPSP--;
			*pPSP = 0;
		}

		user_tasks[i].psp_value = (uint32_t)pPSP;
	}
}

void enable_processor_faults(void){

	uint32_t *pSHCSR = (uint32_t*)0xE000ED24;

	*pSHCSR |= (1<<16); // mem manage
	*pSHCSR |= (1<<17); // bus fault
	*pSHCSR |= (1<<18); // usage fault


}

uint32_t get_psp_value(void){
	return user_tasks[current_task].psp_value;
}

void save_psp_value(uint32_t current_psp_value){
	user_tasks[current_task].psp_value = current_psp_value;
}

void update_next_task(void){ //update tasks in a round robin fashion
	int state = TASK_BLOCKED_STATE;

	for(int i = 0; i < MAX_TASKS; i++){
		current_task++;
		current_task %= MAX_TASKS;
		state = user_tasks[current_task].current_state;
		if((state == TASK_RUNNING_STATE) && (current_task!=0))
			break;
	}

	if(state!=TASK_RUNNING_STATE) // in the scenario where all tasks are blocked, the idle state will run.
		current_task = 0;
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

void schedule(void){
	uint32_t* pICSR = (uint32_t*)0xE000ED04;
	*pICSR |= (1<<28);
}

void task_delay(uint32_t tick_count){
	if(current_task){
		user_tasks[current_task].block_count = g_tick_count + tick_count;
		user_tasks[current_task].current_state = TASK_BLOCKED_STATE;
		schedule(); //schedule the PendSV
	}
}

void update_global_tick_count(void){
	g_tick_count++;
}

void unblock_tasks(void){
	for(int i = 0; i < MAX_TASKS; i++){
		if(user_tasks[i].current_state != TASK_RUNNING_STATE){
			if(user_tasks[i].block_count == g_tick_count){
				user_tasks[i].current_state = TASK_RUNNING_STATE;
			}
		}
	}
}

__attribute__((naked)) void PendSV_Handler(void){ //handler mode -> MSP

	//1. Save the context of the current task

	__asm volatile("MRS R0,PSP"); //get the current PSP value

	__asm volatile("STMDB R0!,{R4-R11}"); //stmdb decrements, then stores register. This is to save the stack frame r4 - r11

	__asm volatile("PUSH {LR}"); //save LR value before branching to new function in the next line.

	__asm volatile("BL save_psp_value"); // save the psp value.



	//2. Retrieve the context of the next task

	__asm volatile("BL update_next_task"); //decide the next task

	__asm volatile("BL get_psp_value"); //get the next task's psp value
	// R0 holds the new psp value

	__asm volatile("LDMIA R0!,{R4-R11}"); //load multiple registers and increment pointer after, this is to retrieve the next task's stack frame (r4-r11)

	__asm volatile("MSR PSP,R0"); //update the new psp and exit

	__asm volatile("POP {LR}");

	__asm volatile("BX LR"); //exit sequence
}

void SysTick_Handler(void){

	uint32_t* pICSR = (uint32_t*)0xE000ED04; // pointer to ICSR register

	update_global_tick_count();
	unblock_tasks();

	//pend the PendSV exception:
	*pICSR |= (1<<28);
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


