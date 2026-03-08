/*
 * idme_default_table_t950x4.h
 *
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

/*!
 * @file idme_default_table_t950x4.h
 * @brief This file contains idme default table values
 *  in the userstore partition
 *
 */

#ifndef __IDME_DEFAULT_TABLE_T950X4_H__
#define __IDME_DEFAULT_TABLE_T950X4_H__

const struct idme_init_values idme_default_values[] = {
	{ { "board_id", 16, 1, 0444 },
		/* Default Board ID value */
		"ffffff0000000000"
	},
	{ { "serial", 16, 1, 0444 },
		/* Default DSN value */
		"0000000000000000"
	},
	{ { "mac_addr", 16, 1, 0444 },
		/* Default MAC address */
		"0"
	},
	{ { "mac_sec", 32, 1, 0444 },
		/* Default MAC secret */
		"0"
	},
	{ { "bt_mac_addr", 16, 1, 0444 },
		/* Default BT MAC address */
		"0"
	},
	{ { "product_name", 32, 1, 0444 },
		/* Product name, acos 2.4 */
		"0"
	},
	{ { "productid", 32, 1, 0444 },
		/* Default Primary Product ID */
		"0"
	},
	{ { "productid2", 32, 1, 0444 },
		/* Default Secondary Product ID */
		"0"
	},
	{ { "bootmode", 4, 1, 0444 },
		/* Default Bootmode */
		"1"
	},
	{ { "postmode", 4, 1, 0444 },
		/* Default Postmode */
		"0"
	},
	{ { "bootcount", 8, 1, 0444 },
		/* Initial Bootcount */
		"0"
	},
	{ { "manufacturing", 512, 1, 0444 },
		/* Manufacturer-specific data */
		""
	},
	{ { "eth_mac_addr", 16, 1, 0444 },
		/* Default MAC address for ethernet */
		"0"
	},
	{ { "eth_ip_addr", 16, 1, 0444 },
		/* Initial default IPAddress */
		"0"
	},
	{ { "device_type_id", 32, 1, 0444 },
		/* Default device type value */
		"0"
	},
	{ { "unlock_code", 1024, 1, 0444 },
		/* Unlock code */
		""
	},
	{ { "t_unlock_code", 512, 1, 0444 },
		/* Temporary unlock signed code, base64 encoded */
		""
	},
	{ { "t_unlock_cert", 1024, 1, 0444 },
		/* Temporary unlock signed certificate, base64 encoded */
		""
	},
	{ { "sensorcal", 160, 1, 0444 },
		/* Sensor Calibration Data */
		"0"
	},
	{ { "wifi_mfg", 512, 1, 0444 },
		/* wifi Calibration Data */
		"0"
	},
	{ { "bt_mfg", 128, 1, 0444 },
		/* bt Calibration Data */
		"0"
	},
	{ { "fos_flags", 8, 1, 0444 },
		/* device specific flag */
		"40"
	},
	{ { "dev_flags", 8, 1, 0444 },
		/* device specific flag */
		"0"
	},
	{ { "usr_flags", 8, 1, 0444 },
		/* device specific flag */
		"0"
	},
	{ { "model_name", 256, 1, 0444 },
		/* device specific flag */
		"tvconfig/model/model_sum.ini"
	},
	{ { "mfr_name", 32, 1, 0444 },
		/* Manufacturer name */
		"0"
	},
	{ { "mfr_model", 32, 1, 0444 },
		/* Manufacturer model */
		"0"
	},
	{ { "product_model", 32, 1, 0444 },
		/* product model */
		"0"
	},
	{ { "transition_done", 4, 1, 0444 },
		/* transition done flag */
		"0"
	},
	{ { "config_name", 64, 1, 0444 },
		/* name for board config.ini file */
		"0"
	},
	{ { "oem_data", 1024, 1, 0444 },
		/* oem specific configuration */
		"0"
	},
	{ { "memc", 4, 1, 0444 },
		/* memc present? */
		"1"
	},
	{ { "region", 32, 1, 0444 },
		/* region flag, default to US */
		"US"
	},
	{ { "res1", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res2", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res3", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res4", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res5", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res6", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res7", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "res8", 64, 1, 0444 },
		/* reserve Parameters */
		""
	},
	{ { "", 0, 0, 0 }, 0 },
};


#endif /* __IDME_DEFAULT_TABLE_T950X4_H__ */
