/*
 *
 */

#include "Task.h"

Task::Task(uint32_t taskPeriod, uint32_t taskDuration, void (*beginTask)(), void (*endTask)())
{
  this->_taskPeriod = taskPeriod;
  this->_taskDuration = taskDuration;
  this->_beginTask = beginTask;
  this->_endTask = endTask;
  this->_beginTask();

  this->_sleepCycle = min((uint32_t)(MAX_SLEEP_CYCLE), (taskPeriod - taskDuration));
}

void Task::beginTask()
{
  (*this->_beginTask)();
}

void Task::endTask()
{
  (*this->_endTask)();
}

void Task::initTimer()
{
  this->_initTimestamp = millis();
}

void Task::readRTCMemory()
{
  // Read struct from RTC memory
  if (ESP.rtcUserMemoryRead(0, (uint32_t *)&this->_rtcData, sizeof(this->_rtcData)))
  {
    Serial.println("Read: ");
    Serial.println(this->_rtcData.seconds);
    Serial.println(this->_rtcData.checkSum);
    Serial.println(this->_rtcData.sleepDuration);
    Serial.println(this->_rtcData.taskSeconds);
    Serial.println(this->_rtcData.taskInProgress);
    Serial.println();
  }
}

void Task::handle()
{
  this->readRTCMemory();
  // check first run
  if (this->_rtcData.checkSum != calcCheckSum())
  {
    // init values
    this->_rtcData.sleepDuration = this->_sleepCycle;
    this->_rtcData.seconds = this->_taskPeriod; // leads to start TASK on power up
    this->_rtcData.taskInProgress = false;
    this->_rtcData.taskSeconds = 0;
  }
  else
  {
    // increment main counter
    this->_rtcData.seconds += this->_rtcData.sleepDuration;
    Serial.print("work time: ");
    Serial.println(this->_rtcData.seconds);

    // increment task duration counter
    if (this->_rtcData.taskInProgress)
    {
      this->_rtcData.taskSeconds += this->_rtcData.sleepDuration;
      Serial.print("task duration: ");
      Serial.println(this->_rtcData.taskSeconds);
    }
  }

  this->checkTaskPeriod();
  this->handleTaskInProgress();
}

void Task::checkTaskPeriod()
{
  if (this->_rtcData.seconds >= this->_taskPeriod)
  {
    Serial.println("begining task");

    // reset counter
    this->_rtcData.seconds -= this->_taskPeriod;

    this->_rtcData.taskInProgress = true;
    this->beginTask();
  }
}

void Task::handleTaskInProgress()
{
  if (this->_rtcData.taskInProgress)
  {
    if (this->_rtcData.taskSeconds >= this->_taskDuration)
    {
      Serial.println("ending task");
      this->_endTask();
      // set sleep duration
      this->_rtcData.taskSeconds = 0;
      this->_rtcData.taskInProgress = false;
    }
  }
}

void Task::setSleepDuration()
{
  if (this->_rtcData.taskInProgress)
  {
    this->_rtcData.sleepDuration = min(this->_taskDuration - this->_rtcData.taskSeconds, this->_sleepCycle);
  }
  else
  {
    this->_rtcData.sleepDuration = min(this->_taskPeriod - this->_rtcData.seconds, this->_sleepCycle);
  }
}

void Task::sleep()
{
  // update counter
  this->_rtcData.seconds += (millis() - this->_initTimestamp) / 1000;

  this->setSleepDuration();

  // update check sum
  this->_rtcData.checkSum = this->calcCheckSum();

  // write to RTC memory
  if (ESP.rtcUserMemoryWrite(0, (uint32_t *)&this->_rtcData, sizeof(this->_rtcData)))
  {
    Serial.println("Writing...");
  }

  Serial.print("work time: ");
  Serial.println(this->_rtcData.seconds);

  Serial.print("Going into deep sleep for ");
  Serial.print(this->_rtcData.sleepDuration);
  Serial.println(" seconds.");

  ESP.deepSleep(this->_rtcData.sleepDuration * 1e6);
}

uint32_t Task::calcCheckSum()
{
  return FIRSTRUN_SEED + this->_rtcData.seconds;
}
