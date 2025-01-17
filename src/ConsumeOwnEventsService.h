// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

class Configuration;

class ConsumeOwnEventsService : public Service
{
public:
  ConsumeOwnEventsService(byte bufferCapacity = 4);
  virtual Processed handleMessage(unsigned int opc, VlcbMessage *msg) override {return NOT_PROCESSED;}
  
  virtual byte getServiceID() override 
  {
    return SERVICE_ID_CONSUME_OWN_EVENTS;
  }
  virtual byte getServiceVersionID() override 
  {
    return 1;
  }
  
  bool available(void);
  VlcbMessage *peek(void);
  VlcbMessage *get(void);
  void put(const VlcbMessage *msg);
  void clear(void);
  
  // Diagnostic metrics access
  unsigned int getNumberOfPuts(void);
  unsigned int getNumberofGets(void);
  unsigned int getOverflows(void);
  unsigned int getHighWaterMark(void);   // High Watermark
    
private:
  byte bufUse(void);

  byte capacity;
  byte head = 0;
  byte tail = 0;
  byte size = 0;
  bool full = false;
  
  // Diagnostic metrics
  byte hwm = 0;  // High watermark
  byte numPuts = 0;
  byte numGets = 0;
  byte numOverflows = 0;
  
  VlcbMessage *buffer;
};

}  // VLCB