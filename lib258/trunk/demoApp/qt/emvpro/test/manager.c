// 172.16.40.80


#include <include.h>
#include <osicc.h>

#pragma arm section rwdata ,code = "manager"
void manager(struct seven *EventInfo)
{
	unsigned char   EventType;
	unsigned char   EventNum;

	EventType = (unsigned char )EventInfo->event_type;
	EventNum  = (unsigned char )EventInfo->event_nr;

	EventInfo->pt_eve_out->treatment_status = NOK;
	switch ( EventType)
	{
		case EVECIR:
			switch(EventNum)
			{
				case EVE_MASTER_APPLI:
					MASAPP_Event();
					break;
				default:
					break;	
			}
			break;
		case EVECOM:
			switch(EventNum)
			{
				case EVE_POWER_ON:
					
					EventInfo->pt_eve_out->treatment_status = OK;
					break;					
				default:
					break;	
			}
			break;
		default:
			break;		
	}
}	       		

#pragma arm section code ,rwdata

