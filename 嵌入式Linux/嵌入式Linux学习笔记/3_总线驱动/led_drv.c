#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <asm/uaccess.h>

static int major;
static struct class *cls;
static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;

static int led_open(struct inode *inode, struct file *file)
{
    *gpio_con &= ~(0x3 << (pin * 2));
    *gpio_con |= 0x1 << (pin * 2);

    ////////////////////////////////////////
    printk("gpio_con=0x%08x\n", gpio_con);
    printk("gpio_dat=0x%08x\n", gpio_dat);
    printk("pin=%d\n", pin);
    ////////////////////////////////////////

    return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
    int val;

    copy_from_user(&val, buf, count);

    if(val == 1)
    {
        *gpio_dat &= ~(1 << pin);
    }
    else
    {
        *gpio_dat |= (1 << pin);
    }

    ////////////////////////////////////////
    printk("gpio_con=0x%08x\n", gpio_con);
    printk("gpio_dat=0x%08x\n", gpio_dat);
    printk("pin=%d, val=%d\n", pin, val);
    ////////////////////////////////////////

    return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open  = led_open,
    .write = led_write,
};

static int led_probe(struct platform_device *pdev)
{
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    gpio_con = ioremap(res->start, res->end - res->start + 1);
    gpio_dat = gpio_con + 1;

    res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    pin = res->start;

    printk("led_probe, found led\n");
    major = register_chrdev(0, "myled", &led_fops);
    cls = class_create(THIS_MODULE, "myled");
    class_device_create(cls, NULL, MKDEV(major, 0), NULL, "led");
    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    printk("led_remove, remove led\n");
    class_device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, "myled");
    iounmap(gpio_con);
    return 0;
}

static struct platform_driver led_drv = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "myled",
    }
};

static int led_drv_init(void)
{
    return platform_driver_register(&led_drv);
}

static void led_drv_exit(void)
{
    platform_driver_unregister(&led_drv);
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");

