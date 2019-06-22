#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

#include <linux/input.h>
#include <linux/interrupt.h>

struct pin_desc{
    int irq;
    char *name;
	unsigned int pin;
	unsigned int key_val;
};

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0;
static struct timer_list timer_list;
static struct pin_desc *irq_pd;
static struct input_dev *buttons_dev;

struct pin_desc pins_desc[4] = {
	{IRQ_EINT0,  "S2", S3C2410_GPF0,  KEY_L},
	{IRQ_EINT2,  "S3", S3C2410_GPF2,  KEY_S},
	{IRQ_EINT11, "S4", S3C2410_GPG3,  KEY_ENTER},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},
};

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
    irq_pd = (struct pin_desc *)dev_id;
    mod_timer(&timer_list, msecs_to_jiffies(500));
	return IRQ_RETVAL(IRQ_HANDLED);
}

static void button_timer_callback(unsigned long dev_id)
{
	struct pin_desc * pindesc = irq_pd;
	unsigned int pinval;

    if(!pindesc)
    {
        return;
    }

	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if (pinval)
	{
		/* 松开: 最后一个参数：0-松开， 1：按下 */
        input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
	}
	else
	{
		/* 按下 */
        input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
	}
    input_sync(buttons_dev);

    ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
}

static int buttons_init(void)
{
    int i;

    /* 1. 分配一个input_dev结构体 */
    buttons_dev = input_allocate_device();

    /* 2. 设置 */
    /* 2.1 能产生哪类事件 */
    set_bit(EV_KEY, buttons_dev->evbit);

    /* 2.2 能产生这类操作里的哪类事件: L, S, ENTER, LEFTSHIFT */
    set_bit(KEY_L, buttons_dev->keybit);
    set_bit(KEY_S, buttons_dev->keybit);
    set_bit(KEY_ENTER, buttons_dev->keybit);
    set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

    /* 3. 注册 */
    input_register_device(buttons_dev);

    /* 4. 硬件相关的操作 */
    init_timer(&timer_list);
    timer_list.function = button_timer_callback;
    add_timer(&timer_list);

    for(i = 0; i < 4; i++)
    {
	    request_irq(pins_desc[i].irq, buttons_irq, IRQT_BOTHEDGE, pins_desc[i].name, &pins_desc[i]);
    }

    return 0;
}

static void buttons_exit(void)
{
    int i;

    for(i = 0; i < 4; i++)
    {
	    free_irq(pins_desc[i].irq, &pins_desc[i]);
    }
    del_timer(&timer_list);
    input_unregister_device(buttons_dev);
    input_free_device(buttons_dev);
}

module_init(buttons_init);

module_exit(buttons_exit);
