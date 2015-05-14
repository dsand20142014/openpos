/************************************************************
 *
 *************************************************************
 */
#include "sys_linux.h"
#include "oslib.h"
#include "main.h"
#include "osmenu.h"


#define OSMENUITEM_OFFSET	2
char bt_text[105];


unsigned char OSMENU_Display(unsigned char *pucTitle,OSMENU *pMENU)
{
    unsigned char ucI;
    unsigned char ucKey;
    unsigned char ucExitFlag;
    unsigned char ucMenuCount;
    unsigned char ucMenuCurrCount;
    unsigned char ucMenuPage;
    unsigned char ucMenuCurrPage;
    unsigned char ucMenuPageCount;
    unsigned char ucMenuSelect;
    unsigned char ucMenuNewSelect;
     char aucID[5];
    int ieventid;


    ucMenuCount = 0;
    while (pMENU[ucMenuCount].pucText)
    {
        ucMenuCount++;
    }
    if (ucMenuCount == 0)
    {
        return(OSMENUERR_SUCCESS);
    }
    ucMenuPage = (ucMenuCount - 1) / OSMENU_PAGEMENUITEM;
    ucMenuCurrPage = 0;

    ucMenuSelect = 0;

    do{
        OSMMI_ClrDisplay(ASCIIFONT57, 255);
        Os__display_qt(ASCIIFONT57, 0, 0, pucTitle, WIDGET_LABEL, 0, 0);

        memset(aucID, 0, sizeof(aucID));
        sprintf((char *)aucID, "%d", ucMenuCurrPage + 1);
   
        Os__display_qt(ASCIIFONT57, 0, 20, aucID, WIDGET_LABEL, 0, NULL);

        ucMenuNewSelect = ucMenuSelect;
        ucMenuCurrCount = ucMenuCurrPage * OSMENU_PAGEMENUITEM;

        for (ucI = ucMenuCurrCount, ucMenuSelect = 0, ucMenuPageCount = 0;
             (ucI < ucMenuCount) && (ucMenuSelect < OSMENU_PAGEMENUITEM);
             ucI++, ucMenuSelect++, ucMenuPageCount++)
        {
            memset(aucID, 0, sizeof(aucID));
            sprintf((char *)aucID, "%d.", ucMenuSelect + 1);
     
            strncpy(bt_text, aucID, strlen(aucID));
            strcpy((char *)(bt_text+strlen(aucID)), (char *)(pMENU[ucI].pucText));
            Os__display_qt(0x30, 1 + ucMenuSelect, 0, bt_text, WIDGET_BUTTON, 1 + ucMenuSelect, NULL);
        }


        ucMenuSelect = ucMenuNewSelect;
        if (ucMenuSelect >= ucMenuPageCount)
        {
            ucMenuSelect = ucMenuPageCount - 1;
        }

        do{
            ucExitFlag = 0;
            ucMenuNewSelect = ucMenuSelect;
            ucKey = Os__xget_key();
            
            switch( ucKey )
            {
            case KEY_CLEAR:
                return(OSMENUERR_SUCCESS);
                
            case KEY_F1:
               
                break;
            case KEY_F2:
                
                break;
            case KEY_UP:
                ucExitFlag = 1;
                if( ucMenuCurrPage )
                {
                    ucMenuCurrPage --;
                }else
                {
                    //ucMenuCurrPage ++;
                    ucMenuCurrPage = ucMenuPage;// to last page liugm 2010-11-26
                    if( ucMenuCurrPage > ucMenuPage )
                        ucMenuCurrPage = 0;
                    if(  (ucMenuCurrPage == ucMenuPage )
                         &&(ucMenuCurrPage == 0)
                         )
                        ucExitFlag = 0;
                }
                if( ucExitFlag )
                    ucMenuSelect = 0;
                break;
            case KEY_DOWN:
                ucExitFlag = 1;
                ucMenuCurrPage ++;
                if( ucMenuCurrPage > ucMenuPage )
                    ucMenuCurrPage = 0;
                if(  (ucMenuCurrPage == ucMenuPage )
                     &&(ucMenuCurrPage == 0)
                     )
                    ucExitFlag = 0;
                if( ucExitFlag )
                    ucMenuSelect = 0;
                break;
            default:
                if(  (ucKey >='1')&&(ucKey <='9'))
                {
                    ucKey -= '0';
                    if( ucKey <= ucMenuPageCount )
                        ucMenuNewSelect = ucKey-1;
                }
                break;
            }
            if( !ucExitFlag )
            {
                if( ucMenuNewSelect != ucMenuSelect )
                {
                    ucI = ucMenuCurrPage*OSMENU_PAGEMENUITEM+ucMenuSelect;
                    
                    Os__display_qt(0x30, 1 + ucMenuSelect, 0, pMENU[ucI].pucText, WIDGET_BUTTON, 1 + ucMenuSelect, NULL);
                    ucMenuSelect = ucMenuNewSelect;
                    ucI = ucMenuCurrPage*OSMENU_PAGEMENUITEM+ucMenuSelect;
                    
                    Os__display_qt(0x30, 1 + ucMenuSelect, 0, pMENU[ucI].pucText, WIDGET_BUTTON, 1 + ucMenuSelect, NULL);
                }
            }else
            {
                ucMenuSelect = ucMenuNewSelect;
            }

            if ((ucKey >= 1) && (ucKey <= 9))
            {
                /*2010-6-29 */

                int select_key = ucKey + ucMenuCurrPage * OSMENU_PAGEMENUITEM;

                if (select_key > ucMenuCount || ucKey > OSMENU_PAGEMENUITEM )
                {
                    ucExitFlag = 0;
                }else
                {
                    if (pMENU[ucMenuCurrCount+ucMenuSelect].pfnProc)
                    {
                        ieventid = (*(pMENU[ucMenuCurrCount+ucMenuSelect].pfnProc))();
                    }
                    ucExitFlag = 1;
                }
            }
        }while( !ucExitFlag );
        if (ieventid == OSEVENTID_PAMLOAD)
        {
            return(ieventid);
        }
    }while(1);
}

/////////////////////////END //////////////////////////////////////////////
