#ifndef HEADER_UINPP_FROM_HPP
#define HEADER_UINPP_FROM_HPP

#include <vector>
#include <linux/input.h>

#include <strut/split.hpp>
#include <uinpp/ui_event.hpp>
#include <uinpp/ui_event_sequence.hpp>

#include "evdev_helper.hpp"

inline
UIEvent
UIEvent_from_char(char c)
{
  UIEvent ev;
  ev.m_slot_id = SLOTID_AUTO;
  ev.m_device_id = DEVICEID_AUTO;
  ev.m_device_id_resolved = false;
  ev.type = EV_KEY;

  // FIXME: not very complete, should use UIEventSequence and do a bit more magic
  switch(c)
  {
    case 'a': ev.code = str2key("XK_a"); break;
    case 'b': ev.code = str2key("XK_b"); break;
    case 'c': ev.code = str2key("XK_c"); break;
    case 'd': ev.code = str2key("XK_d"); break;
    case 'e': ev.code = str2key("XK_e"); break;
    case 'f': ev.code = str2key("XK_f"); break;
    case 'g': ev.code = str2key("XK_g"); break;
    case 'h': ev.code = str2key("XK_h"); break;
    case 'i': ev.code = str2key("XK_i"); break;
    case 'j': ev.code = str2key("XK_j"); break;
    case 'k': ev.code = str2key("XK_k"); break;
    case 'l': ev.code = str2key("XK_l"); break;
    case 'm': ev.code = str2key("XK_m"); break;
    case 'n': ev.code = str2key("XK_n"); break;
    case 'o': ev.code = str2key("XK_o"); break;
    case 'p': ev.code = str2key("XK_p"); break;
    case 'q': ev.code = str2key("XK_q"); break;
    case 'r': ev.code = str2key("XK_r"); break;
    case 's': ev.code = str2key("XK_s"); break;
    case 't': ev.code = str2key("XK_t"); break;
    case 'u': ev.code = str2key("XK_u"); break;
    case 'v': ev.code = str2key("XK_v"); break;
    case 'w': ev.code = str2key("XK_w"); break;
    case 'x': ev.code = str2key("XK_x"); break;
    case 'y': ev.code = str2key("XK_y"); break;
    case 'z': ev.code = str2key("XK_z"); break;
    case '0': ev.code = str2key("XK_0"); break;
    case '1': ev.code = str2key("XK_1"); break;
    case '2': ev.code = str2key("XK_2"); break;
    case '3': ev.code = str2key("XK_3"); break;
    case '4': ev.code = str2key("XK_4"); break;
    case '5': ev.code = str2key("XK_5"); break;
    case '6': ev.code = str2key("XK_6"); break;
    case '7': ev.code = str2key("XK_7"); break;
    case '8': ev.code = str2key("XK_8"); break;
    case '9': ev.code = str2key("XK_9"); break;

    case '.': ev.code = str2key("XK_period"); break;

    case '\n': ev.code = str2key("XK_enter"); break;

    default:  ev.code = str2key("XK_space"); break;
  }
  return ev;
}

inline
UIEvent
UIEvent_from_string(const std::string& str)
{
  switch(get_event_type(str))
  {
    case EV_REL: return str2rel_event(str); break;
    case EV_ABS: return str2abs_event(str); break;
    case EV_KEY: return str2key_event(str); break;
    default: throw std::runtime_error("unknown event type");
  }
}

inline
UIEventSequence
UIEventSequence_from_string(const std::string& value)
{
  std::vector<UIEvent> sequence;

  auto ev_tokens = strut::split(value, '+');
  int k = 0;
  for(auto m = ev_tokens.begin(); m != ev_tokens.end(); ++m, ++k)
  {
    sequence.push_back(str2key_event(*m));
  }

  return sequence;
}

inline
UIEventSequence
UIEventSequence_from_char(char c)
{
  switch(c)
  {
    case 'A': return UIEventSequence_from_string("XK_Shift_L+XK_a");
    case 'B': return UIEventSequence_from_string("XK_Shift_L+XK_b");
    case 'C': return UIEventSequence_from_string("XK_Shift_L+XK_c");
    case 'D': return UIEventSequence_from_string("XK_Shift_L+XK_d");
    case 'E': return UIEventSequence_from_string("XK_Shift_L+XK_e");
    case 'F': return UIEventSequence_from_string("XK_Shift_L+XK_f");
    case 'G': return UIEventSequence_from_string("XK_Shift_L+XK_g");
    case 'H': return UIEventSequence_from_string("XK_Shift_L+XK_h");
    case 'I': return UIEventSequence_from_string("XK_Shift_L+XK_i");
    case 'J': return UIEventSequence_from_string("XK_Shift_L+XK_j");
    case 'K': return UIEventSequence_from_string("XK_Shift_L+XK_k");
    case 'L': return UIEventSequence_from_string("XK_Shift_L+XK_l");
    case 'M': return UIEventSequence_from_string("XK_Shift_L+XK_m");
    case 'N': return UIEventSequence_from_string("XK_Shift_L+XK_n");
    case 'O': return UIEventSequence_from_string("XK_Shift_L+XK_o");
    case 'P': return UIEventSequence_from_string("XK_Shift_L+XK_p");
    case 'Q': return UIEventSequence_from_string("XK_Shift_L+XK_q");
    case 'R': return UIEventSequence_from_string("XK_Shift_L+XK_r");
    case 'S': return UIEventSequence_from_string("XK_Shift_L+XK_s");
    case 'T': return UIEventSequence_from_string("XK_Shift_L+XK_t");
    case 'U': return UIEventSequence_from_string("XK_Shift_L+XK_u");
    case 'V': return UIEventSequence_from_string("XK_Shift_L+XK_v");
    case 'W': return UIEventSequence_from_string("XK_Shift_L+XK_w");
    case 'X': return UIEventSequence_from_string("XK_Shift_L+XK_x");
    case 'Y': return UIEventSequence_from_string("XK_Shift_L+XK_y");
    case 'Z': return UIEventSequence_from_string("XK_Shift_L+XK_z");

    case 'a': return UIEventSequence_from_string("XK_a");
    case 'b': return UIEventSequence_from_string("XK_b");
    case 'c': return UIEventSequence_from_string("XK_c");
    case 'd': return UIEventSequence_from_string("XK_d");
    case 'e': return UIEventSequence_from_string("XK_e");
    case 'f': return UIEventSequence_from_string("XK_f");
    case 'g': return UIEventSequence_from_string("XK_g");
    case 'h': return UIEventSequence_from_string("XK_h");
    case 'i': return UIEventSequence_from_string("XK_i");
    case 'j': return UIEventSequence_from_string("XK_j");
    case 'k': return UIEventSequence_from_string("XK_k");
    case 'l': return UIEventSequence_from_string("XK_l");
    case 'm': return UIEventSequence_from_string("XK_m");
    case 'n': return UIEventSequence_from_string("XK_n");
    case 'o': return UIEventSequence_from_string("XK_o");
    case 'p': return UIEventSequence_from_string("XK_p");
    case 'q': return UIEventSequence_from_string("XK_q");
    case 'r': return UIEventSequence_from_string("XK_r");
    case 's': return UIEventSequence_from_string("XK_s");
    case 't': return UIEventSequence_from_string("XK_t");
    case 'u': return UIEventSequence_from_string("XK_u");
    case 'v': return UIEventSequence_from_string("XK_v");
    case 'w': return UIEventSequence_from_string("XK_w");
    case 'x': return UIEventSequence_from_string("XK_x");
    case 'y': return UIEventSequence_from_string("XK_y");
    case 'z': return UIEventSequence_from_string("XK_z");

    case '0': return UIEventSequence_from_string("XK_0");
    case '1': return UIEventSequence_from_string("XK_1");
    case '2': return UIEventSequence_from_string("XK_2");
    case '3': return UIEventSequence_from_string("XK_3");
    case '4': return UIEventSequence_from_string("XK_4");
    case '5': return UIEventSequence_from_string("XK_5");
    case '6': return UIEventSequence_from_string("XK_6");
    case '7': return UIEventSequence_from_string("XK_7");
    case '8': return UIEventSequence_from_string("XK_8");
    case '9': return UIEventSequence_from_string("XK_9");
    case '.': return UIEventSequence_from_string("XK_period");
    case '\n': return UIEventSequence_from_string("XK_enter");

    default:  return UIEventSequence_from_string("XK_space"); break;
  }
}

#endif

/* EOF */
