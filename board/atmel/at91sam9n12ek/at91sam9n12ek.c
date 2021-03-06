/*
 * (C) Copyright 2013 Atmel Corporation
 * Josh Wu <josh.wu@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9x5_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <atmel_hlcdc.h>
#include <atmel_mci.h>
#include <netdev.h>

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */
#ifdef CONFIG_NAND_ATMEL
static void at91sam9n12ek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Assign CS3 to NAND/SmartMedia Interface */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;
	/* Configure databus */
	csa &= ~AT91_MATRIX_NFD0_ON_D16; /* nandflash connect to D0~D15 */
	/* Configure IO drive */
	csa |= AT91_MATRIX_EBI_EBI_IOSR_NORMAL;

	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
		AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(6),
		&smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(7),
		&smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		AT91_SMC_MODE_DBW_8 |
#endif
		AT91_SMC_MODE_TDF_CYCLE(1),
		&smc->cs[3].mode);

	/* Configure RDY/BSY pin */
	at91_set_pio_input(AT91_PIO_PORTD, 5, 1);

	/* Configure ENABLE pin for NandFlash */
	at91_set_pio_output(AT91_PIO_PORTD, 4, 1);

	at91_set_a_periph(AT91_PIO_PORTD, 0, 1);    /* NAND OE */
	at91_set_a_periph(AT91_PIO_PORTD, 1, 1);    /* NAND WE */
	at91_set_a_periph(AT91_PIO_PORTD, 2, 1);    /* ALE */
	at91_set_a_periph(AT91_PIO_PORTD, 3, 1);    /* CLE */
}
#endif

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	.vl_col = 480,
	.vl_row = 272,
	.vl_clk = 9000000,
	.vl_bpix = LCD_BPP,
	.vl_sync = 0,
	.vl_tft = 1,
	.vl_hsync_len = 5,
	.vl_left_margin = 8,
	.vl_right_margin = 43,
	.vl_vsync_len = 10,
	.vl_upper_margin = 4,
	.vl_lower_margin = 12,
	.mmio = ATMEL_BASE_LCDC,
};

void lcd_enable(void)
{
	at91_set_pio_output(AT91_PIO_PORTC, 25, 0);	/* power up */
}

void lcd_disable(void)
{
	at91_set_pio_output(AT91_PIO_PORTC, 25, 1);	/* power down */
}

#ifdef CONFIG_LCD_INFO
void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
	int i;
	char temp[32];

	lcd_printf("%s\n", U_BOOT_VERSION);
	lcd_printf("ATMEL Corp\n");
	lcd_printf("at91@atmel.com\n");
	lcd_printf("%s CPU at %s MHz\n",
		ATMEL_CPU_NAME,
		strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;
	nand_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += nand_info[i].size;
	lcd_printf("  %ld MB SDRAM, %ld MB NAND\n",
		dram_size >> 20,
		nand_size >> 20);
}
#endif /* CONFIG_LCD_INFO */
#endif /* CONFIG_LCD */

/* SPI chip select control */
#ifdef CONFIG_ATMEL_SPI
#include <spi.h>
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs < 2;
}

void spi_cs_activate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 0:
		at91_set_pio_output(AT91_PIO_PORTA, 14, 0);
		break;
	case 1:
		at91_set_pio_output(AT91_PIO_PORTA, 7, 0);
		break;
	}
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 0:
		at91_set_pio_output(AT91_PIO_PORTA, 14, 1);
		break;
	case 1:
		at91_set_pio_output(AT91_PIO_PORTA, 7, 1);
		break;
	}
}
#endif /* CONFIG_ATMEL_SPI */

#ifdef CONFIG_GENERIC_ATMEL_MCI
int board_mmc_init(bd_t *bd)
{
	at91_mci_hw_init();

	return atmel_mci_init((void *)ATMEL_BASE_HSMCI0);
}
#endif

#ifdef CONFIG_KS8851_MLL
void at91sam9n12ek_ks8851_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[2].setup);
	writel(AT91_SMC_PULSE_NWE(7) | AT91_SMC_PULSE_NCS_WR(7) |
	       AT91_SMC_PULSE_NRD(7) | AT91_SMC_PULSE_NCS_RD(7),
	       &smc->cs[2].pulse);
	writel(AT91_SMC_CYCLE_NWE(9) | AT91_SMC_CYCLE_NRD(9),
	       &smc->cs[2].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_BAT | AT91_SMC_MODE_DBW_16 |
	       AT91_SMC_MODE_TDF_CYCLE(1),
	       &smc->cs[2].mode);

	/* Configure NCS2 PIN */
	at91_set_b_periph(AT91_PIO_PORTD, 19, 0);
}
#endif

int board_early_init_f(void)
{
	/* Enable clocks for all PIOs */
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	writel((1 << ATMEL_ID_PIOAB) | (1 << ATMEL_ID_PIOCD), &pmc->pcer);

	at91_seriald_hw_init();
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_NAND_ATMEL
	at91sam9n12ek_nand_hw_init();
#endif

#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 0);
#endif

#ifdef CONFIG_LCD
	at91_lcd_hw_init();
#endif

#ifdef CONFIG_KS8851_MLL
	at91sam9n12ek_ks8851_hw_init();
#endif

	return 0;
}

#ifdef CONFIG_KS8851_MLL
int board_eth_init(bd_t *bis)
{
	return ks8851_mll_initialize(0, CONFIG_KS8851_MLL_BASEADDR);
}
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
					CONFIG_SYS_SDRAM_SIZE);
	return 0;
}
