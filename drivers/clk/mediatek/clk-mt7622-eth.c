// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Chen Zhong <chen.zhong@mediatek.com>
 *	   Sean Wang <sean.wang@mediatek.com>
 */

#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#include "clk-mtk.h"
#include "clk-gate.h"

#include <dt-bindings/clock/mt7622-clk.h>

#define GATE_ETH(_id, _name, _parent, _shift)			\
	GATE_MTK(_id, _name, _parent, &eth_cg_regs, _shift, &mtk_clk_gate_ops_no_setclr_inv)

static const struct mtk_gate_regs eth_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

static const struct mtk_gate eth_clks[] = {
	GATE_ETH(CLK_ETH_HSDMA_EN, "eth_hsdma_en", "eth_sel", 5),
	GATE_ETH(CLK_ETH_ESW_EN, "eth_esw_en", "eth_500m", 6),
	GATE_ETH(CLK_ETH_GP2_EN, "eth_gp2_en", "txclk_src_pre", 7),
	GATE_ETH(CLK_ETH_GP1_EN, "eth_gp1_en", "txclk_src_pre", 8),
	GATE_ETH(CLK_ETH_GP0_EN, "eth_gp0_en", "txclk_src_pre", 9),
};

static const struct mtk_gate_regs sgmii_cg_regs = {
	.set_ofs = 0xE4,
	.clr_ofs = 0xE4,
	.sta_ofs = 0xE4,
};

#define GATE_SGMII(_id, _name, _parent, _shift)			\
	GATE_MTK(_id, _name, _parent, &sgmii_cg_regs, _shift, &mtk_clk_gate_ops_no_setclr_inv)

static const struct mtk_gate sgmii_clks[] = {
	GATE_SGMII(CLK_SGMII_TX250M_EN, "sgmii_tx250m_en",
		   "ssusb_tx250m", 2),
	GATE_SGMII(CLK_SGMII_RX250M_EN, "sgmii_rx250m_en",
		   "ssusb_eq_rx250m", 3),
	GATE_SGMII(CLK_SGMII_CDR_REF, "sgmii_cdr_ref",
		   "ssusb_cdr_ref", 4),
	GATE_SGMII(CLK_SGMII_CDR_FB, "sgmii_cdr_fb",
		   "ssusb_cdr_fb", 5),
};

static u16 rst_ofs[] = { 0x34, };

static const struct mtk_clk_rst_desc clk_rst_desc = {
	.version = MTK_RST_SIMPLE,
	.rst_bank_ofs = rst_ofs,
	.rst_bank_nr = ARRAY_SIZE(rst_ofs),
};

static int clk_mt7622_ethsys_init(struct platform_device *pdev)
{
	struct clk_hw_onecell_data *clk_data;
	struct device_node *node = pdev->dev.of_node;
	int r;

	clk_data = mtk_alloc_clk_data(CLK_ETH_NR_CLK);

	mtk_clk_register_gates(node, eth_clks, ARRAY_SIZE(eth_clks),
			       clk_data);

	r = of_clk_add_hw_provider(node, of_clk_hw_onecell_get, clk_data);
	if (r)
		dev_err(&pdev->dev,
			"could not register clock provider: %s: %d\n",
			pdev->name, r);

	mtk_register_reset_controller_with_dev(&pdev->dev, &clk_rst_desc);

	return r;
}

static int clk_mt7622_sgmiisys_init(struct platform_device *pdev)
{
	struct clk_hw_onecell_data *clk_data;
	struct device_node *node = pdev->dev.of_node;
	int r;

	clk_data = mtk_alloc_clk_data(CLK_SGMII_NR_CLK);

	mtk_clk_register_gates(node, sgmii_clks, ARRAY_SIZE(sgmii_clks),
			       clk_data);

	r = of_clk_add_hw_provider(node, of_clk_hw_onecell_get, clk_data);
	if (r)
		dev_err(&pdev->dev,
			"could not register clock provider: %s: %d\n",
			pdev->name, r);

	return r;
}

static const struct of_device_id of_match_clk_mt7622_eth[] = {
	{
		.compatible = "mediatek,mt7622-ethsys",
		.data = clk_mt7622_ethsys_init,
	}, {
		.compatible = "mediatek,mt7622-sgmiisys",
		.data = clk_mt7622_sgmiisys_init,
	}, {
		/* sentinel */
	}
};

static int clk_mt7622_eth_probe(struct platform_device *pdev)
{
	int (*clk_init)(struct platform_device *);
	int r;

	clk_init = of_device_get_match_data(&pdev->dev);
	if (!clk_init)
		return -EINVAL;

	r = clk_init(pdev);
	if (r)
		dev_err(&pdev->dev,
			"could not register clock provider: %s: %d\n",
			pdev->name, r);

	return r;
}

static struct platform_driver clk_mt7622_eth_drv = {
	.probe = clk_mt7622_eth_probe,
	.driver = {
		.name = "clk-mt7622-eth",
		.of_match_table = of_match_clk_mt7622_eth,
	},
};
module_platform_driver(clk_mt7622_eth_drv);
MODULE_LICENSE("GPL");
