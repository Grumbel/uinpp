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

#ifndef HEADER_UINPP_PARSE_HPP
#define HEADER_UINPP_PARSE_HPP

#include <linux/input.h>
#include <stdint.h>
#include <string>

namespace uinpp {

input_id parse_input_id(const std::string& str);
uint32_t parse_device_id(const std::string& str);

inline uint32_t create_device_id(uint16_t slot_id, uint16_t type_id)
{
  return (slot_id << 16) | type_id;
}

inline uint16_t get_type_id(uint32_t device_id)
{
  return static_cast<uint16_t>(device_id & 0xffff);
}

inline uint16_t get_slot_id(uint32_t device_id)
{
  return static_cast<uint16_t>(((device_id) >> 16) & 0xffff);
}

/** in: "BTN_A@2"
    out: "BTN_A", SLOTID_AUTO, 2

    in: "BTN_A@mouse.2"
    out: "BTN_A", 3, DEVICEID_MOUSE
 */
void split_event_name(const std::string& str, std::string* event_str, int* slot_id, int* device_id);

uint16_t str2deviceid(const std::string& device);
uint16_t str2slotid(const std::string& slot);

} // namespace uinpp

#endif

/* EOF */
