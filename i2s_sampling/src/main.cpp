#include <Arduino.h>
// #include <WiFi.h>
// #include <HTTPClient.h>
// #include "WiFiCredentials.h"
#include "I2SMEMSSampler.h"
#include "ADCSampler.h"

 ADCSampler *adcSampler = NULL;
 I2SSampler *i2sSampler = NULL;


// i2s config for using the internal ADC
i2s_config_t adcI2SConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s config for reading from left channel of I2S
i2s_config_t i2sMemsConfigLeftChannel = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s pins
i2s_pin_config_t i2sPins = {
    .bck_io_num = GPIO_NUM_32,
    .ws_io_num = GPIO_NUM_25,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = GPIO_NUM_33};

// how many samples to read at once
const int SAMPLE_SIZE = 16384;


// Task to write samples from ADC to our server
void adcWriterTask(void *param) {
  I2SSampler *sampler = (I2SSampler *)param;
  int16_t *samples = (int16_t *)malloc(sizeof(uint16_t) * SAMPLE_SIZE);
  if (!samples) {
    Serial.println("Failed to allocate memory for samples");
    return;
  }
  while (true) {
    int samples_read = sampler->read(samples, SAMPLE_SIZE);
    
    for (int i = 0; i < samples_read; i++)
    {
     printf("adc: %d \n",samples[i]);
     
    }
  
   
    
  }  
}

void i2sMemsWriterTask(void *param) {
  I2SSampler *sampler = (I2SSampler *)param;
  int16_t *samples = (int16_t *)malloc(sizeof(uint16_t) * SAMPLE_SIZE);
  if (!samples) {
    Serial.println("Failed to allocate memory for samples");
    return;
  }
  while (true) {
    int samples_read = sampler->read(samples, SAMPLE_SIZE);
    
    for (int i = 0; i < samples_read; i++)
    {
      printf("i2s:%d\n ",samples[i]);
      
    }
    
   
  }
}

void setup()
{
  Serial.begin(115200);
  

  // input from analog microphones such as the MAX9814 or MAX4466
  // internal analog to digital converter sampling using i2s
  // create our samplers
  adcSampler = new ADCSampler(ADC_UNIT_1, ADC1_CHANNEL_7, adcI2SConfig);

  //set up the adc sample writer task
  TaskHandle_t adcWriterTaskHandle;
  adcSampler->start();
  xTaskCreatePinnedToCore(adcWriterTask, "ADC Writer Task", 4096, adcSampler, 1, &adcWriterTaskHandle, 1);

  // Direct i2s input from INMP441 or the SPH0645
  i2sSampler = new I2SMEMSSampler(I2S_NUM_0, i2sPins, i2sMemsConfigLeftChannel, false);
  i2sSampler->start();
  // set up the i2s sample writer task
  TaskHandle_t i2sMemsWriterTaskHandle;
  xTaskCreatePinnedToCore(i2sMemsWriterTask, "I2S Writer Task", 4096, i2sSampler, 1, &i2sMemsWriterTaskHandle, 1);

  // // start sampling from i2s device
}

void loop() {
 
}
