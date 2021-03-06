/*
 * Copyright 2008-2015, Marvell International Ltd.
 * All Rights Reserved.
 */

#include <wmstdio.h>
#include <boot_flags.h>
#include <wmlog.h>

#define boot_l(...)	\
	wmlog("boot", ##__VA_ARGS__)

#define REVID_MASK 0x1FF

/* permanent boot flags storage */
uint32_t g_boot_flags;
uint32_t g_prev_version_fw;
uint32_t g_rst_cause;

void boot_init(void)
{
	/* Read boot flags stored by boot2 */
	g_boot_flags = *BOOT_FLAGS;

	if (g_boot_flags & BOOT_MAIN_FIRMWARE_BAD_CRC) {
		g_prev_version_fw = 1;
	}

	g_rst_cause = boot_store_reset_cause();
}

void boot_report_flags(void)
{
	boot_l("SDK-%s gcc-%s (%s %s)", SDK_VERSION,
			__VERSION__, __DATE__, __TIME__);
	boot_l("Boot Flags: 0x%x", g_boot_flags);

	boot_l(" - Partition Table: %d ",
			!(!(g_boot_flags & BOOT_PARTITION_TABLE_MASK)));

	boot_l(" - Firmware Partition: %d ",
		       g_boot_flags & BOOT_PARTITION_MASK);

	if (g_boot_flags & BOOT_MAIN_FIRMWARE_BAD_CRC) {
		boot_l(" - Backup firmware due to CRC error in main"
			" firmware");
	}
	/* Bits of register SYS_CTRL->REV_ID can be described
	 * as follows:
	 *             [31:24]         Company_ID
	 *             [23:20]         Foundry
	 *             [19:16]         Process
	 *             [15:14]         Project_ID
	 *             [8:6]           Project_Rev_first_part
	 *             [5:3]           Project_Rev_second_part
	 *             [2:0]           Project_Rev_third_part
	 */
	boot_l("Boot Info:");
	boot_l(" - Chip revision id: 0x%x",
		((SYS_CTRL->REV_ID.WORDVAL) & (REVID_MASK)));

	boot_l("Reset Cause Register: 0x%x", boot_reset_cause());
	if (boot_reset_cause() & (1<<5))
		boot_l(" - Watchdog reset bit is set");
}

int last_reset_reason(void)
{
	return boot_reset_cause();
}

