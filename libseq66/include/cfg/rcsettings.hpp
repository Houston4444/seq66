#if ! defined SEQ66_RCSETTINGS_HPP
#define SEQ66_RCSETTINGS_HPP

/*
 *  This file is part of seq66.
 *
 *  seq66 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  seq66 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with seq66; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * \file          rcsettings.hpp
 *
 *  This module declares/defines a settings class that is also exposed
 *  globally in this application.
 *
 * \library       seq66 application
 * \author        Chris Ahlstrom
 * \date          2015-09-22
 * \updates       2020-11-15
 * \license       GNU GPLv2 or above
 *
 *  This collection of variables describes the options of the application,
 *  accessible from the command-line or from the "rc" file.
 */

#include <string>

#include "app_limits.h"                 /* SEQ66_DEFAULT_SEQS_IN_SET        */
#include "cfg/basesettings.hpp"         /* seq66::basesettings class        */
#include "cfg/recent.hpp"               /* seq66::recent class              */
#include "ctrl/keycontainer.hpp"        /* seq66::keycontainer class        */
#include "ctrl/midicontrolin.hpp"       /* seq66::midicontrolin class       */
#include "ctrl/midicontrolout.hpp"      /* seq66::midicontrolout class      */
#include "play/mutegroups.hpp"          /* list of mute groups              */
#include "play/clockslist.hpp"          /* list of seq66::e_clock settings  */
#include "play/inputslist.hpp"          /* list of boolean input settings   */
#include "play/mutegroups.hpp"          /* map of seqq66::mutes stanzas     */

/*
 *  Do not document a namespace; it breaks Doxygen.
 */

namespace seq66
{

/**
 *  These control sizes.  We'll try changing them and see what happens.
 *  Increasing these value spreads out the pattern grids a little bit and
 *  makes the Patterns panel slightly bigger.  Seems like it would be
 *  useful to make these values user-configurable.
 *
 *  Copped from globals.h.
 */

/**
 *  Maximum number of groups that can be supported.  Basically, the number of
 *  groups set in the "rc" file.  32 groups can be filled.  This is a permanent
 *  maximum because we really can't support more than 32 keystrokes to support
 *  selecting a mute-group.
 */

const int c_max_groups = SEQ66_DEFAULT_GROUP_MAX;

/**
 *  Maximum number of screen sets that can be supported.  Basically, the number
 *  of times the Patterns Panel can be filled.  Up to 32 sets can be created.
 *  This is a permanent maximum because we really can't support more than 32
 *  keystrokes to support selecting a screenset.
 */

const int c_max_sets = SEQ66_DEFAULT_SET_MAX;

/**
 *  Maximum number of set keys that can be supported.  32 keys can be assigned
 *  in the Options / Keyboard tab and "rc" file.  This value applies to the
 *  "[keyboard-group]" and "[keyboard-control]" sections.
 */

const int c_max_set_keys = SEQ66_SET_KEYS_MAX;

/**
 *  This class contains "global" options that can be read from the "rc" file
 *  (class rcfile) and its related "mutes" and "ctrl" files.
 */

class rcsettings final : public basesettings
{
    /*
     * Too many friends need access to the protected setters.  And we should
     * make cmdlineopts a class, not just a module of standalone functions.
     */

    friend class cmdlineopts;
    friend class clinsmanager;
    friend class converter;
    friend class midicontrolfile;
    friend class mutegroupsfile;
    friend class optionsfile;
    friend class performer;
    friend class rcfile;
    friend class rtmidi_info;

public:

    /**
     *  Provides mutually-exclusive codes for the mouse-handling used by the
     *  application.  Moved here from the globals.h module.  The fruity mode
     *  will probably never be supported in seq66, though.
     */

    enum class interaction
    {
        seq24,            /**< Use the normal mouse interactions. */
        fruity,           /**< The "fruity" mouse interactions.   */
        maximum           /**< Keep this last... a size value.    */
    };

    /**
     *  Provides mutually-exclusive codes for handling the reading of
     *  mute-groups from the "rc" file versus the "MIDI" file.  There's no GUI
     *  way to set this item yet.
     *
     *  mute_group_handling::mutes: In this option, the mute groups are
     *  writtin only to the "mutes" (formerly "rc") file.
     *
     *  mute_group_handling::midi: In this option, the mute groups are
     *  only written to the "rc" file if the MIDI file did not contain
     *  non-zero mute groups.  This option prevents the contamination of the
     *  "rc" mute-groups by the MIDI file's mute-groups.  We're going to make
     *  this the default option.  NEEDS FIXING!
     *
     *  mute_group_handling::both: This is the legacy (seq66) option, which
     *  reads the mute-groups from the MIDI file, and saves them back to the
     *  "rc" file and to the MIDI file.  However, for Seq66 MIDI files such as
     *  b4uacuse-stress.midi, seq66 never reads the mute-groups in that MIDI
     *  file!  In any case, this can be considered a corruption of the "rc"
     *  file.
     */

    enum class mute_group_handling
    {
        mutes,          /**< Save mute groups to "mutes" files.     */
        midi,           /**< Write mute groups only to MIDI file.   */
        both,           /**< Write the mute groups to both files.   */
        maximum         /**< Keep this last... a size value.        */
    };

private:

    /**
     *  The list of output clocks.
     */

    clockslist m_clocks;

    /**
     *  The list of input bus statuses.
     */

    inputslist m_inputs;

    /**
     *  Holds a set of mute-groups ("mutes") read from a configuration file.
     */

    mutegroups m_mute_groups;

    /**
     *  Indicates if we want to load the key controls.  Generally, we do.
     *  This setting is made by the [midi-control-flags] load-key-control
     *  entry in the MIDI control section.
     */

    bool m_load_key_controls;

    /**
     *  Holds the key-container.
     */

    keycontainer m_keycontainer;

    /**
     *  Indicates if we want to load the MIDI controls.  Generally, we do.
     *  This setting is made by the [midi-control-flags] load-midi-control
     *  entry in the MIDI control section.
     */

    bool m_load_midi_controls;

    /**
     *  Provides a way to pick which input buss will be used as the MIDI
     *  controller device.
     */

    bussbyte m_midi_control_buss;

    /**
     *  Holds all of the MIDI controls stanzas, even one that are inactive.
     *  This is necessary for being able to write out a full section of
     *  keystroke controls and MIDI controls.
     */

    midicontrolin m_midi_control_in;

    /**
     *  Holds the MIDI control out stanzas.
     */

    midicontrolout m_midi_control_out;

    /*
     * Much more complete descriptions of these options can be found in the
     * seq66.rc file.
     */

    bool m_show_ui_sequence_key;    /**< Moved from key_performers.         */
    bool m_show_ui_sequence_number; /**< Moved from key_performers.         */

    /**
     *  Provides the Song Position, in 16th notes, at which MIDI clocking will
     *  begin if a MIDI buss is set the the MIDI Clock Mode setting.
     *
     *  Held for midibase::set_clock_mod().
     */

    bool m_clock_mod;

    bool m_verbose;                 /**< Message-showing setting.           */
    bool m_auto_option_save;        /**< [auto-option-save] setting.        */
    bool m_lash_support;            /**< Enable LASH, if compiled in.       */
    bool m_allow_mod4_mode;         /**< Allow Mod4 to hold drawing mode.   */
    bool m_allow_snap_split;        /**< Allow snap-split of a trigger.     */
    bool m_allow_click_edit;        /**< Allow double-click edit pattern.   */
    bool m_show_midi;               /**< Show MIDI events to console.       */
    bool m_priority;                /**< Run at high priority (Linux only). */
    bool m_pass_sysex;              /**< Pass SysEx to outputs, not ready.  */
    bool m_with_jack_transport;     /**< Enable synchrony with JACK.        */
    bool m_with_jack_master;        /**< Serve as a JACK transport Master.  */
    bool m_with_jack_master_cond;   /**< Serve as JACK Master if possible.  */
    bool m_with_jack_midi;          /**< Use JACK MIDI.                     */
    bool m_song_start_mode;         /**< Use song mode versus live mode.    */
    bool m_filter_by_channel;       /**< Record only sequence channel data. */
    bool m_manual_ports;            /**< [manual-ports] setting.            */
    int m_manual_port_count;        /**< [manual-ports] port count.         */
    bool m_reveal_ports;            /**< [reveal-ports] setting.            */
    bool m_print_keys;              /**< Show hot-key in main window slot.  */
    bool m_device_ignore;           /**< From seq66 module, unused!         */
    int m_device_ignore_num;        /**< From seq66 module, unused!         */
    interaction m_interaction_method; /**< Interaction method: no support.  */

    /**
     *  Indicates if empty mute-groups get saved to the MIDI file.
     */

    mute_group_handling m_mute_group_save;

    /**
     *  Provides the name of current MIDI file.  Under normal usage, it is the
     *  full file specification, including the path to the file.  Under session
     *  management, it is the base name (e.g. "song.midi") of the file with
     *  the new m_midi_filepath variable prepended to the base name.
     */

    std::string m_midi_filename;

    /**
     *  Provides the base name for MIDI files.  This value is meant to be used
     *  only under session management, where all files must be read and written
     *  from the same non-standard (get the pun?) directory.  It will be empty
     *  under normal operation.
     */

    std::string m_midi_filepath;

    /**
     *  Holds the JACK UUID value that makes this JACK connection unique.
     */

    std::string m_jack_session_uuid;

    /**
     *  Holds the directory from which the last MIDI file was opened (or
     *  saved).
     */

    std::string m_last_used_dir;

    /**
     *  Holds the current "rc" and "user" configuration base directory.  This
     *  value is ".config/sequencer66" by default.  For usage, it is normally
     *  expanded.
     */

    std::string m_config_directory;

    /**
     *  Holds the current "rc" configuration filename.  This value is
     *  "qseq66.rc" by default.
     */

    std::string m_config_filename;

    /**
     *  The full expanded path to the configuration directory.  This value is
     *  created, by default, by concatenating $HOME and ".config/seq66".
     *  However it can be reset completely by the full_config_directory()
     *  function to what ever the user needs (e.g. for usage with the Non
     *  Session Manager).
     */

    mutable std::string m_full_config_directory;

    /**
     *  Holds the current "user" configuration filename.  This value is
     *  "qseq66.usr" by default.
     */

    std::string m_user_filename;

    /**
     *  Indicates if the user wants to use a [midi-control] section from a
     *  separate file, for the convenience of changing the MIDI control setup
     *  without a lot of editing.  This value is set to true if a
     *  [midi-control-file] section is encountered.
     */

    bool m_use_midi_control_file;

    /**
     *  The base name of the MIDI control file, if applicable.  This file is
     *  located only in the specific "rc"/"usr" HOME directory,
     *  m_config_directory.
     */

    std::string m_midi_control_filename;

    /**
     *  Indicates if the user wants to use a [mute-group] section from a
     *  separate file, for the convenience of changing the setup
     *  without a lot of editing.  This value is set to true if a
     *  [mute-group] section is encountered.
     */

    bool m_use_mute_group_file;

    /**
     *  The base name of the mute-group file, if applicable.  This file is
     *  located only in the specific "rc"/"usr" HOME directory,
     *  m_config_directory.
     */

    std::string m_mute_group_filename;

    /**
     *  Indicates if the user wants to use the play-list stored in the "rc"
     *  file.  This value is stored as well.
     */

    bool m_playlist_active;

    /**
     *  Provides the full name of a play-list file, such as "tunes.playlist"
     *  or "/home/dude/.config/seq66/tunes.playlist".
     *
     *  It is used only if playlist mode is active.  This file is always
     *  located in the configuration directory (which can be modified from the
     *  command-line).
     */

    std::string m_playlist_filename;

    /**
     *  Holds the base directory of all the MIDI files in all the playlists.
     *  Normally useful when MIDI files are in an NSM session directory or a
     *  directory separate from where Seq66 is run.  Normally empty.
     */

    std::string m_playlist_midi_base;

    /**
     *  Indicates if the user wants to use the note-mapper stored in the "rc"
     *  file.  This value is stored as well.
     */

    bool m_notemap_active;

    /**
     *  Provides the name of a note-mapping file to use.  This is a feature
     *  adapted and modified from our "midicvt" project.
     */

    std::string m_notemap_filename;

    /**
     *  Holds the application name, e.g. "seq66", "qpseq66", "qrseq66",
     *  "seq66cli", or, most commonly, "qseq66".  This is a constant, set to
     *  SEQ66_APP_NAME, but obtained via the seq_app_name() function.  Do not
     *  confuse it with the client name, which defaults to "seq66" no matter
     *  what the application name.
     */

    const std::string m_application_name;

    /**
     *  New value to allow the user to violate the MIDI specification and use a
     *  track other than the first track (#0) as the MIDI tempo track.
     */

    int m_tempo_track_number;

    /**
     *  Holds a few MIDI file-names most recently used.  Although this is a
     *  vector, we do not let it grow past SEQ66_RECENT_FILES_MAX.
     *
     *  New feature from Oli Kester's kepler34 project.
     *
     * std::vector<std::string> m_recent_files;
     */

    recent m_recent_files;

    /**
     *  If true, this flag indicates to open the most recent MIDI file, which is
     *  the first in the list.  This flag is set and used only at startup time,
     *  after the "session" is created.
     */

    bool m_load_most_recent;

public:

    rcsettings ();
    rcsettings (const rcsettings & rhs) = default;
    rcsettings & operator = (const rcsettings & rhs) = default;
    virtual ~rcsettings () = default;

    std::string make_config_filespec
    (
        const std::string & base,
        const std::string & ext
    ) const;

    std::string config_filespec () const;
    std::string config_filespec (const std::string & altname) const;
    std::string user_filespec () const;
    std::string user_filespec (const std::string & altname) const;
    std::string midi_control_filespec () const;
    std::string mute_group_filespec () const;
    std::string playlist_filespec () const;
    void clear_playlist ();
    std::string notemap_filespec () const;
    virtual void set_defaults () override;

    const clockslist & clocks () const
    {
        return m_clocks;
    }

    clockslist & clocks ()
    {
        return m_clocks;
    }

    const inputslist & inputs () const
    {
        return m_inputs;
    }

    inputslist & inputs ()
    {
        return m_inputs;
    }

    const mutegroups & mute_groups () const
    {
        return m_mute_groups;
    }

    mutegroups & mute_groups ()
    {
        return m_mute_groups;
    }

    bool load_key_controls () const
    {
        return m_load_key_controls;
    }

    const keycontainer & key_controls () const
    {
        return m_keycontainer;
    }

    keycontainer & key_controls ()
    {
        return m_keycontainer;
    }

    bool load_midi_control_in () const
    {
        return m_load_midi_controls;
    }

    bussbyte midi_control_buss () const
    {
        return m_midi_control_buss;
    }

    const midicontrolin & midi_control_in () const
    {
        return m_midi_control_in;
    }

    midicontrolin & midi_control_in ()
    {
        return m_midi_control_in;
    }

    bool add_midicontrol_stanza
    (
        const std::string & kn,
        automation::category cat, int slotnumber,
        int a[6], int b[6], int c[6]
    );
    bool add_blank_stanza
    (
        const std::string & kn, automation::category cat, int slotnumber
    );

    const midicontrolout & midi_control_out () const
    {
        return m_midi_control_out;
    }

    midicontrolout & midi_control_out ()
    {
        return m_midi_control_out;
    }

    bool show_ui_sequence_key () const
    {
        return m_show_ui_sequence_key;
    }

    bool show_ui_sequence_number () const
    {
        return m_show_ui_sequence_number;
    }

    int get_clock_mod () const
    {
        return m_clock_mod;
    }

    bool verbose () const
    {
        return m_verbose;
    }

    bool auto_option_save () const
    {
        return m_auto_option_save;
    }

    bool lash_support () const
    {
        return m_lash_support;
    }

    bool allow_mod4_mode () const
    {
        return m_allow_mod4_mode;
    }

    bool allow_snap_split () const
    {
        return m_allow_snap_split;
    }

    bool allow_click_edit () const
    {
        return m_allow_click_edit;
    }

    bool show_midi () const
    {
        return m_show_midi;
    }

    bool priority () const
    {
        return m_priority;
    }

    bool pass_sysex () const
    {
        return m_pass_sysex;
    }

    bool with_jack_transport () const
    {
        return m_with_jack_transport;
    }

    bool with_jack_master () const
    {
        return m_with_jack_master;
    }

    bool with_jack_master_cond () const
    {
        return m_with_jack_master_cond;
    }

    bool with_jack_midi () const
    {
        return m_with_jack_midi;
    }

    bool song_start_mode () const
    {
        return m_song_start_mode;
    }

    void with_jack_transport (bool flag)
    {
        m_with_jack_transport = flag;
    }

    void with_jack_master (bool flag)
    {
        m_with_jack_master = flag;
    }

    void with_jack_master_cond (bool flag)
    {
        m_with_jack_master_cond = flag;
    }

    void with_jack_midi (bool flag)
    {
        m_with_jack_midi = flag;
    }

    /**
     * \getter m_with_jack_transport m_with_jack_master, and
     * m_with_jack_master_cond, to save client code some trouble.  Do not
     * confuse these original options with the new "no JACK MIDI" option.
     */

    bool with_jack () const
    {
        return
        (
            m_with_jack_transport || m_with_jack_master || m_with_jack_master_cond
        );
    }

    bool filter_by_channel () const
    {
        return m_filter_by_channel;
    }

    bool manual_ports () const
    {
        return m_manual_ports;
    }

    int manual_port_count () const
    {
        return m_manual_port_count;
    }

    bool reveal_ports () const
    {
        return m_reveal_ports;
    }

    bool print_keys () const
    {
        return m_print_keys;
    }

    bool device_ignore () const
    {
        return m_device_ignore;
    }

    int device_ignore_num () const
    {
        return m_device_ignore_num;
    }

    /*
     * Currently not supported in Seq66.
     */

    interaction interaction_method () const
    {
        return m_interaction_method;
    }

    mute_group_handling mute_group_save () const
    {
        return m_mute_group_save;
    }

    std::string mute_group_save_label () const;

    /**
     * \getter m_mute_group_save
     *
     * \return
     *      Returns true if mute-group-handling is set to mutes or both.
     */

    bool mute_group_save_to_rc () const
    {
        return
        (
            m_mute_group_save == mute_group_handling::mutes ||
            m_mute_group_save == mute_group_handling::both
        );
    }

    /**
     * \getter m_mute_group_save
     *
     * \return
     *      Returns true if mute-group-handling is set to midi or both.
     */

    bool mute_group_save_to_midi () const
    {
        return
        (
            m_mute_group_save == mute_group_handling::midi ||
            m_mute_group_save == mute_group_handling::both
        );
    }

    const std::string & midi_filename () const
    {
        return m_midi_filename;
    }

    void midi_filename (const std::string & value)
    {
        m_midi_filename = value;
    }

    void session_midi_filename (const std::string & value);

    const std::string & midi_filepath () const
    {
        return m_midi_filepath;
    }

    void midi_filepath (const std::string & value)
    {
        m_midi_filepath = value;
    }

    const std::string & jack_session_uuid () const
    {
        return m_jack_session_uuid;
    }

    const std::string & last_used_dir () const
    {
        return m_last_used_dir;
    }

    void last_used_dir (const std::string & value);

    /**
     * \setter m_recent_files
     *
     *  First makes sure the filename is not already present, and removes
     *  the back entry from the list, if it is full (SEQ66_RECENT_FILES_MAX)
     *  before adding it.  Now the full pathname is added.
     *
     * \param fname
     *      Provides the full path to the MIDI file that is to be added to
     *      the recent-files list.
     *
     * \return
     *      Returns true if the file-name was able to be added.
     */

    bool add_recent_file (const std::string & filename)
    {
        return m_recent_files.add(filename);
    }

    bool append_recent_file (const std::string & filename)
    {
        return m_recent_files.append(filename);
    }

    bool remove_recent_file (const std::string & filename)
    {
        return m_recent_files.remove(filename);
    }

    void clear_recent_files ()
    {
        m_recent_files.clear();
    }

    bool load_most_recent () const
    {
        return m_load_most_recent;
    }

    const std::string & config_directory () const
    {
        return m_config_directory;
    }

    void set_config_files (const std::string & value);
    bool has_home_config_path (const std::string & name);
    std::string home_config_directory () const;
    std::string trim_home_directory (const std::string & filepath);

    const std::string & config_filename () const
    {
        return m_config_filename;
    }

    bool playlist_active () const
    {
        return m_playlist_active;
    }

    bool notemap_active () const
    {
        return m_notemap_active;
    }

    const std::string & playlist_filename () const;

    const std::string & midi_base_directory () const
    {
        return m_playlist_midi_base;
    }

    const std::string & notemap_filename () const
    {
        return m_notemap_filename;
    }

    const std::string & user_filename () const
    {
        return m_user_filename;
    }

     bool use_midi_control_file () const
     {
        return m_use_midi_control_file;
     }

     const std::string & midi_control_filename () const
     {
        return m_midi_control_filename;
     }

     const std::string & mute_group_filename () const
     {
        return m_mute_group_filename;
     }

     bool use_mute_group_file () const
     {
        return m_use_mute_group_file;
     }

    const std::string application_name () const
    {
        return m_application_name;
    }

    std::string app_client_name () const;

    int tempo_track_number () const
    {
        return m_tempo_track_number;
    }

    std::string recent_file (int index, bool shorten = true) const;

    int recent_file_count () const
    {
        return m_recent_files.count();
    }

protected:

    void load_key_controls (bool flag)
    {
        m_load_key_controls = flag;
    }

    void load_midi_control_in (bool flag)
    {
        m_load_midi_controls = flag;
    }

    void midi_control_buss (bussbyte b)
    {
        m_midi_control_buss = b;
    }

    void show_ui_sequence_key (bool flag)
    {
        m_show_ui_sequence_key = flag;
    }

    void show_ui_sequence_number (bool flag)
    {
        m_show_ui_sequence_number = flag;
    }

    /**
     *  Set the clock mod to the given value, if legal.
     *
     * \param clockmod
     *      If this value is not equal to 0, it is used to set the static
     *      member m_clock_mod.
     */

    void set_clock_mod (int clockmod)
    {
        if (clockmod != 0)
            m_clock_mod = clockmod;
    }

    void verbose (bool flag)
    {
        m_verbose = flag;
    }

    void auto_option_save (bool flag)
    {
        m_auto_option_save = flag;
    }

    void lash_support (bool flag)
    {
        m_lash_support = flag;
    }

    void allow_mod4_mode (bool flag)
    {
        m_allow_mod4_mode = flag;
    }

    void allow_snap_split (bool flag)
    {
        m_allow_snap_split = flag;
    }

    void allow_click_edit (bool flag)
    {
        m_allow_click_edit = flag;
    }

    void show_midi (bool flag)
    {
        m_show_midi = flag;
    }

    void priority (bool flag)
    {
        m_priority = flag;
    }

    void pass_sysex (bool flag)
    {
        m_pass_sysex = flag;
    }

    void song_start_mode (bool flag)
    {
        m_song_start_mode = flag;
    }

    void filter_by_channel (bool flag)
    {
        m_filter_by_channel = flag;
    }

    void manual_ports (bool flag)
    {
        m_manual_ports = flag;
    }

    void manual_port_count (int count)
    {
        if (count <= 0)
            count = SEQ66_OUTPUT_BUSS_MAX;

        m_manual_port_count = count;
    }

    void reveal_ports (bool flag)
    {
        m_reveal_ports = flag;
    }

    void print_keys (bool flag)
    {
        m_print_keys = flag;
    }

    void device_ignore (bool flag)
    {
        m_device_ignore = flag;
    }

     void use_midi_control_file (bool flag)
     {
        m_use_midi_control_file = flag;
     }

     void use_mute_group_file (bool flag)
     {
        m_use_mute_group_file = flag;
     }

     void midi_control_filename (const std::string & name)
     {
        m_midi_control_filename = name;
     }

     void mute_group_filename (const std::string & name)
     {
        m_mute_group_filename = name;
     }

    void playlist_active (bool flag)
    {
        m_playlist_active = flag;
    }

    void notemap_active (bool flag)
    {
        m_notemap_active = flag;
    }

    void  midi_base_directory (const std::string & mbd)
    {
        m_playlist_midi_base = mbd;
    }

    void load_most_recent (bool f)
    {
        m_load_most_recent = f;
    }

    bool mute_group_save (const std::string & v);

    bool interaction_method (int v)
    {
        return interaction_method(static_cast<interaction>(v));
    }

    /*
     * The setters for non-bool values, defined in the cpp file because
     * they do some heavier validation.
     */

    void tempo_track_number (int track);
    void device_ignore_num (int value);
    bool interaction_method (interaction value);
    bool mute_group_save (mute_group_handling mgh);
    void jack_session_uuid (const std::string & value);
    void full_config_directory (const std::string & value, bool addhome = false);
    void config_directory (const std::string & value);
    void config_filename (const std::string & value);
    void playlist_filename (const std::string & value);
    bool playlist_filename_checked (const std::string & value);
    void user_filename (const std::string & value);

    void notemap_filename (const std::string & fn)
    {
        m_notemap_filename = fn;
    }

};          // class rcsettings

}           // namespace seq66

#endif      // SEQ66_RCSETTINGS_HPP

/*
 * rcsettings.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

