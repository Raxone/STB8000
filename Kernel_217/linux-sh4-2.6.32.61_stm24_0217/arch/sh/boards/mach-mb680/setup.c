/*
 * arch/sh/boards/st/mb680/setup.c
 *
 * Copyright (C) 2008 STMicroelectronics Limited
 * Author: Stuart Menefy (stuart.menefy@st.com)
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 *
 * STMicroelectronics STx7105 Mboard support.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/phy.h>
#include <linux/leds.h>
#include <linux/lirc.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/stm/platform.h>
#include <linux/stm/stx7105.h>
#include <linux/stm/pci-glue.h> //pci_synopsys is now pci-glue, was renamed
#include <linux/mtd/mtd.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/partitions.h>
#include <linux/bpa2.h>
#include <asm/irq-ilc.h>
#include <mach/common.h>
#include "../mach-st/mb705-epld.h"
#include <linux/stm/pio.h> //j00zek spdif enable need it


#define PIO13_2 stm_gpio(13, 2) //reset usb1
#define PIO13_3 stm_gpio(13, 3) //reset usb2
#define PIO16_2 stm_gpio(16, 2) //reset dvb-t
#define PIO15_3 stm_gpio(15, 3) //reset dvb-s

#define MB680_PIO_PHY_RESET stm_gpio(6, 6)
//#define MB680_PIO_PCI_SERR stm_gpio(6, 4)
//#define MB680_PIO_PCI_RESET stm_gpio(15, 6)
//#define MB680_PIO_MII_BUS_SWITCH stm_gpio(11, 2)

const char *LMI_IO_partalias[] = { "audio", "v4l2-coded-video-buffers", "BPA2_Region1", "v4l2-video-buffers" ,
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
 	    .size  = 0x06000000, /* 92 Mb */
 	    .flags = 0,
 	    .aka   = LMI_IO_partalias
    }
 };


static void __init mb680_setup(char** cmdline_p)
{
	printk(KERN_INFO "STMicroelectronics STx7105 Mboard initialisation\n");

	stx7105_early_device_init();

	stx7105_configure_asc(2, &(struct stx7105_asc_config) {
			.hw_flow_control = 0,
			.is_console = 1, });
	bpa2_init(bpa2_parts_table, ARRAY_SIZE(bpa2_parts_table));

}

static int mb680_phy_reset(void *bus)
{
	gpio_set_value(MB680_PIO_PHY_RESET, 0);
	udelay(150);
	gpio_set_value(MB680_PIO_PHY_RESET, 1);

	return 0;
}

static struct stmmac_mdio_bus_data stmmac_mdio_bus = {
	.bus_id = 0,
	.phy_reset = mb680_phy_reset,
	.phy_mask = 0,
};

// Nor Flash Configuration SagemCom UHD88/ESI88
static struct mtd_partition nor_parts[] = {
	{
		.name   = "NOR uboot 512KB",
		.size   = 0x00080000,	//512KB
		.offset = 0,
	}, {
		.name   = "NOR kernel 2.5MB",
		.size   = 0x00280000,	//2.5MB
		.offset = 0x00080000,
	}, {
		.name   = "NOR rootFS",
		.size   = MTDPART_SIZ_FULL,
		.offset = MTDPART_OFS_APPEND,
	}
};

static struct platform_device esi88_nor_device = {
	.name		= "physmap-flash",
	.id		= -1,
	.num_resources	= 1,
	.resource	= (struct resource[]) {
	STM_PLAT_RESOURCE_MEM(0, 0x20000*512),
	},
	.dev.platform_data = &(struct physmap_flash_data) {
		.width		= 2,
		.nr_parts	= ARRAY_SIZE(nor_parts),
		.parts		= nor_parts
	},
};

static struct platform_device *mb680_devices[] __initdata = {
	&esi88_nor_device,
};

static int __init mb680_devices_init(void)
{
	stx7105_configure_sata(0);

	stx7105_configure_pwm(&(struct stx7105_pwm_config) {
			.out0 = stx7105_pwm_out0_pio4_4,
			.out1 = stx7105_pwm_out1_disabled, });
	stx7105_configure_ssc_i2c(0, &(struct stx7105_ssc_config) {
			.routing.ssc1.sclk = stx7105_ssc0_sclk_pio2_2,
			.routing.ssc1.mtsr = stx7105_ssc0_mtsr_pio2_3, });
	stx7105_configure_ssc_i2c(1, &(struct stx7105_ssc_config) {
			.routing.ssc1.sclk = stx7105_ssc1_sclk_pio2_5,
			.routing.ssc1.mtsr = stx7105_ssc1_mtsr_pio2_6, });
 	/* HDMI I2C bus - j00zek chyba jednak jest przydatne, bez mam kernel panic przez null w HDMI, jak tuner nie podlaczony */
	stx7105_configure_ssc_i2c(3, &(struct stx7105_ssc_config) {
			.routing.ssc3.sclk = stx7105_ssc3_sclk_pio3_6,
			.routing.ssc3.mtsr = stx7105_ssc3_mtsr_pio3_7, });
//j00zek end
	/*
	 * Note that USB port configuration depends on jumper
	 * settings:
	 *
	 *	  PORT 0	       		PORT 1
	 *	+-----------------------------------------------------------
	 * norm	|  4[4]	J5A:2-3			 4[6]	J10A:2-3
	 * alt	| 12[5]	J5A:1-2  J6F:open	14[6]	J10A:1-2  J11G:open
	 * norm	|  4[5]	J5B:2-3			 4[7]	J10B:2-3
	 * alt	| 12[6]	J5B:1-2  J6G:open	14[7]	J10B:1-2  J11H:open
	 */

	stx7105_configure_usb(0, &(struct stx7105_usb_config) {
			.ovrcur_mode = stx7105_usb_ovrcur_disabled,
			.pwr_enabled = 1,
			.routing.usb0.ovrcur = stx7105_usb0_ovrcur_pio4_4,
			.routing.usb0.pwr = stx7105_usb0_pwr_pio4_5, });
	stx7105_configure_usb(1, &(struct stx7105_usb_config) {
			.ovrcur_mode = stx7105_usb_ovrcur_active_low,
			.pwr_enabled = 1,
			.routing.usb1.ovrcur = stx7105_usb1_ovrcur_pio4_6,
			.routing.usb1.pwr = stx7105_usb1_pwr_pio4_7, });

	gpio_request(MB680_PIO_PHY_RESET, "notPioResetMII");
	gpio_direction_output(MB680_PIO_PHY_RESET, 1);

	gpio_request(PIO16_2, "PIO16_2 Reset DVBT");
	gpio_direction_output(PIO16_2, 1);

	gpio_request(PIO15_3, "PIO15_3 Reset DVBS");
	gpio_direction_output(PIO15_3, 1);

	gpio_request(PIO13_2, "PIO13_2 USB1_PWR");
	gpio_direction_output(PIO13_2, 1);

	gpio_request(PIO13_3, "PIO13_3 USB2_PWR");
	gpio_direction_output(PIO13_3, 1);

	stx7105_configure_ethernet(0, &(struct stx7105_ethernet_config) {
			.mode = stx7105_ethernet_mode_mii,
			.ext_clk = 1,
			.phy_bus = 0,
			.phy_addr = -1,
			.mdio_bus_data = &stmmac_mdio_bus,
		});

	/* enable optical out */
	stpio_request_pin(10, 6, "opt_out", STPIO_ALT_OUT);

	stx7105_configure_lirc(&(struct stx7105_lirc_config) {
			.rx_mode = stx7105_lirc_rx_mode_ir,
			.tx_enabled = 0,
			.tx_od_enabled = 0,
			});

	return platform_add_devices(mb680_devices, ARRAY_SIZE(mb680_devices));
}
arch_initcall(mb680_devices_init);

static void __iomem *mb680_ioport_map(unsigned long port, unsigned int size)
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

static void __init mb680_init_irq(void)
{
#ifndef CONFIG_SH_ST_MB705
	/* Configure STEM interrupts as active low. */
	set_irq_type(ILC_EXT_IRQ(1), IRQ_TYPE_LEVEL_LOW);
	set_irq_type(ILC_EXT_IRQ(2), IRQ_TYPE_LEVEL_LOW);
#endif
}

struct sh_machine_vector mv_mb680 __initmv = {
	.mv_name		= "STx7105 Mboard",
	.mv_setup		= mb680_setup,
	.mv_nr_irqs		= NR_IRQS,
	.mv_init_irq		= mb680_init_irq,
	.mv_ioport_map		= mb680_ioport_map,
	STM_PCI_IO_MACHINE_VEC
};
