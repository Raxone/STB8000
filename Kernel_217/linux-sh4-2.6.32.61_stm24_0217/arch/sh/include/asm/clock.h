#ifndef __ASM_SH_CLOCK_H
#define __ASM_SH_CLOCK_H

#ifdef CONFIG_SH_CLK
#include <linux/sh_clk.h>
#endif
#ifdef CONFIG_STM_DRIVERS
#include <linux/stm/clk.h>
#endif

/* Should be defined by processor-specific code */
void __deprecated arch_init_clk_ops(struct clk_ops **, int type);
int __init arch_clk_init(void);

/* arch/sh/kernel/cpu/clock-cpg.c */
int __init __deprecated cpg_clk_init(void);

/* arch/sh/kernel/cpu/clock.c */
int clk_init(void);

#endif /* __ASM_SH_CLOCK_H */
