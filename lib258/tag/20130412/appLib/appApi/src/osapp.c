#define OSAPPDATA
#include "sand_incs.h"

/* Filename: dl_call.c */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int call_so(void)
{
    void *handle;	/* shared library 的 'handle' 指標 */
    double (*manager)(void *);   /* 指向 shared library 裡的函數 */
    char *error;	/* 記錄 dynamic loader 的錯誤訊息 */
    char *file, i=2;
    file="./libapp1.so";
    while (i--)
    {


        /* 開啟 shared library 'libm' */
        handle = dlopen (file, RTLD_LAZY);
        if (!handle)
        {
            fprintf (stderr, "%s\n", dlerror());
            exit(1);
        }

        dlerror();    /* Clear any existing error */

        /* 在 handle 指向的 shared library 裡找到 "cos" 函數,
         * 並傳回他的 memory address
         */
        manager = dlsym(handle, "manager");
        if ((error = dlerror()) != NULL)
        {
            fprintf (stderr, "%s\n", error);
            exit(1);
        }

        /* indirect function call (函數指標呼叫),
         * 呼叫所指定的函數
         */
        (*manager)("helloWWW");

        dlclose(handle);

        file = "./libapp2.so";
    }
    return 0;
}




unsigned char OSAPP_Main(void)
{
    ucOSCurrAppID = 0;
    ucOSMasterAppID = 0;

    OSAPP_List();

    if ( OSAPP_Load() )
    {
        return(OSERR_SUCCESS);
    }

    OSAPP_ConvOrder(aucBuf);
    if ( !ucOSAppNB )
    {
        OSAPP_Maintence();
        return(OSERR_SUCCESS);
    }
}



