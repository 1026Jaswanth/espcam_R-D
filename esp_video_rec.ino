// this code is intended to record video and save ** TechVTJ ** source
#include "esp_camera.h" 
#include "Arduino.h" 
#include "FS.h"                // SD Card ESP32 
#include "SD_MMC.h"            // SD Card ESP32 
#include "soc/soc.h"           // Disable brownout problems 
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems 
#include "driver/rtc_io.h" 
#include <EEPROM.h>            // read and write from flash memory 
 
#define EEPROM_SIZE 1 
 
#define PWDN_GPIO_NUM     32 
#define RESET_GPIO_NUM    -1 
#define XCLK_GPIO_NUM      0 
#define SIOD_GPIO_NUM     26 
#define SIOC_GPIO_NUM     27 
 
#define Y9_GPIO_NUM       35 
#define Y8_GPIO_NUM       34 
#define Y7_GPIO_NUM       39 
#define Y6_GPIO_NUM       36 
#define Y5_GPIO_NUM       21 
#define Y4_GPIO_NUM       19 
#define Y3_GPIO_NUM       18 
#define Y2_GPIO_NUM        5 
#define VSYNC_GPIO_NUM    25 
#define HREF_GPIO_NUM     23 
#define PCLK_GPIO_NUM     22 
 
int videoNumber = 0; 
 
void setup() { 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector 
 
  Serial.begin(115200); 
 
  camera_config_t config; 
  config.ledc_channel = LEDC_CHANNEL_0; 
  config.ledc_timer = LEDC_TIMER_0; 
  config.pin_d0 = Y2_GPIO_NUM; 
  config.pin_d1 = Y3_GPIO_NUM; 
  config.pin_d2 = Y4_GPIO_NUM; 
  config.pin_d3 = Y5_GPIO_NUM; 
  config.pin_d4 = Y6_GPIO_NUM; 
  config.pin_d5 = Y7_GPIO_NUM; 
  config.pin_d6 = Y8_GPIO_NUM; 
  config.pin_d7 = Y9_GPIO_NUM; 
  config.pin_xclk = XCLK_GPIO_NUM; 
  config.pin_pclk = PCLK_GPIO_NUM; 
  config.pin_vsync = VSYNC_GPIO_NUM; 
  config.pin_href = HREF_GPIO_NUM; 
  config.pin_sscb_sda = SIOD_GPIO_NUM; 
  config.pin_sscb_scl = SIOC_GPIO_NUM; 
  config.pin_pwdn = PWDN_GPIO_NUM; 
  config.pin_reset = RESET_GPIO_NUM; 
  config.xclk_freq_hz = 20000000; 
  config.pixel_format = PIXFORMAT_JPEG; 
 
  if (psramFound()) { 
    config.frame_size = FRAMESIZE_UXGA; 
    config.jpeg_quality = 10; 
    config.fb_count = 2; 
  } else { 
    config.frame_size = FRAMESIZE_SVGA; 
    config.jpeg_quality = 12; 
    config.fb_count = 1; 
  } 
 
  // Initialize Camera 
  esp_err_t err = esp_camera_init(&config); 
  if (err != ESP_OK) { 
    Serial.printf("Camera init failed with error 0x%x", err); 
    return; 
  } 
 
  // Mount SD Card 
  if (!SD_MMC.begin()) { 
    Serial.println("SD Card Mount Failed"); 
    return; 
  } 
 
  // Initialize EEPROM 
  EEPROM.begin(EEPROM_SIZE); 
  videoNumber = EEPROM.read(0) + 1; 
} 
 
void loop() { 
  camera_fb_t * fb = NULL; 
   
  // Start recording video 
  Serial.println("Recording video..."); 
 
  String path = "/video" + String(videoNumber) + ".mp4"; // Change extension to .mp4 if needed 
  fs::FS &fs = SD_MMC; 
 
  Serial.printf("Video file name: %s\n", path.c_str()); 
  File file = fs.open(path.c_str(), FILE_WRITE); 
   
  if (!file) { 
    Serial.println("Failed to open file in writing mode"); 
    return; 
  }  
 
  // Record video for 10 seconds 
  for (int i = 0; i < 100; i++) { 
    fb = esp_camera_fb_get(); 
    if (!fb) { 
      Serial.println("Camera capture failed"); 
      break; 
    } 
    file.write(fb->buf, fb->len); 
    esp_camera_fb_return(fb); 
    delay(100); // Adjust delay as per desired frame rate 
  } 
 
  file.close(); 
  Serial.printf("Saved file to path: %s\n", path.c_str()); 
 
  // Increment video number and store in EEPROM 
  videoNumber++; 
  EEPROM.write(0, videoNumber); 
  EEPROM.commit(); 
  Serial.println("Video recording finished."); 
  delay(1000); // Delay before starting the next recording
}
