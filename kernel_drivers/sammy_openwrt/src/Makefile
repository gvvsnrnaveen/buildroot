obj-m+=sammy.o
obj-m+=sammy_hcsr04.o
obj-m+=sammy_hcsr04_interrupt.o
obj-m+=sammy_workqueue_global.o
obj-m+=sammy_workqueue_global_delayed.o
obj-m+=sammy_workqueue_global_cpu.o
obj-m+=sammy_workqueue_init.o
obj-m+=sammy_workqueue_cancel.o
obj-m+=sammy_threaded_irq.o

all:
	$(MAKE) -C $(LINUX_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_DIR) M=$(PWD) clean
