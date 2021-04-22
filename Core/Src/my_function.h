/*
 * my_function.h
 *
 *  Created on: Apr 22, 2021
 *      Author: bill.wang
 */

#ifndef SRC_MY_FUNCTION_H_
#define SRC_MY_FUNCTION_H_

extern volatile uint32_t cnt_hall_period;
extern volatile uint32_t hall_period;
extern volatile uint32_t cnt_10us;

void main_init();
void Hall_Period_Init(uint32_t period);

#endif /* SRC_MY_FUNCTION_H_ */
