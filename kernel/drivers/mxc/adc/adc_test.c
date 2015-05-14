/*
 * adc_test.c
 *
 *  Created on: 2012-7-28
 *      Author: leon
 */
#include <stdio.h>
#include <linux/ioctl.h>
#include <linux/imx_adc.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define DEVICE_NAME_ADC  "/dev/imx_adc"
#define IPS420_ADC_CHANNEL_0    _IOWR('p',0xb5, int)
#define IPS420_ADC_CHANNEL_1    _IOWR('p',0xb6, int)
#define IPS420_ADC_CHANNEL_2    _IOWR('p',0xb7, int)

int main(void)
{
    int fd_adc = 0;
	int tmp;
	int i, j;
	int value = 0;
	int count = 0;
	fd_adc = open(DEVICE_NAME_ADC, O_RDWR);

    while(1)
    {
	tmp = ioctl(fd_adc, IPS420_ADC_CHANNEL_0, &value);
	printf("\r\nADC  Value:0x%x\r\n", value);

	tmp = ioctl(fd_adc, IPS420_ADC_CHANNEL_1, &value);
	printf("\r\nADC  Value:0x%x\r\n", value);

	tmp = ioctl(fd_adc, IPS420_ADC_CHANNEL_2, &value);
	printf("\r\nADC  Value:0x%x\r\n", value);
    }
    close(fd_adc);
}
