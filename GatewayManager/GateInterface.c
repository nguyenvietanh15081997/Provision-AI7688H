/*
 * GateInterface.c
 *
 *  Created on: Oct 21, 2020
 *      Author: haha
 */

#include "../GatewayManager/GateInterface.h"

static ringbuffer_t 		vrts_ringbuffer_Data;
static mraa_uart_context	vrts_UARTContext;
static unsigned char 		vruc_GWIF_InUARTData;
static TS_GWIF_IncomingData	*vrts_GWIF_IncomeMessage;
static unsigned char		vrsc_GWIF_TempBuffer[TEMPBUFF_LENGTH] = {0};
static uint16_t				vrui_GWIF_LengthMeassge;
//static uint16_t				vrui_GWIF_LengthMeassgeSave;
static bool					vrb_GWIF_UpdateLate = false;
static bool					vrb_GWIF_CheckNow = false;
static bool					vrb_GWIF_RestartMessage = true;

uint8_t OUTMESSAGE_MACSelect[9]    = {0xE9, 0xFF, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t OUTMESSAGE_GetPro[3] 	   = {0xE9, 0xFF, 0x0C};
uint8_t OUTMESSAGE_Provision[28]   = {0};
uint8_t OUTMESSAGE_BindingALl[22]  = {0xe9, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x60, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48};
//static uint16_t unicast_address;


uint8_t flag_selectmac     = 0;
uint8_t flag_getpro_info   = 0;
uint8_t flag_getpro_element= 0;
uint8_t flag_provision     = 0;
uint8_t flag_mac           = 1;
uint8_t flag_scan          = 1;
uint8_t flag_done          = 1;
unsigned int timeout=0;


/*
 * Khoi tao chuong trinh giao tiep vooi Gateway bao gom:
 * - Khoi tao UART
 * - Khoi tao chuoi luu tru du lieu
 */
void GWIF_Init (void){
	ring_init(&vrts_ringbuffer_Data, RINGBUFFER_LEN, sizeof(uint8_t));
	vrts_UARTContext = mraa_uart_init(UART_INTERFACE);
	mraa_uart_set_baudrate(vrts_UARTContext, UART_BAUDRATE);
	mraa_uart_set_mode(vrts_UARTContext, UART_LENGTHDATA, MRAA_UART_PARITY_NONE, 1);
	mraa_uart_set_flowcontrol(vrts_UARTContext, 0, 0);

	// Dua con tro du lieu ve mang du lieu nhan ve
	vrts_GWIF_IncomeMessage = (TS_GWIF_IncomingData *)vrsc_GWIF_TempBuffer;
}


/*
 * Ham xu ly truyen ban tin tu cac tien trinh khac den Gateway
 * Hien tai dang cho phep xu ly toi da 1 ban tin den dung semaphore
 * TODO: Can nang cap len Semaphore ho tro nhieu ban tin den
 */
void GWIF_WriteMessage (void){
	if(pthread_mutex_trylock(&vrpth_SHAREMESS_Send2GatewayLock) == 0){
		if(vrb_SHAREMESS_Send2GatewayAvailabe == true){
			vrb_SHAREMESS_Send2GatewayAvailabe = false;
			mraa_uart_write(vrts_UARTContext, (const char *)vrsc_SHAREMESS_Send2GatewayMessage, vrui_SHAREMESS_Send2GatewayLength);
			printf("Put to Gateway\n");
		}
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
	}
}

/*
 * Doc du lieu tu bo dem UART va luu tru vao Ring Buffer cua he thong
 */
void GWIF_Read2Buffer (void){
	// Neu hang doi chua day Hoac Du lieu con trong UART thi dua du lieu do vao hang doi
	while((vrts_ringbuffer_Data.count < RINGBUFFER_LEN) && mraa_uart_data_available(vrts_UARTContext, 0)){
		mraa_uart_read(vrts_UARTContext,(char *)(&vruc_GWIF_InUARTData), 1);
		ring_push_head(&vrts_ringbuffer_Data, (void*)&vruc_GWIF_InUARTData);
	}
}

/*
 * Kiem tra tinh xac thuc cua du lieu den
 * Neu du lieu dung format thi lay ra so byte tuong ung va gui sang tien trinh xu ly
 * Neu chua dung thi tiep tuc dich du lieu va tim den doan du lieu dung format
 * REVIEW: Kiem tra lai noi dung ham nay
 */
void GWIF_CheckData (void){
	unsigned int vrui_Count;

	// Neu co du lieu trong Buffer
	if(vrts_ringbuffer_Data.count >= 1){
		timeout=0;
		if(vrb_GWIF_UpdateLate == false){

			// Doc du lieu vao Buffer roi chuyen du lieu di tiep
			if(vrb_GWIF_RestartMessage == true){
				if(vrts_ringbuffer_Data.count >= 3){
					ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[0]);
					ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[1]);
					ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[2]);
					vrb_GWIF_RestartMessage = false;
					vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
				}
			}
			else{
				ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[MESSAGE_HEADLENGTH - 1]);
				vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
			}

//			printf("Checking...\n");
//			printf("\tLength:%d\n",vrui_GWIF_LengthMeassge);
//			printf("\tTSCRIPT:%2x\n",vrts_GWIF_IncomeMessage->Opcode);
//			printf("\n");



			if((vrui_GWIF_LengthMeassge) >= MESSAGE_HEADLENGTH){
				if(	(vrts_GWIF_IncomeMessage->Opcode == MESSAGE_OPCODE_01)  || \
					(vrts_GWIF_IncomeMessage->Opcode == MESSAGE_OPCODE_02)  || \
					(vrts_GWIF_IncomeMessage->Opcode == MESSAGE_OPCODE_03)  || \
					(vrts_GWIF_IncomeMessage->Opcode == MESSAGE_OPCODE_04)  || \
					(vrts_GWIF_IncomeMessage->Opcode == MESSAGE_OPCODE_05) ){
					// Truong hop dung format ban tin
					if(vrts_ringbuffer_Data.count >= (vrui_GWIF_LengthMeassge - 1)){
						for(vrui_Count = 0; vrui_Count < (vrui_GWIF_LengthMeassge - 1); vrui_Count++){
							ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[MESSAGE_HEADLENGTH + vrui_Count]);
						}
						// Ban tin da trung khop cau truc, bat co xu ly ban tin
						vrb_GWIF_UpdateLate = false;
						vrb_GWIF_CheckNow = true;
						vrb_GWIF_RestartMessage = true;
						//GWIF_ProcessData();
					}
					else{
						// Neu du lieu den chua duoc chua trong Buffer, tien hanh quet vao lan sau
						vrb_GWIF_UpdateLate = true;
					}
				}
				else{
					// Truong hop khong dung format ban tin
					// dich di 1 Byte va tiep tuc kiem tra
					vrsc_GWIF_TempBuffer[0] = vrsc_GWIF_TempBuffer[1];
					vrsc_GWIF_TempBuffer[1] = vrsc_GWIF_TempBuffer[2];
					vrb_GWIF_RestartMessage = false;
					vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
				}
			}
		}
		else{
			if(vrts_ringbuffer_Data.count >= (vrui_GWIF_LengthMeassge - 1)){
				for(vrui_Count = 0; vrui_Count < (vrui_GWIF_LengthMeassge - 1); vrui_Count++){
					ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[MESSAGE_HEADLENGTH + vrui_Count]);
				}
				vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
				// Ban tin da trung khop cau truc, bat co xu ly ban tin
				vrb_GWIF_UpdateLate = false;
				vrb_GWIF_CheckNow = true;
				vrb_GWIF_RestartMessage = true;
				//GWIF_ProcessData();
			}
		}

	}
	else
	{
		timeout++;
	}

}

/*
 * Ham xu ly du lieu den sau khi da duoc kiem tra
 * Hien tai dung lai o xu ly ban len Terminal
 * TODO: Thuc hien boc ban tin theo cac nhom lenh tuong ung, hoan thien chuan xu ly ban tin den
 */
void GWIF_ProcessData (void){
	unsigned int vrui_Count;
	unsigned int i;
	if(vrb_GWIF_CheckNow){
		vrb_GWIF_CheckNow = false;
		//if(vrts_GWIF_IncomeMessage->Message[0] != 0x88){
			printf("A coming message:\n");
			printf("\tLength:%d\n",vrui_GWIF_LengthMeassge);
			printf("\tTSCRIPT:0x%2x\n",vrts_GWIF_IncomeMessage->Opcode);
			printf("\tMessage:");
			for (vrui_Count = 0; vrui_Count < vrui_GWIF_LengthMeassge-1; vrui_Count++){
				printf("%2x-",vrts_GWIF_IncomeMessage->Message[vrui_Count]);
			}
			printf("\n");

		if(vrts_GWIF_IncomeMessage->Message[0] == 0x88){
			for(i=0; i<6; i++)
			{
				OUTMESSAGE_MACSelect[i+3]=vrts_GWIF_IncomeMessage->Message[i+1];
			}
			flag_selectmac=1;
		}

		if(vrts_GWIF_IncomeMessage->Message[0] == 0x8b){

			OUTMESSAGE_Provision[0]=0xE9;
			OUTMESSAGE_Provision[1]=0xFF;
			OUTMESSAGE_Provision[2]=0x0a;
			//unicast_address = (vrts_GWIF_IncomeMessage->Message[25]) | (vrts_GWIF_IncomeMessage->Message[26]<<8);
			for (i=0;i<25;i++)
			{
				OUTMESSAGE_Provision[i+3]=vrts_GWIF_IncomeMessage->Message[i+2];
			}
//			OUTMESSAGE_Provision[26]=(unicast_address-1);
//			OUTMESSAGE_Provision[27]=(unicast_address-1)>>8;
			flag_getpro_info=1;
		}

		if(vrts_GWIF_IncomeMessage->Message[0] == 0x8c)
		{
			flag_getpro_element=1;
		}

		if(vrts_GWIF_IncomeMessage->Message[0] == 0x89 && vrts_GWIF_IncomeMessage->Message[1] == 0x01)
		{
			flag_provision =1;
		}

		if(vrts_GWIF_IncomeMessage->Message[0] == 0x8a && vrts_GWIF_IncomeMessage->Message[1] == 0x01)
		{
			//flag_mac=1;
			flag_scan=1;
		}
		if(vrts_GWIF_IncomeMessage->Message[0] == 0x82)
		{
			flag_done=1;
			flag_mac=1;
		}
	}
}


/*
 * Tien trinh xu ly ban tin giao tiep Gateway bao gom
 * - Nhan ban tin ve
 * - Xu ly ban tin ve
 * - Truyen du lieu tu tac vu khac den Gateway
 * TODO: Tiep tuc kiem tra va hoan thien
 */
void *GWINF_Thread(void *vargp)
{
	GWIF_Init();
	while(1){
		GWIF_WriteMessage();
		GWIF_Read2Buffer();
		GWIF_CheckData();
		GWIF_ProcessData();
		usleep(200);
	}
    return NULL;
}
