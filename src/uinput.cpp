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

#include "uinput.hpp"

#include <assert.h>
#include <iostream>
#include <math.h>
#include <stdexcept>
#include <stdio.h>
#include <unistd.h>
#include <sstream>

#include <strut/split.hpp>
#include <logmich/log.hpp>

#include "ui_abs_event_collector.hpp"
#include "ui_key_event_collector.hpp"
#include "ui_rel_event_collector.hpp"

namespace uinpp {

namespace {

int hexstr2int(const std::string& str)
{
  unsigned int value = 0;
  if (sscanf(str.c_str(), "%x", &value) == 1)
  {
    return value;
  }
  else if (sscanf(str.c_str(), "0x%x", &value) == 1)
  {
    return value;
  }
  else
  {
    std::ostringstream err;
    err << "couldn't convert '" << str << "' to int";
    throw std::runtime_error(err.str());
  }
}

uint16_t hexstr2uint16(const std::string& str)
{
  return static_cast<uint16_t>(hexstr2int(str));
}

} // namespace

struct input_id
UInput::parse_input_id(const std::string& str)
{
  struct input_id usbid;

  // default values
  usbid.bustype = BUS_USB;
  usbid.vendor  = 0;
  usbid.product = 0;
  usbid.version = 0;

  // split string at ':'
  std::vector<std::string> args = strut::split(str, ':');

  if (args.size() == 2)
  { // VENDOR:PRODUCT
    usbid.vendor  = hexstr2uint16(args[0]);
    usbid.product = hexstr2uint16(args[1]);
  }
  else if (args.size() == 3)
  { // VENDOR:PRODUCT:VERSION
    usbid.vendor  = hexstr2uint16(args[0]);
    usbid.product = hexstr2uint16(args[1]);
    usbid.version = hexstr2uint16(args[2]);
  }
  else if (args.size() == 4)
  { // VENDOR:PRODUCT:VERSION:BUS
    usbid.vendor  = hexstr2uint16(args[0]);
    usbid.product = hexstr2uint16(args[1]);
    usbid.version = hexstr2uint16(args[2]);
    usbid.bustype = hexstr2uint16(args[3]);
  }
  else
  {
    throw std::runtime_error("incorrect number of arguments");
  }

  return usbid;
}

uint32_t
UInput::parse_device_id(const std::string& str)
{
  // FIXME: insert magic to resolve symbolic names, merge with same code in set_device_name
  std::string::size_type p = str.find('.');

  uint16_t device_id;
  uint16_t slot_id;

  if (p == std::string::npos)
  {
    device_id = str2deviceid(str.substr());
    slot_id   = SLOTID_AUTO;
  }
  else if (p == 0)
  {
    device_id = DEVICEID_AUTO;
    slot_id   = str2slotid(str.substr(p+1));
  }
  else
  {
    device_id = str2deviceid(str.substr(0, p));
    slot_id   = str2slotid(str.substr(p+1));
  }

  return UInput::create_device_id(slot_id, device_id);
}

UInput::UInput() :
  m_uinput_devs(),
  m_device_names(),
  m_device_usbids(),
  m_collectors(),
  m_rel_repeat_lst(),
  m_extra_events(true)
{
}

UInput::~UInput()
{
}

void
UInput::set_extra_events(bool extra_events)
{
  m_extra_events = extra_events;
}

struct input_id
UInput::get_device_usbid(uint32_t device_id) const
{
  uint16_t slot_id = get_slot_id(device_id);
  uint16_t type_id = get_type_id(device_id);

  auto it = m_device_usbids.find(device_id);
  if (it != m_device_usbids.end())
  {
    // found an exact match, return it
    return it->second;
  }
  else
  {
    it = m_device_usbids.find(create_device_id(slot_id, DEVICEID_AUTO));
    if (it != m_device_usbids.end())
    {
      return it->second;
    }
    else
    {
      it = m_device_usbids.find(create_device_id(SLOTID_AUTO, type_id));
      if (it != m_device_usbids.end())
      {
        return it->second;
      }
      else
      {
        struct input_id usbid;
        usbid.bustype = 0;
        usbid.vendor  = 0;
        usbid.product = 0;
        usbid.version = 0;
        return usbid;
      }
    }
  }
}

std::string
UInput::get_device_name(uint32_t device_id) const
{
  uint16_t slot_id = get_slot_id(device_id);
  uint16_t type_id = get_type_id(device_id);

  auto it = m_device_names.find(device_id);
  if (it != m_device_names.end())
  {
    // found an exact match, return it
    return it->second;
  }
  else
  {
    it = m_device_names.find(create_device_id(slot_id, DEVICEID_AUTO));
    if (it != m_device_names.end())
    {
      // found a match for the slot, build a name and return it
      std::ostringstream str;
      str << it->second;
      switch(type_id)
      {
        case 0:
        case DEVICEID_JOYSTICK:
          break;

        case DEVICEID_MOUSE:
          str << " - Mouse Emulation";
          break;

        case DEVICEID_KEYBOARD:
          str << " - Keyboard Emulation";
          break;

        default:
          str << " - " << device_id+1;
          break;
      }
      return str.str();
    }
    else
    {
      it = m_device_names.find(create_device_id(SLOTID_AUTO, type_id));

      if (it != m_device_names.end())
      {
        // found match for the type, build name and return it
        std::ostringstream str;
        str << it->second;
        if (slot_id > 0)
        {
          str << " #" << (slot_id+1);
        }
        return str.str();
      }
      else
      {
        std::ostringstream str;

        it = m_device_names.find(create_device_id(SLOTID_AUTO, DEVICEID_AUTO));

        if (it != m_device_names.end())
        {
          str << it->second;
        }
        else
        {
          str << "Xbox Gamepad (userspace driver)";
        }

        switch(type_id)
        {
          case DEVICEID_JOYSTICK:
            break;

          case DEVICEID_MOUSE:
            str << " - Mouse Emulation";
            break;

          case DEVICEID_KEYBOARD:
            str << " - Keyboard Emulation";
            break;

          default:
            str << " - " << device_id+1;
            break;
        }
        if (slot_id > 0)
        {
          str << " #" << (slot_id+1);
        }
        return str.str();
      }
    }
  }
}

LinuxUinput*
UInput::create_uinput_device(uint32_t device_id)
{
  // DEVICEID_AUTO should not happen at this point as the user should
  // have called resolve_device_id()
  assert(device_id != DEVICEID_AUTO);

  auto const it = m_uinput_devs.find(device_id);
  if (it != m_uinput_devs.end())
  {
    // device already exist, so return it
    return it->second.get();
  }
  else
  {
    log_debug("create device: {}", device_id);
    DeviceType device_type;

    if (!m_extra_events)
    {
      device_type = DeviceType::GENERIC;
    }
    else
    {
      switch (device_id)
      {
        case DEVICEID_JOYSTICK:
          device_type = DeviceType::JOYSTICK;
          break;

        case DEVICEID_MOUSE:
          device_type = DeviceType::MOUSE;
          break;

        case DEVICEID_KEYBOARD:
          device_type = DeviceType::KEYBOARD;
          break;

        default:
          device_type = DeviceType::GENERIC;
          break;
      }
    }

    std::string dev_name = get_device_name(device_id);
    auto dev = std::make_unique<LinuxUinput>(device_type, dev_name, get_device_usbid(device_id));
    LinuxUinput* dev_tmp = dev.get();
    m_uinput_devs.insert(std::pair<int, std::unique_ptr<LinuxUinput> >(device_id, std::move(dev)));

    log_debug("created uinput device: {} - '{}`", device_id, dev_name);

    return dev_tmp;
  }
}

UIEventEmitter*
UInput::add(const UIEvent& ev)
{
  LinuxUinput* dev = create_uinput_device(ev.get_device_id());

  switch(ev.get_type())
  {
    case EV_KEY:
      dev->add_key(static_cast<uint16_t>(ev.get_code()));
      break;

    case EV_REL:
      dev->add_rel(static_cast<uint16_t>(ev.get_code()));
      break;

    case EV_ABS:
      dev->add_abs(static_cast<uint16_t>(ev.get_code()),
                   0, 0, 0, 0 /* min, max, fuzz, flat */); // BROKEN
      break;
  }

  return create_emitter(ev.get_device_id(), ev.get_type(), ev.get_code());
}

UIEventEmitter*
UInput::add_key(uint32_t device_id, int ev_code)
{
  LinuxUinput* dev = create_uinput_device(device_id);
  dev->add_key(static_cast<uint16_t>(ev_code));

  return create_emitter(device_id, EV_KEY, ev_code);
}

UIEventEmitter*
UInput::add_rel(uint32_t device_id, int ev_code)
{
  LinuxUinput* dev = create_uinput_device(device_id);
  dev->add_rel(static_cast<uint16_t>(ev_code));

  return create_emitter(device_id, EV_REL, ev_code);
}

UIEventEmitter*
UInput::add_abs(uint32_t device_id, int ev_code, int min, int max, int fuzz, int flat)
{
  LinuxUinput* dev = create_uinput_device(device_id);
  dev->add_abs(static_cast<uint16_t>(ev_code), min, max, fuzz, flat);

  return create_emitter(device_id, EV_ABS, ev_code);
}

void
UInput::add_ff(uint32_t device_id, uint16_t code)
{
  LinuxUinput* dev = create_uinput_device(device_id);
  dev->add_ff(code);
}

UIEventEmitter*
UInput::create_emitter(int device_id, int type, int code)
{
  // search for an already existing emitter
  for(auto i = m_collectors.begin(); i != m_collectors.end(); ++i)
  {
    if (static_cast<int>((*i)->get_device_id()) == device_id &&
        (*i)->get_type() == type &&
        (*i)->get_code() == code)
    {
      return (*i)->create_emitter();
    }
  }

  // no emitter found, create a new one
  switch(type)
  {
    case EV_ABS:
      {
        m_collectors.push_back(std::make_unique<UIAbsEventCollector>(*this, device_id, type, code));
        return m_collectors.back()->create_emitter();
      }

    case EV_KEY:
      {
        m_collectors.push_back(std::make_unique<UIKeyEventCollector>(*this, device_id, type, code));
        return m_collectors.back()->create_emitter();
      }

    case EV_REL:
      {
        m_collectors.push_back(std::make_unique<UIRelEventCollector>(*this, device_id, type, code));
        return m_collectors.back()->create_emitter();
      }

    default:
      assert(false && "unknown type");
      return {};
  }
}

void
UInput::finish()
{
  for(auto i = m_uinput_devs.begin(); i != m_uinput_devs.end(); ++i)
  {
    i->second->finish();
  }
}

std::vector<LinuxUinput*>
UInput::get_linux_uinputs() const
{
  std::vector<LinuxUinput*> result;
  for (auto& it : m_uinput_devs) {
    result.emplace_back(it.second.get());
  }
  return result;
}

void
UInput::send(uint32_t device_id, int ev_type, int ev_code, int value)
{
  get_uinput(device_id)->send(static_cast<uint16_t>(ev_type), static_cast<uint16_t>(ev_code), value);
}

void
UInput::update(int msec_delta)
{
  for(auto i = m_rel_repeat_lst.begin(); i != m_rel_repeat_lst.end(); ++i)
  {
    i->second.time_count += msec_delta;

    // FIXME: shouldn't send out events multiple times, but accumulate
    // them instead and send out only once
    while (i->second.time_count >= i->second.repeat_interval)
    {
      // value can be float, but be can only send out int, so keep
      // track of the rest we don't send
      int i_value = static_cast<int>(i->second.value + truncf(i->second.rest));
      i->second.rest -= truncf(i->second.rest);
      i->second.rest += i->second.value - truncf(i->second.value);

      get_uinput(i->second.code.get_device_id())->send(EV_REL, static_cast<uint16_t>(i->second.code.code), i_value);
      i->second.time_count -= i->second.repeat_interval;
    }
  }

  for(auto i = m_uinput_devs.begin(); i != m_uinput_devs.end(); ++i)
  {
    i->second->update(msec_delta);
  }
}

void
UInput::sync()
{
  for(auto i = m_collectors.begin(); i != m_collectors.end(); ++i)
  {
    (*i)->sync();
  }

  for(auto i = m_uinput_devs.begin(); i != m_uinput_devs.end(); ++i)
  {
    i->second->sync();
  }
}

void
UInput::send_rel_repetitive(const UIEvent& code, float value, int repeat_interval)
{
  if (repeat_interval < 0)
  { // remove rel_repeats from list
    // FIXME: should send the last value still in the repeater
    m_rel_repeat_lst.erase(code);
    // no need to send a event for rel, as it defaults to 0 anyway
  }
  else
  { // add rel_repeats to list
    auto const it = m_rel_repeat_lst.find(code);

    if (it == m_rel_repeat_lst.end())
    {
      RelRepeat rel_rep;
      rel_rep.code  = code;
      rel_rep.value = value;
      rel_rep.rest  = 0.0f;
      rel_rep.time_count = 0;
      rel_rep.repeat_interval = repeat_interval;
      m_rel_repeat_lst.insert(std::pair<UIEvent, RelRepeat>(code, rel_rep));

      // Send the event once
      get_uinput(code.get_device_id())->send(EV_REL, static_cast<uint16_t>(code.code), static_cast<int32_t>(value));
    }
    else
    {
      // FIXME: send old value, store new value for rest

      it->second.code  = code;
      it->second.value = value;
      // it->second.time_count = do not touch this
      it->second.repeat_interval = repeat_interval;
    }
  }
}

LinuxUinput*
UInput::get_uinput(uint32_t device_id) const
{
  auto const it = m_uinput_devs.find(device_id);
  if (it != m_uinput_devs.end())
  {
    return it->second.get();
  }
  else
  {
    assert(0);
    std::ostringstream str;
    str << "Couldn't find uinput device: " << device_id;
    throw std::runtime_error(str.str());
  }
}

void
UInput::set_device_usbids(const std::map<uint32_t, struct input_id>& device_usbids)
{
  m_device_usbids = device_usbids;
}

void
UInput::set_device_names(const std::map<uint32_t, std::string>& device_names)
{
  m_device_names = device_names;
}

void
UInput::set_ff_callback(int device_id, const std::function<void (uint8_t, uint8_t)>& callback)
{
  get_uinput(device_id)->set_ff_callback(callback);
}

int
UInput::find_jsdev_number()
{
  for(int i = 0; ; ++i)
  {
    char filename1[32];
    char filename2[32];

    sprintf(filename1, "/dev/input/js%d", i);
    sprintf(filename2, "/dev/js%d", i);

    if (access(filename1, F_OK) != 0 && access(filename2, F_OK) != 0)
      return i;
  }
}

int
UInput::find_evdev_number()
{
  for(int i = 0; ; ++i)
  {
    char filename[32];

    sprintf(filename, "/dev/input/event%d", i);

    if (access(filename, F_OK) != 0)
      return i;
  }
}

} // namespace uinpp

/* EOF */
