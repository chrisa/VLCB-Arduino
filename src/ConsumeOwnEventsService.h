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
  virtual void setController(Controller *cntrl) override;
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
  void put(const VlcbMessage *msg);
  VlcbMessage *peek(void);
  VlcbMessage *get(void);
//  bool full(void);
  void clear(void);
  bool empty(void);
  byte bufUse(void);
  byte free_slots(void);
  unsigned int puts();
  unsigned int gets();
//  byte hwm(void);
  unsigned int overflows(void);
    
private:
  Controller *controller;
  Configuration *module_config;  // Shortcut to reduce indirection code.
  
  byte numItems = 4;
  
  VlcbMessage *buffer = malloc(numItems * sizeof(VlcbMessage));
  
  byte head = 0;
  byte tail = 0;
  byte hwm = 0;
  byte capacity = numItems;
  byte size = 0;
  byte numPuts = 0;
  byte numGets = 0;
  byte numOverflows = 0;
  bool full = false;
  
};

}  // VLCB