/*****************************************************************************
*  Copyright 2011 - 2012 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include <linux/io.h>
#include<mach/pwr_mgr.h>
#include<plat/pwr_mgr.h>
#include <linux/uaccess.h>
#include <mach/clock.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/mfd/bcmpmu.h>
#include <linux/err.h>

#ifdef CONFIG_KONA_AVS
#include <plat/kona_avs.h>
#endif

#include "capri_avs.h"
#include "volt_tbl.h"
#include <mach/chipregHw_inline.h>

/*sysfs interface to read PMU vlt table*/
static u32 csr_vlt_table[SR_VLT_LUT_SIZE];
module_param_array_named(csr_vlt_table, csr_vlt_table, uint, NULL, S_IRUGO);

static int vlt_tbl_init;

u8 csr_vlt_table_ss[A9_FREQ_MAX][SR_VLT_LUT_SIZE] = {
	[A9_FREQ_1_GHZ] = PMU_CSR_VLT_TBL_SS_1000M,
	[A9_FREQ_1200_MHZ] = PMU_CSR_VLT_TBL_SS_1200M,
	[A9_FREQ_1500_MHZ] = PMU_CSR_VLT_TBL_SS_1500M,
};

u8 csr_vlt_table_ts[A9_FREQ_MAX][SR_VLT_LUT_SIZE] = {
	[A9_FREQ_1_GHZ] = PMU_CSR_VLT_TBL_TS_1000M,
	[A9_FREQ_1200_MHZ] = PMU_CSR_VLT_TBL_TS_1200M,
	[A9_FREQ_1500_MHZ] = PMU_CSR_VLT_TBL_TS_1500M,
};

u8 csr_vlt_table_tt[A9_FREQ_MAX][SR_VLT_LUT_SIZE] = {
	[A9_FREQ_1_GHZ] = PMU_CSR_VLT_TBL_TT_1000M,
	[A9_FREQ_1200_MHZ] = PMU_CSR_VLT_TBL_TT_1200M,
	[A9_FREQ_1500_MHZ] = PMU_CSR_VLT_TBL_TT_1500M,
};

u8 csr_vlt_table_tf[A9_FREQ_MAX][SR_VLT_LUT_SIZE] = {
	[A9_FREQ_1_GHZ] = PMU_CSR_VLT_TBL_TF_1000M,
	[A9_FREQ_1200_MHZ] = PMU_CSR_VLT_TBL_TF_1200M,
	[A9_FREQ_1500_MHZ] = PMU_CSR_VLT_TBL_TF_1500M,
};

u8 csr_vlt_table_ff[A9_FREQ_MAX][SR_VLT_LUT_SIZE] = {
	[A9_FREQ_1_GHZ] = PMU_CSR_VLT_TBL_FF_1000M,
	[A9_FREQ_1200_MHZ] = PMU_CSR_VLT_TBL_FF_1200M,
	[A9_FREQ_1500_MHZ] = PMU_CSR_VLT_TBL_FF_1500M,
};

#ifdef CONFIG_KONA_AVS
u8 vlt_table[SR_VLT_LUT_SIZE];
#endif

const u8 *bcmpmu_get_sr_vlt_table(int sr, u32 freq_inx,
				  u32 silicon_type_csr,
				  u32 silicon_type_msr, u32 silicon_type_vsr)
{
	u8 *csr_vlt_table;
	u8 *msr_vlt_table;
	u8 *vsr_vlt_table;
	u32 i;

	pr_info("%s: sr = %i\n", __func__, sr);
	pr_info("%s: freq_inx = %d\n", __func__, freq_inx);
	pr_info("%s: silicon_type_csr = %d\n", __func__, silicon_type_csr);
	pr_info("%s: silicon_type_msr = %d\n", __func__, silicon_type_msr);
	pr_info("%s: silicon_type_vsr = %d\n", __func__, silicon_type_vsr);

	vlt_tbl_init = 1;

	BUG_ON(!vlt_tbl_init || freq_inx > A9_FREQ_1500_MHZ);

#ifdef CONFIG_KONA_AVS

#if 1
	switch (silicon_type_csr) {
	case SILICON_SS:
		csr_vlt_table = csr_vlt_table_ss[freq_inx];
		break;

	case SILICON_TS:
		csr_vlt_table = csr_vlt_table_ts[freq_inx];
		break;

	case SILICON_TT:
		csr_vlt_table = csr_vlt_table_tt[freq_inx];
		break;

	case SILICON_TF:
		csr_vlt_table = csr_vlt_table_tf[freq_inx];
		break;

	case SILICON_FF:
		csr_vlt_table = csr_vlt_table_ff[freq_inx];
		break;

	default:
		BUG();
	}

	switch (silicon_type_msr) {
	case SILICON_SS:
		msr_vlt_table = csr_vlt_table_ss[freq_inx];
		break;

	case SILICON_TS:
		msr_vlt_table = csr_vlt_table_ts[freq_inx];
		break;

	case SILICON_TT:
		msr_vlt_table = csr_vlt_table_tt[freq_inx];
		break;

	case SILICON_TF:
		msr_vlt_table = csr_vlt_table_tf[freq_inx];
		break;

	case SILICON_FF:
		msr_vlt_table = csr_vlt_table_ff[freq_inx];
		break;

	default:
		BUG();
	}

	switch (silicon_type_vsr) {
	case SILICON_SS:
		vsr_vlt_table = csr_vlt_table_ss[freq_inx];
		break;

	case SILICON_TS:
		vsr_vlt_table = csr_vlt_table_ts[freq_inx];
		break;

	case SILICON_TT:
		vsr_vlt_table = csr_vlt_table_tt[freq_inx];
		break;

	case SILICON_TF:
		vsr_vlt_table = csr_vlt_table_tf[freq_inx];
		break;

	case SILICON_FF:
		vsr_vlt_table = csr_vlt_table_ff[freq_inx];
		break;

	default:
		BUG();
	}

	for (i = 0; i < SR_VLT_LUT_SIZE; i++) {
		switch (i) {
		case MSR_ECON:
			vlt_table[i] = msr_vlt_table[i];
			break;

		case MSR_NORMAL:
			vlt_table[i] = msr_vlt_table[i];
			break;

		case MSR_TURBO:
			vlt_table[i] = msr_vlt_table[i];
			break;

		case VSR_NORMAL:
			vlt_table[i] = vsr_vlt_table[i];
			break;

		default:
			vlt_table[i] = csr_vlt_table[i];

		}
	}
#else
	switch (silicon_type) {
	case SILICON_SS:
		return csr_vlt_table_ss;

	case SILICON_TT:
		return csr_vlt_table_tt;

	case SILICON_FF:
		return csr_vlt_table_ff;

	default:
		BUG();
	}

#endif
	return vlt_table;

#else
	return csr_vlt_table_ss[freq_inx];
#endif
}

#define MHZ(x) ((x)*1000*1000)
#define GHZ(x) (MHZ(x)*1000)

static const u32 a9_freq_list[A9_FREQ_MAX] = {
	[A9_FREQ_1_GHZ] = GHZ(1),
	[A9_FREQ_1200_MHZ] = MHZ(1200),
	[A9_FREQ_1500_MHZ] = MHZ(1500),
};

int pm_init_pmu_sr_vlt_map_table(u32 silicon_type_csr,
				 u32 silicon_type_msr, u32 silicon_type_vsr,
				 int freq_id)
{
#define RATE_ADJ 10
	struct clk *a9_pll_chnl1;
	int inx;
	unsigned long rate;
	u8 *vlt_table;
	u8 vlt_table40[SR_VLT_LUT_SIZE];

	a9_pll_chnl1 = clk_get(NULL, A9_PLL_CHNL1_CLK_NAME_STR);

	BUG_ON(IS_ERR_OR_NULL(a9_pll_chnl1));

	rate = clk_get_rate(a9_pll_chnl1);

	pr_info("%s:\n"
		"   silicon type csr = %d\n"
		"   silicon type msr = %d\n"
		"   silicon type vsr = %d\n"
		"   A9 clock rate = %d\n"
		"   ATE freq = %d\n",
		__func__,
		silicon_type_csr,
		silicon_type_msr, silicon_type_vsr, (u32)rate, freq_id);

	rate += RATE_ADJ;

	for (inx = A9_FREQ_MAX - 1; inx >= 0; inx--) {

		if (rate / a9_freq_list[inx])
			break;
	}

#if 0
	if (inx < 0) {
		pr_info("%s: BUG => No maching freq found!!!\n", __func__);
		BUG();
	}

	/**
	 * BUG if the frequency type reported by Kona ATE AVS
	 * is not same as A9 PLL channel configuration
	 */
	if (freq_id >= 0) {
		pr_info("%s: freq_id = %d\n", __func__, freq_id);
		pr_info("%s: BUG => ATE frequency != A9 PLL !!!\n", __func__);
		BUG_ON(inx != freq_id);
	}
#endif

	pr_info("%s: A9 PLL frequency Index = %d\n", __func__, inx);

	/**
	 * Frequency reported by AVS is not same as PLL configuration??
	 * Decision taken here is:
	 * 1.   if AVS reported frequency > PLL configuration : use
	 * voltage table for PLL configured frequency
	 * 2.   if AVS reported frequency < PLL configuration : This
	 * is ideally not possible (device may not work !!).
	 * Report an error
	 * otherwise assume slow silicon
	 */

	if (freq_id < inx) {
		silicon_type_csr = SILICON_TS;
		silicon_type_msr = SILICON_TS;
		silicon_type_vsr = SILICON_TS;
		pr_info("%s: ATE frequency < A9 PLL configuration!!"
			"Silicon Types Set to Slow\n", __func__);
	}

	if (chipregHw_getChipIdRev() <= 0xA1) {
		silicon_type_csr = SILICON_TS;
		silicon_type_msr = SILICON_TS;
		silicon_type_vsr = SILICON_TS;

		pr_info("%s: A1 Chip, Silicon Types Set to Slow. "
			"silicon_type_csr = %d, "
			"silicon_type_msr = %d, "
			"silicon_type_vsr = %d\n",
			__func__, silicon_type_csr,
			silicon_type_msr, silicon_type_vsr);
	}

	vlt_table = (u8 *)bcmpmu_get_sr_vlt_table(0, (u32)inx,
						  silicon_type_csr,
						  silicon_type_msr,
						  silicon_type_vsr);

	for (inx = 0; inx < SR_VLT_LUT_SIZE; inx++) {
		vlt_table40[inx] = vlt_table[inx] | 0x40;
		csr_vlt_table[inx] = vlt_table[inx];
		pr_info("Idx = %2d, voltage = %3d  (0x%02x) (0x%02x)\n",
			inx, vlt_table[inx], vlt_table[inx], vlt_table40[inx]);
	}
	return pwr_mgr_pm_i2c_var_data_write(vlt_table40, SR_VLT_LUT_SIZE);
}
