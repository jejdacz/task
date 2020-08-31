#include <ESP8266WiFi.h>
#include "Task.h"


#define TASK_PERIOD 60            //seconds
#define TASK_DURATION 20 //seconds

void (*btp)(){&tbt};
void (*etp)(){&tet};

Task task(TASK_PERIOD,TASK_DURATION,btp,etp);

void tbt()
{
  Serial.println("Begin Task");
}

void tet()
{
  Serial.println("End Task");
}

void setup()
{

  task.initTimer();

  Serial.begin(115200);
  delay(1000);
  Serial.println();
  
  WiFi.mode(WIFI_OFF);
  task.handle();

  task.sleep();
  
}

void loop()
{
}


