/*
 * LoRa_App_slave.h V1.1
 *
 *  Created on: Sep 28, 2020
 *      Author: Wayne Wu
 *
 *20200928----V1.1
 *Add Sleep ,WakeUp and CheckJoinStatus Function.
 */

#ifndef INC_LORA_APP_SLAVE_H_
#define INC_LORA_APP_SLAVE_H_



#endif /* INC_LORA_APP_SLAVE_H_ */

/* Private includes ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
//LoRa parameter
#define RadioFreq 915000000
#define SF 9 // SF12~SF7
#define TxTimeBySec 60	//unit: Second
#define CF_DevEUI "mac set_deveui 00F2A1C6BE5EA702"//64bits
#define CF_AppEUI "mac set_appeui 70B3D57ED00317FB"//64bits
#define CF_DevAddr "mac set_devaddr 26011191"//32bits
#define CF_NwksKey "mac set_nwkskey 444B86E2C9B20060B0A57F6C76DF2650"//128bits
#define CF_AppsKey "mac set_appskey EB2B73053348BCB5D6C8035EDA5CDE3A"//128bits
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
