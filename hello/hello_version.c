#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/utsname.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
/* Add your code here */


static char *whom = "Skanda";
module_param(whom, charp, 0);

static struct timeval init_tv;

static int hello_elapsed_time(void)
{
	struct timeval current_tv;
	do_gettimeofday(&current_tv);
	return ((int)current_tv.tv_sec) - ((int) init_tv.tv_sec);
}

/*static int hello_elapsed_proc_info
	(char *buffer, char **start, off_t fpos, int length)
{
	int size;
	size = sprintf(buffer, "The hello_version module was loaded %d
				seconds ago\n",hello_elapsed_time());
	return size;
}*/
static int __init hello_init(void)
{
	printk(KERN_INFO "Hello %s. You are currently using Linux %s\n", whom, utsname()->version);
	do_gettimeofday(&init_tv);

/*	if(create_proc_info_entry("hello version elapsed",
		0, NULL, hello_elapsed_proc_info) == NULL)
		return -ENOMEM;
*/
	return 0;
}

static void __exit hello_exit(void)
{
	/*remove_proc_entry("hello_version_elapsed", NULL);*/
	printk(KERN_INFO "Goodbye. This module lived %d seconds\n", hello_elapsed_time());
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Greeting Module");
MODULE_AUTHOR("Skanda Guruanand");
