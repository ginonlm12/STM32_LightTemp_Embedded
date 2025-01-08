/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "app_touchgfx.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Components/ili9341/ili9341.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define REFRESH_COUNT           ((uint32_t)1386)   /* SDRAM refresh counter */
#define SDRAM_TIMEOUT           ((uint32_t)0xFFFF)
#define TEMP_GPIO  GPIOG
#define TEMP_PIN GPIO_PIN_2

/**
 * @brief  FMC SDRAM Mode definition register defines
 */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

#define I2C3_TIMEOUT_MAX                    0x3000 /*<! The value of the maximal timeout for I2C waiting loops */
#define SPI5_TIMEOUT_MAX                    0x1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

DMA2D_HandleTypeDef hdma2d;

I2C_HandleTypeDef hi2c3;

LTDC_HandleTypeDef hltdc;

SPI_HandleTypeDef hspi5;

SDRAM_HandleTypeDef hsdram1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for GUI_Task */
osThreadId_t GUI_TaskHandle;
const osThreadAttr_t GUI_Task_attributes = { .name = "GUI_Task", .stack_size =
		8192 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* USER CODE BEGIN PV */
osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = { .name = "myQueue01" };

osMessageQueueId_t myQueue02Handle;
const osMessageQueueAttr_t myQueue02_attributes = { .name = "myQueue02" };

osMessageQueueId_t myQueue03Handle;
const osMessageQueueAttr_t myQueue03_attributes = { .name = "myQueue03" };

uint16_t lux_value;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C3_Init(void);
static void MX_SPI5_Init(void);
static void MX_FMC_Init(void);
static void MX_LTDC_Init(void);
static void MX_DMA2D_Init(void);
void StartDefaultTask(void *argument);
extern void TouchGFX_Task(void *argument);

/* USER CODE BEGIN PFP */
static void BSP_SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram,
		FMC_SDRAM_CommandTypeDef *Command);

static uint8_t I2C3_ReadData(uint8_t Addr, uint8_t Reg);
static void I2C3_WriteData(uint8_t Addr, uint8_t Reg, uint8_t Value);
static uint8_t I2C3_ReadBuffer(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer,
		uint16_t Length);

/* SPIx bus function */
static void SPI5_Write(uint16_t Value);
static uint32_t SPI5_Read(uint8_t ReadSize);
static void SPI5_Error(void);

/* Link function for LCD peripheral */
void LCD_IO_Init(void);
void LCD_IO_WriteData(uint16_t RegValue);
void LCD_IO_WriteReg(uint8_t Reg);
uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize);
void LCD_Delay(uint32_t delay);

/* IOExpander IO functions */
void IOE_Init(void);
void IOE_ITConfig(void);
void IOE_Delay(uint32_t Delay);
void IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
uint8_t IOE_Read(uint8_t Addr, uint8_t Reg);
uint16_t IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer,
		uint16_t Length);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static LCD_DrvTypeDef *LcdDrv;

uint32_t I2c3Timeout = I2C3_TIMEOUT_MAX; /*<! Value of Timeout when I2C communication fails */
uint32_t Spi5Timeout = SPI5_TIMEOUT_MAX; /*<! Value of Timeout when SPI communication fails */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
TIM_HandleTypeDef htim1;
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM6_Init 0 */

	/* USER CODE END TIM6_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM6_Init 1 */

	/* USER CODE END TIM6_Init 1 */
	htim1.Instance = TIM6;
	htim1.Init.Prescaler = 89;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 999;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM6_Init 2 */

	/* USER CODE END TIM6_Init 2 */

}
int init_temp;
int init_lux;
int init_toggle;
#define FLASH_SECTOR_ADDRESS  0x080E0000
typedef struct {
	int temp;
	int lux;
	int toggle;
} SensorData;

SensorData sensorData;
void saveData() {
	sensorData.temp = init_temp;
	sensorData.lux = init_lux;
	sensorData.toggle = init_toggle;
}
void updateData() {
	init_temp = sensorData.temp;
	init_lux = sensorData.lux;
	init_toggle = sensorData.toggle;
}
void saveStructToFlash() {
	saveData();
	HAL_FLASH_Unlock();

	FLASH_Erase_Sector(FLASH_SECTOR_11, VOLTAGE_RANGE_3);

	uint32_t *pData = (uint32_t*) &sensorData;
	for (int i = 0; i < sizeof(SensorData) / 4; i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
		FLASH_SECTOR_ADDRESS + (i * 4), *(pData + i));
	}

	HAL_FLASH_Lock();
}

void loadStructFromFlash() {
	SensorData *pFlashData = (SensorData*) FLASH_SECTOR_ADDRESS;

	if (pFlashData->temp > 0 && pFlashData->temp <= 50 && pFlashData->lux > 0
			&& pFlashData->lux <= 1000
			&& (pFlashData->toggle == 0 || pFlashData->toggle == 1)) {
		sensorData = *pFlashData;
		updateData();
	} else {
		sensorData.temp = 25;
		sensorData.lux = 50;
		sensorData.toggle = 1;
		updateData();
	}
}
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */
	loadStructFromFlash();
	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_RTC_Init();
	MX_CRC_Init();
	MX_I2C3_Init();
	MX_SPI5_Init();
	MX_FMC_Init();
	MX_LTDC_Init();
	MX_DMA2D_Init();
	MX_TouchGFX_Init();
	/* Call PreOsInit function */
	MX_TouchGFX_PreOSInit();
	/* USER CODE BEGIN 2 */
	MX_TIM1_Init();
	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();
	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	myQueue01Handle = osMessageQueueNew(8, sizeof(uint8_t),
			&myQueue01_attributes);
	myQueue02Handle = osMessageQueueNew(8, sizeof(uint8_t),
			&myQueue02_attributes);
	myQueue03Handle = osMessageQueueNew(8, sizeof(uint8_t),
			&myQueue03_attributes);
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	/* creation of GUI_Task */
	GUI_TaskHandle = osThreadNew(TouchGFX_Task, NULL, &GUI_Task_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

	/* Start scheduler */
	osKernelStart();
	HAL_TIM_Base_Start_IT(&htim1);
	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */
//		RTC_TimeTypeDef sTime = { 0 };
//		RTC_DateTypeDef sDate = { 0 };
//
//		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
//		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
//		char buf[100];
//		sprintf(buf, "%02d/%02d/20%02d %02d:%02d:%02d\n", sDate.Date,
//				sDate.Month, sDate.Year, sTime.Hours, sTime.Minutes,
//				sTime.Seconds);
//		HAL_UART_Transmit(&huart1, (const uint8_t*) buf, strlen(buf), 1000);
		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void) {

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	RTC_TimeTypeDef sTime = { 0 };
	RTC_DateTypeDef sDate = { 0 };

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		Error_Handler();
	}

	/* USER CODE BEGIN Check_RTC_BKUP */

	/* USER CODE END Check_RTC_BKUP */

	/** Initialize RTC and set the Time and Date
	 */
	sTime.Hours = 7;
	sTime.Minutes = 15;
	sTime.Seconds = 26;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
		Error_Handler();
	}
	sDate.WeekDay = RTC_WEEKDAY_TUESDAY;
	sDate.Month = RTC_MONTH_JANUARY;
	sDate.Date = 5;
	sDate.Year = 25;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief CRC Initialization Function
 * @param None
 * @retval None
 */
static void MX_CRC_Init(void) {

	/* USER CODE BEGIN CRC_Init 0 */

	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */

	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	if (HAL_CRC_Init(&hcrc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */

	/* USER CODE END CRC_Init 2 */

}

/**
 * @brief DMA2D Initialization Function
 * @param None
 * @retval None
 */
static void MX_DMA2D_Init(void) {

	/* USER CODE BEGIN DMA2D_Init 0 */

	/* USER CODE END DMA2D_Init 0 */

	/* USER CODE BEGIN DMA2D_Init 1 */

	/* USER CODE END DMA2D_Init 1 */
	hdma2d.Instance = DMA2D;
	hdma2d.Init.Mode = DMA2D_M2M;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
	hdma2d.Init.OutputOffset = 0;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0;
	if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN DMA2D_Init 2 */

	/* USER CODE END DMA2D_Init 2 */

}

/**
 * @brief I2C3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C3_Init(void) {

	/* USER CODE BEGIN I2C3_Init 0 */

	/* USER CODE END I2C3_Init 0 */

	/* USER CODE BEGIN I2C3_Init 1 */

	/* USER CODE END I2C3_Init 1 */
	hi2c3.Instance = I2C3;
	hi2c3.Init.ClockSpeed = 400000;
	hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c3.Init.OwnAddress1 = 0;
	hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c3.Init.OwnAddress2 = 0;
	hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c3) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_DISABLE)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C3_Init 2 */

	/* USER CODE END I2C3_Init 2 */

}

/**
 * @brief LTDC Initialization Function
 * @param None
 * @retval None
 */
static void MX_LTDC_Init(void) {

	/* USER CODE BEGIN LTDC_Init 0 */

	/* USER CODE END LTDC_Init 0 */

	LTDC_LayerCfgTypeDef pLayerCfg = { 0 };

	/* USER CODE BEGIN LTDC_Init 1 */

	/* USER CODE END LTDC_Init 1 */
	hltdc.Instance = LTDC;
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	hltdc.Init.HorizontalSync = 9;
	hltdc.Init.VerticalSync = 1;
	hltdc.Init.AccumulatedHBP = 29;
	hltdc.Init.AccumulatedVBP = 3;
	hltdc.Init.AccumulatedActiveW = 269;
	hltdc.Init.AccumulatedActiveH = 323;
	hltdc.Init.TotalWidth = 279;
	hltdc.Init.TotalHeigh = 327;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;
	hltdc.Init.Backcolor.Red = 0;
	if (HAL_LTDC_Init(&hltdc) != HAL_OK) {
		Error_Handler();
	}
	pLayerCfg.WindowX0 = 0;
	pLayerCfg.WindowX1 = 240;
	pLayerCfg.WindowY0 = 0;
	pLayerCfg.WindowY1 = 320;
	pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	pLayerCfg.FBStartAdress = 0;
	pLayerCfg.ImageWidth = 240;
	pLayerCfg.ImageHeight = 320;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN LTDC_Init 2 */
	/*Select the device */
	LcdDrv = &ili9341_drv;
	/* LCD Init */
	LcdDrv->Init();

	LcdDrv->DisplayOff();
	/* USER CODE END LTDC_Init 2 */

}

/**
 * @brief SPI5 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI5_Init(void) {

	/* USER CODE BEGIN SPI5_Init 0 */

	/* USER CODE END SPI5_Init 0 */

	/* USER CODE BEGIN SPI5_Init 1 */

	/* USER CODE END SPI5_Init 1 */
	/* SPI5 parameter configuration*/
	hspi5.Instance = SPI5;
	hspi5.Init.Mode = SPI_MODE_MASTER;
	hspi5.Init.Direction = SPI_DIRECTION_2LINES;
	hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi5.Init.NSS = SPI_NSS_SOFT;
	hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi5.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi5) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI5_Init 2 */

	/* USER CODE END SPI5_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void) {

	/* USER CODE BEGIN FMC_Init 0 */

	/* USER CODE END FMC_Init 0 */

	FMC_SDRAM_TimingTypeDef SdramTiming = { 0 };

	/* USER CODE BEGIN FMC_Init 1 */

	/* USER CODE END FMC_Init 1 */

	/** Perform the SDRAM1 memory initialization sequence
	 */
	hsdram1.Instance = FMC_SDRAM_DEVICE;
	/* hsdram1.Init */
	hsdram1.Init.SDBank = FMC_SDRAM_BANK2;
	hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
	hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
	hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
	hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
	hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
	hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
	hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
	hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_DISABLE;
	hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_1;
	/* SdramTiming */
	SdramTiming.LoadToActiveDelay = 2;
	SdramTiming.ExitSelfRefreshDelay = 7;
	SdramTiming.SelfRefreshTime = 4;
	SdramTiming.RowCycleDelay = 7;
	SdramTiming.WriteRecoveryTime = 3;
	SdramTiming.RPDelay = 2;
	SdramTiming.RCDDelay = 2;

	if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK) {
		Error_Handler();
	}

	/* USER CODE BEGIN FMC_Init 2 */

	FMC_SDRAM_CommandTypeDef command;

	/* Program the SDRAM external device */
	BSP_SDRAM_Initialization_Sequence(&hsdram1, &command);
	/* USER CODE END FMC_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE,
	VSYNC_FREQ_Pin | RENDER_TIME_Pin | FRAME_RATE_Pin | MCU_ACTIVE_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_13, GPIO_PIN_RESET);

	/*Configure GPIO pins : VSYNC_FREQ_Pin RENDER_TIME_Pin FRAME_RATE_Pin MCU_ACTIVE_Pin */
	GPIO_InitStruct.Pin = VSYNC_FREQ_Pin | RENDER_TIME_Pin | FRAME_RATE_Pin
			| MCU_ACTIVE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : PC2 */
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PD12 PD13 */
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
 * @brief  Perform the SDRAM external memory initialization sequence
 * @param  hsdram: SDRAM handle
 * @param  Command: Pointer to SDRAM command structure
 * @retval None
 */
static void BSP_SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram,
		FMC_SDRAM_CommandTypeDef *Command) {
	__IO uint32_t tmpmrd = 0;

	/* Step 1:  Configure a clock configuration enable command */
	Command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	Command->AutoRefreshNumber = 1;
	Command->ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

	/* Step 2: Insert 100 us minimum delay */
	/* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
	HAL_Delay(1);

	/* Step 3: Configure a PALL (precharge all) command */
	Command->CommandMode = FMC_SDRAM_CMD_PALL;
	Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	Command->AutoRefreshNumber = 1;
	Command->ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

	/* Step 4: Configure an Auto Refresh command */
	Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	Command->AutoRefreshNumber = 4;
	Command->ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

	/* Step 5: Program the external memory mode register */
	tmpmrd = (uint32_t) SDRAM_MODEREG_BURST_LENGTH_1 |
	SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
	SDRAM_MODEREG_CAS_LATENCY_3 |
	SDRAM_MODEREG_OPERATING_MODE_STANDARD |
	SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
	Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	Command->AutoRefreshNumber = 1;
	Command->ModeRegisterDefinition = tmpmrd;

	/* Send the command */
	HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

	/* Step 6: Set the refresh rate counter */
	/* Set the device refresh rate */
	HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT);
}

/**
 * @brief  IOE Low Level Initialization.
 */
void IOE_Init(void) {
	//Dummy function called when initializing to stmpe811 to setup the i2c.
	//This is done with cubmx and is therfore not done here.
}

/**
 * @brief  IOE Low Level Interrupt configuration.
 */
void IOE_ITConfig(void) {
	//Dummy function called when initializing to stmpe811 to setup interupt for the i2c.
	//The interupt is not used in our case, therefore nothing is done here.
}

/**
 * @brief  IOE Writes single data operation.
 * @param  Addr: I2C Address
 * @param  Reg: Reg Address
 * @param  Value: Data to be written
 */
void IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value) {
	I2C3_WriteData(Addr, Reg, Value);
}

/**
 * @brief  IOE Reads single data.
 * @param  Addr: I2C Address
 * @param  Reg: Reg Address
 * @retval The read data
 */
uint8_t IOE_Read(uint8_t Addr, uint8_t Reg) {
	return I2C3_ReadData(Addr, Reg);
}

/**
 * @brief  IOE Reads multiple data.
 * @param  Addr: I2C Address
 * @param  Reg: Reg Address
 * @param  pBuffer: pointer to data buffer
 * @param  Length: length of the data
 * @retval 0 if no problems to read multiple data
 */
uint16_t IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer,
		uint16_t Length) {
	return I2C3_ReadBuffer(Addr, Reg, pBuffer, Length);
}

/**
 * @brief  IOE Delay.
 * @param  Delay in ms
 */
void IOE_Delay(uint32_t Delay) {
	HAL_Delay(Delay);
}

/**
 * @brief  Writes a value in a register of the device through BUS.
 * @param  Addr: Device address on BUS Bus.
 * @param  Reg: The target register address to write
 * @param  Value: The target register value to be written
 */
static void I2C3_WriteData(uint8_t Addr, uint8_t Reg, uint8_t Value) {
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_I2C_Mem_Write(&hi2c3, Addr, (uint16_t) Reg,
	I2C_MEMADD_SIZE_8BIT, &Value, 1, I2c3Timeout);

	/* Check the communication status */
	if (status != HAL_OK) {
		/* Re-Initialize the BUS */
		//I2Cx_Error();
	}
}

/**
 * @brief  Reads a register of the device through BUS.
 * @param  Addr: Device address on BUS Bus.
 * @param  Reg: The target register address to write
 * @retval Data read at register address
 */
static uint8_t I2C3_ReadData(uint8_t Addr, uint8_t Reg) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t value = 0;

	status = HAL_I2C_Mem_Read(&hi2c3, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value,
			1, I2c3Timeout);

	/* Check the communication status */
	if (status != HAL_OK) {
		/* Re-Initialize the BUS */
		//I2Cx_Error();
	}
	return value;
}

/**
 * @brief  Reads multiple data on the BUS.
 * @param  Addr: I2C Address
 * @param  Reg: Reg Address
 * @param  pBuffer: pointer to read data buffer
 * @param  Length: length of the data
 * @retval 0 if no problems to read multiple data
 */
static uint8_t I2C3_ReadBuffer(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer,
		uint16_t Length) {
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_I2C_Mem_Read(&hi2c3, Addr, (uint16_t) Reg,
	I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2c3Timeout);

	/* Check the communication status */
	if (status == HAL_OK) {
		return 0;
	} else {
		/* Re-Initialize the BUS */
		//I2Cx_Error();
		return 1;
	}
}

/**
 * @brief  Reads 4 bytes from device.
 * @param  ReadSize: Number of bytes to read (max 4 bytes)
 * @retval Value read on the SPI
 */
static uint32_t SPI5_Read(uint8_t ReadSize) {
	HAL_StatusTypeDef status = HAL_OK;
	uint32_t readvalue;

	status = HAL_SPI_Receive(&hspi5, (uint8_t*) &readvalue, ReadSize,
			Spi5Timeout);

	/* Check the communication status */
	if (status != HAL_OK) {
		/* Re-Initialize the BUS */
		SPI5_Error();
	}

	return readvalue;
}

/**
 * @brief  Writes a byte to device.
 * @param  Value: value to be written
 */
static void SPI5_Write(uint16_t Value) {
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_SPI_Transmit(&hspi5, (uint8_t*) &Value, 1, Spi5Timeout);

	/* Check the communication status */
	if (status != HAL_OK) {
		/* Re-Initialize the BUS */
		SPI5_Error();
	}
}

/**
 * @brief  SPI5 error treatment function.
 */
static void SPI5_Error(void) {
	/* De-initialize the SPI communication BUS */
	//HAL_SPI_DeInit(&SpiHandle);
	/* Re- Initialize the SPI communication BUS */
	//SPIx_Init();
}

void LCD_IO_Init(void) {
	/* Set or Reset the control line */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
}

/**
 * @brief  Writes register value.
 */
void LCD_IO_WriteData(uint16_t RegValue) {
	/* Set WRX to send data */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);

	/* Reset LCD control line(/CS) and Send data */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	SPI5_Write(RegValue);

	/* Deselect: Chip Select high */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
}

/**
 * @brief  Writes register address.
 */
void LCD_IO_WriteReg(uint8_t Reg) {
	/* Reset WRX to send command */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

	/* Reset LCD control line(/CS) and Send command */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	SPI5_Write(Reg);

	/* Deselect: Chip Select high */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
}

/**
 * @brief  Reads register value.
 * @param  RegValue Address of the register to read
 * @param  ReadSize Number of bytes to read
 * @retval Content of the register value
 */
uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize) {
	uint32_t readvalue = 0;

	/* Select: Chip Select low */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);

	/* Reset WRX to send command */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

	SPI5_Write(RegValue);

	readvalue = SPI5_Read(ReadSize);

	/* Set WRX to send data */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);

	/* Deselect: Chip Select high */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);

	return readvalue;
}

/**
 * @brief  Wait for loop in ms.
 * @param  Delay in ms.
 */
void LCD_Delay(uint32_t Delay) {
	HAL_Delay(Delay);
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */

// Khởi tạo timer chạy tick để có thể cho hệ thống dừng đến us
void Delay_us(uint16_t us) {
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while (__HAL_TIM_GET_COUNTER(&htim1) < us)
		;
}
// Khởi tạo chân đọc đữ liệu từ DS18B20 ở đây là chân PG2
void GPIO_SetState(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t Mode) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
//    HAL_GPIO_DeInit(GPIOx, GPIO_Pin);

	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = Mode;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
// Gửi tín hiệu đến DS18B20 để biết bắt đầu đọc tín hiệu nhiệt độ
void OneWire_Reset(void) {
	GPIO_SetState(TEMP_GPIO, TEMP_PIN, GPIO_MODE_OUTPUT_OD);
	HAL_GPIO_WritePin(TEMP_GPIO, TEMP_PIN, GPIO_PIN_RESET);
	Delay_us(500);

	GPIO_SetState(TEMP_GPIO, TEMP_PIN, GPIO_MODE_INPUT);
	Delay_us(500);
}
// Ghi dữ liệu vào DS18B20 để đo nhiệt độ ở 0x44 hoặc đọc ở 0xBE
void OneWire_Write(uint8_t data) {
	uint8_t count;
	for (count = 0; count < 8; ++count) {
		GPIO_SetState(TEMP_GPIO, TEMP_PIN, GPIO_MODE_OUTPUT_OD);
		HAL_GPIO_WritePin(TEMP_GPIO, TEMP_PIN, GPIO_PIN_RESET);
		Delay_us(2);

		if (data & 0x01)
			HAL_GPIO_WritePin(TEMP_GPIO, TEMP_PIN, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(TEMP_GPIO, TEMP_PIN, GPIO_PIN_RESET);

		data >>= 1;
		Delay_us(60);

		GPIO_SetState(TEMP_GPIO, TEMP_PIN, GPIO_MODE_INPUT);
		Delay_us(2);
	}
}
// đọc giá trị từ DS18B20 trả về dãy 8 bit
uint8_t OneWire_Read(void) {
	uint8_t count, data = 0;

	for (count = 0; count < 8; ++count) {
		GPIO_SetState(TEMP_GPIO, TEMP_PIN, GPIO_MODE_OUTPUT_OD);
		HAL_GPIO_WritePin(TEMP_GPIO, TEMP_PIN, GPIO_PIN_RESET);
		Delay_us(2);

		GPIO_SetState(TEMP_GPIO, TEMP_PIN, GPIO_MODE_INPUT);
		Delay_us(8);

		if (HAL_GPIO_ReadPin(TEMP_GPIO, TEMP_PIN) == GPIO_PIN_SET) {
			data |= 0x01 << count;
		}

		Delay_us(120);
	}

	return data;
}
// Chuyển giá trị từ dãy 8 bit sáng giá trị float
float ds18b20_read(void) {
	uint8_t busy = 0, temp1, temp2;
	uint16_t temp3, TimeOut;
	float result;
	OneWire_Reset();
	OneWire_Write(0xCC);
	OneWire_Write(0x44);
	while ((busy == 0) && TimeOut < 2000) {
		busy = OneWire_Read();
		osDelay(1);
		TimeOut++;
	}
	OneWire_Reset();
	OneWire_Write(0xCC);
	OneWire_Write(0xBE);
	temp1 = OneWire_Read();
	temp2 = OneWire_Read();
	temp3 = temp2;
	temp3 <<= 8;
	temp3 |= temp1;
	result = (float) temp3 / 16.0;
	osDelay(200);
	return (result);
}
#define BH1750_ADDRESS			(0x23<<1)

#define	BH1750_POWER_DOWN		0x00
#define	BH1750_POWER_ON			0x01
#define	BH1750_RESET			0x07
#define	BH1750_DEFAULT_MTREG	69

#define BH1750_CONVERSION_FACTOR	1.2

typedef enum {
	BH1750_OK = 0, BH1750_ERROR = 1
} BH1750_STATUS;

typedef enum {
	CONTINUOUS_HIGH_RES_MODE = 0x10,
	CONTINUOUS_HIGH_RES_MODE_2 = 0x11,
	CONTINUOUS_LOW_RES_MODE = 0x13,
	ONETIME_HIGH_RES_MODE = 0x20,
	ONETIME_HIGH_RES_MODE_2 = 0x21,
	ONETIME_LOW_RES_MODE = 0x23
} bh1750_mode;

I2C_HandleTypeDef *bh1750_i2c;	// Handler to I2C interface
bh1750_mode Bh1750_Mode;	// Current sensor mode
uint8_t Bh1750_Mtreg;	// Current MT register value
// gửi tín hiệu để khởi động lại BH1750
BH1750_STATUS BH1750_Reset(void) {
	uint8_t tmp = 0x07;
	if (HAL_OK
			== HAL_I2C_Master_Transmit(bh1750_i2c, BH1750_ADDRESS, &tmp, 1, 10))
		return BH1750_OK;

	return BH1750_ERROR;
}
// Điêu chỉnh độ nhạy khi do ánh sáng, thời gian đo càng dài thì độ nhạy càng cao
BH1750_STATUS BH1750_SetMtreg(uint8_t Mtreg) {
	HAL_StatusTypeDef retCode;
	if (Mtreg < 31 || Mtreg > 254) {
		return BH1750_ERROR;
	}

	Bh1750_Mtreg = Mtreg;

	uint8_t tmp[2];

	tmp[0] = (0x40 | (Mtreg >> 5));
	tmp[1] = (0x60 | (Mtreg & 0x1F));

	retCode = HAL_I2C_Master_Transmit(bh1750_i2c, BH1750_ADDRESS, &tmp[0], 1,
			10);
	if (HAL_OK != retCode) {
		return BH1750_ERROR;
	}

	retCode = HAL_I2C_Master_Transmit(bh1750_i2c, BH1750_ADDRESS, &tmp[1], 1,
			10);
	if (HAL_OK == retCode) {
		return BH1750_OK;
	}

	return BH1750_ERROR;
}
// Khởi tạo cảm biến BH1750 và lưu lại địa chỉ của con trở I2C
BH1750_STATUS BH1750_Init(I2C_HandleTypeDef *hi2c) {
	bh1750_i2c = hi2c;
	if (BH1750_OK == BH1750_Reset()) {
		if (BH1750_OK == BH1750_SetMtreg(BH1750_DEFAULT_MTREG)) // Set default value;
			return BH1750_OK;
	}
	return BH1750_ERROR;
}
// Thay đổi trạng thái nguồn của BH1750 đẻ tiết kiệm năng lượng
BH1750_STATUS BH1750_PowerState(uint8_t PowerOn) {
	PowerOn = (PowerOn ? 1 : 0);
	if (HAL_OK
			== HAL_I2C_Master_Transmit(bh1750_i2c, BH1750_ADDRESS, &PowerOn, 1,
					10))
		return BH1750_OK;

	return BH1750_ERROR;
}

// Thiết lập chế độ làm việc của BH1750 chế độ liên tục hoặc 1 lần, phân giải cao hoặc thấp
BH1750_STATUS BH1750_SetMode(bh1750_mode Mode) {
	if (!((Mode >> 4) || (Mode >> 5)))
		return BH1750_ERROR;
	if ((Mode & 0x0F) > 3)
		return BH1750_ERROR;

	Bh1750_Mode = Mode;
	if (HAL_OK
			== HAL_I2C_Master_Transmit(bh1750_i2c, BH1750_ADDRESS, &Mode, 1,
					10))
		return BH1750_OK;

	return BH1750_ERROR;
}

// Khởi động lại phép đo nếu phép đo đang ở chế độ đo thủ công
BH1750_STATUS BH1750_TriggerManualConversion(void) {
	if (BH1750_OK == BH1750_SetMode(Bh1750_Mode)) {
		return BH1750_OK;
	}
	return BH1750_ERROR;
}
//Đọc giá trị ánh sáng từ cảm biến và chuyển dữ liệu sang float và trả về
BH1750_STATUS BH1750_ReadLight(float *Result) {
	float result;
	uint8_t tmp[2];

	if (HAL_OK
			== HAL_I2C_Master_Receive(bh1750_i2c, BH1750_ADDRESS, tmp, 2, 10)) {
		result = (tmp[0] << 8) | (tmp[1]);

		if (Bh1750_Mtreg != BH1750_DEFAULT_MTREG) {
			result *= (float) ((uint8_t) BH1750_DEFAULT_MTREG
					/ (float) Bh1750_Mtreg);
		}

		if (Bh1750_Mode == ONETIME_HIGH_RES_MODE_2
				|| Bh1750_Mode == CONTINUOUS_HIGH_RES_MODE_2) {
			result /= 2.0;
		}

		*Result = result / (float) BH1750_CONVERSION_FACTOR;
		return BH1750_OK;
	}
	return BH1750_ERROR;
}
#define ON 1
#define OFF 0
void LedControl(int value) {
	if (value == ON) {
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_SET); // Bật LED
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET); // Tắt LED
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
	}
}
void FanControl(int value) {
	if (value == ON) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET); // Bật quạt
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // Tắt quạt
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_RESET);
	}
}
void StartDefaultTask(void *argument) {
	/* USER CODE BEGIN 5 */
	uint32_t lastSaveTime = osKernelGetTickCount();
	float BH1750_lux;
	// Khởi tạo cảm biến BH1750
	BH1750_Init(&hi2c3);
	BH1750_SetMode(CONTINUOUS_HIGH_RES_MODE_2);
	/* Infinite loop */
	for (;;) {
		// Đọc giá trị cảm biến DS1820
		uint8_t temperature = (uint8_t) ds18b20_read();
		// Gửi giá trị nhiệt độ vào hàng đợi
		osMessageQueuePut(myQueue01Handle, &temperature, 0, 10);
		//Đọc giá trị cảm biến BH1750
		if (BH1750_OK == BH1750_ReadLight(&BH1750_lux)) {
			uint8_t lux_value = (uint8_t) BH1750_lux;
			// gửi giá trị ánh sáng vào hàng đợi
			osMessageQueuePut(myQueue02Handle, &lux_value, 0, 10);
		}
		// nhận giá trị từ queue gửi từ màn hình điều khiển về
		uint8_t res;
		if (osMessageQueueGetCount(myQueue03Handle) > 0) {
			osMessageQueueGet(myQueue03Handle, &res, NULL, osWaitForever);
			if (res == 'L')
				LedControl(ON);
			if (res == 'l')
				LedControl(OFF);
			if (res == 'F')
				FanControl(ON);
			if (res == 'f')
				FanControl(OFF);
		}
		uint32_t currentTime = osKernelGetTickCount();
		if ((currentTime - lastSaveTime) >= 30000) {
			saveStructToFlash();
			lastSaveTime = currentTime;
		}
		osDelay(300);
	}
	/* USER CODE END 5 */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
