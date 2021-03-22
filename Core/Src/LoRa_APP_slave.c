/*
 * LoRa_App_slave.c V2.0
 *
 *  Created on: Mar 23, 2021
 *      Author: Wayne Wu
 *
 *20200928----V1.1
 *Add Sleep ,WakeUp and CheckJoinStatus Function.
 *20210311----V1.2
 *Add setup option: MaxEIRP, Tx power, Channel, Bandwidth.
 *Improve Frequency setup Function. Send data false by leave APB Mode issue.
 *20210322----2.0
 *Add all responses process flow.
 *Add Hardware Reset function in wake-up step by chip crash.
 *Add receive flag by loRaWaN in A class mode.
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

/* Private typedef -----------------------------------------------------------*/
USART_LoRa USARTLoRa;
LoRa_ConfigureStatusTypeDef LoRaCFStatus=LoRa_CF_UnConfig;
TIM_HandleTypeDef htim2;

/* Private variables ---------------------------------------------------------*/

void LoRa_USART(UART_HandleTypeDef *huart)
{
    while(USARTLoRa.ResStatus!=LoRa_Sleep)
    {
		switch(USARTLoRa.Status)
	    {
			case WakeUp:
				WakeUpTrigger(huart);
				if(USARTLoRa.ResStatus!=LoRa_HDRest)
					USARTLoRa.Status=CheckJoinST;
				else
					USARTLoRa.ResStatus=LoRa_OK;
				break;

			case CheckJoinST:
				USARTLoRa.ResStatus=CheckJoinStatus(huart);
				if(USARTLoRa.ResStatus==LoRa_joined)
				 	USARTLoRa.Status=SendData;
				else
					USARTLoRa.Status=Init;
				break;

	    	case Init:
	    		//-------Reset Factor--------
	    		LoRaFactorReset(huart);
	    		//-------Reset chip--------
	    		LoRaChipReset(huart);
	    		USARTLoRa.ResStatus=LoRaInit(huart);
	    		if(USARTLoRa.ResStatus==LoRa_OK)
	    		{
	    			LoRaChipReset(huart);
	    			USARTLoRa.Status=JoinABP;
	    		}
	    		break;

	    	case JoinABP:
	    		USARTLoRa.ResStatus=ActiveABPMode(huart);
	    		if(USARTLoRa.ResStatus==LoRa_OK)
	    			USARTLoRa.Status=SendData;
	    		else if(USARTLoRa.ResStatus==LoRa_Join_keys_not_init || USARTLoRa.ResStatus==LoRa_unexpected_Err)
	    			//-------if ActiveABPMode fail, Reset chip--------
	    			USARTLoRa.Status=Init;
	    		break;

	    	case SendData:
	    		USARTLoRa.sendflag = 1;
            	if(USARTLoRa.sendflag ==1)
            	{
            		//------Data Format transfer int & float to hex------------------------------------------------------------------
					//transfer data to string array:DataStrBuffer

            		//***********Put the Sensor Data Here to Transmit by LoRa*****************
            		//TODO:--Wayne20200723
            		sprintf(USARTLoRa.DataStrBuffer,"%.3f,%.3f,%.3f", Zstatistic_value.Statistic_FreqOvall,
            		            				Zstatistic_value.Statistic_p2p, Zstatistic_value.Statistic_SpeedOvall);
            		//sprintf(USARTLoRa.DataStrBuffer,"1234567890");
            		//************************************************************************

            		//transfer data to hex array:DataHexBuffer
            		USARTLoRa.DataStrSize = min(Uart_Buffer_Size, strlen(USARTLoRa.DataStrBuffer));
            		static char toHexn, n;
            		memset( USARTLoRa.DataHexBuffer, 0, strlen(USARTLoRa.DataHexBuffer) ); //clear DataHexBuffer
					for(toHexn=0; toHexn < USARTLoRa.DataStrSize; toHexn++)
					{
						memset( USARTLoRa.DataHextemp, 0, strlen(USARTLoRa.DataHextemp) ); //clear DataHextemp
						sprintf(USARTLoRa.DataHextemp,"%x",USARTLoRa.DataStrBuffer[toHexn]);
						USARTLoRa.DataHexBuffer[n]=USARTLoRa.DataHextemp[0];
						USARTLoRa.DataHexBuffer[n+1]=USARTLoRa.DataHextemp[1];
						n+=2;
					}
					n = 0;
					//--------------------------------------------------------------------------------------------------------------------
					//transmit LED status
					//HAL_GPIO_WritePin(RLED_Port, RLED , GPIO_PIN_SET);
					USARTLoRa.ResStatus=LoRaTransmitData(huart);
	    			if(USARTLoRa.ResStatus==LoRa_OK)
	    				USARTLoRa.Status = waitTxRes;
	    			else if(USARTLoRa.ResStatus==LoRa_Tx_not_joined)
	    				USARTLoRa.Status = JoinABP;
	    			else if(USARTLoRa.ResStatus==LoRa_unexpected_Err)
	    				USARTLoRa.Status = Init;

            	}
	    		break;

	    	case waitTxRes:
	    		USARTLoRa.ResStatus=Check_Tx2stResMsg();
	    		if(USARTLoRa.ResStatus == LoRa_OK	|| USARTLoRa.ResStatus == LoRa_Tx2_RxReceived)
	    		{
	    			//transmit LED status
	    			//HAL_GPIO_WritePin(RLED_Port, RLED , GPIO_PIN_RESET);
	    			USARTLoRa.sendflag = 0;
	    			USARTLoRa.Status = EnterSleepMode;
	    		}
	    		else if( USARTLoRa.ResStatus == LoRa_Tx2_err)
	    			USARTLoRa.Status = SendData;
	    		else if( USARTLoRa.ResStatus == LoRa_unexpected_Err)
	    			USARTLoRa.Status = Init;
	    		break;

	    	case EnterSleepMode:
				USARTLoRa.ResStatus=EnterSleep(huart);
				if(USARTLoRa.ResStatus==LoRa_Sleep)
					USARTLoRa.Status=WakeUp;
				else
					USARTLoRa.Status = Init;
				break;

	    	case CMDdebug:
	    		LoRaCommand(huart, "mac get_ch_count");
	    		while(!USARTLoRa.Revflag);
				USARTLoRa.Revflag = 0;
	    		USARTLoRa.ResStatus=LoRaResCheck("\n\r>> v1.6.5\n");
	    		if(USARTLoRa.ResStatus == LoRa_OK)
	    			__NOP();
	    		else
	    			__NOP();
				LoRaCommand(huart, "mac get_tx_confirm");
				while(!USARTLoRa.Revflag);
				USARTLoRa.Revflag = 0;
	    		USARTLoRa.ResStatus=LoRaResCheck("\n\r>> v1.6.5\n");
	    		if(USARTLoRa.ResStatus == LoRa_OK)
	    			__NOP();
	    		else
	    			__NOP();

	    }
	}
}



LoRa_StatusTypeDef LoRaInit(UART_HandleTypeDef *huart)
{
	//---------------Set 0~15 channel radio frequency-------------------------
	LoRaCFStatus=LoRa_CF_FEQ;
	char c;
	uint32_t ferqoffset;
	ferqoffset = RadioFreq;
	for(c=0;c<=15;c++)
	{
		char frequcmd[32];
		sprintf(frequcmd,"mac set_ch_freq %d %d",c, ferqoffset);
		LoRaCommand(huart, frequcmd);
		USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
		if(USARTLoRa.ResStatus!=LoRa_OK)
			return USARTLoRa.ResStatus;
		//Save configure
		USARTLoRa.ResStatus=LoRaSaveCF(huart);
		if(USARTLoRa.ResStatus != LoRa_OK)
			return USARTLoRa.ResStatus;
		//offset 200KHz
		ferqoffset = ferqoffset + 200000;
		if(c == Gateway_Channel-1)
			ferqoffset = RadioFreq;
	}

	//----------------Set Channel Bandwidth------------------------------------
	char ChBWcmd[32];
	sprintf(ChBWcmd,"mac set_ch_count %d %d", Channel, BandWidth);
	LoRaCommand(huart, ChBWcmd);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	//----------------Set Data rate(SF)--------------------------------------
	LoRaCommand(huart, CF_SFDataRate);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	//----------------Turn off auto Data rate--------------------------------------
	LoRaCommand(huart, ADRoff);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;
	//----------------Set MaxEIRP--------------------------------------
	LoRaCommand(huart, CF_MaxEIRP);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	//----------------Set Transmit Power--------------------------------------
	char txpowercmd[32];
	sprintf(txpowercmd,"mac set_power %d", TxPower);
	LoRaCommand(huart, txpowercmd);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	//----------------Off Tx Confirm--------------------------------------
	LoRaCommand(huart, Offtxconfirm);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	//----------------Set Device EUI--------------------------------------
	LoRaCommand(huart, CF_DevEUI);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	//----------------Set Application EUI--------------------------------------
	LoRaCommand(huart, CF_AppEUI);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	//----------------Set Device Address--------------------------------------
	LoRaCommand(huart, CF_DevAddr);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	//----------------Set Network Session Key--------------------------------------
	LoRaCommand(huart, CF_NwksKey);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	//----------------Set Application Session Key--------------------------------------
	LoRaCommand(huart, CF_AppsKey);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus!=LoRa_OK)
		return USARTLoRa.ResStatus;
	//Save configure
	USARTLoRa.ResStatus=LoRaSaveCF(huart);
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

}

LoRa_StatusTypeDef ActiveABPMode(UART_HandleTypeDef *huart)
{
	LoRaCommand(huart, ActiveABP);
	USARTLoRa.ResStatus=Check_JoinResMsg();
	if(USARTLoRa.ResStatus == LoRa_OK)
	{
		USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> accepted\n", "\n\r>> unsuccess\n");
		if(USARTLoRa.ResStatus != LoRa_OK)
			return USARTLoRa.ResStatus;
	}
	else
		return USARTLoRa.ResStatus;
}

LoRa_StatusTypeDef LoRaTransmitData(UART_HandleTypeDef *huart)
{
	char RoLaTxCMD[Uart_Buffer_Size];
    sprintf(RoLaTxCMD,"%s%s",SendUcfP1, USARTLoRa.DataHexBuffer);
	LoRaCommand(huart, RoLaTxCMD);
	USARTLoRa.ResStatus=Check_Tx1stResMsg();
	if(USARTLoRa.ResStatus == LoRa_OK)
		return LoRa_OK;
	else
		return USARTLoRa.ResStatus;
}



void LoRaCommand(UART_HandleTypeDef *huart,  uint32_t *cmd)
{
	memset(USARTLoRa.buffer, 0, strlen(USARTLoRa.buffer) ); //clear usart buffer
	sprintf(USARTLoRa.buffer,cmd);
	USARTLoRa.bufferSize = min(Uart_Buffer_Size, strlen(USARTLoRa.buffer));
	USARTLoRa.sendTimeout = 50 ;
	/**
	* @param huart   UART handle.
	* @param pData   Pointer to data buffer.
	* @param Size    Amount of data to be received.
	* @param Timeout Timeout duration.(ms)
	*/
	while(HAL_UART_Transmit(huart, &USARTLoRa.buffer, USARTLoRa.bufferSize, USARTLoRa.sendTimeout )!=HAL_OK);

}
LoRa_StatusTypeDef LoRaResCheck(uint32_t *Res)
{

	//while(!USARTLoRa.Revflag);
	memset(USARTLoRa.Res_Check, 0, strlen(USARTLoRa.Res_Check) ); //clear LoRa response check string
	sprintf(USARTLoRa.Res_Check,Res);
	//USARTLoRa.Revflag=0;

	memset(USARTLoRa.RevDatabk, 0, strlen(USARTLoRa.RevDatabk) ); //clear LoRa response check string
	sprintf(USARTLoRa.RevDatabk, USARTLoRa.RevData);

	if(strcmp( USARTLoRa.RevData, USARTLoRa.Res_Check ) == 0)
	{
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		return LoRa_OK;
	}
	else
	{
		return LoRa_Res_Err;
	}
}


LoRa_StatusTypeDef Check_2ResMsg(uint32_t *Res1, uint32_t *Res2)
{
	//wait data Response
	while(!USARTLoRa.Revflag);
	USARTLoRa.Revflag=0;
	if((USARTLoRa.ResStatus=LoRaResCheck(Res1))==LoRa_OK)
	{
		if(Res1 =="\n\r>> joined\n")
			return LoRa_joined;
		else
			return LoRa_OK;

	}
	else if((USARTLoRa.ResStatus=LoRaResCheck(Res2))==LoRa_OK)
	{
		if(Res2 =="\n\r>> unjoined\n")
			return LoRa_unjoined;
		else if(Res2 =="\n\r>> unsuccess\n")
			return LoRa_Join_FAIL;
		else
			return LoRa_Res_Err;
	}

	else
	{
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		return LoRa_unexpected_Err;
	}
}

LoRa_StatusTypeDef Check_JoinResMsg()
{
	//wait data Response
	while(!USARTLoRa.Revflag);
	USARTLoRa.Revflag=0;
	if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n"))==LoRa_OK)
		return LoRa_OK;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Invalid\n"))==LoRa_OK)
		return LoRa_Join_Invalid;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> keys_not_init\n"))==LoRa_OK)
		return LoRa_Join_keys_not_init;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> no_free_ch\n"))==LoRa_OK)
		return LoRa_Join_no_free_ch;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> busy\n"))==LoRa_OK)
		return LoRa_Join_busy;
	else
	{
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		return LoRa_unexpected_Err;
	}
}

LoRa_StatusTypeDef Check_Tx1stResMsg()
{
	//wait data Response
	while(!USARTLoRa.Revflag);
	USARTLoRa.Revflag=0;
	if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n"))==LoRa_OK)
		return LoRa_OK;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Invalid\n"))==LoRa_OK)
		return LoRa_Tx_Invalid;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> not_joined\n"))==LoRa_OK)
		return LoRa_Tx_not_joined;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> no_free_ch\n"))==LoRa_OK)
		return LoRa_Tx_no_free_ch;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> busy\n"))==LoRa_OK)
		return LoRa_Tx_busy;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> invalid_data_length\n"))==LoRa_OK)
		return LoRa_Tx_invalid_data_length;
	else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> exceeded_data_length\n"))==LoRa_OK)
		return LoRa_Tx_exceeded_data_length;

	else
	{
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		return LoRa_unexpected_Err;
	}
}

LoRa_StatusTypeDef Check_Tx2stResMsg()
{
	if(USARTLoRa.Revflag)
	{
		USARTLoRa.Revflag=0;
		if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> tx_ok\n"))==LoRa_OK)
			return LoRa_OK;
		else if((USARTLoRa.ResStatus=LoRaResCheck("\n\r>> err\n"))==LoRa_OK)
			return LoRa_Tx2_err;
		else if(USARTLoRa.LoRaRxflag == 1)
		{
			USARTLoRa.LoRaRxflag = 0;
			memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
			return LoRa_Tx2_RxReceived;
		}
		else
		{
			memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
			return LoRa_unexpected_Err;
		}

	}
	else
		return LoRa_TxData_WaitRes;
}

void LoRaChipReset(UART_HandleTypeDef *huart)
{
	//USARTLoRa.ResetRevflag=1;
	LoRaCommand(huart, LoRaReset);
	//receive reset response
	memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
	while(!USARTLoRa.Revflag);
	USARTLoRa.Revflag=0;
	USARTLoRa.ResetRevflag=0;
	memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
}

void LoRaFactorReset(UART_HandleTypeDef *huart)
{
	//USARTLoRa.ResetRevflag=1;
	LoRaCommand(huart, LoRaFacReset);
	//receive reset response
	memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
	while(!USARTLoRa.Revflag);
	USARTLoRa.Revflag=0;
	USARTLoRa.ResetRevflag=0;
	memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
}

LoRa_StatusTypeDef WakeUpTrigger(UART_HandleTypeDef *huart)
{
	LoRaCommand(huart, WKUPtigger);
	//wait data Response
	while(!USARTLoRa.Revflag)
	{
		LoRachipCrashDetect_HDRST(&htim2);
		if(USARTLoRa.ResStatus==LoRa_HDRest)
			return USARTLoRa.ResStatus;
	}
	LoRachipCrashDetect_Stop(&htim2);
	USARTLoRa.Revflag = 0;
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Unknown command!\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
	{
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		if(USARTLoRa.ResetRevflag)
		{
			USARTLoRa.ResetRevflag = 0;
			return LoRa_PowerUp;
		}
		else
			return LoRa_WaKeUp;
	}
	else
		return LoRa_OK;
}

LoRa_StatusTypeDef CheckJoinStatus(UART_HandleTypeDef *huart)
{
	LoRaCommand(huart, CkJoinSt);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> joined\n", "\n\r>> unjoined\n");
	return USARTLoRa.ResStatus;

}

LoRa_StatusTypeDef EnterSleep(UART_HandleTypeDef *huart)
{
	LoRaCommand(huart, Sleep4200s);

	//wait data Response
	while(!USARTLoRa.Revflag);
	USARTLoRa.Revflag = 0;
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> sleep 4200 sec uart_on\n");
	if(USARTLoRa.ResStatus == LoRa_OK)
	{
		return LoRa_Sleep;
	}
	else
	{
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		return LoRa_unexpected_Err;
	}
}

LoRa_StatusTypeDef LoRaSaveCF(UART_HandleTypeDef *huart)
{
	LoRaCommand(huart, LoRaSave);
	USARTLoRa.ResStatus=Check_2ResMsg("\n\r>> Ok\n", "\n\r>> Invalid\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return USARTLoRa.ResStatus;

	LoRaCommand(huart, LoRaSave);
	//wait data Response
	while(!USARTLoRa.Revflag);
	USARTLoRa.Revflag = 0;
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus == LoRa_OK)
		return LoRa_OK;
	else
	{
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		return LoRa_unexpected_Err;
	}
}
LoRa_StatusTypeDef LoRachipCrashDetect_HDRST(TIM_HandleTypeDef *htim)
{
	//ChipCrashDetectTimer start
	if(USARTLoRa.CrashDetectStartflag == 0)
	{
		HAL_TIM_Base_Start_IT(&htim2);
		USARTLoRa.CrashDetectStartflag = 1;
	}
	//if LoRa chip no response than 100ms, Hardware Reset the chip
	if(USARTLoRa.CrashTimerCount>=100)
	{
		LoRachipCrashDetect_Stop(&htim);
		//Hardware Reset
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_RESET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET);

		//receive reset response
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		while(!USARTLoRa.Revflag);
		USARTLoRa.Revflag=0;
		USARTLoRa.ResetRevflag=0;
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		USARTLoRa.ResStatus = LoRa_HDRest;
		return USARTLoRa.ResStatus;
	}
}
void LoRachipCrashDetect_Stop(TIM_HandleTypeDef *htim)
{
	USARTLoRa.CrashTimerCount=0;
	HAL_TIM_Base_Stop_IT(&htim2);
	USARTLoRa.CrashDetectStartflag = 0;
}
