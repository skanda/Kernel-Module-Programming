#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/input-polldev.h>

/* Add your code here */

int ret;

#define DEVICE_NAME "nunchuck"

static const struct i2c_device_id nunchuck_id[] = {
	{"nunchuck", 0},
	{ }
};

MODULE_DEVICE_TABLE(i2c, nunchuck_id);

#ifdef CONFIG_OF
static const struct of_device_id nunchuck_dt_ids[] = {
	{ .compatible = "nintendo,nunchuck", },
	{ }
};

MODULE_DEVICE_TABLE(of, nunchuck_dt_ids);
#endif

void nunchuck_write_registers(struct i2c_client *client, const char *buf, int count)
{
	int status;
	
	status = i2c_master_send(client, buf, count);

	if(status < 0)
	{
		printk("Error writing to nunchuck: %i\n", status);
	}/*
	else
	{
		printk("%i bytes written to nunchuck\n", status);
	}*/

	udelay(1000);
}

void nunchuck_read_registers(struct i2c_client *client, char *buf, int count)
{
	const char read[] = {0x00};
	int status;

	mdelay(10);
	
	nunchuck_write_registers(client, read, sizeof(read));
	mdelay(9);

	status = i2c_master_recv(client, buf, count);

	if(status < 0)
        {
                printk("Error reading from nunchuck: %i\n", status);
        }/*
        else
        {
                printk("%i bytes read from nunchuck\n", status);
        }*/
}

bool zPressed(char *buf)
{
	bool zPress = false;
	char c = buf[5];

	c &= 0x01;
	
	if(c == 0x00)
	{
		zPress = true;
	}
	else
	{
		zPress = false;
	}
	return zPress;
}

bool cPressed(char *buf)
{
	bool cPress = false;
	char c = buf[5];

	c &= 0x02;
	
	if(c == 0x00)
	{
		cPress = true;
	}
	else
	{
		cPress = false;
	}
	return cPress;
}

struct nunchuck_dev
{
	struct i2c_client *i2c_client;
	struct input_polled_dev *polled_input;
};

void nunchuck_poll(struct input_polled_dev *polled_input)
{
	char buf[6];
	bool cPress;
	bool zPress;

	struct nunchuck_dev *nunchuck = polled_input->private;
	struct i2c_client *client = nunchuck->i2c_client;

	nunchuck_read_registers(client, buf, sizeof(buf));

	cPress = cPressed(buf);
	zPress = zPressed(buf);

	input_event(polled_input->input, EV_KEY, BTN_Z, zPress);
	input_event(polled_input->input, EV_KEY, BTN_C, cPress);
	input_sync(polled_input->input);
}

static int nunchuck_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	const char init1[] = {0xf0, 0x55};
	const char init2[] = {0xfb, 0x00};
//	char buf[6];
//	unsigned int i;
//	bool cPress;
//	bool zPress;
	struct nunchuck_dev *nunchuck;
	struct input_polled_dev *polled_input;
	struct input_dev *input;
	int result;

	printk("nunchuck: probe function called\n");

	nunchuck_write_registers(client, init1, sizeof(init1));
	nunchuck_write_registers(client, init2, sizeof(init2));

//	nunchuck_read_registers(client, buf, sizeof(buf));
//	nunchuck_read_registers(client, buf, sizeof(buf));

	/*for(i=0;i<sizeof(buf);i++)
	{
		printk("%x ", buf[i]);
	}
	printk("\n");*/

/*	cPress = cPressed(buf);
	if(cPress)
	{
		printk("C button pressed\n");
	}

	zPress = zPressed(buf);
	if(zPress)
	{
		printk("Z button pressed\n");
	}*/

	nunchuck = devm_kzalloc(&client->dev, sizeof(struct nunchuck_dev), GFP_KERNEL);

	if(!nunchuck) {
		dev_err(&client->dev, "Failed to allocated memory for logical device\n");
	return -ENOMEM;
	}
	
	nunchuck->i2c_client = client;
	i2c_set_clientdata(client, nunchuck);

	polled_input = input_allocate_polled_device();
	if(!polled_input) {
		pr_err("Failed to allocate memory for polling device\n");
		return -ENOMEM;
	}

	polled_input->poll_interval = 50;
	polled_input->poll = nunchuck_poll;

	input = polled_input->input;

	input->name = "Wii Nunchuck";
	input->id.bustype = BUS_I2C;
	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_C, input->keybit);
	set_bit(BTN_Z, input->keybit);

	nunchuck->polled_input = polled_input;
	polled_input->private = nunchuck;

	input->dev.parent = &client->dev;

	result = input_register_polled_device(polled_input);

	if(result < 0) {
		input_free_polled_device(polled_input);
		pr_err("Failed to register polled device\n");
		return -ENOMEM;
	}

	return 0;
}

static int nunchuck_remove(struct i2c_client *client)
{
	struct nunchuck_dev *nunchuck = i2c_get_clientdata(client);
	struct input_polled_dev *polled_input = nunchuck->polled_input;
	printk("nunchuck: remove function called\n");

	input_unregister_polled_device(polled_input);
	input_free_polled_device(polled_input);

	return 0;
}

static struct i2c_driver nunchuck_driver = {
	.probe = nunchuck_probe,
	.remove = nunchuck_remove,
	.id_table = nunchuck_id,
	.driver = {
		.name = "nunchuck",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(nunchuck_dt_ids),
	},
};

module_i2c_driver(nunchuck_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple i2c driver for nintendo nunchuck");
MODULE_AUTHOR("Skanda Guruanand");
