#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>

#define BPI_R3_I2C_BUS 0
#define UNO_I2C_ID 9

// Buffers for sending and receiving i2c communication
char i2c_send_buffer[16] = {0};
char i2c_recv_buffer[16] = {0};

// length of buffers
unsigned int i2c_send_buffer_length = 0;
unsigned int i2c_recv_buffer_length = 0;

// proc fs entry
static struct proc_dir_entry *sammy_proc;

// I2C adapter 
struct i2c_adapter *adapter;

// I2C client
struct i2c_client *client;

// I2C board information
struct i2c_board_info arduino_info = {
	I2C_BOARD_INFO("arduino", UNO_I2C_ID)
};

// Function to send data via I2C as master
static void sammy_send_data_to_arduino(u8 *data, int len){
	int result = 0;
	result = i2c_master_send(client, data, len);
	if(result < 0){
		printk(KERN_ERR "SAMMY: Failed to send the data\n");
		return ;
	}

	printk(KERN_INFO "SAMMY: Data sent successfully: %d bytes\n", result);
}

// Function to receive data via I2C as master
static void sammy_recv_data_from_arduino(void){
	i2c_recv_buffer_length = 0;
	memset(i2c_recv_buffer, 0, sizeof(i2c_recv_buffer));
	i2c_recv_buffer_length = i2c_master_recv(client, i2c_recv_buffer, 3);
	if(i2c_recv_buffer_length < 0){
		printk(KERN_ERR "SAMMY: Data receiving failed\n");
	}
	printk(KERN_INFO "SAMMY: received: %s", i2c_recv_buffer);
}

// Function to init I2C adapter and client
static int sammy_i2c_sender_init(void){
	adapter = i2c_get_adapter(BPI_R3_I2C_BUS);
	if(!adapter){
		printk(KERN_ERR "SAMMY: failed to get the i2c bus number\n");
		return -ENODEV;
	}

	client = i2c_new_client_device(adapter,  &arduino_info);
	if(!client){
		printk(KERN_ERR "SAMMY: Failed to create i2c client device\n");
		i2c_put_adapter(adapter);
		return -EINVAL;
	}

	return 0;
}

// Function to read procfs file
// We will not use any read mode in this example
static ssize_t sammy_proc_read(struct file *fp, char *ubuf, size_t count, loff_t *ppos){
	return -1;
}

// Function to write to procfs file
// We will read the data in kernel module and send to I2C Device
static ssize_t sammy_proc_write(struct file *fp, const char *ubuf, size_t count, loff_t *ppos){
	if(count > sizeof(i2c_send_buffer)){
		printk(KERN_ERR "SAMMY: buffer size exceeded, send max of %ld bytes\n", sizeof(i2c_send_buffer));
		return -1;
	}
	memset(i2c_send_buffer, 0, sizeof(i2c_send_buffer));
	if(copy_from_user(i2c_send_buffer, ubuf, count))
		return -EFAULT;
	printk(KERN_ERR "SAMMY: Sending: %s\n", i2c_send_buffer);
	// send the data via I2C
	sammy_send_data_to_arduino(i2c_send_buffer, strlen(i2c_send_buffer));

	sammy_recv_data_from_arduino();
	return count;
}

// Proc FS Operations structure
static const struct proc_ops sammy_proc_ops = {
	.proc_read = sammy_proc_read,
	.proc_write = sammy_proc_write
};


// ProcFS init
static int sammy_create_proc_file(void){
	sammy_proc = proc_create("sammy_arduino", 0660, NULL, &sammy_proc_ops);
	if(!sammy_proc){
		printk(KERN_ERR "SAMMY: Failed to create proc file\n");
		return -1;
	}
	return 0;
}	

// Destroy ProcFS
static void sammy_destroy_proc_file(void){
	proc_remove(sammy_proc);
}

// Sammy I2C Kernel Driver Init
static int sammy_uno_i2c_init(void){
	printk(KERN_INFO "SAMMY: sammy arduino uno i2c communication init\n");

	if(sammy_create_proc_file() < 0){
		printk(KERN_ERR "SAMMY: proc failure\n");
		return -1;
	}

	if(sammy_i2c_sender_init() < 0){
		printk(KERN_ERR "SAMMY: failed to setup i2c\n");
	}

	return 0;
}

// Sammy I2C Kernel Driver Exit
static void sammy_uno_i2c_exit(void){
	if(client){
		i2c_unregister_device(client);
	}

	i2c_put_adapter(adapter);

	sammy_destroy_proc_file();
	printk(KERN_INFO "SAMMY: sammy arduino uno i2c communication exit\n");
}


module_init(sammy_uno_i2c_init);
module_exit(sammy_uno_i2c_exit);

MODULE_AUTHOR("G. Naveen Kumar <naveenkumar.gvvsnr@gmail.com>");
MODULE_VERSION("1.0.1");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sammy I2C Communication between BananaPI R3 and Arduino Uno");
