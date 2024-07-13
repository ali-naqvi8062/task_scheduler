/**
 ******************************************************************************

 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

// Function prototypes
void task1_handler(void);
void task2_handler(void);
void task3_handler(void);
void task4_handler(void);

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


