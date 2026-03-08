/*
 * Copyright 2011-2022 Amazon Technologies, Inc. All Rights Reserved.
 * Portions Copyright (C) 2007-2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _LINUX_METRICSLOG_H
#define _LINUX_METRICSLOG_H

typedef enum {
	VITALS_NORMAL = 0,
	VITALS_FGTRACKING,
	VITALS_TIME_BUCKET,
} vitals_type;

/*
 * Android log priority values, in ascending priority order.
 */
typedef enum android_log_priority {
	ANDROID_LOG_UNKNOWN = 0,
	ANDROID_LOG_DEFAULT,	/* only for SetMinPriority() */
	ANDROID_LOG_VERBOSE,
	ANDROID_LOG_DEBUG,
	ANDROID_LOG_INFO,
	ANDROID_LOG_WARN,
	ANDROID_LOG_ERROR,
	ANDROID_LOG_FATAL,
	ANDROID_LOG_SILENT,	/* only for SetMinPriority(); must be last */
} android_LogPriority;

int log_to_metrics(enum android_log_priority priority, const char *domain, char *logmsg);

int log_counter_to_vitals(enum android_log_priority priority,
		const char *domain, const char *program,
		const char *source, const char *key,
		long counter_value, const char *unit,
		const char *metadata, vitals_type type);

int log_timer_to_vitals(enum android_log_priority priority,
		const char *domain, const char *program,
		const char *source, const char *key,
		long timer_value, const char *unit, vitals_type type);

int log_counter_to_vitals_v2(enum android_log_priority priority,
	const char *group_id, const char *schema_id,
	const char *domain, const char *program,
	const char *source, const char *key,
	long counter_value, const char *unit,
	const char *metadata, vitals_type type,
	const char *dimensions, const char *annotations);

int log_timer_to_vitals_v2(enum android_log_priority priority,
	const char *group_id, const char *schema_id,
	const char *domain, const char *program,
	const char *source, const char *key,
	long timer_value, const char *unit, vitals_type type,
	const char *dimensions, const char *annotations);

#ifdef CONFIG_AMAZON_MINERVA_METRICS_LOG
/* required keys */
#define PREDEFINED_ESSENTIAL_KEY "_deviceType=;SY,_platform=;SY,_marketPlaceId=;SY,_countryOfResidence=;SY,_otaGroupName=;SY,_softwareVersion=;SY,_buildType=;SY,_hardware=;SY"
#define REQUIRED_VITALS_KEY      "_osFileTag=;SY,_deviceId=;SY,_countryOfResidence=;SY,_deviceType=;SY,_marketPlaceId=;SY,_otaGroupName=;SY,_platform=;SY"
/* OS Key */
#define PREDEFINED_OS_KEY "_osFileTag=;SY"
/* Time Zone Key */
#define PREDEFINED_TZ_KEY "_timeZone=;SY"
/* Model Key */
#define PREDEFINED_MODEL_KEY "_model=;SY"
/* Device Id Key */
#define PREDEFINED_DEVICE_ID_KEY "_deviceId=;SY"
/* Customer Id Key */

#define PREDEFINED_DEVICE_LANGUAGE_KEY "_deviceLanguage=;SY"

/* group ids */
#define KERNEL_METRICS_GROUP_ID                      "uf0h909h"

#define KERNEL_METRICS_TEST_SCHEMA_ID                "6mm6/2/02330410"
/* schema name: KernelMetricsTest*/
#define KERNEL_METRICS_TEST_SPACE_SCHEMA_ID          "gggu/2/02330410"
/* schema name: KernelMetricsTestSpace */
#define KERNEL_METRICS_VITALS_COUNTER_TEST_SCHEMA_ID "3af6/2/02330410"
/* schema name: MetricsTestVitalsCounter*/
#define KERNEL_METRICS_VITALS_TIMER_TEST_SCHEMA_ID   "6syu/2/02330410"
/* schema name: MetricsTestVitalsTimer*/
#define KERNEL_METRICS_SCREEN_DRAIN_SCHEMA_ID        "g6c4/2/02330410"
/* schema name: KernelLEDScreenDrain*/
#define KERNEL_METRICS_MSTAR_SPI_COUNTER_SCHEMA_ID   "3af6/2/02330410"
/* schema name: MetricsTestVitalsCounter*/
#define KERNEL_DISPOUT_SCREEN_DRAIN_SCHEMA_ID        "g6c4/2/02330410"
/* schema name: KernelLEDScreenDrain*/

#endif /* CONFIG_AMAZON_MINERVA_METRICS_LOG */
#endif /* _LINUX_METRICSLOG_H */
