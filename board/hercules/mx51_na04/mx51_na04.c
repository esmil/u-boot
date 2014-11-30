/*
 * (C) Copyright 2014 Emil Renner Berthing <u-boot@esmil.dk>
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx51.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mc13892.h>
#include <usb/ehci-fsl.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_DSE_HIGH)

static const iomux_v3_cfg_t mx51_na04_pads[] = {
	/* LCD */
#define MX51_NA04_LCD_3V3 IMX_GPIO_NR(4, 9)
	MX51_PAD_CSI2_D12__GPIO4_9,
#define MX51_NA04_LVDS_POWER IMX_GPIO_NR(3, 3)
	MX51_PAD_DI1_D0_CS__GPIO3_3,
#define MX51_NA04_BACKLIGHT_CTL IMX_GPIO_NR(3, 4)
	MX51_PAD_DI1_D1_CS__GPIO3_4,
#define MX51_NA04_BACKLIGHT_PWM IMX_GPIO_NR(1, 2)
	MX51_PAD_GPIO1_2__GPIO1_2,

	/* UART 1 */
	MX51_PAD_UART1_RXD__UART1_RXD,
	MX51_PAD_UART1_TXD__UART1_TXD,
	NEW_PAD_CTRL(MX51_PAD_UART1_RTS__UART1_RTS, UART_PAD_CTRL),
	NEW_PAD_CTRL(MX51_PAD_UART1_CTS__UART1_CTS, UART_PAD_CTRL),

#ifdef CONFIG_FEC_MXC
	/* FEC Ethernet */
	NEW_PAD_CTRL(MX51_PAD_EIM_EB2__FEC_MDIO, PAD_CTL_HYS |
		     PAD_CTL_PUS_22K_UP | PAD_CTL_ODE |
		     PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
	MX51_PAD_NANDF_CS3__FEC_MDC,
	NEW_PAD_CTRL(MX51_PAD_EIM_CS3__FEC_RDATA3,
		     MX51_PAD_CTRL_2),
	NEW_PAD_CTRL(MX51_PAD_DI_GP4__FEC_RDATA2,
		     MX51_PAD_CTRL_2),
	NEW_PAD_CTRL(MX51_PAD_DI2_DISP_CLK__FEC_RDATA1,
		     MX51_PAD_CTRL_2),
	NEW_PAD_CTRL(MX51_PAD_DISP2_DAT14__FEC_RDATA0,
		     MX51_PAD_CTRL_2),
	MX51_PAD_NANDF_CS6__FEC_TDATA3,
	MX51_PAD_NANDF_CS5__FEC_TDATA2,
	MX51_PAD_NANDF_CS4__FEC_TDATA1,
	MX51_PAD_DISP2_DAT15__FEC_TDATA0,
	MX51_PAD_DISP2_DAT9__FEC_TX_EN,
	MX51_PAD_NANDF_CS2__FEC_TX_ER,
	MX51_PAD_DISP2_DAT13__FEC_TX_CLK,
	NEW_PAD_CTRL(MX51_PAD_NANDF_RB2__FEC_COL,
		     MX51_PAD_CTRL_4),
	NEW_PAD_CTRL(MX51_PAD_NANDF_RB3__FEC_RX_CLK,
		     MX51_PAD_CTRL_4),
	MX51_PAD_EIM_CS5__FEC_CRS,
	MX51_PAD_EIM_CS4__FEC_RX_ER,
	NEW_PAD_CTRL(MX51_PAD_NANDF_D11__FEC_RX_DV,
		     MX51_PAD_CTRL_4),
#define MX51_NA04_PHY_RESET IMX_GPIO_NR(2, 14)
	NEW_PAD_CTRL(MX51_PAD_EIM_A20__GPIO2_14, PAD_CTL_PUE),
#endif

#ifdef CONFIG_MXC_SPI
	/* SPI 1 */
	NEW_PAD_CTRL(MX51_PAD_CSPI1_MOSI__ECSPI1_MOSI, PAD_CTL_HYS |
		     PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
	NEW_PAD_CTRL(MX51_PAD_CSPI1_MISO__ECSPI1_MISO, PAD_CTL_HYS |
		     PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
	NEW_PAD_CTRL(MX51_PAD_CSPI1_SS1__ECSPI1_SS1,
		     MX51_GPIO_PAD_CTRL),
	MX51_PAD_CSPI1_SS0__ECSPI1_SS0,
	NEW_PAD_CTRL(MX51_PAD_CSPI1_RDY__ECSPI1_RDY, MX51_PAD_CTRL_2),
	NEW_PAD_CTRL(MX51_PAD_CSPI1_SCLK__ECSPI1_SCLK, PAD_CTL_HYS |
		     PAD_CTL_DSE_HIGH | PAD_CTL_SRE_FAST),
#endif

	/* USB Host 1 */
	MX51_PAD_USBH1_CLK__USBH1_CLK,
	MX51_PAD_USBH1_DIR__USBH1_DIR,
	MX51_PAD_USBH1_STP__USBH1_STP,
	MX51_PAD_USBH1_NXT__USBH1_NXT,
	MX51_PAD_USBH1_DATA0__USBH1_DATA0,
	MX51_PAD_USBH1_DATA1__USBH1_DATA1,
	MX51_PAD_USBH1_DATA2__USBH1_DATA2,
	MX51_PAD_USBH1_DATA3__USBH1_DATA3,
	MX51_PAD_USBH1_DATA4__USBH1_DATA4,
	MX51_PAD_USBH1_DATA5__USBH1_DATA5,
	MX51_PAD_USBH1_DATA6__USBH1_DATA6,
	MX51_PAD_USBH1_DATA7__USBH1_DATA7,
#define MX51_NA04_USB_PHY_RESET IMX_GPIO_NR(2, 5)
	MX51_PAD_EIM_D21__GPIO2_5,
#define MX51_NA04_USB_CLK_EN_B IMX_GPIO_NR(2, 27)
	MX51_PAD_EIM_CS2__GPIO2_27,
#define MX51_NA04_USBH1_HUB_RST IMX_GPIO_NR(1, 6)
	NEW_PAD_CTRL(MX51_PAD_GPIO1_6__GPIO1_6, 0),

	/* Power Key */
#define MX51_NA04_POWER_KEY IMX_GPIO_NR(2, 21)
	NEW_PAD_CTRL(MX51_PAD_EIM_A27__GPIO2_21, PAD_CTL_HYS),

	/* Battery LED */
#define MX51_NA04_BATTERY_LED IMX_GPIO_NR(3, 14)
	NEW_PAD_CTRL(MX51_PAD_CSI1_VSYNC__GPIO3_14,
		     PAD_CTL_SRE_FAST),
};

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

u32 get_board_rev(void)
{
	u32 rev = get_cpu_rev();
	if (!gpio_get_value(IMX_GPIO_NR(1, 22)))
		rev |= BOARD_REV_2_0 << BOARD_VER_OFFSET;
	return rev;
}

static void init_lcd(void)
{
	/* Blank the LCD */
	gpio_direction_output(MX51_NA04_LVDS_POWER, 0);
	gpio_direction_output(MX51_NA04_BACKLIGHT_CTL, 0);
	gpio_direction_output(MX51_NA04_BACKLIGHT_PWM, 0);

	/* ..but keep the power on to avoid a nasty
	 * white flash when Linux takes over */
	gpio_direction_output(MX51_NA04_LCD_3V3, 1);
}

static void reset_usb(void)
{
	/* Turn off USB_CLK_EN_B line */
	gpio_direction_output(MX51_NA04_USB_CLK_EN_B, 1);

	/* Assert USB hub reset */
	gpio_direction_output(MX51_NA04_USBH1_HUB_RST, 0);

	/* Assert USB phy reset */
	gpio_direction_output(MX51_NA04_USB_PHY_RESET, 0);
}

#ifdef CONFIG_USB_EHCI_MX5
int board_ehci_hcd_init(int port)
{
	/* De-assert USB phy reset */
	gpio_set_value(MX51_NA04_USB_PHY_RESET, 1);

	/* Drive USB_CLK_EN_B line low */
	gpio_direction_output(MX51_NA04_USB_CLK_EN_B, 0);

	mdelay(2);

	/* De-assert USB usb reset */
	gpio_set_value(MX51_NA04_USBH1_HUB_RST, 1);
	return 0;
}
#endif

#ifdef CONFIG_MXC_SPI
static void power_init(void)
{
	unsigned int val;
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)MXC_CCM_BASE;
	struct pmic *p;
	int ret;

	ret = pmic_init(CONFIG_FSL_PMIC_BUS);
	if (ret)
		return;

	p = pmic_get("FSL_PMIC");
	if (!p)
		return;

	/* Write needed to Power Gate 2 register */
	pmic_reg_read(p, REG_POWER_MISC, &val);
	val &= ~PWGT2SPIEN;
	pmic_reg_write(p, REG_POWER_MISC, val);

	/* Externally powered */
	pmic_reg_read(p, REG_CHARGE, &val);
	val |= ICHRG0 | ICHRG1 | ICHRG2 | ICHRG3 | CHGAUTOB;
	pmic_reg_write(p, REG_CHARGE, val);

	/* power up the system first */
	pmic_reg_write(p, REG_POWER_MISC, PWUP);

	/* Set core voltage to 1.1V */
	pmic_reg_read(p, REG_SW_0, &val);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_100V;
	pmic_reg_write(p, REG_SW_0, val);

	/* Setup VCC (SW2) to 1.25 */
	pmic_reg_read(p, REG_SW_1, &val);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_250V;
	pmic_reg_write(p, REG_SW_1, val);

	/* Setup 1V2_DIG1 (SW3) to 1.25 */
	pmic_reg_read(p, REG_SW_2, &val);
	val = (val & ~SWx_VOLT_MASK) | SWx_1_250V;
	pmic_reg_write(p, REG_SW_2, val);
	udelay(50);

	/* Raise the core frequency to 800MHz */
	writel(0x0, &mxc_ccm->cacrr);

	/* Set switchers in Auto in NORMAL mode & STANDBY mode */
	/* Setup the switcher mode for SW1 & SW2*/
	pmic_reg_read(p, REG_SW_4, &val);
	val = (val & ~((SWMODE_MASK << SWMODE1_SHIFT) |
		(SWMODE_MASK << SWMODE2_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE1_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE2_SHIFT);
	pmic_reg_write(p, REG_SW_4, val);

	/* Setup the switcher mode for SW3 & SW4 */
	pmic_reg_read(p, REG_SW_5, &val);
	val = (val & ~((SWMODE_MASK << SWMODE3_SHIFT) |
		(SWMODE_MASK << SWMODE4_SHIFT)));
	val |= (SWMODE_AUTO_AUTO << SWMODE3_SHIFT) |
		(SWMODE_AUTO_AUTO << SWMODE4_SHIFT);
	pmic_reg_write(p, REG_SW_5, val);

	/* Set VDIG to 1.65V, VGEN3 to 1.8V, VCAM to 2.6V */
	pmic_reg_read(p, REG_SETTING_0, &val);
	val &= ~(VCAM_MASK | VGEN3_MASK | VDIG_MASK);
	val |= VDIG_1_65 | VGEN3_1_8 | VCAM_2_6;
	pmic_reg_write(p, REG_SETTING_0, val);

	/* Set VVIDEO to 2.775V, VAUDIO to 3V, VSD to 3.15V */
	pmic_reg_read(p, REG_SETTING_1, &val);
	val &= ~(VVIDEO_MASK | VSD_MASK | VAUDIO_MASK);
	val |= VSD_3_15 | VAUDIO_3_0 | VVIDEO_2_775;
	pmic_reg_write(p, REG_SETTING_1, val);

	/* Configure VGEN3 and VCAM regulators to use external PNP */
	val = VGEN3CONFIG | VCAMCONFIG;
	pmic_reg_write(p, REG_MODE_1, val);
	udelay(200);

	/* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
	val = VGEN3EN | VGEN3CONFIG | VCAMEN | VCAMCONFIG |
		VVIDEOEN | VAUDIOEN  | VSDEN;
	pmic_reg_write(p, REG_MODE_1, val);
}
#else
static inline void power_init(void) {}
#endif

#ifdef CONFIG_FEC_MXC
static void reset_fec(void)
{
	gpio_direction_output(MX51_NA04_PHY_RESET, 0);
	udelay(500);
	gpio_set_value(MX51_NA04_PHY_RESET, 1);
}
#else
static inline void reset_fec(void) {}
#endif

#ifdef CONFIG_FSL_ESDHC
int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_SD1_CMD__SD1_CMD,
			     PAD_CTL_DSE_MAX | PAD_CTL_HYS |
			     PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_CLK__SD1_CLK,
			     PAD_CTL_DSE_MAX |
			     PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA0__SD1_DATA0,
			     PAD_CTL_DSE_MAX | PAD_CTL_HYS |
			     PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA1__SD1_DATA1,
			     PAD_CTL_DSE_MAX | PAD_CTL_HYS |
			     PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA2__SD1_DATA2,
			     PAD_CTL_DSE_MAX | PAD_CTL_HYS |
			     PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD1_DATA3__SD1_DATA3,
			     PAD_CTL_DSE_MAX | PAD_CTL_HYS |
			     PAD_CTL_PUS_100K_DOWN | PAD_CTL_SRE_FAST),
#define MX51_NA04_SDHC1_CD IMX_GPIO_NR(1, 0)
		NEW_PAD_CTRL(MX51_PAD_GPIO1_0__GPIO1_0, PAD_CTL_HYS),
#define MX51_NA04_SDHC1_WP IMX_GPIO_NR(1, 1)
		NEW_PAD_CTRL(MX51_PAD_GPIO1_1__GPIO1_1, PAD_CTL_HYS),
	};

	static const iomux_v3_cfg_t sd2_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_SD2_CMD__SD2_CMD,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_CLK__SD2_CLK,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_DATA0__SD2_DATA0,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_DATA1__SD2_DATA1,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_DATA2__SD2_DATA2,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_SD2_DATA3__SD2_DATA3,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
#define MX51_NA04_SDHC2_CD IMX_GPIO_NR(1, 7)
		NEW_PAD_CTRL(MX51_PAD_GPIO1_7__GPIO1_7, PAD_CTL_HYS),
#define MX51_NA04_SDHC2_WP IMX_GPIO_NR(1, 5)
		NEW_PAD_CTRL(MX51_PAD_GPIO1_5__GPIO1_5, PAD_CTL_HYS),
	};

	static const iomux_v3_cfg_t sd3_pads[] = {
		NEW_PAD_CTRL(MX51_PAD_NANDF_RDY_INT__SD3_CMD,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_NANDF_CS7__SD3_CLK,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_NANDF_D8__SD3_DATA0,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_NANDF_D9__SD3_DATA1,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_NANDF_D10__SD3_DATA2,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_NANDF_RB0__SD3_DATA3,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_NANDF_D12__SD3_DAT4,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_NANDF_D13__SD3_DAT5,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_NANDF_D14__SD3_DAT6,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
		NEW_PAD_CTRL(MX51_PAD_NANDF_D15__SD3_DAT7,
			     PAD_CTL_DSE_MAX | PAD_CTL_SRE_FAST),
	};

	static struct fsl_esdhc_cfg esdhc_cfg[] = {
		{
			.esdhc_base = MMC_SDHC1_BASE_ADDR,
			.max_bus_width = 4,
		},
		{
			.esdhc_base = MMC_SDHC2_BASE_ADDR,
			.max_bus_width = 4,
		},
		{
			.esdhc_base = MMC_SDHC3_BASE_ADDR,
			.max_bus_width = 8,
		},
	};
	u32 index;
	int ret;

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM; index++) {
		switch (index) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(sd1_pads,
							 ARRAY_SIZE(sd1_pads));
			gpio_direction_input(MX51_NA04_SDHC1_CD);
			gpio_direction_input(MX51_NA04_SDHC1_WP);
			esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(sd2_pads,
							 ARRAY_SIZE(sd2_pads));
			gpio_direction_input(MX51_NA04_SDHC2_CD);
			gpio_direction_input(MX51_NA04_SDHC2_WP);
			esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		case 2:
			imx_iomux_v3_setup_multiple_pads(sd3_pads,
							 ARRAY_SIZE(sd3_pads));
			esdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		default:
			printf("Warning: you configured more ESDHC controllers(%d) than supported by the board(3)\n",
			       CONFIG_SYS_FSL_ESDHC_NUM);
			return -EINVAL;
		}
		ret = fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
		if (ret)
			return ret;
	}
	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = mmc->priv;

	switch (cfg->esdhc_base) {
	case MMC_SDHC1_BASE_ADDR:
		return !gpio_get_value(MX51_NA04_SDHC1_CD);
	case MMC_SDHC2_BASE_ADDR:
		return !gpio_get_value(MX51_NA04_SDHC2_CD);
	case MMC_SDHC3_BASE_ADDR:
		return 1;
	}

	return 0;
}

int board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = mmc->priv;

	switch (cfg->esdhc_base) {
	case MMC_SDHC1_BASE_ADDR:
		return !!gpio_get_value(MX51_NA04_SDHC1_WP);
	case MMC_SDHC2_BASE_ADDR:
		return !!gpio_get_value(MX51_NA04_SDHC2_WP);
	case MMC_SDHC3_BASE_ADDR:
		return 0;
	}

	return 0;
}
#endif

int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(mx51_na04_pads,
					 ARRAY_SIZE(mx51_na04_pads));
	init_lcd();

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	gpio_direction_input(MX51_NA04_POWER_KEY);
	gpio_direction_output(MX51_NA04_BATTERY_LED, 0);
	reset_usb();

	return 0;
}

int board_late_init(void)
{
	power_init();
	reset_fec();

	return 0;
}

int checkboard(void)
{
	puts("Board: Hercules eCAFE Slim/EX HD\n");

	return 0;
}
