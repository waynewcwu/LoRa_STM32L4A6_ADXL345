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
#ifndef INC_LORA_APP_SLAVE_H_
#define INC_LORA_APP_SLAVE_H_



#endif /* INC_LORA_APP_SLAVE_H_ */

/* Private includes ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
//LoRa parameter
#define RadioFreq 923000000
#define SF 10 // SF12~SF7
#define MaxEIRP 20 // 8~36(dBm)
#define TxPower	20 // Rang <= MaxEIRP(dBm)
#define Channel 8 //1~16 for 923 Band
#define BandWidth 125 //125 or 500(KHz)
#define TxTimeBySec 60	//unit: Second
#define CF_DevEUI "mac set_deveui 00F2A1C6BE5EA702"//64bits
#define CF_AppEUI "mac set_appeui 70B3D57ED00317FB"//64bits
#define CF_DevAddr "mac set_devaddr 26011191"//32bits
#define CF_NwksKey "mac set_nwkskey 444B86E2C9B20060B0A57F6C76DF2650"//128bits
#define CF_AppsKey "mac set_appskey EB2B73053348BCB5D6C8035EDA5CDE3A"//128bits
#define Gateway_Channel 8
//SF=12 & SF=11 & SF=10, payload max data size = 118bytes
#if( SF == 12)
	#define CF_SFDataRate "mac set_dr 0"
	#define Uart_Buffer_Size 118
#elif( SF == 11)
	#define CF_SFDataRate "mac set_dr 1"
	#define Uart_Buffer_Size 118
#elif( SF == 10)
	#define CF_SFDataRate "mac set_dr 2"
	#define Uart_Buffer_Size 118
//SF=9, payload max data size = 246 bytes
#elif( SF == 9)
	#define CF_SFDataRate "mac set_dr 3"
	#define Uart_Buffer_Size 246
//SF=8 & SF=7 , payload max data size = 246 bytes
#elif( SF == 8)
	#define CF_SFDataRate "mac set_dr 4"
	#define Uart_Buffer_Size 460
#elif( SF == 7)
	#define CF_SFDataRate "mac set_dr 5"
	#define Uart_Buffer_Size 460
#endif

#define Uart_RBuffer_Size 70

//Set MaxEIRP Index command
#if( MaxEIRP == 8 )
	#define CF_MaxEIRP "mac set_max_eirp 0"
#elif( MaxEIRP == 10 )
	#define CF_MaxEIRP "mac set_max_eirp 1"
#elif( MaxEIRP == 12 )
	#define CF_MaxEIRP "mac set_max_eirp 2"
#elif( MaxEIRP == 13 )
	#define CF_MaxEIRP "mac set_max_eirp 3"
#elif( MaxEIRP == 14 )
	#define CF_MaxEIRP "mac set_max_eirp 4"
#elif( MaxEIRP == 16 )
	#define CF_MaxEIRP "mac set_max_eirp 5"
#elif( MaxEIRP == 18 )
	#define CF_MaxEIRP "mac set_max_eirp 6"
#elif( MaxEIRP == 20 )
	#define CF_MaxEIRP "mac set_max_eirp 7"
#elif( MaxEIRP == 21 )
	#define CF_MaxEIRP "mac set_max_eirp 8"
#elif( MaxEIRP == 24 )
	#define CF_MaxEIRP "mac set_max_eirp 9"
#elif( MaxEIRP == 26 )
	#define CF_MaxEIRP "mac set_max_eirp 10"
#elif( MaxEIRP == 27 )
	#define CF_MaxEIRP "mac set_max_eirp 11"
#elif( MaxEIRP == 29 )
	#define CF_MaxEIRP "mac set_max_eirp 12"
#elif( MaxEIRP == 20 )
	#define CF_MaxEIRP "mac set_max_eirp 13"
#elif( MaxEIRP == 33 )
	#define CF_MaxEIRP "mac set_max_eirp 14"
#elif( MaxEIRP == 36 )
	#define CF_MaxEIRP "mac set_max_eirp 16"
#endif


/* Exported types ------------------------------------------------------------*/
/**
  * @brief  LoRa Initial Status structures definition
  */
typedef enum
{
  LoRa_OK,
  LoRa_Res_Err,
  LoRa_Join_Invalid,
  LoRa_Join_keys_not_init,
  LoRa_Join_no_free_ch,
  LoRa_Join_busy,
  LoRa_Join_FAIL,
  LoRa_TxData_FAIL,
  LoRa_TxData_WaitRes,
  LoRa_Tx_Invalid,
  LoRa_Tx_not_joined,
  LoRa_Tx_no_free_ch,
  LoRa_Tx_busy,
  LoRa_Tx_invalid_data_length,
  LoRa_Tx_exceeded_data_length,
  LoRa_Tx2_err,
  LoRa_Tx2_RxReceived,
  LoRa_WaKeUp,
  LoRa_PowerUp,
  LoRa_joined,
  LoRa_unjoined,
  LoRa_Sleep,
  LoRa_Sleep_FAIL,
  LoRa_unexpected_Err,
  LoRa_HDRest,
} LoRa_StatusTypeDef;




typedef struct
{
	_Bool sendflag;
	_Bool Revflag;
	_Bool ResetRevflag;
	_Bool LoRaRxflag;
	_Bool CrashDetectStartflag;

	//Data format change buffer, hex <> Dec&float
	unsigned char __attribute__ ((aligned (32))) DataStrBuffer[Uart_Buffer_Size];
	unsigned char __attribute__ ((aligned (32))) DataHexBuffer[Uart_Buffer_Size];
	uint8_t DataHextemp[2];
	//usart transfermit
	unsigned char __attribute__ ((aligned (32))) buffer[Uart_Buffer_Size];
	//char __attribute__ ((aligned (32))) buffer[Uart_Buffer_Size];
	uint8_t Rbuffer;
	unsigned char __attribute__ ((aligned (32))) RevData[Uart_RBuffer_Size];
	unsigned char __attribute__ ((aligned (32))) RevDatabk[Uart_RBuffer_Size];
	unsigned char __attribute__ ((aligned (32))) RxHead[11];

	uint8_t Res_Check[32];
	uint32_t Command;
	uint8_t RevStrEndCount;
	uint8_t RevStrCount;
	uint8_t RxCount;
	uint32_t sendTimeout;
	uint32_t bufferSize;
	uint32_t DataStrSize;
	char Status;
	char ResStatus;
	uint8_t TxTimerSecCount;
	int CrashTimerCount;
} USART_LoRa;

//Define Acsip LoRa AT Command
#define LoRaSave	"mac save"
#define LoRaReset	"sip reset"
#define LoRaFacReset"sip factory_reset"
#define ActiveABP	"mac join abp"
#define SendUcfP1	"mac tx ucnf 1 "
#define ADRoff		"mac set_adr off"
#define WKUPtigger  "WakeUp" // send any string to interrupted (waked up) by UART
#define CkJoinSt  	"mac get_join_status"
#define Sleep4200s	"sip sleep 4200 uart_on" //sleep 4200 seconds, uart Trigger wake up
#define Offtxconfirm	"mac set_tx_confirm off"

/* Exported constants --------------------------------------------------------*/
enum
{
	WakeUp,
	CheckJoinST,
	Init,
	JoinABP,
	SendData,
	waitTxRes,
	EnterSleepMode,
	CMDdebug,
};

typedef enum
{
	LoRa_CF_UnConfig,
	LoRa_CF_FEQ,
  	LoRa_CF_SF,
  	LoRa_CF_ADR,
  	LoRa_CF_DevEUI,
  	LoRa_CF_AppEUI,
  	LoRa_CF_DevAddr,
  	LoRa_CF_NwksKey,
  	LoRa_CF_AppsKey,
  	LoRa_CF_MaxEIRP,
  	LoRa_CF_TxPower,
 	LoRa_CF_ChBW,
 	LoRa_CF_Finished,
}LoRa_ConfigureStatusTypeDef;


/* Exported functions prototypes ---------------------------------------------*/
void LoRa_USART(UART_HandleTypeDef *huart);
void LoRaCommand(UART_HandleTypeDef *huart,  uint32_t *cmd);
LoRa_StatusTypeDef LoRaInit(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef LoRaResCheck(uint32_t *Res);
LoRa_StatusTypeDef ActiveABPMode(UART_HandleTypeDef *huart);
void LoRaChipReset(UART_HandleTypeDef *huart);
void LoRaFactorReset(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef LoRaTransmitData(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef WakeUpTrigger(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef CheckJoinStatus(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef EnterSleep(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef Check_2ResMsg(uint32_t *Res1, uint32_t *Res2);
LoRa_StatusTypeDef Check_JoinResMsg();
LoRa_StatusTypeDef Check_Tx1stResMsg();
LoRa_StatusTypeDef Check_Tx2stResMsg();
LoRa_StatusTypeDef LoRaSaveCF(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef LoRachipCrashDetect_HDRST(TIM_HandleTypeDef *htim);
void LoRachipCrashDetect_Stop(TIM_HandleTypeDef *htim);
/* Exported macro ------------------------------------------------------------*/
#define min(a, b) ((a) < (b)) ? (a) : (b)
#define max(a, b) ((a) > (b)) ? (a) : (b)
