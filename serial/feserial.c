#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <asm/io.h>
#include <linux/pm_runtime.h>
#include <linux/serial_reg.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/debugfs.h>

/* Add your code here */

#define iobase(d)	(d->regs)

#define ioaddr(d,o)	((void *)(iobase(d) + o*4))

#define feserial_get_drvdata(pdev)	\
	((struct feserial_dev *)platform_get_drvdata(pdev))

#define RW_CHUNK_SIZE	32
#define IOCTL_SERIAL_RESET_COUNTER 0
#define IOCTL_SERIAL_GET_COUNTER 1

#define inc_counter(d)	(d->counter++)
#define rst_counter(d)	(d->counter = 0)
#define get_counter(d)	(d->counter)

#define SERIAL_BUFSIZE 16

struct feserial_dev {
	void __iomem *regs;
	struct miscdevice miscdev;
	struct platform_device *pdev;
	unsigned long counter;
	int irq;
	char serial_buf[SERIAL_BUFSIZE];
	wait_queue_head_t serial_wait;
	int serial_buf_rd;
	int serial_buf_wr;
};

static int flag = 0;
struct dentry *dirret, *fileret;
int filevalue;

static unsigned int reg_read(struct feserial_dev *dev, int offset)
{
	dev_info(&dev->pdev->dev, "Reading value from offset %d (iobase: %p, ioaddr: %p)\n", offset, iobase(dev), ioaddr(dev, offset));
//	printk("Reading value from offset %d (iobase: %p, ioaddr: %p)\n", offset, iobase(dev), ioaddr(dev, offset));
	flag = 1;
	return readl(ioaddr(dev, offset));
}

static void reg_write(struct feserial_dev *dev, int val, int off)
{
	dev_info(&dev->pdev->dev, "Writing %d to offset %d (iobase: %p, ioaddr: %p)\n",val, off, (void *)iobase(dev), (void *)ioaddr(dev, off));//	printk("Writing %d to offset %d (iobase: %p, ioaddr: %p)\n",val, off, (void *)iobase(dev), (void *)ioaddr(dev, off));
	writel(val, ioaddr(dev, off));

}

static irq_handler_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	struct feserial_dev *dev = (struct feserial_dev *) dev_id;
//	printk("Reading interrupt from the device :  %s \n", dev->pdev->name);
	
	
	dev->serial_buf[dev->serial_buf_wr] =(char )reg_read(dev, UART_RX);
	wake_up(&dev->serial_wait);
//	printk("Interrupt Handled! Character read is = '%c', wr-ptr = %d \n", dev->serial_buf[dev->serial_buf_wr], dev->serial_buf_wr);
	dev->serial_buf_wr++;
	if(dev->serial_buf_wr == SERIAL_BUFSIZE) {
		dev->serial_buf_wr = 0;
		printk("Before returning\n");	
	}
	return (irq_handler_t) IRQ_HANDLED;
}

static void uart_write_char(struct feserial_dev *dev, char ch)
{
	reg_write(dev, (int)ch, UART_TX);
	inc_counter(dev);
}

static void uart_write(struct feserial_dev *dev, const char* buf, int count)
{
	unsigned char lsr;
	int i;
	lsr = reg_read (dev, UART_LSR);
	printk("In uart_write\n");
	//while(!(lsr & UART_LSR_THRE))
	//	cpu_relax();
	
	for(i=0;i<count;i++)
		uart_write_char(dev, buf[i]);
}

static ssize_t feserial_read(struct file *file, const char __user *ubuf, size_t count, loff_t *pos)
{
	struct feserial_dev *dev;
//	char c;
	ssize_t result = 0;

	dev = container_of(file->private_data, struct feserial_dev, miscdev);

	wait_event_interruptible(dev->serial_wait, (dev->serial_buf_rd != dev->serial_buf_wr));
//	printk("Writing to user space: %c, rd - ptr = %d \n", dev->serial_buf[dev->serial_buf_rd], dev->serial_buf_rd);
	
//	c = dev->serial_buf[dev->serial_buf_rd++];
	if(copy_to_user((void *)ubuf,(const void *)(&dev->serial_buf[dev->serial_buf_rd]), 1)) {
		dev_err(&dev->pdev->dev, "copy_to_user() failed.\n");
			result = -EFAULT;
	}
	else {
		dev->serial_buf[dev->serial_buf_rd] = 0;
		dev->serial_buf_rd++;
		if(dev->serial_buf_rd == SERIAL_BUFSIZE) {
			printk("Before returning - feserial_read\n");
			dev->serial_buf_rd = 0;
		}
			result = 1;
	}
	return result;

}

static ssize_t feserial_write(struct file *file, const char __user *ubuf, size_t count, loff_t *pos)
{
	char kbuf[RW_CHUNK_SIZE];
	unsigned int chunk_size = RW_CHUNK_SIZE;

	size_t left_to_copy = count;
	ssize_t result = 0;

	struct feserial_dev *dev;

	dev = container_of(file->private_data, struct feserial_dev, miscdev);

	while (left_to_copy > 0) {
		if (count < RW_CHUNK_SIZE)
			chunk_size = (unsigned int)count;

		if(copy_from_user(kbuf, ubuf, chunk_size) > 0) {
			dev_err(&dev->pdev->dev, "copy_from_user() failed.\n");
			result = -EFAULT;
			break;
		}
		
		left_to_copy -= chunk_size;
		uart_write(dev, kbuf, chunk_size);
		result += chunk_size;
	}
	return result;
}

static int feserial_open(struct inode *inode, struct file *file)
{
	return 0;
}

static long feserial_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct feserial_dev *dev;
	unsigned long result;
	long int ret;

	dev = container_of(file->private_data, struct feserial_dev, miscdev);
	
	switch(cmd) {
		case IOCTL_SERIAL_RESET_COUNTER:
			rst_counter(dev);
			ret = 0;
			break;
		case IOCTL_SERIAL_GET_COUNTER:
			result = get_counter(dev);
			ret = copy_to_user((unsigned long *)arg, &result, sizeof(unsigned long));
		if(ret)
			return -EACCES;
		break;
		default:
			return -EINVAL;
		}
	return 0;	
}

static struct file_operations feserial_ops = {
	.owner		= THIS_MODULE,
	.open 		= feserial_open,
	.read 		= feserial_read,
	.write		= feserial_write,
	.unlocked_ioctl = feserial_ioctl,
};



static void feserial_cleanup(struct feserial_dev *dev)
{
	kfree(dev->miscdev.nodename);
}


static int feserial_probe(struct platform_device *pdev)
{
	struct resource *res;
//	unsigned long *base_addr;
//	unsigned long remap_size;
	struct feserial_dev *dev;
	unsigned int uartclk;
	unsigned int baud_divisor;
	int ret = -1;
	int i = 0;
	
	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if(res == NULL){
		printk("Cannot get the resources\n");
		return -ENODEV;
	}
/*	else{
		remap_size = res->end - res->start + 1;
		base_addr = ioremap(res->start, remap_size);
		printk("Resource structure starts with 0x%p\n", base_addr);
	}*/
	
	dirret = debugfs_create_dir("serial", NULL);
	if (!dirret) {
        	// Abort module load.
        	printk(KERN_ALERT "debugfs_example1: failed to create /sys/kernel/debug/example1\n");
        	return -1;
    	}
	
	dev = devm_kzalloc(&pdev->dev, sizeof(struct feserial_dev), GFP_KERNEL);

	if (!dev) {
		dev_err(&pdev->dev,
			"Unable to allocate feserial_dev structure.\n");
		return -ENOMEM;
	}

	dev->pdev = pdev;
	rst_counter(dev);
	dev->irq = platform_get_irq(pdev, 0);
	dev->serial_buf_rd = 0;
	dev->serial_buf_wr = 0;
	for(i=0;i<SERIAL_BUFSIZE;i++)
		dev->serial_buf[i] = 0;

	dev->regs = devm_ioremap_resource(&pdev->dev, res);

	if(!dev->regs) {
		dev_err(&pdev->dev, "Cannot remap registers\n");
		return -ENOMEM;
	}

	dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	dev->miscdev.name = "feserial_misc";
	dev->miscdev.fops = &feserial_ops;
	dev->miscdev.nodename = kasprintf(GFP_KERNEL, "feserial-%x", res->start);
	
	fileret = debugfs_create_u8("counter", 0644, dirret,(u8 *)&dev->counter);
	if (!fileret) {
        	// Abort module load.
        	printk(KERN_ALERT "debugfs_example1: failed to create /sys/kernel/debug/example1/hello\n");
        	return -1;
    	}
	init_waitqueue_head(&dev->serial_wait);
	printk("Dev name: %s\n", dev->pdev->name);
	ret = devm_request_irq(&pdev->dev, dev->irq,(irq_handler_t) irq_handler, 0,(void *)dev, dev); 
	
	platform_set_drvdata(pdev, dev);

	dev = feserial_get_drvdata(pdev);
	if (!dev) {
		dev_err(&pdev->dev, "Can't access feserial_dev structure.\n");
		return -1;	
	}

	of_property_read_u32(pdev->dev.of_node, "clock-frequency", &uartclk);
	baud_divisor = uartclk/16/115200;
	reg_write(dev, 0x07, UART_OMAP_MDR1);
	reg_write(dev, 0x00, UART_LCR);
	reg_write(dev, UART_LCR_DLAB, UART_LCR);
	reg_write(dev, baud_divisor & 0xff, UART_DLL);
	reg_write(dev, (baud_divisor >> 8) & 0xff, UART_DLM);
	reg_write(dev, UART_LCR_WLEN8, UART_LCR);

	reg_write(dev, UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT, UART_FCR);
	reg_write(dev, 0x00, UART_OMAP_MDR1);
	reg_write(dev, UART_IER_RDI, UART_IER);

//	uart_write(dev, 'A',1);

	if(misc_register(&dev->miscdev)) {
		pr_err("feserial_misc: Unable to register misc driver\n");
		ret = -EIO;
		feserial_cleanup(dev);
	}

	return 0;
}

static int feserial_remove(struct platform_device *pdev)
{
	struct feserial_dev *dev;
	dev = feserial_get_drvdata(pdev);
	dev->irq = platform_get_irq(pdev, 0);
	debugfs_remove_recursive(dirret);
	misc_deregister(&dev->miscdev);
	feserial_cleanup(dev);
	pm_runtime_disable(&pdev->dev);
	printk("Called feserial_remove\n");
        return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id serial_dt_ids[] = {
	{ .compatible = "free-electrons,serial", },
	{}
};
MODULE_DEVICE_TABLE(of, serial_dt_ids);
#endif

static const struct platform_device_id serial_id[] = {
	{ "serial", 0},
	{}
};
MODULE_DEVICE_TABLE(platform, serial_id);

static struct platform_driver feserial_driver = {
        .driver = {
                .name = "serial",
                .owner = THIS_MODULE,
		.of_match_table = of_match_ptr(serial_dt_ids),
        },
        .probe = feserial_probe,
        .remove = feserial_remove,
	.id_table = serial_id
};

module_platform_driver(feserial_driver);
MODULE_LICENSE("GPL");
