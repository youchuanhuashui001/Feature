/*
 * Regulator driver for PWM Regulators
 *
 * Copyright (C) 2014 - STMicroelectronics Inc.
 *
 * Author: Lee Jones <lee.jones@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pwm.h>
#include <linux/gpio/consumer.h>
#include <linux/workqueue.h>

struct pwm_continuous_reg_data {
	unsigned int min_uV_dutycycle;
	unsigned int max_uV_dutycycle;
	unsigned int dutycycle_unit;
};

struct pwm_regulator_data {
	/*  Shared */
	struct pwm_device *pwm;

	/* Voltage table */
	struct pwm_voltages *duty_cycle_table;

	/* Continuous mode info */
	struct pwm_continuous_reg_data continuous;

	/* regulator descriptor */
	struct regulator_desc desc;

	/* Regulator ops */
	struct regulator_ops ops;

	int state;

	/* Enable GPIO */
	struct gpio_desc *enb_gpio;

	/* 校准电压的时间周期 */
	unsigned int adjust_time;

	/* sar adc channel */
	unsigned int adc_channel;

	struct delayed_work dw_work;

	/* 允许的误差范围 */
	unsigned int allowable_error;

	unsigned int min_duty_cycle;
	unsigned int max_duty_cycle;
};

struct pwm_voltages {
	unsigned int uV;
	unsigned int dutycycle;
};

extern int gx_saradc_get_channel_voltage(int channel, int *voltage_uv);


void pwm_regulator_adjust_workfn(struct work_struct *work)
{
	struct pwm_regulator_data *drvdata = container_of(to_delayed_work(work), struct pwm_regulator_data, dw_work);
	struct pwm_state pstate;
	unsigned int cur_voltage, cur_duty_cycle;
	unsigned int target_voltage;
	unsigned int tmp_value = 0;
	int i;
	int ret;

#if 0
	int cpu_value, core_value;
	int i;
	unsigned int tmp_cpu_value = 0, tmp_core_value = 0;
//	ret = gx_saradc_get_raw_value(drvdata->fb.adc_channel, &cpu_value);
	for (i = 0; i < 10; i++) {
		ret = gx_saradc_get_channel_voltage(6, &core_value);
		if (ret) {
			pr_info("ret:%d, %d %s\n", ret, __LINE__, __FUNCTION__);
			return;
		}

		ret = gx_saradc_get_channel_voltage(7, &cpu_value);
		if (ret) {
			pr_info("%d %s\n", __LINE__, __FUNCTION__);
			return;
		}

		tmp_core_value += core_value;
		tmp_cpu_value += cpu_value;
	}
	core_value = tmp_core_value / 10;
	cpu_value = tmp_cpu_value / 10;

	pr_info("\n\ncpu_value:%d, core_value:%d\n", cpu_value, core_value);
	pr_info("\n\ntmp_cpu_value:%d, tmp_core_value:%d\n", tmp_cpu_value, tmp_core_value);
	pr_info("cpu_value:%d.%03dV, core_value:%d.%03dV\n\n", cpu_value / 1000000, (cpu_value % 1000000) / 1000  ,core_value / 1000000, (core_value % 1000000) / 1000);
#else

	for (i = 0; i < 10; i++) {
		ret = gx_saradc_get_channel_voltage(drvdata->adc_channel, &cur_voltage);
		if (ret) {
			pr_info("Failed to get channel voltage: %d\n", ret);
			return;
		}
		tmp_value += cur_voltage;
	}
	cur_voltage = tmp_value / 10;

#endif

	pwm_get_state(drvdata->pwm, &pstate);
	cur_duty_cycle = pwm_get_relative_duty_cycle(&pstate, 100);

	target_voltage = drvdata->duty_cycle_table[drvdata->state].uV;

	pr_info("cur_voltage:%d, cur_duty_cycle:%d, target_voltage:%d \n", cur_voltage, cur_duty_cycle, target_voltage);

	if (abs(cur_voltage - target_voltage) > drvdata->allowable_error) {
		if (cur_voltage < target_voltage) {
			cur_duty_cycle--;
		} else {
			cur_duty_cycle++;
		}
		if (cur_duty_cycle <= drvdata->min_duty_cycle)
			cur_duty_cycle = drvdata->min_duty_cycle;
		if (cur_duty_cycle >= drvdata->max_duty_cycle)
			cur_duty_cycle = drvdata->max_duty_cycle;

		pr_info("modify cur_duty_cycle:%d\n", cur_duty_cycle);

		pwm_set_relative_duty_cycle(&pstate, cur_duty_cycle, 100);

		ret = pwm_apply_state(drvdata->pwm, &pstate);
		if (ret) {
			pr_info("Failed to configure PWM: %d\n", ret);
			return;
		}
	}

	pr_info("delayed work finish.\n");

	schedule_delayed_work(&drvdata->dw_work, msecs_to_jiffies(drvdata->adjust_time));
}

/**
 * Voltage table call-backs
 */
static void pwm_regulator_init_state(struct regulator_dev *rdev)
{
	struct pwm_regulator_data *drvdata = rdev_get_drvdata(rdev);
	struct pwm_state pwm_state;
	unsigned int dutycycle;
	int i;

	pwm_get_state(drvdata->pwm, &pwm_state);
	dutycycle = pwm_get_relative_duty_cycle(&pwm_state, 100);

	for (i = 0; i < rdev->desc->n_voltages; i++) {
		if ((dutycycle == drvdata->duty_cycle_table[i].dutycycle) || \
			((drvdata->duty_cycle_table[i].dutycycle - dutycycle) < 10)) {
			drvdata->state = i;
			return;
		}
	}
}

static int pwm_regulator_get_voltage_sel(struct regulator_dev *rdev)
{
	struct pwm_regulator_data *drvdata = rdev_get_drvdata(rdev);

	if (drvdata->state < 0)
		pwm_regulator_init_state(rdev);

	return drvdata->state;
}

static int pwm_regulator_set_voltage_sel(struct regulator_dev *rdev,
					 unsigned selector)
{
	struct pwm_regulator_data *drvdata = rdev_get_drvdata(rdev);

//	drvdata->target_voltage = drvdata->duty_cycle_table[selector].uV;
	pr_info("drvdata->state:%d, drvdata->target_voltage:%d\n", drvdata->state, drvdata->duty_cycle_table[selector].uV);

	// TODO：这里设置的好像是档位，就是在数组中的第几个
	drvdata->state = selector;

	return 0;
}

static int pwm_regulator_list_voltage(struct regulator_dev *rdev,
				      unsigned selector)
{
	struct pwm_regulator_data *drvdata = rdev_get_drvdata(rdev);

	if (selector >= rdev->desc->n_voltages)
		return -EINVAL;

	return drvdata->duty_cycle_table[selector].uV;
}

static int pwm_regulator_enable(struct regulator_dev *dev)
{
	struct pwm_regulator_data *drvdata = rdev_get_drvdata(dev);

	gpiod_set_value_cansleep(drvdata->enb_gpio, 1);

	// TODO: 确认是不是启动之前一定会执行这个 enable，这样就可以不在 init 执行调度，放到这里来处理
//	if ((!delayed_work_pending(&drvdata->dw_work)) && (!work_busy(&drvdata->dw_work.work))) {
//		schedule_delayed_work(&drvdata->dw_work, msecs_to_jiffies(drvdata->adjust_time));
//	}

	pr_info("\n\n\n");
	pr_info("!!!!!!!!!!!!!!!!!!!!!!!!!!! enable !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	pr_info("!!!!!!!!!!!!!!!!!!!!!!!!!!! enable !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	pr_info("!!!!!!!!!!!!!!!!!!!!!!!!!!! enable !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	pr_info("!!!!!!!!!!!!!!!!!!!!!!!!!!! enable !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	pr_info("!!!!!!!!!!!!!!!!!!!!!!!!!!! enable !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	pr_info("\n\n\n");

	return pwm_enable(drvdata->pwm);
}

static int pwm_regulator_disable(struct regulator_dev *dev)
{
	struct pwm_regulator_data *drvdata = rdev_get_drvdata(dev);

	pwm_disable(drvdata->pwm);

	gpiod_set_value_cansleep(drvdata->enb_gpio, 0);

	//TODO:
//	if (drvdata->dw_work) {
	cancel_delayed_work_sync(&drvdata->dw_work);
//	}

	return 0;
}

static int pwm_regulator_is_enabled(struct regulator_dev *dev)
{
	struct pwm_regulator_data *drvdata = rdev_get_drvdata(dev);

	if (drvdata->enb_gpio && !gpiod_get_value_cansleep(drvdata->enb_gpio))
		return false;

	return pwm_is_enabled(drvdata->pwm);
}

static int pwm_regulator_get_voltage(struct regulator_dev *rdev)
{
	struct pwm_regulator_data *drvdata = rdev_get_drvdata(rdev);
	unsigned int min_uV_duty = drvdata->continuous.min_uV_dutycycle;
	unsigned int max_uV_duty = drvdata->continuous.max_uV_dutycycle;
	unsigned int duty_unit = drvdata->continuous.dutycycle_unit;
	int min_uV = rdev->constraints->min_uV;
	int max_uV = rdev->constraints->max_uV;
	int diff_uV = max_uV - min_uV;
	struct pwm_state pstate;
	unsigned int diff_duty;
	unsigned int voltage;

	pwm_get_state(drvdata->pwm, &pstate);

	voltage = pwm_get_relative_duty_cycle(&pstate, duty_unit);

	/*
	 * The dutycycle for min_uV might be greater than the one for max_uV.
	 * This is happening when the user needs an inversed polarity, but the
	 * PWM device does not support inversing it in hardware.
	 */
	if (max_uV_duty < min_uV_duty) {
		voltage = min_uV_duty - voltage;
		diff_duty = min_uV_duty - max_uV_duty;
	} else {
		voltage = voltage - min_uV_duty;
		diff_duty = max_uV_duty - min_uV_duty;
	}

	voltage = DIV_ROUND_CLOSEST_ULL((u64)voltage * diff_uV, diff_duty);

	return voltage + min_uV;
}

static int pwm_regulator_set_voltage(struct regulator_dev *rdev,
				     int req_min_uV, int req_max_uV,
				     unsigned int *selector)
{
	struct pwm_regulator_data *drvdata = rdev_get_drvdata(rdev);
	unsigned int min_uV_duty = drvdata->continuous.min_uV_dutycycle;
	unsigned int max_uV_duty = drvdata->continuous.max_uV_dutycycle;
	unsigned int duty_unit = drvdata->continuous.dutycycle_unit;
	int min_uV = rdev->constraints->min_uV;
	int max_uV = rdev->constraints->max_uV;
	int diff_uV = max_uV - min_uV;
	struct pwm_state pstate;
	unsigned int diff_duty;
	unsigned int dutycycle;
	int ret;

	pwm_init_state(drvdata->pwm, &pstate);

	/*
	 * The dutycycle for min_uV might be greater than the one for max_uV.
	 * This is happening when the user needs an inversed polarity, but the
	 * PWM device does not support inversing it in hardware.
	 */
	if (max_uV_duty < min_uV_duty)
		diff_duty = min_uV_duty - max_uV_duty;
	else
		diff_duty = max_uV_duty - min_uV_duty;

	dutycycle = DIV_ROUND_CLOSEST_ULL((u64)(req_min_uV - min_uV) *
					  diff_duty,
					  diff_uV);

	if (max_uV_duty < min_uV_duty)
		dutycycle = min_uV_duty - dutycycle;
	else
		dutycycle = min_uV_duty + dutycycle;

	pwm_set_relative_duty_cycle(&pstate, dutycycle, duty_unit);

	ret = pwm_apply_state(drvdata->pwm, &pstate);
	if (ret) {
		dev_err(&rdev->dev, "Failed to configure PWM: %d\n", ret);
		return ret;
	}

	return 0;
}

static struct regulator_ops pwm_regulator_voltage_table_ops = {
	.set_voltage_sel = pwm_regulator_set_voltage_sel,
	.get_voltage_sel = pwm_regulator_get_voltage_sel,
	.list_voltage    = pwm_regulator_list_voltage,
	.map_voltage     = regulator_map_voltage_iterate,
	.enable          = pwm_regulator_enable,
	.disable         = pwm_regulator_disable,
	.is_enabled      = pwm_regulator_is_enabled,
};

static struct regulator_ops pwm_regulator_voltage_continuous_ops = {
	.get_voltage = pwm_regulator_get_voltage,
	.set_voltage = pwm_regulator_set_voltage,
	.enable          = pwm_regulator_enable,
	.disable         = pwm_regulator_disable,
	.is_enabled      = pwm_regulator_is_enabled,
};

static struct regulator_desc pwm_regulator_desc = {
	.name		= "pwm-regulator",
	.type		= REGULATOR_VOLTAGE,
	.owner		= THIS_MODULE,
	.supply_name    = "pwm",
};

static int pwm_regulator_init_table(struct platform_device *pdev,
				    struct pwm_regulator_data *drvdata)
{
	struct device_node *np = pdev->dev.of_node;
	struct pwm_voltages *duty_cycle_table;
	unsigned int length = 0;
	int ret;
	int i;

	of_find_property(np, "voltage-table", &length);

	if ((length < sizeof(*duty_cycle_table)) ||
	    (length % sizeof(*duty_cycle_table))) {
		dev_err(&pdev->dev, "voltage-table length(%d) is invalid\n",
			length);
		return -EINVAL;
	}

	duty_cycle_table = devm_kzalloc(&pdev->dev, length, GFP_KERNEL);
	if (!duty_cycle_table)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "voltage-table",
					 (u32 *)duty_cycle_table,
					 length / sizeof(u32));
	if (ret) {
		dev_err(&pdev->dev, "Failed to read voltage-table: %d\n", ret);
		return ret;
	}

	drvdata->min_duty_cycle = drvdata->max_duty_cycle = duty_cycle_table[0].dutycycle;
	for (i = 0; i < (length / sizeof(u32)) / 2; i++) {
		if (drvdata->min_duty_cycle > duty_cycle_table[i].dutycycle)
			drvdata->min_duty_cycle = duty_cycle_table[i].dutycycle;
		if (drvdata->max_duty_cycle < duty_cycle_table[i].dutycycle)
			drvdata->max_duty_cycle = duty_cycle_table[i].dutycycle;
	}

	ret = of_property_read_u32(np, "ajust_time", &drvdata->adjust_time);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get ajust_time: %d\n", ret);
		return ret;
	}

	ret = of_property_read_u32(np, "allowable_error", &drvdata->allowable_error);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get allowable_error: %d\n", ret);
		return ret;
	}

	ret = of_property_read_u32(np, "adc_channel", &drvdata->adc_channel);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get adc channel: %d\n", ret);
		return ret;
	}

	if (drvdata->adc_channel) {
		INIT_DELAYED_WORK(&drvdata->dw_work, pwm_regulator_adjust_workfn);
		schedule_delayed_work(&drvdata->dw_work, msecs_to_jiffies(drvdata->adjust_time));
	}

	drvdata->state			= -ENOTRECOVERABLE;
	drvdata->duty_cycle_table	= duty_cycle_table;
	memcpy(&drvdata->ops, &pwm_regulator_voltage_table_ops,
	       sizeof(drvdata->ops));
	drvdata->desc.ops = &drvdata->ops;
	drvdata->desc.n_voltages	= length / sizeof(*duty_cycle_table);

	return 0;
}

static int pwm_regulator_init_continuous(struct platform_device *pdev,
					 struct pwm_regulator_data *drvdata)
{
	u32 dutycycle_range[2] = { 0, 100 };
	u32 dutycycle_unit = 100;

	memcpy(&drvdata->ops, &pwm_regulator_voltage_continuous_ops,
	       sizeof(drvdata->ops));
	drvdata->desc.ops = &drvdata->ops;
	drvdata->desc.continuous_voltage_range = true;

	of_property_read_u32_array(pdev->dev.of_node,
				   "pwm-dutycycle-range",
				   dutycycle_range, 2);
	of_property_read_u32(pdev->dev.of_node, "pwm-dutycycle-unit",
			     &dutycycle_unit);

	if (dutycycle_range[0] > dutycycle_unit ||
	    dutycycle_range[1] > dutycycle_unit)
		return -EINVAL;

	drvdata->continuous.dutycycle_unit = dutycycle_unit;
	drvdata->continuous.min_uV_dutycycle = dutycycle_range[0];
	drvdata->continuous.max_uV_dutycycle = dutycycle_range[1];

	return 0;
}

static int pwm_regulator_probe(struct platform_device *pdev)
{
	const struct regulator_init_data *init_data;
	struct pwm_regulator_data *drvdata;
	struct regulator_dev *regulator;
	struct regulator_config config = { };
	struct device_node *np = pdev->dev.of_node;
	enum gpiod_flags gpio_flags;
	int ret;

	if (!np) {
		dev_err(&pdev->dev, "Device Tree node missing\n");
		return -EINVAL;
	}

	drvdata = devm_kzalloc(&pdev->dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	memcpy(&drvdata->desc, &pwm_regulator_desc, sizeof(drvdata->desc));

	if (of_find_property(np, "voltage-table", NULL))
		ret = pwm_regulator_init_table(pdev, drvdata);
	else
		ret = pwm_regulator_init_continuous(pdev, drvdata);
	if (ret)
		return ret;

	init_data = of_get_regulator_init_data(&pdev->dev, np,
					       &drvdata->desc);
	if (!init_data)
		return -ENOMEM;

	config.of_node = np;
	config.dev = &pdev->dev;
	config.driver_data = drvdata;
	config.init_data = init_data;

	drvdata->pwm = devm_pwm_get(&pdev->dev, NULL);
	if (IS_ERR(drvdata->pwm)) {
		ret = PTR_ERR(drvdata->pwm);
		dev_err(&pdev->dev, "Failed to get PWM: %d\n", ret);
		return ret;
	}

	if (init_data->constraints.boot_on || init_data->constraints.always_on)
		gpio_flags = GPIOD_OUT_HIGH;
	else
		gpio_flags = GPIOD_OUT_LOW;
	drvdata->enb_gpio = devm_gpiod_get_optional(&pdev->dev, "enable",
						    gpio_flags);
	if (IS_ERR(drvdata->enb_gpio)) {
		ret = PTR_ERR(drvdata->enb_gpio);
		dev_err(&pdev->dev, "Failed to get enable GPIO: %d\n", ret);
		return ret;
	}

	ret = pwm_adjust_config(drvdata->pwm);
	if (ret)
		return ret;

	regulator = devm_regulator_register(&pdev->dev,
					    &drvdata->desc, &config);
	if (IS_ERR(regulator)) {
		ret = PTR_ERR(regulator);
		dev_err(&pdev->dev, "Failed to register regulator %s: %d\n",
			drvdata->desc.name, ret);
		return ret;
	}

	return 0;
}

static const struct of_device_id pwm_of_match[] = {
	{ .compatible = "pwm-regulator" },
	{ },
};
MODULE_DEVICE_TABLE(of, pwm_of_match);

static struct platform_driver pwm_regulator_driver = {
	.driver = {
		.name		= "pwm-regulator",
		.of_match_table = of_match_ptr(pwm_of_match),
	},
	.probe = pwm_regulator_probe,
};

module_platform_driver(pwm_regulator_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lee Jones <lee.jones@linaro.org>");
MODULE_DESCRIPTION("PWM Regulator Driver");
MODULE_ALIAS("platform:pwm-regulator");
