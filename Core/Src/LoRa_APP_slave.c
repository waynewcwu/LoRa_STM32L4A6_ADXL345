/*
 * LoRa_App_slave.c V1.1
 *
 *  Created on: Sep 28, 2020
 *      Author: Wayne Wu
 *
 *20200928----V1.1
 *Add Sleep ,WakeUp and CheckJoinStatus Function.
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

/* Private typedef -----------------------------------------------------------*/
USART_LoRa USARTLoRa;

/* Private variables ---------------------------------------------------------*/

void LoRa_USART(UART_HandleTypeDef *huart)
{
    while(USARTLoRa.ResStatus!=LoRa_Sleep)
    {
		switch(USARTLoRa.Status)
	    {
			case WakeUp:
				WakeUpTrigger(huart);
				USARTLoRa.Status=CheckJoinST;
				break;

			case CheckJoinST:
				USARTLoRa.ResStatus=CheckJoinStatus(huart);
				if(USARTLoRa.ResStatus==LoRa_joined)
					USARTLoRa.Status=SendData;
				else
					USARTLoRa.Status=Init;
				break;

	    	case Init:
	    		//-------Reset chip--------
	    		LoRaChipReset(huart);
	    		USARTLoRa.ResStatus=LoRaInit(huart);
	    		if(USARTLoRa.ResStatus==LoRa_OK)
	    		{
	    			LoRaChipReset(huart);
	    			USARTLoRa.Status=JoinABP;
	    		}
	    		else
	    			//-------if initial fail, Reset chip-------------
	    			//LoRaChipReset(huart);
	    		break;

	    	case JoinABP:
	    		USARTLoRa.ResStatus=ActiveABPMode(huart);
	    		if(USARTLoRa.ResStatus==LoRa_OK)
	    			USARTLoRa.Status=SendData;
	    		else
	    		{
	    			//-------if ActiveABPMode fail, Reset chip--------
	    			LoRaChipReset(huart);
	    			USARTLoRa.Status=Init;
	    		}
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

            	}
	    		break;

	    	case waitTxRes:
	    		USARTLoRa.ResStatus=LoRaTxResCheck("\n\r>> tx_ok\n");
	    		if(USARTLoRa.ResStatus == LoRa_OK)
	    		{
	    			//transmit LED status
	    			//HAL_GPIO_WritePin(RLED_Port, RLED , GPIO_PIN_RESET);
	    			USARTLoRa.sendflag = 0;
	    			USARTLoRa.Status = EnterSleepMode;
	    		}
	    		break;

	    	case EnterSleepMode:
				USARTLoRa.ResStatus=EnterSleep(huart);
				if(USARTLoRa.ResStatus==LoRa_Sleep)
					USARTLoRa.Status=WakeUp;
				break;
	    }
	}
}



LoRa_StatusTypeDef LoRaInit(UART_HandleTypeDef *huart)
{
	//---------------Set 0~15 channel radio frequency-------------------------
	char c;
	for(c=0;c<=15;c++)
	{
		char frequcmd[32];
		sprintf(frequcmd,"mac set_ch_freq %d %d",c, RadioFreq);
		LoRaCommand(huart, frequcmd);
		USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
		if(USARTLoRa.ResStatus != LoRa_OK)
			return LoRa_CF_FEQ_FAIL;
		//Save configure
		LoRaCommand(huart, LoRaSave);
		USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
		if(USARTLoRa.ResStatus != LoRa_OK)
			return LoRa_SAVE_FAIL;
	}

	//----------------Set Data rate(SF)--------------------------------------
	LoRaCommand(huart, CF_SFDataRate);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_CF_SF_FAIL;
	//Save configure
	LoRaCommand(huart, LoRaSave);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_SAVE_FAIL;

	//----------------Turn off auto Data rate--------------------------------------
		LoRaCommand(huart, ADRoff);
		USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
		if(USARTLoRa.ResStatus != LoRa_OK)
			return LoRa_CF_ADR_FAIL;
		//Save configure
		LoRaCommand(huart, LoRaSave);
		USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
		if(USARTLoRa.ResStatus != LoRa_OK)
			return LoRa_SAVE_FAIL;

	//----------------Set Device EUI--------------------------------------
	LoRaCommand(huart, CF_DevEUI);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_CF_DevEUI_FAIL;
	//Save configure
	LoRaCommand(huart, LoRaSave);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_SAVE_FAIL;

	//----------------Set Application EUI--------------------------------------
	LoRaCommand(huart, CF_AppEUI);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_CF_AppEUI_FAIL;
	//Save configure
	LoRaCommand(huart, LoRaSave);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_SAVE_FAIL;

	//----------------Set Device Address--------------------------------------
	LoRaCommand(huart, CF_DevAddr);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_CF_DevAddr_FAIL;
	//Save configure
	LoRaCommand(huart, LoRaSave);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_SAVE_FAIL;

	//----------------Set Network Session Key--------------------------------------
	LoRaCommand(huart, CF_NwksKey);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_CF_NwksKey_FAIL;
	//Save configure
	LoRaCommand(huart, LoRaSave);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_SAVE_FAIL;

	//----------------Set Application Session Key--------------------------------------
	LoRaCommand(huart, CF_AppsKey);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_CF_AppsKey_FAIL;
	//Save configure
	LoRaCommand(huart, LoRaSave);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
		return LoRa_SAVE_FAIL;

}

LoRa_StatusTypeDef ActiveABPMode(UART_HandleTypeDef *huart)
{
	LoRaCommand(huart, ActiveABP);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus == LoRa_OK)
	{
		USARTLoRa.ResStatus=LoRaResCheck("\n\r>> accepted\n");
		if(USARTLoRa.ResStatus != LoRa_OK)
			return LoRa_JoinABP_FAIL;
	}
	else
		return LoRa_ABPCMD_FAIL;
}

LoRa_StatusTypeDef LoRaTransmitData(UART_HandleTypeDef *huart)
{
	char RoLaTxCMD[Uart_Buffer_Size];
    sprintf(RoLaTxCMD,"%s%s",SendUcfP1, USARTLoRa.DataHexBuffer);
	LoRaCommand(huart, RoLaTxCMD);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Ok\n");
	if(USARTLoRa.ResStatus == LoRa_OK)
		return LoRa_OK;
	else
		return LoRa_TxCMD_FAIL;
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
	//memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
	while(!USARTLoRa.Revflag);
	memset(USARTLoRa.Res_Check, 0, strlen(USARTLoRa.Res_Check) ); //clear LoRa response check string
	sprintf(USARTLoRa.Res_Check,Res);
	USARTLoRa.Revflag=0;

	if(strcmp( USARTLoRa.RevData, USARTLoRa.Res_Check ) == 0)
	{
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		return LoRa_OK;

	}
	else
	{
		memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
		return LoRa_Res_Err;
	}
}

LoRa_StatusTypeDef LoRaTxResCheck(uint32_t *Res)
{
	if(USARTLoRa.Revflag)
	{
		memset(USARTLoRa.Res_Check, 0, strlen(USARTLoRa.Res_Check) ); //clear LoRa response check string
		sprintf(USARTLoRa.Res_Check,Res);
		USARTLoRa.Revflag=0;

		if(strcmp( USARTLoRa.RevData, USARTLoRa.Res_Check ) == 0)
		{
			memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
			return LoRa_OK;
		}
		else
		{
			memset(USARTLoRa.RevData, 0, strlen(USARTLoRa.RevData) ); //clear LoRa RevData
			return LoRa_TxData_FAIL;
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

LoRa_StatusTypeDef WakeUpTrigger(UART_HandleTypeDef *huart)
{
	LoRaCommand(huart, WKUPtigger);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> Unknown command!\n");
	if(USARTLoRa.ResStatus != LoRa_OK)
	{
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
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> joined\n");
	if(USARTLoRa.ResStatus == LoRa_OK)
		return LoRa_joined;
	else
		return LoRa_unjoined;
}

LoRa_StatusTypeDef EnterSleep(UART_HandleTypeDef *huart)
{
	LoRaCommand(huart, Sleep4200s);
	USARTLoRa.ResStatus=LoRaResCheck("\n\r>> sleep 4200 sec uart_on\n");
	if(USARTLoRa.ResStatus == LoRa_OK)
		return LoRa_Sleep;
	else
		return LoRa_Sleep_FAIL;
}
