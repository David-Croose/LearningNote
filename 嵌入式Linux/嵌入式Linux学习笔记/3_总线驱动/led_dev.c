#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/platform_device.h>

static struct resource led_resource[] = {
    [0] = {
        .start = 0x56000050,
        .end   = 0x56000050 + 8 - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = 4,
        .end   = 4,
        .flags = IORESOURCE_IRQ,
    },
};

static void led_release(struct device *dev)
{
    /* 不用做事情 */
}

static struct platform_device led_dev = {
    .name         = "myled",
    .id           = -1,
    .num_resources= ARRAY_SIZE(led_resource),
    .resource     = led_resource,
    .dev          = {
        .release  = led_release,
    }
};

static int led_dev_init(void)
{
    platform_device_register(&led_dev);
    return 0;
}

static void led_dev_exit(void)
{
    platform_device_unregister(&led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");

