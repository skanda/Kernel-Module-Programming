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

/* Add your code here */

#define iobase(d)	(d->regs)

#define ioaddr(d,o)	((void *)(iobase(d) + o*4))

#define feserial_get_drvdata(pdev)	\
	((struct feserial_dev *)platform_get_drvdata(pdev))

struct feserial_dev {
	void __iomem *regs;
	struct miscdevice miscdev;
	struct platform_device *pdev;
};

static size_t feserial_read(struct file *file, const char __user *ubuf, size_t count, loff_t *pos)
{
	return -EINVAL;
}

static size_t feserial_write(struct file *file, const char __user *ubuf, size_t count, loff_t *pos)
{
	return -EINVAL;
}

static int feserial_open(struct inode *inode, struct file *file)
{
	return 0;
}

static long feserial_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static const struct file_operations feserial_ops = {
	.owner		= THIS_MODULE,
	.open 		= feserial_open,
	.read 		= feserial_read,
	.write		= feserial_write,
	.unlocked_ioctl = feserial_ioctl,
};
static unsigned int reg_read(struct feserial_dev *dev, int offset)
{
	dev_info(&dev->pdev->dev, "Reading value from offset %d (iobase: %p, ioaddr: %p)\n", offset, iobase(dev), ioaddr(dev, offset));
//	printk("Reading value from offset %d (iobase: %p, ioaddr: %p)\n", offset, iobase(dev), ioaddr(dev, offset));
	return readl(ioaddr(dev, offset));
}

static void reg_write(struct feserial_dev *dev, int val, int off)
{
	dev_info(&dev->pdev->dev, "Writing %d to offset %d (iobase: %p, ioaddr: %p)\n",val, off, (void *)iobase(dev), (void *)ioaddr(dev, off));//	printk("Writing %d to offset %d (iobase: %p, ioaddr: %p)\n",val, off, (void *)iobase(dev), (void *)ioaddr(dev, off));
	writel(val, ioaddr(dev, off));

}

static void uart_write(struct feserial_dev *dev, const char ch)
{
	unsigned char lsr;
	lsr = reg_read (dev, UART_LSR);
	printk("In uart_write\n");
	//while(!(lsr & UART_LSR_THRE))
	//	cpu_relax();

	reg_write(dev,(int)ch,UART_TX);
}

static void feserial_cleanup(struct feserial_dev *dev)
{
	kfree(dev->miscdev.nodename);
}

static int feserial_probe(struct platform_device *pdev)
{
	struct resource *res;
	unsigned long *base_addr;
	unsigned long remap_size;
	struct feserial_dev *dev;
	unsigned int uartclk;
	unsigned int baud_divisor;
	int ret = -1;
	
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
	
	dev = devm_kzalloc(&pdev->dev, sizeof(struct feserial_dev), GFP_KERNEL);

	if (!dev) {
		dev_err(&pdev->dev,
			"Unable to allocate feserial_dev structure.\n");
		return -ENOMEM;
	}

	dev->pdev = pdev;

	dev->regs = devm_ioremap_resource(&pdev->dev, res);

	if(!dev->regs) {
		dev_err(&pdev->dev, "Cannot remap registers\n");
		return -ENOMEM;
	}

	dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	dev->miscdev.name = "feserial_misc";
	dev->miscdev.fops = &feserial_ops;
	dev->miscdev.nodename = kasprintf(GFP_KERNEL, "feserial-%x", res->start);
	
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

	uart_write(dev, 'A');

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
