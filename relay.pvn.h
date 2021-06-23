#ifndef RELAY_PVN_H
#define RELAY_PVN_H
#include <Arduino.h>
#ifdef PCF8575_LIB_VERSION
#include "PCF8575.h"
#endif

#define EVERY_MS(x)                  \
  static uint32_t tmr = -(x);        \
  bool flag = millis() - tmr >= (x); \
  if (flag)                          \
    tmr += (x);                      \
  if (flag)

class relay_pvn
{
private:
  uint8_t count;
  bool *state;
  bool *onState;
  uint8_t *PIN;
  uint32_t *pulseWidth;
  uint32_t *lastOn;
  void writePin(uint8_t pinNumber, uint8_t value);
  //void initPin(uint8_t pinNumber, uint8_t mode);
  bool readPin(uint8_t pinNumber);
#ifdef PCF8575_LIB_VERSION
  PCF8575 *ext_pcf8575;
#endif
  uint8_t type;
  enum relay_pin_type
  {
    PHYSICAL_PIN,
    PCF8575_PIN
  };

public:
  relay_pvn(const uint8_t relayCount, const uint8_t *RELAY_PIN_NUMBERS, const bool RELAY_ON = true);
  #ifdef PCF8575_LIB_VERSION
  relay_pvn(const uint8_t relayCount, const uint8_t *RELAY_PIN_NUMBERS, PCF8575 &ext, const bool RELAY_ON = true);
  #endif
  ~relay_pvn();
  bool setPulseWidth(uint8_t number, uint32_t pulseWidth);
  uint32_t getPulseWidth(uint8_t number);
  bool setOnState(uint8_t number, bool state);
  bool switchOn(uint8_t number);
  bool switchOff(uint8_t number);
  bool invert(uint8_t number);
  bool getState(uint8_t number);
  bool pulseOn(uint8_t number);
  bool pulseOn(uint8_t number, uint32_t pulseWidth);
  bool init();
  bool init(bool *onState);
  bool init(bool *onState, bool *startState);
  bool init0(bool *startState);
  bool loop();
  bool loop(uint32_t millisLoop);
};

relay_pvn::relay_pvn(uint8_t relayCount, const uint8_t *RELAY_PIN_NUMBERS, const bool RELAY_ON)
{
  count = relayCount;
  state = (bool *)malloc(count);
  PIN = new uint8_t[count];
  onState = (bool *)malloc(count);
  pulseWidth = new uint32_t[count];
  lastOn = new uint32_t[count];
  for (uint8_t i = 0; i < count; i++)
  {
    this->PIN[i] = RELAY_PIN_NUMBERS[i];
    this->state[i] = !RELAY_ON;
    this->onState[i] = RELAY_ON;
    this->pulseWidth[i] = 0;
    this->lastOn[i] = 0;
  }
  type = PHYSICAL_PIN;
};
#ifdef PCF8575_LIB_VERSION
relay_pvn::relay_pvn(const uint8_t relayCount, const uint8_t *RELAY_PIN_NUMBERS, PCF8575 &ext, const bool RELAY_ON)
{
  count = relayCount;
  state = (bool *)malloc(count);
  PIN = new uint8_t[count];
  onState = (bool *)malloc(count);
  pulseWidth = new uint32_t[count];
  lastOn = new uint32_t[count];
  for (uint8_t i = 0; i < count; i++)
  {
    this->PIN[i] = RELAY_PIN_NUMBERS[i];
    this->state[i] = !RELAY_ON;
    this->onState[i] = RELAY_ON;
    this->pulseWidth[i] = 0;
    this->lastOn[i] = 0;
  }
  this->ext_pcf8575 = &ext;
  type = PCF8575_PIN;
};
#endif
relay_pvn::~relay_pvn()
{
  count = 0;
  delete[] state;
  delete[] onState;
  delete[] PIN;
  delete[] pulseWidth;
  delete[] lastOn;
}

bool relay_pvn::setPulseWidth(uint8_t number, uint32_t pulseWidth)
{
  if (number < count)
  {
    this->pulseWidth[number] = pulseWidth;
    return true;
  }
  return false;
}

uint32_t relay_pvn::getPulseWidth(uint8_t number)
{
  if (number < count)
  {
    return pulseWidth[number];
  }
  return false;
}

bool relay_pvn::setOnState(uint8_t number, bool state)
{
  if (number < count)
  {
    this->onState[number] = state;
    return true;
  }
  return false;
}

bool relay_pvn::pulseOn(uint8_t number)
{
  if (number < count)
  {
    lastOn[number] = millis();
    switchOn(number);
    return true;
  }
  return false;
}

bool relay_pvn::pulseOn(uint8_t number, uint32_t pulseWidth)
{
  if (number < count)
  {
    this->pulseWidth[number] = pulseWidth;
    this->lastOn[number] = millis();
    switchOn(number);
    Serial.println("Relay pulse " + String(this->pulseWidth[number]) + " " + String(this->lastOn[number]));
    return true;
  }
  return false;
}

bool relay_pvn::init()
{
  if (type == PHYSICAL_PIN)
  {
    for (uint8_t i = 0; i < count; i++)
    {
      pinMode(PIN[i], OUTPUT);
      writePin(PIN[i], !onState[i]);
    }
  }
#ifdef PCF8575_LIB_VERSION
  else if (type == PCF8575_PIN)
  {
    ext_pcf8575->begin();
    for (uint8_t i = 0; i < count; i++)
    {
      ext_pcf8575->write(PIN[i], !onState[i]);
    }
  }
#endif
  else
  {
    return false;
  }
  return true;
}

/*
init with onState array
*/
bool relay_pvn::init(bool *onState)
{
  for (uint8_t i = 0; i < count; i++)
  {
    this->onState[i] = onState[i];
  }
  return init();
}

bool relay_pvn::init(bool *onState, bool *startState)
{
  for (uint8_t i = 0; i < count; i++)
  {
    this->onState[i] = onState[i];
  }
  return init0(startState);
}

bool relay_pvn::init0(bool *startState)
{
  if (type == PHYSICAL_PIN)
  {
    for (uint8_t i = 0; i < count; i++)
    {
      pinMode(PIN[i], OUTPUT);
      Serial.print("relayStSt: ");
      Serial.print(startState[i]);
      Serial.print(" onState: ");
      Serial.print(onState[i]);
      writePin(PIN[i], startState[i] ? onState[i] : !onState[i]);
    }
  }
#ifdef PCF8575_LIB_VERSION
  else if (type == PCF8575_PIN)
  {
    ext_pcf8575->begin();
    for (uint8_t i = 0; i < count; i++)
    {
      ext_pcf8575->write(PIN[i], startState[i] ? onState[i] : !onState[i]);
    }
  }
#endif
  else
  {
    return false;
  }
  return true;
}

bool relay_pvn::switchOn(uint8_t number)
{
  if (number < count)
  {
    writePin(PIN[number], onState[number]);
    state[number] = onState[number];
    return true;
  }
  return false;
}

bool relay_pvn::switchOff(uint8_t number)
{
  if (number < count)
  {
    writePin(PIN[number], !onState[number]);
    state[number] = !onState[number];
    return true;
  }
  return false;
}

bool relay_pvn::invert(uint8_t number)
{
  if (getState(number))
  {
    return switchOff(number);
  }
  else
  {
    return switchOn(number);
  }
}

bool relay_pvn::getState(uint8_t number)
{
  bool tmp{0};
  if (number < count)
  {
    tmp = (onState[number] == readPin(PIN[number]));
  }
  return tmp;
}

bool relay_pvn::loop()
{
  return loop(millis());
}

bool relay_pvn::loop(uint32_t millisLoop)
{
  for (uint8_t i = 0; i < count; i++)
  {
    if (state[i] == onState[i])
    {
      if (pulseWidth[i])
      {
        if (millisLoop > lastOn[i] + pulseWidth[i])
        {
          switchOff(i);
        }
      }
    }
  }
  return true;
}

void relay_pvn::writePin(uint8_t pinNumber, uint8_t value)
{
  if (type == PHYSICAL_PIN)
  {
    digitalWrite(pinNumber, value);
    Serial.println("phys writePin" + String(pinNumber) + " " + String(value));
    state[pinNumber] = value;
  }
#ifdef PCF8575_LIB_VERSION
  else if (type == PCF8575_PIN)
  {
    ext_pcf8575->write(pinNumber, value);
    Serial.println("pcf8575 writePin" + String(pinNumber) + " " + String(value));
    state[pinNumber] = value;
  }
#endif
};
/*
void relay_pvn::initPin(uint8_t pinNumber, uint8_t mode)
{
  pinMode(pinNumber, mode);
};
*/
bool relay_pvn::readPin(uint8_t pinNumber)
{
  if (type == PHYSICAL_PIN)
  {
    state[pinNumber] = digitalRead(pinNumber);
    return state[pinNumber];
  }
#ifdef PCF8575_LIB_VERSION
  else if (type == PCF8575_PIN)
  {
    state[pinNumber] = ext_pcf8575->read(pinNumber);
    return state[pinNumber];
  }
#endif
  else
    return false;
};

#endif