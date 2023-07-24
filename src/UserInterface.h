#pragma once

class UserInterface
{
public:

  enum RequestedAction {
    NONE,
    CHANGE_MODE,
    RENEGOTIATE,
    ENUMERATION
  };

  virtual void run() = 0;
  virtual void indicateResetting() =0;
  virtual void indicateResetDone() = 0;
  virtual void indicateActivity() = 0;
  virtual void indicateMode(byte mode) = 0;
  virtual bool resetRequested() = 0;
  virtual RequestedAction checkRequestedAction() = 0;
};