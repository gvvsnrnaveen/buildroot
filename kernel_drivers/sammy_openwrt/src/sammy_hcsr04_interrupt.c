#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>

#define HCSR04_TRIGGER_PIN 475
#define HCSR04_ECHO_PIN 456
#define DISTANCE_UNIT 58

static struct task_struct *thread;
int hcsr04_echo_irq_number = 0;
ktime_t start_time, end_time;
s64 delta_us;
int distance;


static irqreturn_t echo_irq_handler(int irq, void *dev){
	if(gpio_get_value(HCSR04_ECHO_PIN)){
		// register start time at gpio value is 1
		start_time = ktime_get();
	} else {
		// register end time at gpio value is 0
		end_time = ktime_get();

		// Calculate the time elapsed in microseconds
		delta_us = ktime_us_delta(end_time, start_time);

		// Calculate distance in centimeters
		distance = delta_us / DISTANCE_UNIT;
		if(distance < 100){
			printk(KERN_INFO "SAMMY: IRQMODE: Distance: %d cms\n", distance);
		}
	}
	return IRQ_HANDLED;
}

static int send_trigger_pulses(void *data) {
	while(!kthread_should_stop()){
		printk(KERN_INFO "SAMMY: Trigger pulse sent\n");
		mdelay(500);

		gpio_set_value(HCSR04_TRIGGER_PIN, 0);
		udelay(2);
		gpio_set_value(HCSR04_TRIGGER_PIN, 1);
		udelay(10);
		gpio_set_value(HCSR04_TRIGGER_PIN, 0);

	}

	return 0;
}

static int setup_gpio_pins(void){
	int result = 0;
	// Setup the TRIGGER and ECHO GPIO pins
	if (gpio_request(HCSR04_TRIGGER_PIN, "HCSR04_TRIGGER_PIN") ||
			gpio_request(HCSR04_ECHO_PIN, "HCSR04_ECHO_PIN")) {
		printk(KERN_ERR "SAMMY: Failed to request GPIO pins\n");
		return -1;
	}

	// Initialize GPIO pins
	if (gpio_direction_output(HCSR04_TRIGGER_PIN, 0) ||
			gpio_direction_input(HCSR04_ECHO_PIN)) {
		printk(KERN_ERR "SAMMY: Failed to initialize GPIO pins\n");
		gpio_free(HCSR04_TRIGGER_PIN);
		gpio_free(HCSR04_ECHO_PIN);
		return -1;
	}

	// setup the interrupt handlers on the GPIO
	hcsr04_echo_irq_number = gpio_to_irq(HCSR04_ECHO_PIN);
	if(hcsr04_echo_irq_number <= 0){
		printk(KERN_ERR "SAMMY: Failed to get the irq number\n");
		gpio_free(HCSR04_TRIGGER_PIN);
		gpio_free(HCSR04_ECHO_PIN);
		return -1;
	}
	printk(KERN_INFO "SAMMY: IRQ_NUMBER: %d\n", hcsr04_echo_irq_number);

	result = request_irq(hcsr04_echo_irq_number, echo_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "hcsr04_interrupt_handler", NULL);
	if(result){
		printk(KERN_ERR "SAMMY: Failed to request irq\n");
		gpio_free(HCSR04_TRIGGER_PIN);
		gpio_free(HCSR04_ECHO_PIN);
		return -1;
	}

	return 0;
}

static void free_gpio_pins(void){
	// free up gpio pins
	gpio_free(HCSR04_TRIGGER_PIN);
	gpio_free(HCSR04_ECHO_PIN);
}

static int __init sammy_hcsr04_init(void) {
	printk(KERN_INFO "SAMMY: HC-SR04 module init phase\n");
	
	//setup the gpio pins
	setup_gpio_pins();

	// start the thread for measuring the distance
	thread = kthread_run(send_trigger_pulses, NULL, "sammy_hcsr04_thread");
	if(IS_ERR(thread)){
		free_gpio_pins();
		return -1;
	}

	printk(KERN_INFO "SAMMY: HC-SR04 module init success\n");
	return 0;
}

static void __exit sammy_hcsr04_exit(void) {
	// cleanup the interrupt handler
	free_irq(hcsr04_echo_irq_number, NULL);

	// stop the thread
	if(thread){
		kthread_stop(thread);
	}

	
	// Free GPIO pins
	free_gpio_pins();
	printk(KERN_INFO "SAMMY: HC-SR04 module unload success\n");
}

module_init(sammy_hcsr04_init);
module_exit(sammy_hcsr04_exit);

MODULE_AUTHOR("G. Naveen Kumar");
MODULE_DESCRIPTION("SAMMY HC-SR04 Ultrasonic Sensor Kernel Driver");
MODULE_VERSION("1.0.1");
MODULE_LICENSE("GPL");
