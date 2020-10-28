/*
 * GateInterface.h
 *
 *  Created on: Oct 21, 2020
 *      Author: duanlc
 */

#ifndef GATEWAYMANAGER_GATEINTERFACE_H_
#define GATEWAYMANAGER_GATEINTERFACE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <mraa.h>

#include "RingBuffer.h"
#include "ShareMessage.h"


#define RINGBUFFER_LEN 		(1024)

#define UART_INTERFACE		(1)
#define UART_BAUDRATE		(115200)
#define UART_LENGTHDATA		(8)

#define TEMPBUFF_LENGTH		(64)
#define MESSAGE_MAXLENGTH	(61)
#define MESSAGE_HEADLENGTH	(3)


#define MESSAGE_OPCODE_01	(0x80)
#define MESSAGE_OPCODE_02	(0x90)
#define MESSAGE_OPCODE_03	(0x91)
#define MESSAGE_OPCODE_04	(0xFA)
#define MESSAGE_OPCODE_05	(0x92)


typedef struct IncomingData{
	uint8_t 	Length[2];
	uint8_t 	Opcode;
	uint8_t		Message[MESSAGE_MAXLENGTH];
} TS_GWIF_IncomingData;


extern uint8_t  OUTMESSAGE_MACSelect[9] ;
extern uint8_t  OUTMESSAGE_GetPro[3] ;
extern uint8_t  OUTMESSAGE_BindingALl[22];
extern uint8_t  OUTMESSAGE_Provision[28];


extern uint8_t flag_selectmac;
extern uint8_t flag_getpro_info;
extern uint8_t flag_getpro_element;
extern uint8_t flag_provision;
extern uint8_t flag_mac;
extern uint8_t flag_scan;
extern uint8_t flag_done;

extern unsigned int timeout;

void GWIF_Init (void);
void GWIF_WriteMessage (void);
void GWIF_Read2Buffer (void);
void GWIF_CheckData (void);
void GWIF_ProcessData (void);
void *GWINF_Thread(void *vargp);

#ifdef __cplusplus
}
#endif



#endif /* GATEWAYMANAGER_GATEINTERFACE_H_ */
