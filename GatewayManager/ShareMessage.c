/*
 * ShareMessage.c
 *
 *  Created on: Oct 22, 2020
 *      Author: duanlc
 */


#include "../GatewayManager/ShareMessage.h"

int8_t 		vrsc_SHAREMESS_Send2GatewayMessage[OUTMESSAGE_MAXLENGTH];
uint16_t	vrui_SHAREMESS_Send2GatewayLength;
bool		vrb_SHAREMESS_Send2GatewayAvailabe;
pthread_mutex_t	vrpth_SHAREMESS_Send2GatewayLock = PTHREAD_MUTEX_INITIALIZER;
