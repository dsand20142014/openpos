_statistics	wstats;		// wireless stats
#endif /* WIRELESS_EXT */
    BOOL                    bCommit;

} DEVICE_INFO, *PSDevice;


//PLICE_DEBUG->


 inline  static	VOID   EnQueue (PSDevice pDevice,PSRxMgmtPacket  pRxMgmtPacket)
{
	//printk("Enter EnQueue:tail is %d\n",pDevice->rxManeQueue.tail);
	if ((pDevice->rxManeQueue.tail+1) % NUM == pDevice->rxManeQueue.head)
	{
		//printk("Queue is Full,tail is %d\n",pDevice->rxManeQueue.tail);
		return ;
	}
	else
	{
		pDevice->rxManeQueue.tail = (pDevice->rxManeQueue.tail+1)% NUM;
		pDevice->rxManeQueue.Q[pDevice->rxManeQueue.tail] = pRxMgmtPacket;
		pDevice->rxManeQueue.packet_num++;
		//printk("packet num is %d\n",pDevice->rxManeQueue.packet_num);
	}
}




	inline  static  PSRxMgmtPacket DeQueue (PSDevice pDevice)
{
	PSRxMgmtPacket  pRxMgmtPacket;
	if (pDevice->rxManeQueue.tail == pDevice->rxManeQueue.head)
	{
		printk("Queue is Empty\n");
		return NULL;
	}
	else
	{
		int	x;
		//x=pDevice->rxManeQueue.head = (pDevice->rxManeQueue.head+1)%NUM;
		pDevice->rxManeQueue.head = (pDevice->rxManeQueue.head+1)%NUM;
		x = pDevice->rxManeQueue.head;
		//printk("Enter DeQueue:head is %d\n",x);
		pRxMgmtPacket = pDevice->rxManeQueue.Q[x];
		pDevice->rxManeQueue.packet_num--;
		return pRxMgmtPacket;
	}
}

VOID	InitRxManagementQueue(PSDevice   pDevice);



//PLICE_DEBUG<-






inline static BOOL device_get_ip(PSDevice pInfo) {
    struct in_device* in_dev=(struct in_device*) pInfo->dev->ip_ptr;
    struct in_ifaddr* ifa;

    if (in_dev!=NULL) {
        ifa=(struct in_ifaddr*) in_dev->ifa_list;
        if (ifa!=NULL) {
            memcpy(pInfo->abyIPAddr,&ifa->ifa_address,4);
            return TRUE;
        }
    }
    return FALSE;
}



static inline PDEVICE_RD_INFO alloc_rd_info(void) {
    PDEVICE_RD_INFO  ptr;
    if ((ptr = (PDEVICE_RD_INFO)kmalloc((int)sizeof(DEVICE_RD_INFO), (int)GFP_ATOMIC)) == NULL)
        return NULL;
    else {
        memset(ptr,0,sizeof(DEVICE_RD_INFO));
        return ptr;
    }
}

static inline PDEVICE_TD_INFO alloc_td_info(void) {
    PDEVICE_TD_INFO  ptr;
    if ((ptr = (PDEVICE_TD_INFO)kmalloc((int)sizeof(DEVICE_TD_INFO), (int)GFP_ATOMIC))==NULL)
        return NULL;
    else {
        memset(ptr,0,sizeof(DEVICE_TD_INFO));
