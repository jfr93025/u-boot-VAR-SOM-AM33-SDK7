/*
 * sys_proto.h
 *
 * System information header
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_
#include <linux/mtd/omap_gpmc.h>
#include <asm/arch/cpu.h>

#define BOARD_REV_ID	0x0

u32 get_cpu_rev(void);
u32 get_sysboot_value(void);

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void);
#endif

extern struct ctrl_stat *cstat;
u32 get_device_type(void);
void save_omap_boot_params(void);
void setup_clocks_for_console(void);
void mpu_pll_config_val(int mpull_m);
void ddr_pll_config(unsigned int ddrpll_M);

void sdelay(unsigned long);

struct gpmc_cs;
void gpmc_init(void);
void enable_gpmc_cs_config(const u32 *gpmc_config, struct gpmc_cs *cs, u32 base,
			u32 size);
void omap_nand_switch_ecc(uint32_t, uint32_t);

void set_uart_mux_conf(void);
void set_mux_conf_regs(void);
void sdram_init(void);
u32 wait_on_value(u32, u32, void *, u32);
#ifdef CONFIG_NOR_BOOT
void enable_norboot_pin_mux(void);
#endif
void am33xx_spl_board_init(void);
int am335x_get_efuse_mpu_max_freq(struct ctrl_dev *cdev);
int am335x_get_tps65910_mpu_vdd(int sil_rev, int frequency);
#endif
