/*
************************************************************************
*  General driver management
************************************************************************
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "osdrv.h"
#include "osdriver.h"

static DRVTBL drvtbl[] =
{
        /*   id              name  version   type  allow_share  calling_cnt block_tasks drv_op */
        {DRVID_MMI, "MMI", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_PRN, "PRN", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_PAD, "PAD", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_EEP, "EEP", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_MAG, "MAG", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_DAT, "DAT", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_COM, "COM", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_ICC, "ICC", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_I2C, "IIC", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_CHE, "CHE", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_PME, "PME", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_GSM, "GSM", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_ADC, "ADC", 0x10,   1,      0,     0,        NULL,      NULL},
        {DRVID_EXP, "EXP", 0x10,   1,      0,     0,        NULL,      NULL},/* example */
};
void OSDRV_init(void)
{
        int i=0;

        while (i < sizeof(drvtbl) / sizeof(drvtbl[0]))
        {
                drvtbl[i++].drv_op = NULL;
        }
}

int OSDRV_install(unsigned char drvid, DRVOP *opration)
{
        int retval;

        if ( drvid > DRV_MAXNB)
        {
                retval = DRVERR_NOTEXIST;
        }
        else if (!opration)
        {
                retval = DRVERR_OPERATION;
        }
        else
        {
                if (drvtbl[drvid].drv_op == NULL)       /* have installed before  */
                {
                        drvtbl[drvid].drv_op = opration;
                        if (drvtbl[drvid].drv_op->drv_init) /* if necessary to initialize */
                        {
                                drvtbl[drvid].drv_op->drv_init();
                        }
                }
                retval = DRVERR_SUCCESS;
        }
        return retval;
}

int OSDRV_remove(unsigned char drvid)
{
        int retval;

        if ( drvid > DRV_MAXNB)
        {
                retval = DRVERR_NOTEXIST;
        }
        else
        {
                if (drvtbl[drvid].drv_op != NULL)         /* have removed before  */
                {
                        if (drvtbl[drvid].drv_op->drv_remove) /* if necessary to remove*/
                        {
                                drvtbl[drvid].drv_op->drv_remove();
                        }
                        drvtbl[drvid].drv_op = NULL;
                }
                retval = DRVERR_SUCCESS;
        }
        return retval;
}



int OSDRV_call(DRV *order)
{
        int retval;
        unsigned char drvid;
        DRVOUT *pdrvout;    
        
        retval = DRVERR_SUCCESS;
        if ( order->ucDrvID > DRV_MAXNB )
        {
                retval = DRVERR_NOTEXIST;
        }
        else
        {
        
                drvid = order->ucDrvID;
                pdrvout = order->pDrvOut;

                if (drvtbl[drvid].drv_calling_cnt > 0)   /* the driver is busy */
                {
                        retval = DRVERR_BUSY;
                        goto callexit;
                }
                              

                if (drvtbl[drvid].drv_op == NULL)        /* the driver is absent */
                {
                        pdrvout->ucGenStatus = DRVST_ABSENT;
                        pdrvout->ucDrvStatus = DRVST_ABSENT;
                        retval = DRVERR_ABSENT;
                        goto callexit;
                }
                

                drvtbl[drvid].drv_calling_cnt++;
                
                pdrvout->ucGenStatus = DRVST_RUNNING;
                                      
                switch (order->ucDrvCMD)
                {
                case DRVCMD_EXECUTE:
                        if (drvtbl[drvid].drv_op->drv_exec )
                        {                       
                                retval = (*(drvtbl[drvid].drv_op->drv_exec))(order->pDrvIn, order->pDrvOut);
                        }
                        break;
                case DRVCMD_ABORT:
                        if (drvtbl[drvid].drv_op->drv_abort )
                        {
                                retval = (*(drvtbl[drvid].drv_op->drv_abort))(order->pDrvIn, order->pDrvOut);
                        }
                        break;
                case DRVCMD_RESET:
                        if (drvtbl[drvid].drv_op->drv_reset )
                        {
                                retval = (*(drvtbl[drvid].drv_op->drv_reset))(order->pDrvIn, order->pDrvOut);
                        }
                        break;
                case DRVCMD_STATEREQ:
                        if (drvtbl[drvid].drv_op->drv_state )
                        {
                                retval = (*(drvtbl[drvid].drv_op->drv_state))(order->pDrvIn, order->pDrvOut);
                        }
                        break;
                case DRVCMD_IOCTL:
                        if (drvtbl[drvid].drv_op->drv_ioctl )
                        {
                                retval = (*(drvtbl[drvid].drv_op->drv_ioctl))(order->pDrvIn->ucType, order->pDrvIn->aucData);
                        }
                        break;
                case DRVCMD_TEST:
                        if (drvtbl[drvid].drv_op->drv_test )
                        {
                                retval = (*(drvtbl[drvid].drv_op->drv_test))();
                        }
                        break;
                default:
                        retval = DRVERR_BADCMD;
                        break;
                }
                drvtbl[drvid].drv_calling_cnt--;
        }
        
     //   printf("end osdrv_call\n\n");
        
callexit:
        return retval;
}

unsigned char OSDRV_get_version(unsigned char drvid)
{
        return (drvtbl[drvid].drv_version);
}

unsigned char *OSDRV_get_name(unsigned char drvid)
{
        return (drvtbl[drvid].drv_name);
}


/* for support API */
int OSDRV_execute(unsigned char drvid, DRVIN *pdrvin, DRVOUT *pdrvout)
{
        DRV order;
        order.ucDrvID  = drvid;
        order.ucDrvCMD = DRVCMD_EXECUTE;
        order.pDrvIn   = pdrvin;
        order.pDrvOut  = pdrvout;
        return (OSDRV_call(&order));
}

unsigned char OSDRV_wait(DRVOUT *drvout)
{
        while (  (drvout->ucGenStatus == DRVST_RUNNING)
                        || (drvout->ucGenStatus == DRVST_WAIT)
              );
        return(drvout->ucDrvStatus);
}


void OSDRV_abort(unsigned char drvid)
{
        //DRV drv;

        //drv.ucDrvID  = drvid;
        //drv.ucDrvCMD = DRVCMD_ABORT;
        //OSDRV_call(&drv);
}

// API interface
void Os__abort_drv (unsigned char drvid)
{
        OSDRV_abort(drvid);
}

void Os__call_drv (DRV *order)
{
		OSDRV_call(order);
}

unsigned char Os__wait_drv (DRVOUT *out)
{
        return OSDRV_wait(out);
}

/*zy add 0717*/
DRVOUT *OSDRV_CallAsync(DRIVER *pDrv)
{
}


//////////////////// END ////////////////////////////////////
//
