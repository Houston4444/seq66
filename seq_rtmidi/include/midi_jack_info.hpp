#if ! defined SEQ66_MIDI_JACK_INFO_HPP
#define SEQ66_MIDI_JACK_INFO_HPP

/**
 * \file          midi_jack_info.hpp
 *
 *    A class for holding the current status of the JACK system on the host.
 *
 * \library       seq66 application
 * \author        Chris Ahlstrom
 * \date          2017-01-01
 * \updates       2019-02-09
 * \license       See the rtexmidi.lic file.  Too big for a header file.
 *
 *    We need to have a way to get all of the JACK information of
 *    the midi_jack module.  This module provides that information.
 *
 *  GitHub issue #165: enabled a build and run with no JACK support.
 */

#include "seq66-config.h"

#if defined SEQ66_JACK_SUPPORT

#include "mastermidibus_rm.hpp"
#include "midi_info.hpp"                /* seq66::midi_port_info etc.       */
#include "midi/midibus.hpp"             /* seq66::midibus                   */

#include <jack/jack.h>
#include "midi_jack_data.hpp"           /* seq66::midi_jack_data            */

/*
 * Do not document the namespace; it breaks Doxygen.
 */

namespace seq66
{
    class mastermidibus;
    class midi_jack;

/**
 *  The class for handling JACK MIDI port enumeration.
 */

class midi_jack_info : public midi_info
{
    friend class midi_jack;
    friend int jack_process_io (jack_nframes_t nframes, void * arg);

private:

    /**
     *  Holds the port data.  Not for use with the multi-client option.
     *  This list is iterated in the input and output portions of the JACK
     *  process callback.
     */

    std::vector<midi_jack *> m_jack_ports;

    /**
     *  Holds the JACK sequencer client pointer so that it can be used
     *  by the midibus objects.  This is actually an opaque pointer; there is
     *  no way to get the actual fields in this structure; they can only be
     *  accessed through functions in the JACK API.  Note that it is also
     *  stored as a void pointer in midi_info::m_midi_handle.
     *
     *  In multi-client mode, this pointer is the output client pointer.
     */

    jack_client_t * m_jack_client;

    /**
     *  Holds the JACK input client pointer if multi-client mode is in force.
     *  Otherwise, it is an unused null pointer.
     */

    jack_client_t * m_jack_client_2;

public:

    midi_jack_info
    (
        const std::string & appname,
        int ppqn    = SEQ66_DEFAULT_PPQN,       /* 192    */
        midibpm bpm = SEQ66_DEFAULT_BPM         /* 120.0  */
    );
    virtual ~midi_jack_info ();

    /**
     * \getter m_jack_client
     *      This is the platform-specific version of midi_handle().
     */

    jack_client_t * client_handle ()
    {
        return m_jack_client;
    }

    virtual bool api_get_midi_event (event * inev);
    virtual bool api_connect ();
    virtual int api_poll_for_midi ();       /* disposable??? */
    virtual void api_set_ppqn (int p);
    virtual void api_set_beats_per_minute (midibpm b);
    virtual void api_port_start (mastermidibus & masterbus, int bus, int port);
    virtual void api_flush ();

private:

    virtual int get_all_port_info ();

    /**
     * \getter m_jack_client
     *      This is the platform-specific version of midi_handle().
     */

    void client_handle (jack_client_t * j)
    {
        m_jack_client = j;
    }

    jack_client_t * connect ();
    void disconnect ();
    void extract_names
    (
        const std::string & fullname,
        std::string & clientname,
        std::string & portname
    );

private:

    /**
     *  Adds a pointer to a JACK port.
     */

    bool add (midi_jack & mj)
    {
        m_jack_ports.push_back(&mj);
        return true;
    }

};          // midi_jack_info

/*
 * Free functions in the seq66 namespace
 */

void silence_jack_errors (bool silent = true);
void silence_jack_info (bool silent = true);

}           // namespace seq66

#endif      // SEQ66_JACK_SUPPORT

#endif      // SEQ66_MIDI_JACK_INFO_HPP

/*
 * midi_jack_info.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

