#ifndef MXC_EPIT_H_INCLUDED
#define MXC_EPIT_H_INCLUDED

typedef enum
{
    EPIT1=1,
    EPIT2=2,
}
mxc_epit;

#define EPIT1_BASE  0x53F94000
#define EPIT2_BASE  0x53F98000

#define EPITCR_OFFSET       0x0000
#define EPITSR_OFFSET       0x0004
#define EPITLR_OFFSET       0x0008
#define EPITCMPR_OFFSET     0x000C
#define EPITCNR_OFFSET      0x0010

#define EPIT1_CR_PHYADDR    (EPIT1_BASE+EPITCR_OFFSET)
#define EPIT1_SR_PHYADDR    (EPIT1_BASE+EPITSR_OFFSET)
#define EPIT1_LR_PHYADDR    (EPIT1_BASE+EPITLR_OFFSET)
#define EPIT1_CMPR_PHYADDR  (EPIT1_BASE+EPITCMPR_OFFSET)
#define EPIT1_CNR_PHYADDR   (EPIT1_BASE+EPITCNR_OFFSET)

#define EPIT2_CR_PHYADDR    (EPIT2_BASE+EPITCR_OFFSET)
#define EPIT2_SR_PHYADDR    (EPIT2_BASE+EPITSR_OFFSET)
#define EPIT2_LR_PHYADDR    (EPIT2_BASE+EPITLR_OFFSET)
#define EPIT2_CMPR_PHYADDR  (EPIT2_BASE+EPITCMPR_OFFSET)
#define EPIT2_CNR_PHYADDR   (EPIT2_BASE+EPITCNR_OFFSET)

#define EPIT1_INT	28
#define EPIT2_INT	27

struct epit_dev
{
    volatile u32 *epit1_cr;
    volatile u32 *epit1_sr;
    volatile u32 *epit1_lr;
    volatile u32 *epit1_cmpr;
    volatile u32 *epit1_cnr;

    volatile u32 *epit2_cr;
    volatile u32 *epit2_sr;
    volatile u32 *epit2_lr;
    volatile u32 *epit2_cmpr;
    volatile u32 *epit2_cnr;
};

extern void epit_set_cmpr(mxc_epit n, u32 val);
extern void epit_set_lr(mxc_epit n, u32 val);
extern void epit_set_rld(mxc_epit n, u32 val);
extern void epit_set_enmod(mxc_epit n, u32 val);
extern void epit_set_prescalar(mxc_epit n, u32 val);
extern void epit_clear_ocif(mxc_epit n);
extern void epit_set_clksrc(mxc_epit n,u32 val);
extern void epit_set_ocien(mxc_epit n,bool flag);
extern void epit_set_om(mxc_epit n, u32 val);
extern void epit_disable(mxc_epit n);
extern void epit_enable(mxc_epit n);
extern void epit_pr_allreg(mxc_epit n);
extern void epit_set_iovw(mxc_epit n,bool flag);
extern void epit_set_waiten(mxc_epit n,bool flag);
extern void epit_set_stopen(mxc_epit n,bool flag);

#endif //MXC_EPIT_H_INCLUDED
