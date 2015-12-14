/*
 * arch/sh/boards/mach-hdk7105/setup.c
 *
 * Copyright (C) 2008 STMicroelectronics Limited
 * Author: Stuart Menefy (stuart.menefy@st.com)
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 *
 * STMicroelectronics HDK7105-SDK support.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/leds.h>
#include <linux/lirc.h>
#include <linux/gpio.h>
#include <linux/phy.h>
#include <linux/tm1668.h>
#include <linux/stm/platform.h>
#include <linux/stm/stx7105.h>
#include <linux/stm/emi.h>
#include <linux/stm/nand.h>
#include <linux/stm/nand_devices.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/physmap.h>
#include <linux/stm/pci-glue.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/flash.h>
#include <asm/irq-ilc.h>
#include <linux/bpa2.h>
#include <linux/stm/pio.h>

#define HDK7105_PIO_PHY_RESET stm_gpio(15, 5)
//#define HDK7105_GPIO_FLASH_WP stm_gpio(1, 2)

const char *LMI_IO_partalias[] = { "v4l2-coded-video-buffers", "BPA2_Region1", "v4l2-video-buffers" ,
                                   "coredisplay-video", "gfx-memory", "BPA2_Region0", "LMI_VID", NULL };

/*
0x40000000 - 0x403FFFFF - cocpu 1 ram (4mb)
0x40400000 - 0x407FFFFF - cocpu 2 ram (4mb)
0x40800000 - 0x47FFFFFF - linux   (120mb) 
0x48000000 - 0x49FFFFFF - bigphys ( 32mb)
0x4A000000 - 0x4FFFFFFF - lmi_io  ( 96mb)
*/
static struct bpa2_partition_desc bpa2_parts_table[] = {
    {
 	    .name  = "bigphysarea",
 	    .start = 0x48000000,
 	    .size  = 0x02000000, /* 32 Mb */
 	    .flags = 0,
 	    .aka   = NULL
    }, 
    {
 	    .name  = "LMI_IO",
 	    .start = 0x4A000000,
 	    .size  = 0x06000000, /* 96 Mb */
 	    .flags = 0,
 	    .aka   = LMI_IO_partalias
    }, 
 };

static void __init hdk7105_setup(char **cmdline_p)
{
	printk(KERN_INFO "STMicroelectronics HDK7105 "
			"board initialisation\n");

	#define OR32( a, b ) ( iowrite32( ( ioread32( a ) | b ), a ));
	#define UPDATE32( a, b, c ) ( iowrite32(( ( ioread32( a ) & b ) | c ), a) );
	#define WHILE_NE32( a, b, c ) while( ( ioread32( a ) & b ) != c );
	#define POKE32( a, b ) iowrite32( b, a );


	/* CKGA_PLL0_CFG ->setup pll0 */
	UPDATE32(0xfe213000, 0xfff80000, 0x00001201)	// overclk
	//	UPDATE32(0xfe213000, 0xfff80000, 0x00000f01)	// non overclk
	
	/* CKGA_PLL1_DIV0_CFG */
	  POKE32(0xfe213b00, 0x00000001)
	
	
	/* CKGA_PLL1_DIV1_CFG */
	  POKE32(0xfe213b04, 0x00000001)
	
	
	/* CKGA_PLL1_DIV2_CFG */
	  POKE32(0xfe213b08, 0x00000001)
	
	
	/* CKGA_PLL1_DIV3_CFG */
	  POKE32(0xfe213b0c, 0x0000000f)
	
	
	/* CKGA_PLL0LS_DIV4_CFG */
	  POKE32(0xfe213a10, 0x00000000)
	

	/* CKGA_PLL1_DIV5_CFG */
	  POKE32(0xfe213b14, 0x00000007)
	
	
	/* CKGA_PLL0LS_DIV6_CFG	*/
	  POKE32(0xfe213a18, 0x00000000)
	
	
	/* CKGA_PLL0LS_DIV7_CFG */
	  POKE32(0xfe213a1c, 0x00000000)
	
	
	/* CKGA_PLL1_DIV8_CFG */
	  POKE32(0xfe213b20, 0x00000003)
	
	
	/* CKGA_PLL1_DIV9_CFG */
	  POKE32(0xfe213b24, 0x00000003)
	
	
	/* CKGA_PLL1_DIV10_CFG */
	  POKE32(0xfe213b28, 0x00000003)
	
	
	/* CKGA_PLL1_DIV11_CFG */
	  POKE32(0xfe213b2c, 0x00000003)
	
	/* CKGA_PLL0LS_DIV12_CFG */
	  POKE32(0xfe213a30, 0x00000001)		// overclk
	
	/* CKGA_PLL1_DIV12_CFG */
	 // POKE32(0xfe213b30, 0x00000002)		// non overclk
	
	
	/* CKGA_PLL1_DIV13_CFG */
	  POKE32(0xfe213b34, 0x0000001f)
	
	
	/* CKGA_PLL1_DIV14_CFG */
	  POKE32(0xfe213b38, 0x00000017)
	
	
	/* CKGA_PLL1_DIV15_CFG */
	  POKE32(0xfe213b3c, 0x00000007)
	
	
	/* CKGA_PLL1_DIV16_CFG */
	  POKE32(0xfe213b40, 0x00000003)
	
	
	/* CKGA_PLL1_DIV17_CFG */
	  POKE32(0xfe213b44, 0x00000003)
	
	
	/* CKGA_CLKOPSRC_SWITCH_CFG	*/
		POKE32(0xfe213014, 0xa9aa59aa)	// overclk
	//	 POKE32(0xfe213014, 0xaaaa59aa) // non overclk
	
	/* CKGA_CLKOPSRC_SWITCH_CFG2 */
	  POKE32(0xfe213024, 0x0000000a)

	/* DDR */

    int lmipl_sel_odt_int_del3_0 = 3;
    int lmipl_dqs_valid_offset8_0 = 0;
    int lmipl_sel_dqs_valid_del3_0 = 0;
    int lmipl_dsq0_offset8_0 = 0x1e2;
    int lmipl_dsq1_offset8_0 = 0x1e2;
    int lmipl_dsq2_offset8_0 = 0x1e2;
    int lmipl_dsq3_offset8_0 = 0x1e2;

//	unsigned int sata = readl( 0xFE209000 + 0x824 );
//	int DET = (sata & 0xf );
	
//	if( DET == 3 )
	{
		lmipl_dsq0_offset8_0 = 0x1ee;
		lmipl_dsq1_offset8_0 = 0x1ee;
		lmipl_dsq2_offset8_0 = 0x1ee;
		lmipl_dsq3_offset8_0 = 0x1ee;
	}
	
    unsigned int a = ((lmipl_sel_dqs_valid_del3_0<<27) | (lmipl_dsq2_offset8_0<<18) | (lmipl_dsq1_offset8_0<<9) | lmipl_dsq0_offset8_0);
    unsigned int b = ((lmipl_sel_odt_int_del3_0<<27) | (lmipl_dqs_valid_offset8_0<<18) | lmipl_dsq3_offset8_0);

	POKE32( 0xfe0011a8, a );	
	POKE32( 0xfe0011ac, b );	// 1dc


	stx7105_early_device_init();

	stx7105_configure_asc(2, &(struct stx7105_asc_config) {
			.hw_flow_control = 0,
			.is_console = 1, });

	bpa2_init(bpa2_parts_table, ARRAY_SIZE(bpa2_parts_table));
}

static struct platform_device hdk7105_leds = {
	.name = "leds-gpio",
	.id = 0,
	.dev.platform_data = &(struct gpio_led_platform_data) {
		.num_leds = 5,
		.leds = (struct gpio_led[]) {
			/* The schematics actually describes these PIOs
			 * the other way round, but all tested boards
			 * had the bi-colour LED fitted like below... */
			{
				.name = "CON",
				.gpio = stm_gpio(4, 2),
				.active_low = 0,
			},
			{
				.name = "MOD",
				.gpio = stm_gpio(4, 3),
				.active_low = 0,
			},
			{
				.name = "IR",
				.gpio = stm_gpio(5, 1),
				.active_low = 0,
			},
			{
				.name = "BLUE",
				.gpio = stm_gpio(5, 2),
				.active_low = 0,
			},
			{
				.name = "RED",
				.gpio = stm_gpio(5, 3),
				.active_low = 1,
			},

		},
	},
};

static int hdk7105_phy_reset(void *bus)
{
	gpio_set_value(HDK7105_PIO_PHY_RESET, 0);
	mdelay(100);
	gpio_set_value(HDK7105_PIO_PHY_RESET, 1);

	return 1;
}

static struct stmmac_mdio_bus_data stmmac_mdio_bus = {
	.bus_id = 0,
	.phy_reset = hdk7105_phy_reset,
	.phy_mask = 0,
	.probed_phy_irq = ILC_EXT_IRQ(6),
};


/* NAND Flash */
struct stm_nand_bank_data hdk7105_nand_flash = {
	.csn		= 2,
	.options	= NAND_NO_AUTOINCR | NAND_USE_FLASH_BBT,
	.nr_partitions	= 2,
	.partitions	= (struct mtd_partition []) {
	 	{
		.name	= "uImage",
		.offset	= 0,
		.size 	= 0x00800000 //1M  //uboot boot mode
		},
		{
		.name	= "RootFS",
		.offset	= 0x00800000,
		.size	= MTDPART_SIZ_FULL 
		}
	},
	.timing_data		= &(struct stm_nand_timing_data) {
		.sig_setup	= 50,		/* times in ns */
		.sig_hold	= 50,
		.CE_deassert	= 0,
		.WE_to_RBn	= 100,
		.wr_on		= 10,
		.wr_off		= 40,
		.rd_on		= 10,
		.rd_off		= 40,
		.chip_delay	= 30,		/* in us */
	},
};



static struct platform_device *hdk7105_devices[] __initdata = {
	&hdk7105_leds,
};

static int __init hdk7105_device_init(void)
{
	struct sysconf_field *sc;
	unsigned long nor_bank_base = 0;
	unsigned long nor_bank_size = 0;

	/* Configure Flash according to boot-device */
	sc = sysconf_claim(SYS_STA, 1, 15, 16, "boot_device");
	switch (sysconf_read(sc)) {
	case 0x0:
		/* Boot-from-NOR: */
		pr_info("Configuring FLASH for boot-from-NOR\n");
		/* NOR mapped to EMIA + EMIB (FMI_A26 = EMI_CSA#) */
		nor_bank_base = emi_bank_base(0);
		nor_bank_size = emi_bank_base(2) - nor_bank_base;
		hdk7105_nand_flash.csn = 2;
		break;
	case 0x1:
		/* Boot-from-NAND */
		pr_info("Configuring FLASH for boot-from-NAND\n");
		nor_bank_base = emi_bank_base(1);
		nor_bank_size = emi_bank_base(2) - nor_bank_base;
		hdk7105_nand_flash.csn = 0;
		break;
	case 0x2:
		/* Boot-from-SPI */
		pr_info("Configuring FLASH for boot-from-SPI\n");
		/* NOR mapped to EMIB, with physical offset of 0x06000000! */
		nor_bank_base = emi_bank_base(1);
		nor_bank_size = emi_bank_base(2) - nor_bank_base;
		hdk7105_nand_flash.csn = 2;
		break;
	default:
		BUG();
		break;
	}
	sysconf_release(sc);

	stx7105_configure_sata(0);


	/*
	 * Fix the reset chain so it correct to start with in case the
	 * watchdog expires or we trigger a reset.
	 */
	sc = sysconf_claim(SYS_CFG, 9, 27, 28, "reset_chain");
	sysconf_write(sc, 0);
	/* Release the sysconf bits so the coprocessor driver can claim them */
	sysconf_release(sc);

	/* I2C_xxxA - HDMI */
	stx7105_configure_ssc_i2c(0, &(struct stx7105_ssc_config) {
			.routing.ssc0.sclk = stx7105_ssc0_sclk_pio2_2,
			.routing.ssc0.mtsr = stx7105_ssc0_mtsr_pio2_3, });

	/* I2C_xxxB -  */
	//stx7105_configure_ssc_i2c(1, &(struct stx7105_ssc_config) {
	//		.routing.ssc1.sclk = stx7105_ssc1_sclk_pio2_5,
	//		.routing.ssc1.mtsr = stx7105_ssc1_mtsr_pio2_6, });

	/* I2C_xxxC - JN1 (NIM), JN3, UT1 (CI chip), US2 (EEPROM) */
	stx7105_configure_ssc_i2c(2, &(struct stx7105_ssc_config) {
			.routing.ssc2.sclk = stx7105_ssc2_sclk_pio3_4,
			.routing.ssc2.mtsr = stx7105_ssc2_mtsr_pio3_5, });
	/* I2C_xxxD - JN2 (NIM), JN4 */
	stx7105_configure_ssc_i2c(3, &(struct stx7105_ssc_config) {
			.routing.ssc3.sclk = stx7105_ssc3_sclk_pio3_6,
			.routing.ssc3.mtsr = stx7105_ssc3_mtsr_pio3_7, });

	stx7105_configure_usb(0, &(struct stx7105_usb_config) {
			.ovrcur_mode = stx7105_usb_ovrcur_active_low,
			.pwr_enabled = 1,
			.routing.usb0.ovrcur = stx7105_usb0_ovrcur_pio4_4,
			.routing.usb0.pwr = stx7105_usb0_pwr_pio4_5, });
	stx7105_configure_usb(1, &(struct stx7105_usb_config) {
			.ovrcur_mode = stx7105_usb_ovrcur_active_low,
			.pwr_enabled = 1,
			.routing.usb1.ovrcur = stx7105_usb1_ovrcur_pio4_6,
			.routing.usb1.pwr = stx7105_usb1_pwr_pio4_7, });

	gpio_request(HDK7105_PIO_PHY_RESET, "eth_phy_reset");
	gpio_direction_output(HDK7105_PIO_PHY_RESET, 1);


	/* M.Schenk 2012.05.21 get PHY IRQ to work */
	set_irq_type(ILC_EXT_IRQ(6), IRQ_TYPE_LEVEL_LOW);
	

	stx7105_configure_ethernet(0, &(struct stx7105_ethernet_config) {
			.mode = stx7105_ethernet_mode_mii,
			.mdint_workaround = 0,
			.ext_clk = 0,
			.phy_bus = 0,
			.phy_addr = 0,
			.mdio_bus_data = &stmmac_mdio_bus,
		});

	stx7105_configure_lirc(&(struct stx7105_lirc_config) {
			.rx_mode = stx7105_lirc_rx_mode_ir,
			.tx_enabled = 0,
			.tx_od_enabled = 0, });

	//gpio_request(HDK7105_GPIO_FLASH_WP, "FLASH_WP");
	//gpio_direction_output(HDK7105_GPIO_FLASH_WP, 1);

	stx7105_configure_nand(&(struct stm_nand_config) {
			.driver = stm_nand_flex,
			.nr_banks = 1,
			.banks = &hdk7105_nand_flash,
			.rbn.flex_connected = -1,});

	/* enable optical out */
	stpio_request_pin(10, 6, "opt_out", STPIO_ALT_OUT);

	stx7105_configure_audio(&(struct stx7105_audio_config) {
			.spdif_player_output_enabled = 1, });
 
	return platform_add_devices(hdk7105_devices,
			ARRAY_SIZE(hdk7105_devices));
}
arch_initcall(hdk7105_device_init);

static void __iomem *hdk7105_ioport_map(unsigned long port, unsigned int size)
{
	/*
	 * If we have PCI then this should never be called because we
	 * are using the generic iomap implementation. If we don't
	 * have PCI then there are no IO mapped devices, so it still
	 * shouldn't be called.
	 */
	BUG();
	return (void __iomem *)CCN_PVR;
}

struct sh_machine_vector mv_hdk7105 __initmv = {
	.mv_name		= "hdk7105",
	.mv_setup		= hdk7105_setup,
	.mv_nr_irqs		= NR_IRQS,
	.mv_ioport_map		= hdk7105_ioport_map,
	STM_PCI_IO_MACHINE_VEC
};
