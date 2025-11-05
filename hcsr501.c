#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>

#define DRV_NAME "hcsr501"

static int gpio = -1;
static int irq  = -1;

module_param(gpio, int, 0444);
MODULE_PARM_DESC(gpio, "GPIO number connected to HC-SR501 (use SAME as in working poll test)");

static dev_t devt;
static struct cdev hcsr_cdev;
static struct class *hcsr_class;

static DECLARE_WAIT_QUEUE_HEAD(hcsr_wq);
static int event_pending;
static int last_value;

static irqreturn_t hcsr_irq_handler(int irq_, void *dev_id)
{
    int value = gpio_get_value(gpio);

    last_value = value;
    event_pending = 1;

    pr_debug(DRV_NAME ": IRQ %d fired -> GPIO=%d value=%d\n", irq_, gpio, value);
    pr_info(DRV_NAME ": IRQ %d fired -> GPIO=%d value=%d\n", irq_, gpio, value);

    wake_up_interruptible(&hcsr_wq);
    return IRQ_HANDLED;
}

static ssize_t hcsr_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    int ret;

    if (len < 1)
    {
        return -EINVAL;
    }

    if (!event_pending) 
    {
        if (filp->f_flags & O_NONBLOCK)
        {
            return -EAGAIN;
        }

        ret = wait_event_interruptible(hcsr_wq, event_pending);
        if (ret)
        {
            return ret;
        }
    }

    event_pending = 0;

    if (copy_to_user(buf, &last_value, 1))
    {
        return -EFAULT;
    }

    return 1;
}

static __poll_t hcsr_poll(struct file *filp, poll_table *wait)
{
    __poll_t mask = 0;

    poll_wait(filp, &hcsr_wq, wait);

    if (event_pending)
    {
        mask |= POLLIN | POLLRDNORM;
    }

    return mask;
}

static const struct file_operations hcsr_fops = 
{
    .owner = THIS_MODULE,
    .read  = hcsr_read,
    .poll  = hcsr_poll,
};

static int __init hcsr_init(void)
{
    int ret;

    pr_info(DRV_NAME ": init start\n");

    if (gpio < 0) 
    {
        pr_err(DRV_NAME ": You must specify gpio=N (e.g. insmod hcsr501.ko gpio=519)\n");
        return -EINVAL;
    }

    ret = alloc_chrdev_region(&devt, 0, 1, DRV_NAME);
    if (ret)
    {
        return ret;
    }

    cdev_init(&hcsr_cdev, &hcsr_fops);
    ret = cdev_add(&hcsr_cdev, devt, 1);
    if (ret)
    {
        goto err_chrdev;
    }

    hcsr_class = class_create(DRV_NAME);
    if (IS_ERR(hcsr_class)) 
    {
        ret = PTR_ERR(hcsr_class);
        goto err_cdev;
    }

    if (IS_ERR(device_create(hcsr_class, NULL, devt, NULL, DRV_NAME))) 
    {
        ret = -ENOMEM;
        goto err_class;
    }

    pr_info(DRV_NAME ": /dev/%s created\n", DRV_NAME);

    pr_info(DRV_NAME ": requesting GPIO %d\n", gpio);
    ret = gpio_request(gpio, DRV_NAME);
    if (ret) 
    {
        pr_err(DRV_NAME ": gpio_request failed (%d)\n", ret);
        goto err_dev;
    }

    ret = gpio_direction_input(gpio);
    if (ret) 
    {
        pr_err(DRV_NAME ": gpio_direction_input failed (%d)\n", ret);
        goto err_gpio;
    }

    irq = gpio_to_irq(gpio);
    if (irq < 0) 
    {
        pr_err(DRV_NAME ": gpio_to_irq(%d) failed (%d)\n", gpio, irq);
        ret = irq;
        goto err_gpio;
    }
    pr_info(DRV_NAME ": mapped GPIO %d -> IRQ %d\n", gpio, irq);

    ret = request_irq(irq, hcsr_irq_handler,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                      DRV_NAME, NULL);
    if (ret) 
    {
        pr_err(DRV_NAME ": request_irq(%d) failed (%d)\n", irq, ret);
        goto err_gpio;
    }
    pr_info(DRV_NAME ": IRQ %d requested for GPIO %d (both edges)\n", irq, gpio);

    last_value = gpio_get_value(gpio);
    pr_info(DRV_NAME ": initial GPIO value = %d\n", last_value);

    pr_info(DRV_NAME ": init done\n");
    return 0;

err_gpio:
    gpio_free(gpio);
err_dev:
    device_destroy(hcsr_class, devt);
err_class:
    class_destroy(hcsr_class);
err_cdev:
    cdev_del(&hcsr_cdev);
err_chrdev:
    unregister_chrdev_region(devt, 1);
    return ret;
}

static void __exit hcsr_exit(void)
{
    pr_info(DRV_NAME ": exit start\n");

    if (irq >= 0)
    {
        free_irq(irq, NULL);
    }

    if (gpio >= 0)
    {
        gpio_free(gpio);
    }

    device_destroy(hcsr_class, devt);
    class_destroy(hcsr_class);
    cdev_del(&hcsr_cdev);
    unregister_chrdev_region(devt, 1);

    pr_info(DRV_NAME ": exit done\n");
}

module_init(hcsr_init);
module_exit(hcsr_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Filip-like");
MODULE_DESCRIPTION("HC-SR501 PIR Linux driver with IRQ + char device");
