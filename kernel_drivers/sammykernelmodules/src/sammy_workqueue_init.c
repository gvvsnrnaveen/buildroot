#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

// USING GPIO 45 - PIN 16 (CON1 Header of Banana PI R3)
#define GPIO_INTR_PIN 456

// variable to store the interrupt number
int gpio_intr_number = 0;

static struct work_struct mywork;


void custom_task(struct work_struct *work);

// Workqueue task to read the GPIO pin value
void custom_task(struct work_struct *work){
	int value = 0;
	printk(KERN_INFO "SAMMY: executing work queue\n");

	value = gpio_get_value(GPIO_INTR_PIN);
	printk(KERN_INFO "SAMMY: GPIO value is: %d\n", value);
}

// ISR Routine
static irqreturn_t irq_handler(int irq, void *dev){
	printk(KERN_INFO "SAMMY: Interrupt occurred\n");

	// schedule the work on workqueue
	schedule_work(&mywork);

	return IRQ_HANDLED;
}

// Initial setup of GPIO and IRQ
static int setup_gpio_and_irq_handlers(void){
	int result = 0;

	// Request the GPIO Pin
	if(gpio_request(GPIO_INTR_PIN, "GPIO_INTR_PIN")){
		printk(KERN_ERR "SAMMY: Failed to request GPIO pins\n");
		return -1;
	}

	// Setup the GPIO Pin
	if(gpio_direction_input(GPIO_INTR_PIN)){
		printk(KERN_ERR "SAMMY: Failed to set the GPIO pin as input\n");
		gpio_free(GPIO_INTR_PIN);
		return -1;
	}

	// Acquire the interrupt number
	gpio_intr_number = gpio_to_irq(GPIO_INTR_PIN);
	if(gpio_intr_number <= 0){
		printk(KERN_ERR "SAMMY: Failed to get the interrupt number for GPIO\n");
		gpio_free(GPIO_INTR_PIN);
		return -1;
	}
	printk(KERN_INFO "SAMMY: IRQ_NUMBER: %d\n", gpio_intr_number);

	// Setup the ISR on GPIO
	result = request_irq(gpio_intr_number, irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_isr_routine", NULL);
	if(result){
		printk(KERN_ERR "SAMMY: Failed to request irq\n");
		gpio_free(GPIO_INTR_PIN);
		return -1;
	}

	printk(KERN_INFO "SAMMY: ISR registered successfully\n");

	return 0;
}

// Free the GPIO Pins
static void free_gpio_pins(void){
	// free up gpio pins
	gpio_free(GPIO_INTR_PIN);
}

// Module init function
static int __init sammy_gpio_workqueue_init(void){
	
	printk(KERN_INFO "SAMMY: sammy gpio workqueue sample init phase\n");

	//setup the gpio pins
	if(setup_gpio_and_irq_handlers() < 0){
		printk(KERN_ERR "SAMMY: Failed to setup gpio and interupt handlers\n");
		return -1;
	}

	// Use the INIT_WORK 
	INIT_WORK(&mywork, custom_task);

	printk(KERN_INFO "SAMMY: sammy gpio workqueue sample init success\n");

	return 0;
}

// Module exit function
static void __exit sammy_gpio_workqueue_exit(void){

	// cleanup the interrupt handler
	free_irq(gpio_intr_number, NULL);

	// Free GPIO pins
	free_gpio_pins();
	printk(KERN_INFO "SAMMY: sammy gpio workqueue module exit success\n");
}


module_init(sammy_gpio_workqueue_init);
module_exit(sammy_gpio_workqueue_exit);

MODULE_AUTHOR("G. Naveen Kumar");
MODULE_DESCRIPTION("SAMMY Workqueue kernel module");
MODULE_VERSION("1.0.1");
MODULE_LICENSE("GPL");
