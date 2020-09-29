/*
 * MYZR Technology Co.,Ltd
 * http://www.myzr.com.cn
 * Tang Bin <tangb@myzr.com.cn>
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/io.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <linux/sizes.h>
#include <linux/fb.h>
#include <miiphy.h>
#include <mmc.h>
#include <mxsfb.h>
#include <netdev.h>
#include <usb.h>
#include <usb/ehci-fsl.h>
#include <asm/imx-common/video.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
        PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
        PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
        PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW |		\
        PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
        PAD_CTL_SPEED_HIGH   |                                   \
        PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST)

#define MDIO_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
        PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST | PAD_CTL_ODE)

#define ENET_CLK_PAD_CTRL  (PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL    (PAD_CTL_PKE | PAD_CTL_PUE |            \
        PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
        PAD_CTL_DSE_40ohm | PAD_CTL_HYS |			\
        PAD_CTL_ODE)

#define LCD_PAD_CTRL    (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP | PAD_CTL_PUE | \
        PAD_CTL_PKE | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm)

#define OTG_ID_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
        PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |		\
        PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | \
        PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#if defined(CONFIG_SYS_USE_NAND)
#define GPMI_PAD_CTRL0 (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP)
#define GPMI_PAD_CTRL1 (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED | \
        PAD_CTL_SRE_FAST)
#define GPMI_PAD_CTRL2 (GPMI_PAD_CTRL0 | GPMI_PAD_CTRL1)
#endif

/* I2C ********************************************************************** */
#ifdef CONFIG_SYS_I2C_MXC
struct i2c_pads_info i2c_pad_info1 = {
    .scl = {
        .i2c_mode =  MX6_PAD_CSI_MCLK__I2C1_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
        .gpio_mode = MX6_PAD_CSI_MCLK__GPIO4_IO17 | MUX_PAD_CTRL(I2C_PAD_CTRL),
        .gp = IMX_GPIO_NR(4, 7),
    },
    .sda = {
        .i2c_mode = MX6_PAD_CSI_PIXCLK__I2C1_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
        .gpio_mode = MX6_PAD_CSI_PIXCLK__GPIO4_IO18 | MUX_PAD_CTRL(I2C_PAD_CTRL),
        .gp = IMX_GPIO_NR(4, 18),
    },
};
#endif

/* DRAM ********************************************************************* */
int dram_init(void)
{
    gd->ram_size = PHYS_SDRAM_SIZE;

    return 0;
}

/* UART ********************************************************************* */
static iomux_v3_cfg_t const uart1_pads[] = {
    MX6_PAD_UART1_TX_DATA__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
    MX6_PAD_UART1_RX_DATA__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
    imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

/* SPI ********************************************************************** */
static iomux_v3_cfg_t const ecspi1_pads[] = {
    MX6_PAD_CSI_DATA00__ECSPI2_SCLK	| MUX_PAD_CTRL(SPI_PAD_CTRL),
    MX6_PAD_CSI_DATA01__GPIO4_IO22	| MUX_PAD_CTRL(SPI_PAD_CTRL),
    MX6_PAD_CSI_DATA02__ECSPI2_MOSI	| MUX_PAD_CTRL(SPI_PAD_CTRL),
    MX6_PAD_CSI_DATA03__ECSPI2_MISO	| MUX_PAD_CTRL(SPI_PAD_CTRL),
};

static iomux_v3_cfg_t const ecspi2_pads[] = {
    MX6_PAD_CSI_DATA04__ECSPI1_SCLK	| MUX_PAD_CTRL(SPI_PAD_CTRL),
    MX6_PAD_CSI_DATA05__GPIO4_IO26	| MUX_PAD_CTRL(SPI_PAD_CTRL),
    MX6_PAD_CSI_DATA06__ECSPI1_MOSI	| MUX_PAD_CTRL(SPI_PAD_CTRL),
    MX6_PAD_CSI_DATA07__ECSPI1_MISO	| MUX_PAD_CTRL(SPI_PAD_CTRL),
};

static void setup_iomux_spi(void)
{
    imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));
    imx_iomux_v3_setup_multiple_pads(ecspi2_pads, ARRAY_SIZE(ecspi2_pads));
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
    if (bus == 0 && cs == 0)
        return IMX_GPIO_NR(4, 22);
    else if (bus == 1 && cs == 0)
        return IMX_GPIO_NR(4, 26);
    else
        return -1;
}

/* uSDHC ******************************************************************** */
static iomux_v3_cfg_t const usdhc1_pads[] = {
    MX6_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

#if defined(CONFIG_SYS_BOOT_NAND)
static iomux_v3_cfg_t const nand_pads[] = {
    MX6_PAD_NAND_DATA00__RAWNAND_DATA00 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_DATA01__RAWNAND_DATA01 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_DATA02__RAWNAND_DATA02 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_DATA03__RAWNAND_DATA03 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_DATA04__RAWNAND_DATA04 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_DATA05__RAWNAND_DATA05 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_DATA06__RAWNAND_DATA06 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_DATA07__RAWNAND_DATA07 | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_CLE__RAWNAND_CLE | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_ALE__RAWNAND_ALE | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_CE0_B__RAWNAND_CE0_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_CE1_B__RAWNAND_CE1_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_RE_B__RAWNAND_RE_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_WE_B__RAWNAND_WE_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_WP_B__RAWNAND_WP_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_READY_B__RAWNAND_READY_B | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
    MX6_PAD_NAND_DQS__RAWNAND_DQS | MUX_PAD_CTRL(GPMI_PAD_CTRL2),
};

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
    {USDHC1_BASE_ADDR, 0, 4},
    {},
};

static void setup_gpmi_nand(void)
{
    struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

    /* config gpmi nand iomux */
    imx_iomux_v3_setup_multiple_pads(nand_pads, ARRAY_SIZE(nand_pads));

    setup_gpmi_io_clk((3 << MXC_CCM_CSCDR1_BCH_PODF_OFFSET) |
            (3 << MXC_CCM_CSCDR1_GPMI_PODF_OFFSET));

    /* enable apbh clock gating */
    setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#else
static iomux_v3_cfg_t const usdhc2_emmc_pads[] = {
    MX6_PAD_NAND_RE_B__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NAND_WE_B__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NAND_DATA00__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NAND_DATA01__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NAND_DATA02__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NAND_DATA03__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NAND_DATA04__USDHC2_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NAND_DATA05__USDHC2_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NAND_DATA06__USDHC2_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NAND_DATA07__USDHC2_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

    /* RST_B */
    MX6_PAD_NAND_ALE__GPIO4_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
    {USDHC1_BASE_ADDR, 0, 4},
    {USDHC2_BASE_ADDR, 0, 8},
};
#endif

int board_mmc_get_env_dev(int dev_no)
{
    return dev_no;
}

int mmc_map_to_kernel_blk(int dev_no)
{
    return dev_no;
}

int board_mmc_getcd(struct mmc *mmc)
{
    struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
    int ret = 0;

    switch (cfg->esdhc_base) {
        case USDHC1_BASE_ADDR:
            ret = 1;
            break;
        case USDHC2_BASE_ADDR:
            ret = 1;
            break;
    }

    return ret;
}

int board_mmc_init(bd_t *bis)
{
    int i, ret;

    for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
        switch (i) {
            case 0:
                imx_iomux_v3_setup_multiple_pads(
                        usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
                usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
                break;
            case 1:
#if !defined(CONFIG_SYS_BOOT_NAND)
                imx_iomux_v3_setup_multiple_pads(
                        usdhc2_emmc_pads, ARRAY_SIZE(usdhc2_emmc_pads));
                usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
                break;
#endif
            default:
                printf("Warning: you configured more USDHC controllers"
                        "(%d) than supported by the board\n", i + 1);
                return -EINVAL;
        }

        ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
        if (ret) {
            printf("Warning: failed to initialize mmc dev %d\n", i);
            return ret;
        }
    }

    return 0;
}

/* ENET ********************************************************************* */
#define ENET1_PHY_RST	IMX_GPIO_NR(5, 9)
static iomux_v3_cfg_t const fec1_pads[] = {
    MX6_PAD_GPIO1_IO06__ENET1_MDIO	| MUX_PAD_CTRL(MDIO_PAD_CTRL),
    MX6_PAD_GPIO1_IO07__ENET1_MDC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_ENET1_TX_DATA0__ENET1_TDATA00	| MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_ENET1_TX_DATA1__ENET1_TDATA01	| MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_ENET1_TX_EN__ENET1_TX_EN		| MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_ENET1_TX_CLK__ENET1_REF_CLK1	| MUX_PAD_CTRL(ENET_CLK_PAD_CTRL),
    MX6_PAD_ENET1_RX_DATA0__ENET1_RDATA00	| MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_ENET1_RX_DATA1__ENET1_RDATA01	| MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_ENET1_RX_ER__ENET1_RX_ER	| MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_ENET1_RX_EN__ENET1_RX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL),
    /* LAN8720 RST */
    MX6_PAD_SNVS_TAMPER9__GPIO5_IO09	| MUX_PAD_CTRL(NO_PAD_CTRL),
    /* LAN8720 INT */
    MX6_PAD_UART1_CTS_B__GPIO1_IO18		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_fec(int fec_id)
{
    if (fec_id == 0)
    {
        imx_iomux_v3_setup_multiple_pads(fec1_pads, ARRAY_SIZE(fec1_pads));
        gpio_direction_output(ENET1_PHY_RST, 0);
        udelay(50);
        gpio_direction_output(ENET1_PHY_RST, 1);
    }
}

int board_eth_init(bd_t *bis)
{
    int ret;

    setup_iomux_fec(CONFIG_FEC_ENET_DEV);

    ret = fecmxc_initialize_multi(bis, -1,
            CONFIG_FEC_MXC_PHYADDR, IMX_FEC_BASE);
    if (ret)
        printf("FEC%d MXC: %s:failed\n", CONFIG_FEC_ENET_DEV, __func__);

    return 0;
}

static int setup_fec(int fec_id)
{
    struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
        = (struct iomuxc_gpr_base_regs *) IOMUXC_GPR_BASE_ADDR;
    int ret;

    if (0 == fec_id) {
        if (check_module_fused(MX6_MODULE_ENET1))
            return -1;

        /* Use 50M anatop loopback REF_CLK1 for ENET1, clear gpr1[13], set gpr1[17]*/
        clrsetbits_le32(&iomuxc_gpr_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
                IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);
    } else {
        if (check_module_fused(MX6_MODULE_ENET2))
            return -1;

        /* Use 50M anatop loopback REF_CLK2 for ENET2, clear gpr1[14], set gpr1[18]. */
        clrsetbits_le32(&iomuxc_gpr_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
                IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);
    }

    ret = enable_fec_anatop_clock(fec_id, ENET_50MHZ);
    if (ret)
        return ret;

    enable_enet_clk(1);

    return 0;
}

int board_phy_config(struct phy_device *phydev)
{
    if (phydev->drv->config)
        phydev->drv->config(phydev);

    return 0;
}

/* VIDEO ******************************************************************** */
#ifdef CONFIG_VIDEO_MXS
static iomux_v3_cfg_t const lcd_pads[] = {
    MX6_PAD_LCD_CLK__LCDIF_CLK | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_ENABLE__LCDIF_ENABLE | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_HSYNC__LCDIF_HSYNC | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_VSYNC__LCDIF_VSYNC | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA00__LCDIF_DATA00 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA01__LCDIF_DATA01 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA02__LCDIF_DATA02 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA03__LCDIF_DATA03 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA04__LCDIF_DATA04 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA05__LCDIF_DATA05 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA06__LCDIF_DATA06 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA07__LCDIF_DATA07 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA08__LCDIF_DATA08 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA09__LCDIF_DATA09 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA10__LCDIF_DATA10 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA11__LCDIF_DATA11 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA12__LCDIF_DATA12 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA13__LCDIF_DATA13 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA14__LCDIF_DATA14 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA15__LCDIF_DATA15 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA16__LCDIF_DATA16 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA17__LCDIF_DATA17 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA18__LCDIF_DATA18 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA19__LCDIF_DATA19 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA20__LCDIF_DATA20 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA21__LCDIF_DATA21 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA22__LCDIF_DATA22 | MUX_PAD_CTRL(LCD_PAD_CTRL),
    MX6_PAD_LCD_DATA23__LCDIF_DATA23 | MUX_PAD_CTRL(LCD_PAD_CTRL),

    /* Use GPIO for Brightness adjustment, duty cycle = period. */
    MX6_PAD_GPIO1_IO05__GPIO1_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

struct lcd_panel_info_t {
    unsigned int lcdif_base_addr;
    int depth;
    void (*enable)(struct lcd_panel_info_t const *dev);
    struct fb_videomode mode;
};

void do_enable_parallel_lcd(struct display_info_t const *dev)
{
    enable_lcdif_clock(dev->bus);

    imx_iomux_v3_setup_multiple_pads(lcd_pads, ARRAY_SIZE(lcd_pads));

    /* Set Brightness to high */
    gpio_direction_output(IMX_GPIO_NR(1, 5) , 1);
}

struct display_info_t const displays[] = {{
#if defined(CONFIG_MYIMX6G)
    .bus = MX6UL_LCDIF1_BASE_ADDR,
#else
    .bus = MX6ULL_LCDIF1_BASE_ADDR,
#endif
    .addr = 0,
    .pixfmt = 24,
    .detect = NULL,
    .enable	= do_enable_parallel_lcd,
    .mode	= {
        .name			= "MY-WVGA",
        .xres           = 800,
        .yres           = 480,
        .pixclock       = 29850,
        .left_margin    = 89,
        .right_margin   = 164,
        .upper_margin   = 23,
        .lower_margin   = 10,
        .hsync_len      = 10,
        .vsync_len      = 10,
        .sync           = 0,
        .vmode          = FB_VMODE_NONINTERLACED
    } }, {
#if defined(CONFIG_MYIMX6G)
    .bus = MX6UL_LCDIF1_BASE_ADDR,
#else
    .bus = MX6ULL_LCDIF1_BASE_ADDR,
#endif
    .addr = 0,
    .pixfmt = 24,
    .detect = NULL,
    .enable	= do_enable_parallel_lcd,
    .mode	= {
        .name			= "MY-HVGA",
        .xres           = 480,
        .yres           = 272,
        .pixclock       = 108695,
        .left_margin    = 8,
        .right_margin   = 4,
        .upper_margin   = 2,
        .lower_margin   = 4,
        .hsync_len      = 41,
        .vsync_len      = 10,
        .sync           = 0,
        .vmode          = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);
#endif

/* USB ********************************************************************** */
#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		(1 << 9)

static iomux_v3_cfg_t const usb_otg_pads[] = {
    MX6_PAD_GPIO1_IO00__ANATOP_OTG1_ID | MUX_PAD_CTRL(OTG_ID_PAD_CTRL),
};

static void setup_usb(void)
{
    imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
            ARRAY_SIZE(usb_otg_pads));
}

int board_usb_phy_mode(int port)
{
    if (port == 1)
        return USB_INIT_HOST;
    else
        return usb_phy_mode(port);
}

int board_ehci_hcd_init(int port)
{
    u32 *usbnc_usb_ctrl;

    if (port > 1)
        return -EINVAL;

    usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
            port * 4);

    /* Set Power polarity */
    setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

    return 0;
}

/* Interface **************************************************************** */
int checkboard(void)
{
#if defined(CONFIG_TARGET_MYIMX6EK140)
    puts("Board: MYIMX6EK140");
#elif defined(CONFIG_TARGET_MYIMX6EK140P)
    puts("Board: MYIMX6EK140P");
#endif

#if defined(CONFIG_MYIMX6G)
    puts("-6G");
#elif defined(CONFIG_MYIMX6Y)
    puts("-6Y");
#endif

#if (CONFIG_DRAM_SIZE == 256)
    puts("-256M");
#elif (CONFIG_DRAM_SIZE == 128)
    puts("-128M");
#elif (CONFIG_DRAM_SIZE == 512)
    puts("-512M");
#endif

    puts("\n");

    return 0;
}

int board_early_init_f(void)
{
    setup_iomux_uart();

    return 0;
}

int board_init(void)
{
    /* Address of boot parameters */
    gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

    setup_iomux_spi();

    setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

    setup_fec(CONFIG_FEC_ENET_DEV);

    setup_usb();

#if defined(CONFIG_SYS_USE_NAND)
    setup_gpmi_nand();
#endif

    return 0;
}

int board_late_init(void)
{
    //board_late_mmc_env_init();

    set_wdog_reset((struct wdog_regs *)WDOG1_BASE_ADDR);

    return 0;
}

u32 get_board_rev(void)
{
    return get_cpu_rev();
}
