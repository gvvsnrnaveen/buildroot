#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/init.h>

#define GPIO_LED_PIN 475
#define GPIO_INTR_PIN 456

// variable to store the interrupt number
static int gpio_intr_number = 0;

static int number_of_blinks = 0;
static int intr_handled = 0;

static struct tasklet_struct sammy_tasklet;

static void sammy_tasklet_isr(unsigned long data);
static void free_gpio_pins(void);


// Bottom half Tasklet ISR
static void sammy_tasklet_isr(unsigned long data){
	int value = 0;
	int i = 0;

	printk(KERN_INFO "SAMMY: Tasklet execution init\n");
	value = gpio_get_value(GPIO_INTR_PIN);
	if (value == 1) {
		printk(KERN_INFO "SAMMY: Led Blinking: %d\n", number_of_blinks);
		for (i = 0; i < number_of_blinks; i++) {
			gpio_set_value(GPIO_LED_PIN, 1);
			mdelay(500);
			gpio_set_value(GPIO_LED_PIN, 0);
			mdelay(500);
		}
	}
	intr_handled = 0;
	printk(KERN_INFO "SAMMY: Tasklet execution exit\n");
}

// Top half ISR
static irqreturn_t irq_handler(int irq, void *dev) {
	printk(KERN_INFO "SAMMY: Interrupt occurred, initiating the tasklet\n");
	if (intr_handled == 1) {
		return IRQ_HANDLED;
	}
	printk(KERN_INFO "SAMMY: increasing blinks\n");
	intr_handled = 1;
	// increase the number of blinks for every interrupt
	number_of_blinks++;

	tasklet_schedule(&sammy_tasklet);

	return IRQ_HANDLED;
}

// Initial setup of GPIO and IRQ
static int setup_gpio_and_irq_handlers(void) {
	int result = 0;

	// Request the IRQ GPIO Pin
	if (gpio_request(GPIO_INTR_PIN, "GPIO_INTR_PIN")) {
		printk(KERN_ERR "SAMMY: Failed to request GPIO pin\n");
		return -1;
	}

	// Request the LED GPIO Pin
	if (gpio_request(GPIO_LED_PIN, "GPIO_LED_PIN")) {
		printk(KERN_ERR "SAMMY: Failed to request LED GPIO pin\n");
		return -1;
	}

	// Setup the IRQ GPIO Pin
	if (gpio_direction_input(GPIO_INTR_PIN)) {
		printk(KERN_ERR "SAMMY: Failed to set the GPIO pin as input\n");
		free_gpio_pins();
		return -1;
	}
	gpio_set_debounce(GPIO_INTR_PIN, 200);

	// Setup the GPIO LED Pin as output
	if (gpio_direction_output(GPIO_LED_PIN, 0)) {
		printk(KERN_ERR "SAMMY: Failed to setup the LED GPIO pin as output\n");
		free_gpio_pins();
		return -1;
	}

	// Acquire the interrupt number
	gpio_intr_number = gpio_to_irq(GPIO_INTR_PIN);
	if (gpio_intr_number <= 0) {
		printk(KERN_ERR "SAMMY: Failed to get the interrupt number for GPIO\n");
		free_gpio_pins();
		return -1;
	}
	printk(KERN_INFO "SAMMY: IRQ_NUMBER: %d\n", gpio_intr_number);

	// Setup the IRQ on GPIO
	result = request_irq(gpio_intr_number, irq_handler, IRQF_TRIGGER_RISING, "gpio_isr_routine", NULL);
	if (result) {
		printk(KERN_ERR "SAMMY: Failed to request irq\n");
		free_gpio_pins();
		return -1;
	}

	printk(KERN_INFO "SAMMY: ISR registered successfully\n");

	return 0;
}

// Free the GPIO Pins
static void free_gpio_pins(void) {
	// free up gpio pins
	gpio_free(GPIO_INTR_PIN);
	gpio_free(GPIO_LED_PIN);
}

// Module init function
static int __init sammy_gpio_tasklet_init(void) {
	printk(KERN_INFO "SAMMY: sammy gpio tasklet sample init phase\n");

	// setup the gpio pins
	if (setup_gpio_and_irq_handlers() < 0) {
		printk(KERN_ERR "SAMMY: Failed to setup gpio and interupt handlers\n");
		return -1;
	}

	tasklet_init(&sammy_tasklet, sammy_tasklet_isr, 0);

	printk(KERN_INFO "SAMMY: sammy gpio tasklet_init sample init success\n");

	return 0;
}

// Module exit function
static void __exit sammy_gpio_tasklet_exit(void) {
	// clean tasklet
	tasklet_kill(&sammy_tasklet);

	// cleanup the interrupt handler
	free_irq(gpio_intr_number, NULL);

	// Free GPIO pins
	free_gpio_pins();
	printk(KERN_INFO "SAMMY: sammy gpio tasklet_init module exit success\n");
}

module_init(sammy_gpio_tasklet_init);
module_exit(sammy_gpio_tasklet_exit);

MODULE_AUTHOR("G. Naveen Kumar");
MODULE_DESCRIPTION("SAMMY Tasklet IRQ kernel module");
MODULE_VERSION("1.0.1");
MODULE_LICENSE("GPL");

