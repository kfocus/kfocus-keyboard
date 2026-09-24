// SPDX-License-Identifier: GPL-2.0+
/*!
 * Copyright (c) 2024 TUXEDO Computers GmbH <tux@tuxedocomputers.com>
 *
 * This file is part of tuxedo-drivers.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/acpi.h>
#include <linux/platform_device.h>
#include <linux/version.h>

#define DRIVER_NAME "gxtp7380"

static void gxtp7380_notify(acpi_handle handle, u32 event, void *context)
{
	struct platform_device *pdev = context;

	kobject_uevent(&pdev->dev.kobj, KOBJ_CHANGE);
}

static int gxtp7380_probe(struct platform_device *pdev)
{
	struct acpi_device *device = ACPI_COMPANION(&pdev->dev);
	int status;

	if (!device)
		return -ENODEV;

	// Legacy struct acpi_driver notify callback replaced by an explicit handler since its removal
	status = acpi_dev_install_notify_handler(device, ACPI_ALL_NOTIFY, gxtp7380_notify, pdev);
	if (status)
		return status;

	kobject_uevent(&pdev->dev.kobj, KOBJ_ADD);
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
static int gxtp7380_remove(struct platform_device *pdev)
#else
static void gxtp7380_remove(struct platform_device *pdev)
#endif
{
	struct acpi_device *device = ACPI_COMPANION(&pdev->dev);

	if (device)
		acpi_dev_remove_notify_handler(device, ACPI_ALL_NOTIFY, gxtp7380_notify);
	kobject_uevent(&pdev->dev.kobj, KOBJ_REMOVE);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
	return 0;
#endif
}

static const struct acpi_device_id gxtp7380_device_ids[] = {
	{ "GXTP7380", 0 },
	{ "", 0 }
};

static struct platform_driver gxtp7380_driver = {
	.probe = gxtp7380_probe,
	.remove = gxtp7380_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.acpi_match_table = gxtp7380_device_ids,
	},
};

module_platform_driver(gxtp7380_driver);

MODULE_AUTHOR("TUXEDO Computers GmbH <tux@tuxedocomputers.com>");
MODULE_DESCRIPTION("Touch panel disable, notify driver");
MODULE_LICENSE("GPL");

MODULE_DEVICE_TABLE(acpi, gxtp7380_device_ids);
