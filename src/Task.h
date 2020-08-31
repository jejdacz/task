#pragma once

#include "Arduino.h"

#define MAX_SLEEP_CYCLE 10 // seconds
#define FIRSTRUN_SEED 125

typedef struct
    {
      uint32_t seconds;
      uint32_t sleepDuration;
      uint32_t taskSeconds;
      boolean taskInProgress;
      uint32_t checkSum;
    } rtcMemStore __attribute__((aligned(4)));

class Task {
  private:
    uint32_t _taskPeriod;
    uint32_t _taskDuration;    
    void (*_beginTask)();
    void (*_endTask)();
    uint32_t _initTimestamp;
    rtcMemStore _rtcData;
    uint32_t _sleepCycle;
    
    void beginTask();
    void endTask();
    void checkTaskPeriod();
    void handleTaskInProgress();
    void readRTCMemory();
    void setSleepDuration(); 
    uint32_t calcCheckSum(); 

  public:
    Task(uint32_t taskPeriod, uint32_t taskDuration, void (*beginTask)(), void (*endTask)());
    void initTimer();
    void handle();
    void sleep();       
};
