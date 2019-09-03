#if ! defined SEQ66_MIDI_PROBE_HPP
#define SEQ66_MIDI_PROBE_HPP

/**
 * \file          midi_probe.hpp
 *
 *    Functions for testing and probing the MIDI support.
 *
 * \library       seq66 application
 * \author        Gary P. Scavone; refactoring by Chris Ahlstrom
 * \date          2016-11-19
 * \updates       2017-01-11
 * \license       See the rtexmidi.lic file.  Too big for a header file.
 *
 */

#include <string>

#include "rtmidi_info.hpp"

/*
 * Do not document the namespace; it breaks Doxygen.
 */

namespace seq66
{

extern std::string midi_api_name (int i);
extern int midi_probe ();
extern bool midi_input_test (rtmidi_info & info, int portindex);

}           // namespace seq66

#endif      // SEQ66_MIDI_PROBE_HPP

/*
 * midi_probe.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

