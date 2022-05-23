// uinpp - Linux uinput library for C++
// Copyright (C) 2008-2022 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_NPP_MULTI_DEVICE_HPP
#define HEADER_NPP_MULTI_DEVICE_HPP

#include <map>

#include "fwd.hpp"
#include "device.hpp"
#include "event.hpp"

namespace uinpp {

/** MultiDevice bundle multiple devices to make it easier to create
    virtual devices that spread across different categories of input, e.g. a
    keyboard with a trackball would both need a mouse device as well as
    a keyboard one */
class MultiDevice
{
public:
  MultiDevice();
  ~MultiDevice();

  /** Device construction functions
      @{*/
  /** Create the usual events that would be present for a device of the given type automatically */
  void set_extra_events(bool extra_events);

  void set_device_names(const std::map<uint32_t, std::string>& device_names);
  void set_device_usbids(const std::map<uint32_t, struct input_id>& device_usbids);
  void set_ff_callback(int device_id, const std::function<void (uint8_t, uint8_t)>& callback);

  EventEmitter* add(const Event& ev);
  EventEmitter* add_rel(uint32_t device_id, int ev_code);
  EventEmitter* add_abs(uint32_t device_id, int ev_code, int min, int max, int fuzz, int flat);
  EventEmitter* add_key(uint32_t device_id, int ev_code);
  void add_ff(uint32_t device_id, uint16_t code);

  /** needs to be called to finish device creation and create the
      device in the kernel */
  void finish();
  /** @} */

  /** Send events to the kernel
      @{*/
  void send(uint32_t device_id, int ev_type, int ev_code, int value);
  void send_rel_repetitive(const Event& code, float value, int repeat_interval);

  /** should be called to signal that all events of the current frame
      have been send */
  void sync();
  /** @} */

  std::vector<Device*> get_devices() const;

  void update(int msec_delta);

private:
  /** create a Device with the given device_id, if some already
      exist return a pointer to it */
  Device* create_uinput_device(uint32_t device_id);

  /** must only be called with a valid device_id */
  Device* get_uinput(uint32_t device_id) const;

  std::string get_device_name(uint32_t device_id) const;
  struct input_id get_device_usbid(uint32_t device_id) const;

  EventEmitter* create_emitter(int device_id, int type, int code);

private:
  struct RelRepeat
  {
    Event code;
    float value;
    float rest;
    int time_count;
    int repeat_interval;
  };

private:
  std::map<uint32_t, std::unique_ptr<Device> > m_devices;
  std::map<uint32_t, std::string> m_device_names;
  std::map<uint32_t, struct input_id> m_device_usbids;
  std::vector<std::unique_ptr<EventCollector>> m_collectors;

  std::map<Event, RelRepeat> m_rel_repeat_lst;

  bool m_extra_events;

private:
  MultiDevice(const MultiDevice&);
  MultiDevice& operator=(const MultiDevice&);
};

} // namespace uinpp

#endif

/* EOF */
