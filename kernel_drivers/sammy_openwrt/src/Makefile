obj-m+=sammy.o
obj-m+=sammy_hcsr04.o
obj-m+=sammy_hcsr04_interrupt.o

all:
	$(MAKE) -C $(LINUX_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(LINUX_DIR) M=$(PWD) clean
