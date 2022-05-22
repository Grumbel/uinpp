/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
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

#include "ui_event.hpp"

#include <assert.h>
#include <fmt/format.h>

#include "uinput.hpp"

namespace uinpp {

bool
UIEvent::is_mouse_button(int ev_code)
{
  return  (ev_code >= BTN_MOUSE && ev_code <= BTN_TASK);
}

bool
UIEvent::is_keyboard_button(int ev_code)
{
  return (ev_code < 256);
}

UIEvent
UIEvent::create(uint16_t device_id, int type, int code)
{
  UIEvent ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = device_id;
  ev.m_device_id_resolved = false;
  ev.type      = type;
  ev.code      = code;
  return ev;
}

UIEvent
UIEvent::invalid()
{
  UIEvent ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = DEVICEID_INVALID;
  ev.m_device_id_resolved = false;
  ev.type      = -1;
  ev.code      = -1;
  return ev;
}

bool
UIEvent::operator<(const UIEvent& rhs)  const
{
  // BROKEN: must take all members into account
  if (m_device_id == rhs.m_device_id)
  {
    if (type == rhs.type)
    {
      return code < rhs.code;
    }
    else if (type > rhs.type)
    {
      return false;
    }
    else // if (type < rhs.type)
    {
      return true;
    }
  }
  else if (m_device_id > rhs.m_device_id)
  {
    return false;
  }
  else // if (device_id < rhs.device_id)
  {
    return true;
  }
}

void
UIEvent::resolve_device_id(int slot, bool extra_devices)
{
  assert(!m_device_id_resolved);

  if (m_slot_id == SLOTID_AUTO)
  {
    m_slot_id = static_cast<uint16_t>(slot);
  }

  if (m_device_id == DEVICEID_AUTO)
  {
    if (extra_devices)
    {
      switch(type)
      {
        case EV_KEY:
          if (is_mouse_button(code))
          {
            m_device_id = DEVICEID_MOUSE;
          }
          else if (is_keyboard_button(code))
          {
            m_device_id = DEVICEID_KEYBOARD;
          }
          else
          {
            m_device_id = DEVICEID_JOYSTICK;
          }
          break;

        case EV_REL:
          m_device_id = DEVICEID_MOUSE;
          break;

        case EV_ABS:
          m_device_id = DEVICEID_JOYSTICK;
          break;
      }
    }
    else
    {
      m_device_id = DEVICEID_JOYSTICK;
    }
  }

  m_device_id_resolved = true;
}

uint32_t
UIEvent::get_device_id() const
{
  assert(m_device_id_resolved);

  return UInput::create_device_id(m_slot_id, m_device_id);
}

uint16_t str2deviceid(const std::string& device)
{
  if (device == "auto" || device.empty())
  {
    return DEVICEID_AUTO;
  }
  else if (device == "mouse")
  {
    return DEVICEID_MOUSE;
  }
  else if (device == "keyboard" || device == "key")
  {
    return DEVICEID_KEYBOARD;
  }
  else if (device == "joystick" || device == "joy")
  {
    return DEVICEID_JOYSTICK;
  }
  else
  {
    int result = std::stoi(device);
    if (result < 0 || result > std::numeric_limits<uint16_t>::max())
    {
      throw std::runtime_error(fmt::format("str2deviceid(): out of range: '{}'", device));
    }
    else
    {
      return static_cast<uint16_t>(result);
    }
  }
}

uint16_t str2slotid(const std::string& slot)
{
  if (slot == "auto" || slot.empty())
  {
    return SLOTID_AUTO;
  }
  else
  {
    int result = std::stoi(slot);
    if (result < 0 || result > std::numeric_limits<uint16_t>::max())
    {
      throw std::runtime_error(fmt::format("str2deviceid(): out of range: '{}'", slot));
    }
    else
    {
      return static_cast<uint16_t>(result);
    }
  }
}

void split_event_name(const std::string& str, std::string* event_str, int* slot_id, int* device_id)
{
  std::string::size_type p = str.find('@');
  if (p == std::string::npos)
  {
    *event_str = str;
    *slot_id   = SLOTID_AUTO;
    *device_id = DEVICEID_AUTO;
  }
  else
  {
    *event_str = str.substr(0, p);
    std::string device = str.substr(p+1);

    p = device.find(".");

    if (p == std::string::npos)
    {
      *slot_id   = SLOTID_AUTO;
      *device_id = str2deviceid(device);
    }
    else
    {
      *slot_id   = str2slotid(device.substr(p+1));
      *device_id = str2deviceid(device.substr(0, p));
    }
  }
}

} // namespace uinpp

/* EOF */
