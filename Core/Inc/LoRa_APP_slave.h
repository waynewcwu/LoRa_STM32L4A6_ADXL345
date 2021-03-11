/*
 * LoRa_App_slave.c V1.2
 *
 *  Created on: Mar 11, 2021
 *      Author: Wayne Wu
 *
 *20200928----V1.1
 *Add Sleep ,WakeUp and CheckJoinStatus Function.
 *20210311----V1.2
 *Add setup option: MaxEIRP, Tx power, Channel, Bandwidth.
 *Improve Frequency setup Function. Send data false by leave APB Mode issue.
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
#define Channel 16 //1~16 for 923 Band
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

#define Uart_RBuffer_Size 64

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
  LoRa_OK      		= 0x00U,
  LoRa_Res_Err      = 0x01U,
  LoRa_CF_FEQ_FAIL	= 0x02U,
  LoRa_SAVE_FAIL	= 0x03U,
  LoRa_CF_SF_FAIL	= 0x04U,
  LoRa_CF_ADR_FAIL	= 0x05U,
  LoRa_CF_DevEUI_FAIL	= 0x06U,
  LoRa_CF_AppEUI_FAIL	= 0x07U,
  LoRa_CF_DevAddr_FAIL	= 0x08U,
  LoRa_CF_NwksKey_FAIL	= 0x09U,
  LoRa_CF_AppsKey_FAIL	= 0x0aU,
  LoRa_ABPCMD_FAIL		= 0x0bU,
  LoRa_JoinABP_FAIL		= 0x0cU,
  LoRa_TxCMD_FAIL		= 0x0dU,
  LoRa_TxData_FAIL		= 0x0eU,
  LoRa_TxData_WaitRes	= 0x0fU,
  LoRa_WaKeUp			= 0x10U,
  LoRa_PowerUp			= 0x11U,
  LoRa_joined			= 0x12U,
  LoRa_unjoined			= 0x13U,
  LoRa_Sleep			= 0x14U,
  LoRa_Sleep_FAIL		= 0x15U,
  LoRa_CF_MaxEIRP_FAIL	= 0x16U,
  LoRa_CF_TxPower_FAIL	= 0x17U,
  LoRa_CF_ChBW_FAIL		= 0x18U,
} LoRa_StatusTypeDef;




typedef struct
{
	_Bool sendflag;
	_Bool Revflag;
	_Bool ResetRevflag;

	//Data format change buffer, hex <> Dec&float
	unsigned char __attribute__ ((aligned (32))) DataStrBuffer[Uart_Buffer_Size];
	unsigned char __attribute__ ((aligned (32))) DataHexBuffer[Uart_Buffer_Size];
	uint8_t DataHextemp[2];
	//usart transfermit
	unsigned char __attribute__ ((aligned (32))) buffer[Uart_Buffer_Size];
	//char __attribute__ ((aligned (32))) buffer[Uart_Buffer_Size];
	uint8_t Rbuffer;
	unsigned char __attribute__ ((aligned (32))) RevData[Uart_RBuffer_Size];

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
} USART_LoRa;

//Define Acsip LoRa AT Command
#define LoRaSave	"mac save"
#define LoRaReset	"sip reset"
#define ActiveABP	"mac join abp"
#define SendUcfP1	"mac tx ucnf 1 "
#define ADRoff		"mac set_adr off"
#define WKUPtigger  "WakeUp" // send any string to interrupted (waked up) by UART
#define CkJoinSt  	"mac get_join_status"
#define Sleep4200s	"sip sleep 4200 uart_on" //sleep 4200 seconds, uart Trigger wake up

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

};

/* Exported functions prototypes ---------------------------------------------*/
void LoRa_USART(UART_HandleTypeDef *huart);
void LoRaCommand(UART_HandleTypeDef *huart,  uint32_t *cmd);
LoRa_StatusTypeDef LoRaInit(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef LoRaResCheck(uint32_t *Res);
LoRa_StatusTypeDef ActiveABPMode(UART_HandleTypeDef *huart);
void LoRaChipReset(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef LoRaTransmitData(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef LoRaTxResCheck(uint32_t *Res);
LoRa_StatusTypeDef WakeUpTrigger(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef CheckJoinStatus(UART_HandleTypeDef *huart);
LoRa_StatusTypeDef EnterSleep(UART_HandleTypeDef *huart);

/* Exported macro ------------------------------------------------------------*/
#define min(a, b) ((a) < (b)) ? (a) : (b)
#define max(a, b) ((a) > (b)) ? (a) : (b)
