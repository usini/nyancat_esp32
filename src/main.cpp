#include <TFT_eSPI.h>
#include <SPI.h>
#include "FS.h"
#include <LITTLEFS.h>
#include <AnimatedGIF.h>
#include "nyancat.h"

#include "AudioFileSourceLittleFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceLittleFS *mp3_file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;

TFT_eSPI tft = TFT_eSPI(); // Ecran TFT
AnimatedGIF gif;
#include "gifdraw.h"
TaskHandle_t Task1;

void Task1code(void *parameter)
{
  for (;;)
  {
    if (gif.open((uint8_t *)nyancat, sizeof(nyancat), GIFDraw))
    {
      Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
      tft.startWrite(); // The TFT chip select is locked low
      while (gif.playFrame(true, NULL))
      {
        yield();
      }
      gif.close();
      tft.endWrite(); // Release TFT chip select for other SPI devices
    }
  }
}

void setup_tft()
{
  Serial.begin(115200);
  tft.begin();
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS is not properly setup");
    return;
  }
  else
  {
    Serial.println("Little FS OK!");
  }
  // tft.setSwapBytes(true);
  xTaskCreatePinnedToCore(
      Task1code, /* Function to implement the task */
      "Task1",   /* Name of the task */
      10000,     /* Stack size in words */
      NULL,      /* Task input parameter */
      2,         /* Priority of the task */
      &Task1,    /* Task handle. */
      0);        /* Core where the task should run */

  gif.begin(BIG_ENDIAN_PIXELS);

  audioLogger = &Serial;
  mp3_file = new AudioFileSourceLittleFS("/nyan.mp3");
  id3 = new AudioFileSourceID3(mp3_file);
  out = new AudioOutputI2S(0, 1);

  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);
}

void setup()
{
  setup_tft();
}

void loop()
{

  if (mp3->isRunning())
  {
    if (!mp3->loop())
    {
      mp3->stop();
      delete mp3_file;
    }
  }
  else
  {
    mp3_file = new AudioFileSourceLittleFS("/nyan.mp3");
    mp3->begin(mp3_file, out);
  }
}