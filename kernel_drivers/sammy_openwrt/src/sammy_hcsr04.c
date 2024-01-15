#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#define HCSR04_TRIGGER_PIN 475
#define HCSR04_ECHO_PIN 456
#define DISTANCE_UNIT 58

static struct task_struct *thread;

static int measure_distance(void *data) {
	ktime_t start_time, end_time;
	s64 delta_us;
	int distance;

	while(!kthread_should_stop()){
		msleep(500);
		// Send a trigger pulse
		gpio_set_value(HCSR04_TRIGGER_PIN, 0);
		udelay(2);
		gpio_set_value(HCSR04_TRIGGER_PIN, 1);
		udelay(10);
		gpio_set_value(HCSR04_TRIGGER_PIN, 0);

		// Wait for the echo pin to go high
		while (gpio_get_value(HCSR04_ECHO_PIN) == 0);

		start_time = ktime_get();

		// Wait for the echo pin to go low
		while (gpio_get_value(HCSR04_ECHO_PIN) == 1);

		end_time = ktime_get();

		// Calculate the time elapsed in microseconds
		delta_us = ktime_us_delta(end_time, start_time);

		// Calculate distance in centimeters
		distance = delta_us / DISTANCE_UNIT;
		if(distance > 200){
			printk(KERN_INFO "SAMMY: sensor distance exceeded\n");
		} else {
			printk(KERN_INFO "SAMMY: Distance: %d cms\n", distance);
		}
	}

	return 0;
}

static int setup_gpio_pins(void){
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

	return 0;
}

static void free_gpio_pins(void){
	gpio_free(HCSR04_TRIGGER_PIN);
	gpio_free(HCSR04_ECHO_PIN);
}

static int __init sammy_hcsr04_init(void) {
	printk(KERN_INFO "SAMMY: HC-SR04 module init phase\n");
	
	//setup the gpio pins
	setup_gpio_pins();

	// start the thread for measuring the distance
	thread = kthread_run(measure_distance, NULL, "sammy_hcsr04_thread");
	if(IS_ERR(thread)){
		free_gpio_pins();
		return -1;
	}

	printk(KERN_INFO "SAMMY: HC-SR04 module init success\n");
	return 0;
}

static void __exit sammy_hcsr04_exit(void) {
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
