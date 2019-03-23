#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *leddrv_class;
static struct class_device	*leddrv_class_dev;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;


static int led_drv_open(struct inode *inode, struct file *file)
{
	//printk("led_drv_open\n");
	/* ����GPF4,5,6Ϊ��� */
	*gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2)) | (0x3<<(6*2)));
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));
	return 0;
}

static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	//printk("led_drv_write\n");

	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// ���
		*gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
	}
	else
	{
		// ���
		*gpfdat |= (1<<4) | (1<<5) | (1<<6);
	}
	
	return 0;
}

static struct file_operations led_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   led_drv_open,     
	.write	=	led_drv_write,	   
};


int major;
static int led_drv_init(void)
{
	major = register_chrdev(0, "led_drv", &led_drv_fops); //(�����������豸�ţ����������֣�����ʹ�õ��ļ�������)

	leddrv_class = class_create(THIS_MODULE, "leddrv");

	leddrv_class_dev = class_device_create(leddrv_class, NULL, MKDEV(major, 0), NULL, "xyz"); /* /dev/xyz �������豸���Զ������ɴ��豸��*/

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);	//��GPIO�������ַӳ��Ϊ���õ������ַ
	gpfdat = gpfcon + 1;

	return 0;
}

static void led_drv_exit(void)
{
	unregister_chrdev(major, "led_drv"); // ж��

	class_device_unregister(leddrv_class_dev);
	class_destroy(leddrv_class);
	iounmap(gpfcon);
}

module_init(led_drv_init);
module_exit(led_drv_exit);


MODULE_LICENSE("GPL");

