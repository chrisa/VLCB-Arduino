// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// 3rd party libraries
#include <Streaming.h>

// Controller library
#include <Controller.h>
#include "VlcbDefs.h"
#include "CanService.h"

//
/// construct a Controller object with an external Configuration object named "config" that is defined
/// in user code
//

namespace VLCB
{

Controller::Controller(UserInterface * ui, Transport * trpt, std::initializer_list<Service *> services)
  : _ui(ui)
  , transport(trpt)
  , services(services)
{
  extern Configuration config;
  module_config = &config;

  transport->setController(this);

  for (Service * service : services)
  {
    service->setController(this);
  }
}

//
/// construct a Controller object with a Configuration object that the user provides.
/// note that this Configuration object must have a lifetime longer than the Controller object.
//
Controller::Controller(UserInterface * ui, Configuration *conf, Transport * trpt, std::initializer_list<Service *> services)
  : _ui(ui)
  , module_config(conf)
  , transport(trpt)
  , services(services)
{
  transport->setController(this);

  for (Service * service : services)
  {
    service->setController(this);
  }
}

//
/// register the user handler for CAN frames
/// default args in .h declaration for opcodes array (NULL) and size (0)
//
void Controller::setFrameHandler(void (*fptr)(CANFrame *msg), byte opcodes[], byte num_opcodes)
{
  framehandler = fptr;
  _opcodes = opcodes;
  _num_opcodes = num_opcodes;
}

//
/// assign the module parameter set
//
void Controller::setParams(unsigned char *mparams)
{
  _mparams = mparams;
}

//
/// assign the module name
//
void Controller::setName(unsigned char *mname)
{
  _mname = mname;
}

//
/// is this an Extended CAN frame ?
//
bool Controller::isExt(CANFrame *amsg)
{
  return (amsg->ext);
}

//
/// is this a Remote frame ?
//
bool Controller::isRTR(CANFrame *amsg)
{
  return (amsg->rtr);
}

//
/// initiate the transition from SLiM to FLiM mode
//
void Controller::initFLiM()
{
  // DEBUG_SERIAL << F("> initiating FLiM negotation") << endl;

  indicateMode(MODE_CHANGING);

  bModeChanging = true;
  timeOutTimer = millis();

  // send RQNN message with current NN, which may be zero if a virgin/SLiM node
  sendMessageWithNN(OPC_RQNN);

  // DEBUG_SERIAL << F("> requesting NN with RQNN message for NN = ") << module_config->nodeNum << endl;
}

void Controller::setFLiM()
{
  // DEBUG_SERIAL << F("> set FLiM") << endl;
  bModeChanging = false;
  module_config->setModuleMode(MODE_FLIM);
  indicateMode(MODE_FLIM);

  // enumerate the CAN bus to allocate a free CAN ID
  startCANenumeration();
}

//
/// set module to SLiM mode
//
void Controller::setSLiM()
{
  // DEBUG_SERIAL << F("> set SLiM") << endl;
  bModeChanging = false;
  module_config->setNodeNum(0);
  module_config->setModuleMode(MODE_SLIM);
  module_config->setCANID(0);

  indicateMode(MODE_SLIM);
}

//
/// revert from FLiM to SLiM mode
//
void Controller::revertSLiM()
{

  // DEBUG_SERIAL << F("> reverting to SLiM mode") << endl;

  // send NNREL message

  sendMessageWithNN(OPC_NNREL);
  setSLiM();
}

//
/// check 30 sec timeout for SLiM/FLiM negotiation with FCU
//
void Controller::checkModeChangeTimeout()
{
  if (bModeChanging && ((millis() - timeOutTimer) >= 30000)) {

    // Revert to previous mode.
    // DEBUG_SERIAL << F("> timeout expired, currentMode = ") << currentMode << F(", mode change = ") << bModeChanging << endl;
    indicateMode(module_config->currentMode);
    bModeChanging = false;
  }
}

//
/// change or re-confirm node number
//

void Controller::renegotiate()
{
  initFLiM();
}

//
/// set the Controller LEDs to indicate the current mode
//

void Controller::indicateMode(byte mode)
{
  // DEBUG_SERIAL << F("> indicating mode = ") << mode << endl;
  if (_ui) {
    _ui->indicateMode(mode);
  }
}

void Controller::indicateActivity()
{
  if (_ui) {
    _ui->indicateActivity();
  }
}

//
/// main Controller message processing procedure
//
void Controller::process(byte num_messages)
{
  // process switch operations if the module is configured with one
  UserInterface::RequestedAction requestedAction = UserInterface::NONE;
  if (_ui)
  {
    _ui->run();

    requestedAction = _ui->checkRequestedAction();
    performRequestedUserAction(requestedAction);
  }

  for (Service * service : services)
  {
    service->process(requestedAction);
  }

  // get received CAN frames from buffer
  // process by default 3 messages per run so the user's application code doesn't appear unresponsive under load
  for (byte mcount = 0 ; transport->available() && mcount < num_messages ; ++mcount)
  {
    // at least one CAN frame is available in the reception buffer
    // retrieve the next one

    _msg = transport->getNextMessage();

    callFrameHandler(&_msg);

    for (Service * service : services)
    {
      // DEBUG_SERIAL << "> Passing raw message to " << endl;
      if (service->handleRawMessage(&_msg) == PROCESSED)
      {
        // DEBUG_SERIAL << "> Service handled raw message" << endl;
        break;
      }
    }

    indicateActivity();

    // is this an extended frame ? we currently ignore these as bootloader, etc data may confuse us !
    if (_msg.ext)
    {
      // DEBUG_SERIAL << F("> extended frame ignored, from CANID = ") << remoteCANID << endl;
      continue;
    }

    //
    /// process the message opcode
    /// if we got this far, it's a standard CAN frame (not extended, not RTR) with a data payload length > 0
    //

    if (_msg.len > 0) 
    {
      unsigned int opc = _msg.data[0];
      for (Service * service : services)
      {
        if (service->handleMessage(opc, &_msg) == PROCESSED)
        {
          break;
        }
      }
    } else {
      // DEBUG_SERIAL << F("> oops ... zero - length frame ?? ") << endl;
    }
  }  // while messages available

  checkModeChangeTimeout();

  // DEBUG_SERIAL << F("> end of opcode processing, time = ") << (micros() - mtime) << "us" << endl;

  //
  /// end of Controller message processing
  //
}

void Controller::performRequestedUserAction(UserInterface::RequestedAction requestedAction)
{
  switch (requestedAction)
  {
    case UserInterface::CHANGE_MODE:
      // initiate mode change
      //Serial << "Controller::process() - changing mode, current mode=" << module_config->currentMode << endl;
      if (!module_config->currentMode)
      {
        initFLiM();
      }
      else
      {
        revertSLiM();
      }
      break;

    case UserInterface::RENEGOTIATE:
      //Serial << "Controller::process() - renegotiate" << endl;
      renegotiate();
      break;

    default:
      break;
  }
}

// Return true if framehandler shall be called for registered opcodes, if any.
bool Controller::filterByOpcodes(const CANFrame *msg) const
{
  if (_num_opcodes == 0)
  {
    return true;
  }
  if (msg->len > 0)
  {
    unsigned int opc = msg->data[0];
    for (byte i = 0; i < _num_opcodes; i++)
    {
      if (opc == _opcodes[i])
      {
        return true;
      }
    }
  }
  return false;
}

void Controller::callFrameHandler(CANFrame *msg)
{
  if (framehandler != NULL)
  {
    if (filterByOpcodes(msg))
    {
      (void)(*framehandler)(msg);
    }
  }
}

void setNN(CANFrame *msg, unsigned int nn)
{
  msg->data[1] = highByte(nn);
  msg->data[2] = lowByte(nn);
}

bool Controller::sendMessageWithNN(int opc)
{
  CANFrame msg;
  msg.len = 3;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  return sendMessage(&msg);
}

bool Controller::sendMessageWithNN(int opc, byte b1)
{
  CANFrame msg;
  msg.len = 4;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  msg.data[3] = b1;
  return sendMessage(&msg);
}

bool Controller::sendMessageWithNN(int opc, byte b1, byte b2)
{
  CANFrame msg;
  msg.len = 5;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  msg.data[3] = b1;
  msg.data[4] = b2;
  return sendMessage(&msg);
}

bool Controller::sendMessageWithNN(int opc, byte b1, byte b2, byte b3)
{
  CANFrame msg;
  msg.len = 6;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  msg.data[3] = b1;
  msg.data[4] = b2;
  msg.data[5] = b3;
  return sendMessage(&msg);
}

bool Controller::sendMessageWithNN(int opc, byte b1, byte b2, byte b3, byte b4, byte b5)
{
  CANFrame msg;
  msg.len = 8;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  msg.data[3] = b1;
  msg.data[4] = b2;
  msg.data[5] = b3;
  msg.data[6] = b4;
  msg.data[7] = b5;
  return sendMessage(&msg);
}

//
/// send a WRACK (write acknowledge) message
//
bool Controller::sendWRACK()
{
  // send a write acknowledgement response
  return sendMessageWithNN(OPC_WRACK);
}

//
/// send a CMDERR (command error) message
//
bool Controller::sendCMDERR(byte cerrno)
{
  // send a command error response
  return sendMessageWithNN(OPC_CMDERR, cerrno);
}

void Controller::sendGRSP(byte opCode, byte serviceType, GrspCodes errCode)
{
  sendMessageWithNN(OPC_GRSP, opCode, serviceType, errCode);
}

void Controller::startCANenumeration()
{
  // Delegate this to the CanService.
  // Find it first.
  for (Service * svc : services)
  {
    if (svc->getServiceID() == 3)
    {
      CanService * canSvc = (CanService *) svc;
      canSvc->startCANenumeration();
    }
  }
}

}