#include <ch32x035.h>
#include <debug.h>

#include "mcu_config.h"

void GPIOInitPin(GPIO_TypeDef* gpio, uint16_t pin, GPIOSpeed_TypeDef speed,
                 GPIOMode_TypeDef mode) {
  GPIO_InitTypeDef init = {0};
  init.GPIO_Pin = pin;
  init.GPIO_Speed = speed;
  init.GPIO_Mode = mode;
  GPIO_Init(gpio, &init);
}

void GPIOInit() {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

  GPIOInitPin(INPUT_L_PORT, INPUT_L_PIN, GPIO_Speed_50MHz, GPIO_Mode_AIN);
  GPIOInitPin(INPUT_R_PORT, INPUT_R_PIN, GPIO_Speed_50MHz, GPIO_Mode_AIN);

  GPIOInitPin(SCK_PORT, SCK_PIN, GPIO_Speed_50MHz, GPIO_Mode_AF_PP);
  GPIOInitPin(MOSI_PORT, MOSI_PIN, GPIO_Speed_50MHz, GPIO_Mode_AF_PP);
  GPIOInitPin(LATCH_PORT, LATCH_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP);
}

void SPIInit() {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  SPI_InitTypeDef init = {0};

  init.SPI_Direction = SPI_Direction_1Line_Tx;
  init.SPI_Mode = SPI_Mode_Master;
  init.SPI_DataSize = SPI_DataSize_8b;
  init.SPI_CPOL = SPI_CPOL_High;
  init.SPI_CPHA = SPI_CPHA_2Edge;
  init.SPI_NSS = SPI_NSS_Soft;
  init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  init.SPI_FirstBit = SPI_FirstBit_MSB;
  init.SPI_CRCPolynomial = 7;

  SPI_Init(SPI1, &init);
  SPI_Cmd(SPI1, ENABLE);
}

uint8_t leds[20] = {0};

void SPISendLEDs() {
  for (int i = 0; i < sizeof(leds); i++) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET) {}
    SPI_I2S_SendData(SPI1, leds[sizeof(leds) - i - 1]);
  }
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET) {}
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) != RESET) {}

  GPIO_SetBits(LATCH_PORT, LATCH_PIN);
  Delay_Us(10);
  GPIO_ResetBits(LATCH_PORT, LATCH_PIN);
}

void SendLevel(uint8_t l, uint8_t r) {
  for (int i = 0; i < sizeof(leds); i++) {
    leds[i] = 0;
  }
  int bit = 1;
  while (l) {
    leds[bit / 8] |= (1 << (bit % 8));
    l--;
    bit++;
    if (bit % 8 == 0) {
      bit++;
    }
  }
  bit = 1 + 5 * 8 * 2;
  while (r) {
    leds[bit / 8] |= (1 << (bit % 8));
    r--;
    bit++;
    if (bit % 8 == 0) {
      bit++;
    }
  }
  SPISendLEDs();
}

void ADCInit() {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  ADC_DeInit(ADC1);
  ADC_CLKConfig(ADC1, ADC_CLK_Div6);

  ADC_InitTypeDef init = {0};

  init.ADC_Mode = ADC_Mode_Independent;
  init.ADC_ScanConvMode = DISABLE;
  init.ADC_ContinuousConvMode = DISABLE;
  init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  init.ADC_DataAlign = ADC_DataAlign_Right;
  init.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &init);
  ADC_Cmd(ADC1, ENABLE);
}

void ADCRead(uint16_t* l, uint16_t* r) {
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_11Cycles);
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {}
  *l = ADC_GetConversionValue(ADC1);

  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_11Cycles);
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {}
  *r = ADC_GetConversionValue(ADC1);
}

void Init() {
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  GPIOInit();
  SPIInit();
  ADCInit();
  Delay_Init();
}

uint16_t kConversionMap[70] = { 11, 12, 13, 14, 15, 16, 18, 19, 21, 23, 25, 27, 30, 33, 35, 39, 42, 46, 50, 55, 60, 65, 71, 77, 84, 92, 100, 109, 119, 129, 141, 154, 168, 183, 199, 217, 237, 258, 282, 307, 335, 365, 398, 434, 473, 516, 562, 613, 668, 728, 794, 865, 944, 1029, 1121, 1223, 1333, 1453, 1584, 1727, 1883, 2052, 2237, 2439, 2659, 2899, 3160, 3446, 3756, 4095 };

uint8_t ConvertToLEDs(uint16_t value) {
  for (int i = 0; i < sizeof(kConversionMap) / sizeof(kConversionMap[0]); i++) {
    if (value < kConversionMap[i]) {
      if (i <= 1) {
        return 1;
      }
      return i - 1;
    }
  }
  return 69;
}

int main() {
  Init();

  uint16_t l_value = 0;
  uint16_t r_value = 0;

  int32_t l_buffered_value = 0;
  int32_t r_buffered_value = 0;

  for (;;) {
    ADCRead(&l_value, &r_value);
    int32_t real_l_value = l_value << 16;
    int32_t real_r_value = r_value << 16;
    l_buffered_value = real_l_value > l_buffered_value ? real_l_value : l_buffered_value;
    r_buffered_value = real_r_value > r_buffered_value ? real_r_value : r_buffered_value;
    SendLevel(ConvertToLEDs(l_buffered_value >> 16), ConvertToLEDs(r_buffered_value >> 16));
    if (l_buffered_value > 0) {
      l_buffered_value -= l_buffered_value >> 8;
    }
    if (l_buffered_value < 0) {
      l_buffered_value = 0;
    }
    if (r_buffered_value > 0) {
      r_buffered_value -= r_buffered_value >> 8;
    }
    if (r_buffered_value < 0) {
      r_buffered_value = 0;
    }
  }
}