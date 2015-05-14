#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>      // open() close()
#include <unistd.h>     // read() write()
#include <stdio.h>
#include <string.h>
#include </usr/include/unistd.h>  //usleep()
#include <errno.h>

#define DEVICE_NAME_ADC "/dev/adc"

#define ADC_CHANNEL_1 	1	/*ADC_CHANNEL is between 0 to 7*/
#define CHARGE_DETECT 	8
#define CHARGE_ON	0x00
#define CHARGE_OFF	(1 << 16)
#define ADC_DETECT	0
#define VOLTAGE_DETECT	0
#define CHARGE_STATE_DETECT 1
#define SUCCESS		0x00

int	fd_adc;

int sdl_adc_init()
{
	int	tmp;
	int	detect_result;

	fd_adc  = open(DEVICE_NAME_ADC,O_RDWR);        
 	if(fd_adc == -1){	
		perror("Open the adc driver unsuccessfully\n");				/*open the adc driver unsuccessfully!*/
		return (-1);
	}
	return SUCCESS;
}


#define sample_times  18

//保留上一次检测值
static unsigned int batt_old[sample_times];

int  battery_capability_detect (void)
{
	int	tmp;
	int	detect_result;
	unsigned int a[sample_times];
	int	i, j;
	int count=0;
	
	unsigned int *p;
	unsigned int sum;
	unsigned int positon;
	unsigned int max=0,min=0xFFFF;	
	
	if(batt_old[0] != 0)
	{
	    memcpy(a, &batt_old[(sample_times>>1)], (sample_times>>1)<<2);
	    positon = sample_times>>1;
	}
	else
	{
	    positon = 0;
	}
	
	for (i=positon; i<sample_times; i++){
retry:
		tmp = ioctl(fd_adc,	ADC_CHANNEL_1);
		if( tmp<0 && (count++ < 5)){
			usleep(30000);
			goto retry;
		}
		a[i] = tmp;
	}
	
    sum = 0;
    p = a;
    for(i=0; i<sample_times; i++)
    {
        if(max <= *p)
        {
            max = *p;
        }
        if(min >= *p)
        {
            min = *p;
        }
        sum += *p;                
        p++;
    }    
    
    sum -= (max + min); 	
	tmp = sum>>4;
	
	memcpy(batt_old, a, sample_times<<2);
	
//	printf("%s %d  vcc adc=%d\n", __func__, __LINE__, tmp); 
	
	return tmp;
}

/*************************************************
*Name：	charge_state_detect()
*Function：   detect the charging state of battery
*Parameters：	none
*Return Value：	
*	        -2——error data；
*		1——battery is charging；
*		0——battery is not charging.
*************************************************/
int charge_state_detect(void)
{
	int	detect_result;
	unsigned int charge_state;
	int count=0;
retry:		
	charge_state = ioctl(fd_adc, CHARGE_DETECT);
	if(charge_state<0 && (count++ < 5)){
			usleep(30000);
			goto retry;
	}
	switch (charge_state)
		{
			case CHARGE_ON:
					         detect_result = 1;			/*battery is charging!*/
					         break;

			case CHARGE_OFF:
					          detect_result = 0;			/*battery is not charging!*/
					          break;

			default:
					          detect_result = -2;			/*error data*/			
		}

	return detect_result;
}

/*************************************************
*Name：	battery_capability_detect_result()
*Function：   detect the capability of battery
*Parameters：none
*Return Value：	
*		4——battery has 80% capability at least；
*	   	3——battery has 60% capability at least；
*	        	2——battery has 40% capability at least；
*		1——battery has 20% capability at least；
*		0——battery has 20% capability at most.
*************************************************/
/* new value for V08 or later  */
#define BATTERY_ADC_4 612  //0x245		//8.3-7.95
#define BATTERY_ADC_3 585  //0x230		// 	~7.6
#define BATTERY_ADC_2 558  //0x215		//	~7.25
#define BATTERY_ADC_1 531  //0x200		//	~6.9

int  battery_capability_detect_result(void)
{
	int	i;
	int	result = 0;
	int	detect_result;
	static int	battery_detect_result_storage[30];
	static int	battery_detect_count = 0;
#if 0
	if (battery_detect_count <= 29){
		battery_detect_result_storage[battery_detect_count] =  battery_capability_detect();
		result = battery_detect_result_storage[battery_detect_count];
		battery_detect_count ++;
	}
	else {
		for (i = 0; i < 29; i++){
			battery_detect_result_storage[i] = battery_detect_result_storage[i+1];
		}
		battery_detect_result_storage[i] = battery_capability_detect();

		for (i = 0;i < 30; i++){
			result += battery_detect_result_storage[i];
		}
		result /= 30;
	}
	//Uart_Printf("result = %d\n", result);
#endif
	result = battery_capability_detect();
	printf("****adc %s****\n",result);
	if (result >= BATTERY_ADC_4) 		detect_result = 4;			/*battery_voltage >= about 7.95v*/
	else if (result >= BATTERY_ADC_3)	detect_result = 3;			/*battery_voltage >= about 7.6v*/
	else if (result >= BATTERY_ADC_2)	detect_result = 2;			/*battery_voltage >= about 7.25v*/
	else if (result >= BATTERY_ADC_1)	detect_result = 1;			/*battery_voltage >= about 6.9v*/
	else 					detect_result = 0;			/*battery_voltage < 	about 6.90v*/	

	return detect_result;
}


int adc_close()
{
	int 	result;

	result = close(fd_adc);
	return result;
}


/*************************************************
*Name：	 tf_detect()
*Function：   detect whether the TF-card is online
*Parameters：none
*Return Value：	
*		0x01——The tf-card is online；
*		0x00——The tf-card is offline.
*************************************************/
unsigned char tf_detect(void)
{
	if(access("/dev/mmcblk0p1",F_OK))
		return 0x00;
	else
		return 0x01;
}

/************************************
int main(void)
{
	unsigned char tf_detect_result;

	tf_detect_result = tf_detect();

	if (tf_detect_result == 0x00)
		Uart_Printf("No tf-card!\n");
	else
		Uart_Printf("Detect the tf-card!\n");

	return 0;
}
*************************************/
