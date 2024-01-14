#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#define SAMMY_GPIO_PIN 475

static struct task_struct *thread;

static int sample_thread_function(void *data){
	while(!kthread_should_stop()){
		printk(KERN_INFO "SAMMY: LED light on\n");
		gpio_set_value(SAMMY_GPIO_PIN, 1);
		msleep(1000);
		printk(KERN_INFO "SAMMY: LED light off\n");
		gpio_set_value(SAMMY_GPIO_PIN, 0);
		msleep(1000);

	}
	return 0;
}

static int sammy_configure_gpio(void){
	int ret = 0;
	ret = gpio_request(SAMMY_GPIO_PIN, "sysfs");
	if(ret){
		printk(KERN_ERR "SAMMY: Failed to get gpio: %d\n", ret);
		return -1;
	}
	ret = gpio_direction_output(SAMMY_GPIO_PIN, 1);
	if(ret){
		printk(KERN_ERR "SAMMY: Failed to set GPIO output direction: %d\n", ret);
		return -1;
	}
	return 0;
}

static int __init sammy_init(void){
	printk(KERN_INFO "SAMMY: Sammy module init\n");

	if(sammy_configure_gpio() < 0){
		printk(KERN_ERR "SAMMY: Failed to configure gpio\n");
		return -1;
	}

	thread = kthread_run(sample_thread_function, NULL, "sammy_thread");
	if(IS_ERR(thread)){
		printk(KERN_ERR "SAMMY: Failed to create the thread\n");
		return -1;
	}
	return 0;
}


static void __exit sammy_cleanup(void){
	printk(KERN_INFO "SAMMY: Sammy module exit\n");
	if(thread){
		kthread_stop(thread);
	}

	gpio_free(SAMMY_GPIO_PIN);
	printk(KERN_INFO "SAMMY: Sammy module exit success\n");
}

module_init(sammy_init);
module_exit(sammy_cleanup);

MODULE_AUTHOR("G. Naveen Kumar");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION("Basic Kernel GPIO Driver");

