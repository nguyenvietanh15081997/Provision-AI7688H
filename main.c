/*
 ============================================================================
 Name        : GatewayManager.c
 Author      : haha
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "GatewayManager/GateInterface.h"

pthread_t vrts_System_ThreadGWIF;
pthread_t vrts_System_TestSend;

 static uint8_t OUTMESSAGE_ScanStart[3] = {0xE9, 0xFF, 0x00};

/*
 * Ham thu nghiem Scan Network de xu ly ban tin ve tu Gateway
 * TODO: Hoan thien va loai bo
 */

void *TestSendThread (void *vargp){
while(1)
{
	if(((flag_scan == 1) && (flag_done == 1)) || (timeout == 48000))
	{
		flag_scan=0;
		timeout=0;
		sleep(3);
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 3;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_ScanStart, vrui_SHAREMESS_Send2GatewayLength);
		printf ("scan\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
	}
	if((flag_selectmac==1) && (flag_mac==1) )
	{
		flag_selectmac=0;
		flag_mac=0;
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength =9;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_MACSelect, vrui_SHAREMESS_Send2GatewayLength);
		printf("Select mac\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
		sleep(2);

		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 3;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_GetPro, vrui_SHAREMESS_Send2GatewayLength);
		printf ("getpro\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
		sleep(1);
	}
	if(flag_getpro_info == 1 && flag_getpro_element == 1)
	{
		flag_getpro_info=0;
		flag_getpro_element=0;
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 28;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_Provision, vrui_SHAREMESS_Send2GatewayLength);
		printf ("provision\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
	}

	if(flag_provision == 1)
	{
		flag_provision=0;
		flag_done=0;
		sleep(2);
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 22;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_BindingALl, vrui_SHAREMESS_Send2GatewayLength);
		printf ("binding all\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
	}
}
	return NULL;
}



/*
 * Ham xu ly chinh. Thuc thi:
 * - Goi cac tac vu tuong ung cua chuong trinh
 * PENDING: Tiep tuc bo sung them cac Task khac
 */
int main(void) {
	puts("--------------Start system123!\n-----------------");
	flag_scan=1;
	flag_mac=1;
	flag_done=1;
	pthread_create(&vrts_System_ThreadGWIF, NULL, GWINF_Thread, NULL);
	pthread_create(&vrts_System_TestSend, NULL, TestSendThread, NULL);
	pthread_join(vrts_System_ThreadGWIF, NULL);
	pthread_join(vrts_System_TestSend, NULL);
	while(1){
	}
	return EXIT_SUCCESS;
}

