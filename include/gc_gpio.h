#ifndef _GC_GPIO_H_
#define _GC_GPIO_H_

#define	GC_GPIO_BAT_CHARGE_DETECT	11
#define	GC_GPIO_MOTOR_ENABLE						19
#define	GC_GPIO_DEBUG_ON										28

#ifdef GC_COACH_FIT_HW_1_0
	#define GC_GPIO_PIN_ACC_INTR_1	9
	#define GC_GPIO_PIN_ACC_INTR_2	8
	
#elif defined GC_COACH_FIT_HW_1_1
	#define GC_GPIO_PIN_ACC_INTR_1	8
	#define GC_GPIO_PIN_ACC_INTR_2	7
	
#elif defined GC_COACH_FIT_HW_1_2
	#define GC_GPIO_PIN_ACC_INTR_1	8
	#define GC_GPIO_PIN_ACC_INTR_2	7
#else
#endif 

#define GC_GPIO_PIN_HR_POWER_EN						29 /* enable power to pah8002 */
#endif 
