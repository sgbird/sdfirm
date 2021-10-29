#include <target/eth.h>
#include <target/clk.h>
#include <target/io.h>

struct dw_xgmac {
	uint32_t clk_rate;
	uint8_t ether_addr[ETH_ALEN];
};

struct dw_xgmac dw_xgmac_ctrl;

#ifdef DW_XGMAC_CLK_CSR
#define dw_xgmac_clk_csr()		DW_XGMAC_CLK_CSR
#else
static uint8_t dw_xgmac_clk_csr(void)
{
	uint32_t clk_rate = dw_xgmac_ctrl.clk_rate;

	if (clk_rate > 400000000)
		return 0x5;
	else if (clk_rate > 350000000)
		return 0x4;
	else if (clk_rate > 300000000)
		return 0x3;
	else if (clk_rate > 250000000)
		return 0x2;
	else if (clk_rate > 150000000)
		return 0x1;
	else
		return 0x0;
}
#endif

static uint32_t dw_xgmac_mdio_c45(uint16_t phyaddr, uint32_t phyreg)
{
	__raw_clearl(_BV(phyaddr), MDIO_Clause_22_Port);

	return MDIO_PA(phyaddr) | MII_REGADDR_C45(phyreg) |
	       MDIO_DA(phyreg >> MII_DEVADDR_C45_OFFSET);
}

static uint32_t dw_xgmac_mdio_c22(uint16_t phyaddr, uint32_t phyreg)
{
	__raw_writel(_BV(phyaddr), MDIO_Clause_22_Port);

	return MDIO_PA(phyaddr) | (phyreg & 0x1F);
}

bool dw_xgmac_mdio_read(uint16_t phyaddr, uint32_t phyreg, uint16_t *phydata)
{
	uint32_t mdio_addr;
	uint32_t mdio_data = MDIO_DATA_DEFAULT;

	if (phyreg & MII_ADDR_C45) {
		phyaddr &= ~MII_ADDR_C45;
		mdio_addr = dw_xgmac_mdio_c45(phyaddr, phyreg);
	} else {
		mdio_addr = dw_xgmac_mdio_c22(phyaddr, phyreg);
		mdio_data |= MDIO_SAADR;
	}

	mdio_data |= MDIO_CR(dw_xgmac_clk_csr());
	mdio_data |= MDIO_CMD(MDIO_SINGLE_READ);

	while (dw_xgmac_mdio_busy());
	__raw_writel(mdio_addr, MDIO_Single_Command_Address);
	__raw_writel(mdio_data, MDIO_Single_Command_Control_Data);
	while (dw_xgmac_mdio_busy());

	mdio_data = __raw_readl(MDIO_Single_Command_Control_Data);
	*phydata = mdio_sdata(mdio_data);
	return true;
}

bool dw_xgmac_mdio_write(uint16_t phyaddr, uint32_t phyreg, uint16_t phydata)
{
	uint32_t mdio_addr;
	uint32_t mdio_data = MDIO_DATA_DEFAULT;

	if (phyreg & MII_ADDR_C45) {
		phyaddr &= ~MII_ADDR_C45;
		mdio_addr = dw_xgmac_mdio_c45(phyaddr, phyreg);
	} else {
		mdio_addr = dw_xgmac_mdio_c22(phyaddr, phyreg);
		mdio_data |= MDIO_SAADR;
	}

	mdio_data |= MDIO_CR(dw_xgmac_clk_csr());
	mdio_data |= MDIO_CMD(MDIO_SINGLE_WRITE);
	mdio_data |= MDIO_SDATA(phydata);

	while (dw_xgmac_mdio_busy());
	__raw_writel(mdio_addr, MDIO_Single_Command_Address);
	__raw_writel(mdio_data, MDIO_Single_Command_Control_Data);
	while (dw_xgmac_mdio_busy());

	return true;
}

static void dw_xgmac_set_ether(unsigned char *addr, unsigned int reg_n)
{
	uint32_t value;

	value = (addr[5] << 8) | addr[4];
	__raw_writel(value | MAC_AE, MAC_Address_High(reg_n));

	value = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
	__raw_writel(value, MAC_Address_Low(reg_n));
}

#ifdef CONFIG_DW_XGMAC_MDIO_PROBE
static dw_xgmac_mdio_probe(void)
{
	uint16_t phy, dev = 0, id1 = 0, id2 = 0;

	/* Probe MDIO devices */
	for (phy = 0; phy < MII_PHYADDR_MAX; phy++) {
		printf("phy%d-dev%d: probing\n", phy, dev);
		dw_xgmac_mdio_read(phy,
			MII_ADDR_C45 | ((uint32_t)dev) << 16 | MII_PHYSID1,
			&id1);
		dw_xgmac_mdio_read(phy,
			MII_ADDR_C45 | ((uint32_t)dev) << 16 | MII_PHYSID1,
			&id2);
		printf("phy%d-dev%d: id - %08x\n",
		       phy, dev, MAKELONG(id2, id1));
	}
}
#else
#define dw_xgmac_mdio_probe()		do { } while (0)
#endif

int dw_xgmac_get_tx_status(struct dw_xgmac_desc *p)
{
	uint32_t tdes3 = p->des3;
	int ret = tx_done;

	if (tdes3 & XGMAC_TDES3_OWN)
		return tx_dma_own;
	if (!(tdes3 & XGMAC_TDES3_LD))
		return tx_not_ls;
	return ret;
}

int dw_xgmac_get_rx_status(struct dw_xgmac_desc *p)
{
	uint32_t rdes3 = p->des3;

	if (rdes3 & XGMAC_RDES3_OWN)
		return rx_dma_own;
	if (rdes3 & XGMAC_RDES3_CTXT)
		return rx_discard_frame;
	if (!(rdes3 & XGMAC_RDES3_LD))
		return rx_not_ls;
	if ((rdes3 & XGMAC_RDES3_ES) && (rdes3 & XGMAC_RDES3_LD))
		return rx_discard_frame;
	return rx_good_frame;
}

void dw_xgmac_init_rx_desc(struct dw_xgmac_desc *p)
{
	dw_xgmac_set_rx_owner(p);
#ifdef CONFIG_SYS_IRQ
	dw_xgmac_enable_rx_ic(p);
#endif
}

void dw_xgmac_init_tx_desc(struct dw_xgmac_desc *p)
{
	p->des0 = p->des1 = p->des2 = p->des3 = 0;
}

void dw_xgmac_prepare_tx_desc(struct dw_xgmac_desc *p, bool is_fs,
			      int len, bool csum_flag, bool tx_own, bool ls)
{
	uint32_t tdes3 = p->des3;

	p->des2 |= XGMAC_TDES2_B1L(len);
	if (is_fs)
		tdes3 |= XGMAC_TDES3_FD;
	else
		tdes3 &= ~XGMAC_TDES3_FD;
	if (csum_flag)
		tdes3 |= XGMAC_TDES3_CIC(XGMAC_CIC_CRC_REPLACEMENT);
	else
		tdes3 &= ~XGMAC_TDES3_CIC(XGMAC_TDES3_CIC_MASK);
	if (ls)
		tdes3 |= XGMAC_TDES3_LD;
	else
		tdes3 &= ~XGMAC_TDES3_LD;
	if (tx_own)
		tdes3 |= XGMAC_TDES3_OWN;

	/* DMA barrier for the first own frame */
	if (is_fs && tx_own)
		dma_wmb();

	p->des3 = tdes3;
}

void dw_xgmac_init_ctrl(void)
{
	/* Configure clk_csr parameters */
	dw_xgmac_ctrl.clk_rate = clk_get_frequency(DW_XGMAC_CLK_SRC);

	/* Reset PCS */
	dw_xpcs_link_up();

	/* Configure ethernet address */
	eth_random_addr(dw_xgmac_ctrl.ether_addr);
	dw_xgmac_set_ether(dw_xgmac_ctrl.ether_addr, 0);

	dw_xgmac_mdio_probe();
}
