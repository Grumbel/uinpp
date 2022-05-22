#include <iostream>
#include <linux/input.h>
#include <unistd.h>

#include "uinput.hpp"

int main()
{
  uinpp::UInput uinput;
  uinput.add_key(uinpp::DEVICEID_KEYBOARD, KEY_A);
  uinput.finish();

  while(1) {
    uinput.send(uinpp::DEVICEID_KEYBOARD, EV_KEY, KEY_A, 1);
    uinput.sync();
    uinput.send(uinpp::DEVICEID_KEYBOARD, EV_KEY, KEY_A, 0);
    uinput.sync();
    sleep(1);
  }

  return 0;
}

/* EOF */
