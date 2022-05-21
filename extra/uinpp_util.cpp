#include <iostream>
#include <linux/input.h>
#include <unistd.h>

#include "uinput.hpp"

int main()
{
  UInput uinput(false);
  uinput.add_key(DEVICEID_KEYBOARD, KEY_A);
  uinput.finish();

  while(1) {
    uinput.send(DEVICEID_KEYBOARD, EV_KEY, KEY_A, 1);
    uinput.sync();
    uinput.send(DEVICEID_KEYBOARD, EV_KEY, KEY_A, 0);
    uinput.sync();
    sleep(1);
  }

  return 0;
}

/* EOF */
