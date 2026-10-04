/*
 * platform indepent driver interface
 *
 * Coypritht (c) 2017 Goodix
 */
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/timer.h>
#include <linux/err.h>

#include "gf_spi.h"

#if defined(USE_SPI_BUS)
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#elif defined(USE_PLATFORM_BUS)
#include <linux/platform_device.h>
#endif

int gf_parse_dts(struct gf_dev *gf_dev)
{
	int rc, gpio;
	struct device *dev = &gf_dev->spi->dev;
	struct device_node *np = dev->of_node;

	gpio = of_get_named_gpio(np, "goodix,gpio_vdd", 0);
	if (gpio < 0)
		return gpio;
	rc = devm_gpio_request(dev, gpio, "goodix_vdd");
	if (rc)
		return rc;
	gf_dev->vdd_gpio = gpio;
	rc = gpio_direction_output(gpio, 1);
	if (rc)
		goto err_cleanup;

	gpio = of_get_named_gpio(np, "goodix,reset_gpio", 0);
	if (gpio < 0) {
		rc = gpio;
		goto err_cleanup;
	}
	rc = devm_gpio_request(dev, gpio, "goodix_reset");
	if (rc)
		goto err_cleanup;
	gf_dev->reset_gpio = gpio;
	rc = gpio_direction_output(gpio, 1);
	if (rc)
		goto err_cleanup;

	gpio = of_get_named_gpio(np, "goodix,irq_gpio", 0);
	if (gpio < 0) {
		rc = gpio;
		goto err_cleanup;
	}
	rc = devm_gpio_request(dev, gpio, "goodix_irq");
	if (rc)
		goto err_cleanup;
	gf_dev->irq_gpio = gpio;
	rc = gpio_direction_input(gpio);
	if (rc)
		goto err_cleanup;

	return 0;

err_cleanup:
	gf_cleanup(gf_dev);
	return rc;
}

void gf_cleanup(struct gf_dev *gf_dev)
{
	struct device *dev = &gf_dev->spi->dev;

	pr_info("[info] %s\n", __func__);

	if (gpio_is_valid(gf_dev->irq_gpio)) {
		devm_gpio_free(dev, gf_dev->irq_gpio);
		gf_dev->irq_gpio = -EINVAL;
	}
	if (gpio_is_valid(gf_dev->reset_gpio)) {
		devm_gpio_free(dev, gf_dev->reset_gpio);
		gf_dev->reset_gpio = -EINVAL;
	}
	if (gpio_is_valid(gf_dev->vdd_gpio)) {
		devm_gpio_free(dev, gf_dev->vdd_gpio);
		gf_dev->vdd_gpio = -EINVAL;
	}
}

int gf_power_on(struct gf_dev *gf_dev)
{
	int rc = 0;

	/* TODO: add your power control here */
	return rc;
}

int gf_power_off(struct gf_dev *gf_dev)
{
	int rc = 0;

	/* TODO: add your power control here */

	return rc;
}

int gf_hw_reset(struct gf_dev *gf_dev, unsigned int delay_ms)
{
	int ret = -1;

	if (gf_dev == NULL) {
		pr_info("Input buff is NULL.\n");
		return ret;
	}
	gpio_direction_output(gf_dev->reset_gpio, 1);
	gpio_set_value(gf_dev->reset_gpio, 0);
	mdelay(3);
	gpio_set_value(gf_dev->reset_gpio, 1);
	mdelay(delay_ms);
	return 0;
}

int gf_irq_num(struct gf_dev *gf_dev)
{
	int ret = -1;
	if (gf_dev == NULL) {
		pr_info("Input buff is NULL.\n");
		return ret;
	} else {
		return gpio_to_irq(gf_dev->irq_gpio);
	}
}
