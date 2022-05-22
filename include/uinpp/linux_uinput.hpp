/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HEADER_LINUX_UINPUT_HPP
#define HEADER_LINUX_UINPUT_HPP

#include <string>
#include <functional>
#include <linux/uinput.h>
#include <stdint.h>

class ForceFeedbackHandler;

class LinuxUinput
{
public:
  enum DeviceType { kGenericDevice, kKeyboardDevice, kMouseDevice, kJoystickDevice };

public:
  LinuxUinput(DeviceType device_type, const std::string& name,
              const struct input_id& usbid_);
  ~LinuxUinput();

  /*@{*/
  /** Create an absolute axis */
  void add_abs(uint16_t code, int min, int max, int fuzz = 0, int flat = 0);

  /** Create an button */
  void add_key(uint16_t code);

  /** Create a relative axis (mice) */
  void add_rel(uint16_t code);

  void add_ff(uint16_t code);

  void set_ff_callback(const std::function<void (uint8_t, uint8_t)>& callback);

  /** Finalized the device creation */
  void finish();
  /*@}*/

  /** Send an input event */
  void send(uint16_t type, uint16_t code, int32_t value);

  /** Sends out a sync event if there is a need for it. */
  void sync();

  /** Update force feedback */
  void update(int msec_delta);

  /** Read incoming data (force feedback, led, etc.), non-blocking */
  void read();

  /** file handle to the underlying device */
  int get_fd() const { return m_fd; }

private:
  DeviceType  m_device_type;
  std::string name;
  struct input_id usbid;

  bool m_finished;

  int m_fd;

  uinput_user_dev user_dev;
  bool key_bit;
  bool rel_bit;
  bool abs_bit;
  bool led_bit;
  bool ff_bit;

  bool abs_lst[ABS_CNT];
  bool rel_lst[REL_CNT];
  bool key_lst[KEY_CNT];
  bool ff_lst[FF_CNT];

  ForceFeedbackHandler* m_ff_handler;
  std::function<void (uint8_t, uint8_t)> m_ff_callback;

  bool needs_sync;

private:
  LinuxUinput (const LinuxUinput&) = delete;
  LinuxUinput& operator= (const LinuxUinput&) = delete;
};

#endif

/* EOF */
