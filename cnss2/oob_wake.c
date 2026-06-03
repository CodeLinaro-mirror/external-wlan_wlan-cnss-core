/*
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
*
* Permission to use, copy, modify, and/or distribute this software for
* any purpose with or without fee is hereby granted, provided that the
* above copyright notice and this permission notice appear in all
* copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
* WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
* AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
* DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
* PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
* TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
*/

#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/pm_wakeup.h>
#ifdef WLAN_GPIO_WAKEUP_LEGACY
#include <linux/gpio.h>
#endif
#ifdef WLAN_GPIO_WAKEUP_GPIOD
#include <linux/gpio/consumer.h>
#endif
#ifdef WLAN_GPIO_WAKEUP_OF_IRQ
#include <linux/of_irq.h>
#endif
#include "oob_wake.h"

#if defined(WLAN_GPIO_WAKEUP_LEGACY) || defined(WLAN_GPIO_WAKEUP_GPIOD) || \
    defined(WLAN_GPIO_WAKEUP_OF_IRQ)

static int gpio_wakeup_irq = -1;
static struct gpio_wakeup_cfg gpio_cfg;

static irqreturn_t cnss_gpio_wakeup_isr(int irq, void *dev)
{
	dev_info(dev, "gpio wakeup IRQ %d triggered\n", irq);
	disable_irq_nosync(gpio_wakeup_irq);
	return IRQ_HANDLED;
}

#ifdef WLAN_GPIO_WAKEUP_LEGACY
static int
cnss_gpio_wakeup_init_legacy(struct device *dev)
{
	unsigned long irq_flag;
	int ret;

	if (gpio_cfg.pin == 0 || gpio_cfg.pin >= 255 ||
	    gpio_cfg.irq_trigger == 0) {
		dev_err(dev, "gpio(%u) trigger(%u) param error\n",
			gpio_cfg.pin, gpio_cfg.irq_trigger);
		return -EINVAL;
	}

	ret = devm_gpio_request_one(dev, gpio_cfg.pin, GPIOF_IN,
				    WLAN_GPIO_WAKEUP_IRQ_LABEL);
	if (ret) {
		dev_err(dev, "devm_gpio_request_one(%u) failed: %d\n",
			gpio_cfg.pin, ret);
		return ret;
	}

	gpio_wakeup_irq = gpio_to_irq(gpio_cfg.pin);
	if (gpio_wakeup_irq <= 0) {
		dev_err(dev, "gpio_to_irq(%u) failed: %d\n",
			gpio_cfg.pin, gpio_wakeup_irq);
		return gpio_wakeup_irq;
	}

	irq_flag = gpio_cfg.irq_trigger | IRQF_ONESHOT;
	ret = devm_request_threaded_irq(dev, gpio_wakeup_irq, NULL,
					cnss_gpio_wakeup_isr, irq_flag,
					WLAN_GPIO_WAKEUP_IRQ_LABEL, dev);
	if (ret) {
		dev_err(dev, "devm_request_irq(%d) failed: %d\n",
			gpio_wakeup_irq, ret);
		return ret;
	}

	ret = enable_irq_wake(gpio_wakeup_irq);
	if (ret) {
		dev_err(dev, "enable_irq_wake(%d) failed: %d\n",
			gpio_wakeup_irq, ret);
		devm_free_irq(dev, gpio_wakeup_irq, dev);
		return ret;
	}

	device_init_wakeup(dev, true);
	return 0;
}

static void
cnss_gpio_wakeup_deinit_legacy(struct device *dev)
{
	disable_irq_wake(gpio_wakeup_irq);
	devm_free_irq(dev, gpio_wakeup_irq, dev);
	device_init_wakeup(dev, false);
}
#else
static int
cnss_gpio_wakeup_init_legacy(struct device *dev)
{
	return 0;
}

static void
cnss_gpio_wakeup_deinit_legacy(struct device *dev)
{
}
#endif /* WLAN_GPIO_WAKEUP_LEGACY */

#ifdef WLAN_GPIO_WAKEUP_GPIOD
static int
cnss_gpio_wakeup_init_gpiod(struct device *dev)
{
	struct gpio_desc *gd;
	unsigned long irq_flag;
	int ret;

	gd = devm_gpiod_get(dev, WLAN_GPIO_WAKEUP_GPIOD_CON_ID, GPIOD_IN);
	if (IS_ERR(gd)) {
		dev_err(dev, "devm_gpiod_get(%s) failed: %ld\n",
			WLAN_GPIO_WAKEUP_GPIOD_CON_ID, PTR_ERR(gd));
		return PTR_ERR(gd);
	}

	gpio_wakeup_irq = gpiod_to_irq(gd);
	if (gpio_wakeup_irq <= 0) {
		dev_err(dev, "gpiod_to_irq failed: %d\n", gpio_wakeup_irq);
		return gpio_wakeup_irq;
	}

	irq_flag = gpio_cfg.irq_trigger | IRQF_ONESHOT;
	ret = devm_request_threaded_irq(dev, gpio_wakeup_irq, NULL,
					cnss_gpio_wakeup_isr, irq_flag,
					WLAN_GPIO_WAKEUP_IRQ_LABEL, dev);
	if (ret) {
		dev_err(dev, "devm_request_irq(%d) failed: %d\n",
			gpio_wakeup_irq, ret);
		return ret;
	}

	ret = enable_irq_wake(gpio_wakeup_irq);
	if (ret) {
		dev_err(dev, "enable_irq_wake(%d) failed: %d\n",
			gpio_wakeup_irq, ret);
		devm_free_irq(dev, gpio_wakeup_irq, dev);
		return ret;
	}

	device_init_wakeup(dev, true);
	return 0;
}

static void
cnss_gpio_wakeup_deinit_gpiod(struct device *dev)
{
	disable_irq_wake(gpio_wakeup_irq);
	devm_free_irq(dev, gpio_wakeup_irq, dev);
	device_init_wakeup(dev, false);
}
#else
static int
cnss_gpio_wakeup_init_gpiod(struct device *dev)
{
	return 0;
}
static void
cnss_gpio_wakeup_deinit_gpiod(struct device *dev)
{
}
#endif /* WLAN_GPIO_WAKEUP_GPIOD */

#ifdef WLAN_GPIO_WAKEUP_OF_IRQ
static int
cnss_gpio_wakeup_init_of_irq(struct device *dev)
{
	int ret;
	unsigned long irq_flag = IRQF_TRIGGER_LOW;

	gpio_wakeup_irq = of_irq_get_byname(dev->of_node,
					    WLAN_GPIO_WAKEUP_OF_IRQ_NAME);
	if (gpio_wakeup_irq <= 0) {
		dev_err(dev, "of_irq_get_byname(%s) failed: %d\n",
			WLAN_GPIO_WAKEUP_OF_IRQ_NAME, gpio_wakeup_irq);
		return gpio_wakeup_irq;
	}

	irq_flag = irq_get_trigger_type(gpio_wakeup_irq);
	if (!irq_flag)
		/* Fall back to INI setting */
		irq_flag = gpio_cfg.irq_trigger;

	irq_flag |= IRQF_ONESHOT;
	dev_info(dev, "wake IRQ: %lu trigger\n", irq_flag);

	ret = devm_request_threaded_irq(dev, gpio_wakeup_irq, NULL,
					cnss_gpio_wakeup_isr, irq_flag,
					WLAN_GPIO_WAKEUP_IRQ_LABEL, dev);
	if (ret) {
		dev_err(dev, "devm_request_irq(%d) failed: %d\n",
			gpio_wakeup_irq, ret);
		return ret;
	}

	ret = enable_irq_wake(gpio_wakeup_irq);
	if (ret) {
		dev_err(dev, "enable_irq_wake(%d) failed: %d\n",
			gpio_wakeup_irq, ret);
		devm_free_irq(dev, gpio_wakeup_irq, dev);
		return ret;
	}

	device_init_wakeup(dev, true);
	return 0;
}

static void
cnss_gpio_wakeup_deinit_of_irq(struct device *dev)
{
	disable_irq_wake(gpio_wakeup_irq);
	devm_free_irq(dev, gpio_wakeup_irq, dev);
	device_init_wakeup(dev, false);
}
#else
static int
cnss_gpio_wakeup_init_of_irq(struct device *dev)
{
	return 0;
}

static void
cnss_gpio_wakeup_deinit_of_irq(struct device *dev)
{
}
#endif /* WLAN_GPIO_WAKEUP_OF_IRQ */

int cnss_gpio_wakeup_init(struct device *dev,
			  const struct gpio_wakeup_cfg *cfg)
{
	int ret;

	if (!cfg)
		return -EINVAL;
	memcpy(&gpio_cfg, cfg, sizeof(*cfg));

	if (gpio_cfg.backend == GPIO_WAKEUP_OF_IRQ)
		ret = cnss_gpio_wakeup_init_of_irq(dev);
	else if (gpio_cfg.backend == GPIO_WAKEUP_GPIOD)
		ret = cnss_gpio_wakeup_init_gpiod(dev);
	else
		ret = cnss_gpio_wakeup_init_legacy(dev);

	if (!ret)
		dev_info(dev, "gpio wakeup init done (irq=%d backend=%u, pin=%u, trigger=%u)\n",
			 gpio_wakeup_irq, gpio_cfg.backend,
			 gpio_cfg.pin, gpio_cfg.irq_trigger);
	return ret;
}
EXPORT_SYMBOL(cnss_gpio_wakeup_init);

int cnss_gpio_wakeup_deinit(struct device *dev)
{
	if (gpio_wakeup_irq <= 0) {
		dev_err(dev, "gpio_wakeup_irq %d invalid\n", gpio_wakeup_irq);
		return -EINVAL;
	}

	if (gpio_cfg.backend == GPIO_WAKEUP_OF_IRQ)
		cnss_gpio_wakeup_deinit_of_irq(dev);
	else if (gpio_cfg.backend == GPIO_WAKEUP_GPIOD)
		cnss_gpio_wakeup_deinit_gpiod(dev);
	else
		cnss_gpio_wakeup_deinit_legacy(dev);

	dev_info(dev, "cnss_gpio_wakeup_deinit done\n");
	return 0;
}
EXPORT_SYMBOL(cnss_gpio_wakeup_deinit);

#else /* !WLAN_GPIO_WAKEUP_LEGACY && !WLAN_GPIO_WAKEUP_GPIOD && !WLAN_GPIO_WAKEUP_OF_IRQ */

int cnss_gpio_wakeup_init(struct device *dev,
			  const struct gpio_wakeup_cfg *cfg)
{
	return 0;
}
EXPORT_SYMBOL(cnss_gpio_wakeup_init);

int cnss_gpio_wakeup_deinit(struct device *dev)
{
	return 0;
}
EXPORT_SYMBOL(cnss_gpio_wakeup_deinit);

#endif /* WLAN_GPIO_WAKEUP_LEGACY || WLAN_GPIO_WAKEUP_GPIOD || WLAN_GPIO_WAKEUP_OF_IRQ */
