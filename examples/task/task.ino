//#include <ESP8266WiFi.h>
#include "TaskManager.h"

#define FIRSTRUN_SEED 125
#define MAX_SLEEP_TIME 60
#define MIN_SLEEP_TIME 1

typedef struct
{
  uint32_t seconds;
  uint32_t checkSum;
  uint32_t sleepTime;
} rtcMemStore __attribute__((aligned(4)));

rtcMemStore rtcData;


TaskManager taskManager;

void tbt()
{
  Serial.println("Begin Task");
  taskManager.addTask(&tet, rtcData.seconds + 20);
  taskManager.addTask(&tbt, rtcData.seconds + 60);
}

void tet()
{
  Serial.println("End Task");
}

void setup()
{
  uint32_t timeStamp = millis();

  Serial.begin(115200);
  delay(500);
  Serial.println();

  if (ESP.rtcUserMemoryRead(0, (uint32_t *)&rtcData, sizeof(rtcData)))
  {
    Serial.println("Read global... ");
    Serial.print("seconds: ");
    Serial.println(rtcData.seconds);
  }

  if (rtcData.checkSum != calcCheckSum())
  {
    Serial.println("checksum failed... ");
    Serial.print("FIRSTRUN_SEED: ");
    Serial.println(FIRSTRUN_SEED);

    rtcData.seconds = millis() / 1000;
    rtcData.sleepTime = 0;

    taskManager.addTask(&tbt, rtcData.seconds + 0);
  }
  else
  {
    if (ESP.rtcUserMemoryRead(16, (uint32_t *)taskManager.getTaskList(), taskManager.getTaskListSize()))
    {
      Serial.println("Read task list: ");
    }
  }

  rtcData.seconds += rtcData.sleepTime;

  taskManager.handle(rtcData.seconds);

  rtcData.seconds += (millis() - timeStamp) / 1000;
  rtcData.sleepTime = max((uint32_t)MIN_SLEEP_TIME, min((uint32_t)(MAX_SLEEP_TIME), taskManager.nextTaskTime(rtcData.seconds) - rtcData.seconds));

  Serial.print("Seconds: ");
  Serial.println(rtcData.seconds);

  rtcData.checkSum = calcCheckSum();

  if (ESP.rtcUserMemoryWrite(0, (uint32_t *)&rtcData, sizeof(rtcData)))
  {
    Serial.println("Writing global data...");
  }

  if (ESP.rtcUserMemoryWrite(16, (uint32_t *)taskManager.getTaskList(), taskManager.getTaskListSize()))
  {
    Serial.println("Writing task list...");
  }

  // Serial.println(taskManager.getTaskListSize());

  Serial.print("next task time: ");
  Serial.println(taskManager.nextTaskTime(rtcData.seconds));

  Serial.print("going sleep for... ");
  Serial.println(rtcData.sleepTime);

  ESP.deepSleep(rtcData.sleepTime * 1e6);
}

void loop()
{
}

uint32_t calcCheckSum()
{
  return FIRSTRUN_SEED + rtcData.seconds;
}

