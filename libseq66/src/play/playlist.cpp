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
 * \file          playlist.cpp
 *
 *  This module declares/defines the base class for managing the <code>
 *  ~/.seq66rc </code> legacy configuration file or the new <code>
 *  ~/.config/seq66/seq66.rc </code> ("rc") configuration file.
 *
 * \library       seq66 application
 * \author        Chris Ahlstrom
 * \date          2018-08-26
 * \updates       2020-08-14
 * \license       GNU GPLv2 or above
 *
 *  Here is a skeletal representation of a Seq66 playlist:
 *
 \verbatim
        [playlist]
        0                       # playlist number, a MIDI value (0 to 127)
        "Downtempo"             # playlist name, for display/selection
        /home/user/midifiles/   # directory where the songs are stored
        10 file1.mid            # MIDI value and file's base-name
        11 file2.midi
        12 file3.midi           # . . .
\endverbatim
 *
 *  See the file data/sample.playlist for a more up-to-date example and
 *  explanation.
 */

#include <cctype>                       /* std::toupper() function          */
#include <iostream>                     /* std::cout                        */
#include <utility>                      /* std::make_pair()                 */
#include <string.h>                     /* memset()                         */

#include "cfg/settings.hpp"             /* seq66::rc()                      */
#include "midi/wrkfile.hpp"             /* seq66::midifile & seq66::wrkfile */
#include "play/playlist.hpp"            /* seq66::playlist support class    */
#include "play/performer.hpp"           /* seq66::performer anchor class    */
#include "util/filefunctions.hpp"       /* functions for file-names         */
#include "util/strfunctions.hpp"        /* strip_quotes()                   */

/*
 *  Do not document a namespace; it breaks Doxygen.
 */

namespace seq66
{

/**
 *  This object is used in returning a reference to a bogus object.
 */

playlist::song_list playlist::sm_dummy;

/**
 *  Principal constructor.
 *
 * \param p
 *      Provides the performer object that will interface between this module
 *      and the rest of the application.
 *
 * \param name
 *      Provides the name of the options file; this is usually a full path
 *      file-specification.
 *
 * \param rcs
 *      Provides a reference to an "rc" settings object to hold current options
 *      and modified options.
 *
 * \param show_on_stdout
 *      If true (the default is false), then the list/song information is
 *      written to stdout, to help with debugging.
 */

playlist::playlist
(
    performer & p,
    const std::string & name,
    rcsettings & rcs,
    bool show_on_stdout
) :
    configfile                  (name, rcs),        // base class constructor
    m_performer                 (p),                // the owner of this object
    m_comments                  (),
    m_play_lists                (),
    m_mode                      (false),
    m_current_list              (m_play_lists.end()),
    m_current_song              (sm_dummy.end()),   // song-list iterator
    m_unmute_set_now            (false),
    m_show_on_stdout            (show_on_stdout)
{
    // No code needed
}

/**
 *  This destructor unregisters this playlist from the performer object.
 */

playlist::~playlist ()
{
    // No code needed
}

/**
 *  Helper function for error-handling.  It assembles a message and then
 *  passes it to append_error_message().
 *
 * \param additional
 *      Additional context information to help in finding the error.
 *
 * \return
 *      Always returns false.
 */

bool
playlist::make_error_message (const std::string & additional)
{
    std::string msg = "Bad [playlist]";
    if (! additional.empty())
    {
        msg += ": ";
        msg += additional;
    }
    errprint(msg.c_str());
    append_error_message(msg);
    return false;
}

/**
 *  Opens the current play-list file and optionally verifies it.
 *
 * \param verify_it
 *      If true (the default), call verify() to make sure the playlist is
 *      sane.
 *
 * \return
 *      Returns true if the file was parseable and verifiable.
 */

bool
playlist::open (bool verify_it)
{
    bool result = parse();
    if (result)
    {
        if (verify_it)
        {
            if (m_show_on_stdout)
                printf("Verifying playlist %s\n", name().c_str());

            result = verify();
            if (result)
                (void) m_performer.clear_all(false);    // reset playlist
        }
    }
    mode(result);
    return result;
}

/**
 *  Parses the ~/.config/seq66/file.playlist file.
 *
 * The next_section() function is like line-after, but scans from the
 * current line in the file.  Necessary here because all the sections
 * have the same name.  After detecting the "[playlist]" section, the
 * following items need to be obtained:
 *
 *      -   Playlist number.  This number is used as the key value for
 *          the playlist. It can be any MIDI value (0 to 127), and the order
 *          of the playlists is based on this number, and selectable via MIDI
 *          control with this number.
 *      -   Playlist name.  A human-readable string describing the
 *          nick-name for the playlist.  This is an alternate way to
 *          look up the playlist.
 *      -   Song directory name.  The directory where the songs are
 *          stored.  If this name is empty, then the song file-names
 *          need to include the individual directories for each file.
 *          But even if not empty, the play-list directory is not used if
 *          the song file-name includes a path, as indicated by "/" or "\".
 *      -   Song file-name, or path to the song file-name.
 *
 * Note that the call to next_section() already gets to the next line
 * of data, which should be the index number of the playlist.
 *
 * \return
 *      Returns true if the file was able to be opened for reading.
 *      Currently, there is no indication if the parsing actually succeeded.
 */

bool
playlist::parse ()
{
    bool result = false;
    std::ifstream file(name(), std::ios::in | std::ios::ate);
    if (file.is_open())
    {
        file.seekg(0, std::ios::beg);                   /* seek to start    */

        /*
         * Not enough.  Call the member function instead.
         *
         * m_play_lists.clear();
         * m_comments.clear();
         */

        clear();

        /*
         * [comments]
         *
         * Header commentary is skipped during parsing.  However, we now try
         * to read an optional comment block, for restoration when rewriting
         * the file.
         */

        if (line_after(file, "[comments]"))             /* gets first line  */
        {
            do
            {
                m_comments += line();
                m_comments += std::string("\n");

            } while (next_data_line(file));
        }

        /*
         * [playlist-options]
         */

        if (line_after(file, "[playlist-options]"))
        {
            int unmute = 0;
            sscanf(scanline(), "%d", &unmute);
            unmute_set_now(unmute != 0);
        }

        /*
         * See banner notes.
         */

        int listcount = 0;
        bool have_section = line_after(file, "[playlist]");
        if (! have_section)
        {
            result = make_error_message("empty or missing section");
        }
        while (have_section)
        {
            int listnumber = -1;
            int songcount = 0;
            play_list_t plist;                          /* current playlist */
            sscanf(scanline(), "%d", &listnumber);          /* playlist number  */
            if (m_show_on_stdout)
                printf("Processing playlist %d\n", listnumber);

            if (next_data_line(file))
            {
                std::string listline = line();
                song_list slist;
                plist.ls_list_name = strip_quotes(listline);
                if (m_show_on_stdout)
                    printf("Playlist name %s\n", listline.c_str());

                if (next_data_line(file))
                {
                    /*
                     * We need to get the song's MIDI control number and it's
                     * directory name.  Make sure the directory name is
                     * canonical and clean.  The existence of the file should
                     * be validated later.  Also determine if the song
                     * file-name already has a directory before using the
                     * play-list's directory.
                     */

                    listline = line();
                    plist.ls_file_directory = clean_path(listline);
                    slist.clear();
                    if (m_show_on_stdout)
                        printf("Playlist directory %s\n", listline.c_str());

                    while (next_data_line(file))
                    {
                        int songnumber = -1;
                        std::string fname;
                        result = scan_song_file(songnumber, fname);
                        if (result)
                        {
                            song_spec_t sinfo;
                            sinfo.ss_index = songcount;
                            sinfo.ss_midi_number = songnumber;
                            if (name_has_directory(fname))
                            {
                                std::string path;
                                std::string filebase;
                                filename_split(fname, path, filebase);
                                sinfo.ss_song_directory = path;
                                sinfo.ss_embedded_song_directory = true;
                                sinfo.ss_filename = filebase;
                            }
                            else
                            {
                                sinfo.ss_song_directory = plist.ls_file_directory;
                                sinfo.ss_embedded_song_directory = false;
                                sinfo.ss_filename = fname;
                            }
                            (void) add_song(slist, sinfo);
                            ++songcount;
                        }
                        else
                        {
                            std::string msg = "scanning song file '";
                            msg += fname;
                            msg += "' failed";
                            result = make_error_message(msg);
                            break;
                        }
                    }

                    /*
                     * Need to deal with a false result still....
                     */

                    if (songcount > 0)
                    {
                        plist.ls_index = listcount;         /* ordinal      */
                        plist.ls_midi_number = listnumber;  /* MIDI mapping */
                        plist.ls_song_count = songcount;
                        plist.ls_song_list = slist;         /* copy temp    */
                        result = add_list(plist);
                    }
                    else
                    {
                        result = make_error_message("no songs");
                        break;
                    }
                }
                else
                {
                    std::string msg = "no list directory in playlist #" +
                        std::to_string(listnumber);

                    result = make_error_message(msg);
                    break;
                }
            }
            else
            {
                std::string msg = "no data in playlist #" +
                    std::to_string(listnumber);

                result = make_error_message(msg);
                break;
            }
            ++listcount;
            have_section = next_section(file, "[playlist]");
        }
        file.close();           /* done parsing the "playlist" file */
    }
    else
    {
        std::string msg = "error opening file [" + name() + "]";
        result = make_error_message(msg);
    }
    if (result)
        (void) reset_list(false);       /* don't clear, just reset values   */
    else
        (void) reset_list(true);        /* clear and just reset values      */

    mode(result);
    return result;
}

/**
 *  Encapsulates some groty code for the parse() function.  It assumes that
 *  next_data_line() has retrieved a file-name line for a song.
 *
 * \param [out] song_number
 *      Holds the song number that was retrieved.  Use it only if not equal
 *      to -1 and if this function returns true.
 *
 * \param [out] song_file
 *      Holds the song file-name that was retrieved.  Use it only if not
 *      empty and if this function returns true.
 *
 * \return
 *      Returns true if this function succeeded.  If false, an error message is
 *      set up.
 */

bool
playlist::scan_song_file (int & song_number, std::string & song_file)
{
    bool result = false;
    int songnumber = -1;
    const char * dirname = &m_line[0];
    int sscount = sscanf(scanline(), "%d", &songnumber);
    if (sscount == EOF || sscount == 0)
    {
        song_number = -1;                                   /* side-effect  */
        song_file.clear();                                  /* side-effect  */
        result = make_error_message("song number missing");
    }
    else
    {
        while (! std::isspace(*dirname))
        {
            if (*dirname == 0)
                break;

            ++dirname;
        }
        while (std::isspace(*dirname))
        {
            if (*dirname == 0)
                break;

            ++dirname;
        }
        bool gotit = std::isalnum(*dirname) || std::ispunct(*dirname);
        if (gotit)
        {
            song_number = songnumber;                       /* side-effect  */
            song_file = dirname;                            /* side-effect  */
            result = true;
        }
        else
        {
            song_number = -1;                               /* side-effect  */
            song_file.clear();                              /* side-effect  */
            result = make_error_message("song file-path missing");
        }
    }
    return result;
}

/**
 *  This options-writing function is just about as complex as the
 *  options-reading function.
 *
 * \return
 *      Returns true if the write operations all succeeded.
 */

bool
playlist::write ()
{
    std::ofstream file(name(), std::ios::out | std::ios::trunc);
    if (! file.is_open())
    {
        errprintf("error opening [%s] for writing\n", name().c_str());
        return false;
    }

    /*
     * Initial comments and MIDI control section.
     */

    file
        << "# Seq66 0.96.0 (and above) playlist file\n"
        << "#\n"
        << "# " << name() << "\n"
        << "# Written on " << current_date_time() << "\n"
        << "#\n"
        << "# This file holds a playlist for Seq66. It consists of one\n"
        << "# or more '[playlist]' sections.  Each section has a user-specified\n"
        << "# number.  This number should range from 0 to 127, but it can go\n"
        << "# higher if the user doesn't need to use MIDI control to select\n"
        << "# a playlist. Ultimately, the playlists are sorted by this number.\n"
        << "#\n"
        << "# Next comes a display name for this list, with or without quotes.\n"
        << "#\n"
        << "# Next comes the name of the directory, always using the UNIX-style\n"
        << "# separator, a forward slash (solidus).  It can optionally be\n"
        << "# terminated with a slash.\n"
        << "#\n"
        << "# The last item is a line containing the MIDI song-control number,\n"
        << "# followed by the name of the MIDI files.  They are sorted by the\n"
        << "# control number, starting from 0.  They can be simple 'base.ext'\n"
        << "# file-names; the playlist directory will be prepended before the\n"
        << "# song is accessed.\n"
        << "#\n"
        << "# If the MIDI file-name already has a directory name, that will be\n"
        << "# used instead.\n"
        ;

    file << "#\n"
        "# The [comments] section can document this file.  Lines starting\n"
        "# with '#' are ignored.  Blank lines are ignored.  Show a\n"
        "# blank line by adding a space character to the line.\n"
        ;

    /*
     * [comments]
     */

    file << "\n" << "[comments]\n" << "\n" << m_comments << "\n";

    /*
     * [playlist-options]
     */

    file
        << "\n" << "[playlist-options]\n" << "\n"
        << (unmute_set_now() ? "1" : "0")
        << "     # If set to 1, when a new song is selected, "
        "immediately unmute it.\n"
        ;

    /*
     * [playlist] sections
     */

    for (const auto & plpair : m_play_lists)
    {
        const play_list_t & pl = plpair.second;
        file
        << "\n"
        << "[playlist]\n"
        << "\n"
        << "# Playlist number, arbitrary but unique. 0 to 127 recommended\n"
        << "# for use with the MIDI playlist control.\n"
        << pl.ls_midi_number << "\n\n"
        << "# Display name of this play list.\n\n"
        << "\"" << pl.ls_list_name << "\"\n\n"
        << "# Default storage directory for the song-files in this playlist.\n\n"
        << pl.ls_file_directory << "\n"
        << "\n"
        << "# Provides the MIDI song-control number (0 to 127), and also the\n"
        << "# base file-name (tune.midi) of each song in this playlist.\n"
        << "# The playlist directory is used, unless the file-name contains its\n"
        << "# own path.\n\n"
        ;

        /*
         * For each song, write the MIDI control number, followed only by
         * the song's file-name, which could include the path-name.
         */

        const song_list & sl = pl.ls_song_list;
        for (const auto & sc : sl)
        {
            const song_spec_t & s = sc.second;
            file << s.ss_midi_number << " " << s.ss_filename << "\n";
        }
    }

    file
        << "\n"
        << "# End of " << name() << "\n#\n"
        << "# vim: sw=4 ts=4 wm=4 et ft=dosini\n"   /* ft for nice colors */
        ;

    file.close();
    return true;
}

/**
 *  Given a file-name, opens that file as a song.  This function holds common
 *  code.  It is similar to read_midi_file() in the midifile module, but
 *  does not affect the recent-files list.
 *
 *  Before the song is loaded, the current song is cleared from memory.
 *  Remember that clear_all() will fail if it detects a sequence being edited.
 *  In that case, this function will fail as well.
 *
 * \param fname
 *      The full path to the file to be opened.
 *
 * \param verifymode
 *      If true, open the file in play-list mode.  Currently, this means
 *      is that some output from the file-opening process is suppressed, and
 *      the performer::clear_all() function is called right after parsing the
 *      song file to verify it.
 */

bool
playlist::open_song (const std::string & fname, bool verifymode)
{
    if (m_performer.is_running())
        m_performer.stop_playing();

    bool result = m_performer.clear_song();
    if (result)
    {
        bool is_wrk = file_extension_match(fname, "wrk");
        int ppqn = 0;
        if (is_wrk)
        {
            wrkfile m(fname, SEQ66_USE_DEFAULT_PPQN, verifymode);
            result = m.parse(m_performer);
            ppqn = m.ppqn();
        }
        else
        {
            midifile m(fname, SEQ66_USE_DEFAULT_PPQN, true, verifymode);
            result = m.parse(m_performer);
            ppqn = m.ppqn();
        }
        if (result)
        {
            if (verifymode)
            {
                /*
                 * This needs to be done only after all songs have been
                 * verified.
                 *
                 * (void) m_performer.clear_all(false);    // reset playlist
                 */
            }
            else
            {
                /*
                 * We save the PPQN value from the file, set this chosen PPQN,
                 * save the current MIDI filename.
                 */

                usr().file_ppqn(ppqn);
                m_performer.set_ppqn(choose_ppqn());
                rc().midi_filename(fname);
                if (unmute_set_now())
                    m_performer.toggle_playing_tracks();
            }
            m_performer.announce_playscreen();
        }
    }
    return result;
}

/**
 *  Selects the song based on the index (row) value, and optionally opens it.
 *
 * \param index
 *      The ordinal location of the song within the current playlist.
 *
 * \param opensong
 *      If true (the default), the song is opened.
 *
 * \return
 *      Returns true if the song is selected, and if specified, is opened
 *      succesfully as well.
 */

bool
playlist::open_select_song_by_index (int index, bool opensong)
{
    bool result = select_song_by_index(index);
    if (result && opensong)
        result = open_current_song();

    return result;
}

/**
 *  Selects the song based on the MIDI control value, and optionally opens it.
 *
 * \param ctrl
 *      The MIDI control to access the song within the current playlist.
 *
 * \param opensong
 *      If true (the default), the song is opened.
 *
 * \return
 *      Returns true if the song is selected, and if specified, is opened
 *      succesfully as well.
 */

bool
playlist::open_select_song_by_midi (int ctrl, bool opensong)
{
    bool result = select_song_by_midi(ctrl);
    if (result && opensong)
        result = open_current_song();

    return result;
}

/**
 *  Goes through all of the playlists and makes sure that all of the song files
 *  are accessible.
 *
 * \param strong
 *      If true, also make sure the MIDI files open without error as well.
 *      The code is similar to read_midi_file() in the midifile module, but it
 *      does not make configuration settings.
 *
 * \return
 *      Returns true if all of the MIDI files are verifiable.
 */

bool
playlist::verify (bool strong)
{
    bool result = ! m_play_lists.empty();
    if (result)
    {
        for (const auto & plpair : m_play_lists)
        {
            const song_list & sl = plpair.second.ls_song_list;
            for (const auto & sci : sl)
            {
                const song_spec_t & s = sci.second;
                std::string fname = song_filepath(s);       // ABORTING HERE
                if (file_exists(fname))
                {
                    if (strong)
                    {
                        /*
                         * The file is parsed.  If the result is false, then the
                         * play-list mode end up false.  Although we don't
                         * really need a playlist-mode flag here, it is useful
                         * to cut down on console output.  Let the caller do
                         * the reporting on errors only.
                         */

                        result = open_song(fname, true);
                        if (! result)
                        {
                            make_file_error_message("song '%s' missing", fname);
                            break;
                        }
                    }
                }
                else
                {
                    std::string fmt = plpair.second.ls_list_name;
                    fmt += ": song '%s' is missing.  Check relative directories.";
                    result = make_file_error_message(fmt, fname);
                    break;
                }
            }
            if (! result)
                break;
        }
    }
    else
    {
        std::string msg = "empty list file '";
        msg += name();
        msg += "'";
        make_error_message(msg);
    }
    return result;
}

/**
 *  Opens/loads the current song.
 *
 * \return
 *      Returns true if there was a song to be opened, and it opened properly.
 */

bool
playlist::open_current_song ()
{
    bool result = false;
    if (m_current_list != m_play_lists.end())
    {
        if (m_current_song != m_current_list->second.ls_song_list.end())
        {
            std::string fname = song_filepath(m_current_song->second);
            result = open_song(fname);
            if (! result)
                (void) make_file_error_message("could not open song '%s'", fname);
        }
    }
    return result;
}

/**
 *
 */

bool
playlist::open_next_list (bool opensong)
{
    bool result = next_list(true);      /* select the next list, first song */
    if (result && opensong)
        result = open_current_song();

    return result;
}

/**
 *
 */

bool
playlist::open_previous_list (bool opensong)
{
    bool result = previous_list(true);  /* select the prev. list, first song */
    if (result && opensong)
        result = open_current_song();

    return result;
}

/**
 *
 */

bool
playlist::open_select_list_by_index (int index, bool opensong)
{
    bool result = select_list_by_index(index, opensong);
    if (result && opensong)
        result = open_current_song();

    return result;
}

/**
 *
 */

bool
playlist::open_select_list_by_midi (int ctrl, bool opensong)
{
    bool result = select_list_by_midi(ctrl, opensong);
    if (result && opensong)
        result = open_current_song();

    return result;
}

/**
 *
 */

bool
playlist::open_next_song (bool opensong)
{
    bool result = next_song();
    if (result && opensong)
        result = open_current_song();

    return result;
}

/**
 *
 */

bool
playlist::open_previous_song (bool opensong)
{
    bool result = previous_song();
    if (result && opensong)
        result = open_current_song();

    return result;
}

/**
 *  Makes a file-error message.
 */

bool
playlist::make_file_error_message
(
    const std::string & fmt,
    const std::string & filename
)
{
    char tmp[256];
    snprintf(tmp, sizeof tmp, fmt.c_str(), filename.c_str());
    make_error_message(tmp);
    return false;
}

/**
 *  Clears the comments and the play-lists, sets the play-list mode to false,
 *  and disables the list and song iterators.
 */

void
playlist::clear ()
{
    m_comments.clear();
    m_play_lists.clear();
    mode(false);
    m_current_list = m_play_lists.end();
    m_current_song = sm_dummy.end();
}

/**
 *  Resets to the first play-list and the first-song in that playlist.
 *
 * \param clearit
 *      If true, clear the playlist no matter what.
 *
 * \return
 *      Returns true if the play-lists where present and the first song of the
 *      first play-list was able to be selected.
 */

bool
playlist::reset_list (bool clearit)
{
    bool result = false;
    if (clearit)
    {
        clear();
    }
    else
    {
        result = ! m_play_lists.empty();
        if (result)
        {
            m_current_list = m_play_lists.begin();
            result = select_song_by_index(0);
        }
    }
    return result;
}

/*
 *  List-container functions.
 */

/**
 *  Adds an already set playlist structure.  It is copied into the list of
 *  play-lists.  It assumes all of the fields in the play-list have been set,
 *  including an empty song-list.
 *
 * \note
 *      Do not reorder!
 *
 * \param plist
 *      Provides a reference to the new playlist to be added.  This parameter
 *      is copied into the list.
 *
 * \return
 *      Returns true if the count of playlists has changed.  However, if a
 *      playlist was simply being modified, this value is false.  So the usage
 *      of the return parameter is dependent upon the context of the call.
 */

bool
playlist::add_list (play_list_t & plist)
{
    bool result = false;
    int count = int(m_play_lists.size());
    int listnumber = plist.ls_midi_number;      /* MIDI control number  */
    if (listnumber >= 0)
    {
        /*
         * std::pair<int, play_list_t>
         */

        auto ls = std::make_pair(listnumber, plist);
        m_play_lists.insert(ls);
        result = int(m_play_lists.size()) == count + 1;
    }
    return result;
}

/**
 *  Selects a play-list with the given index (i.e. a row value).
 *
 * \param index
 *      The index of the play-list re 0.
 *
 * \param selectsong
 *      If true, then the first (0th) song in the play-list is selected.
 *
 * \return
 *      Returns true if the selected play-list is valid.  If true, then the
 *      m_current_list iterator points to the current list.
 */

bool
playlist::select_list_by_index (int index, bool selectsong)
{
    bool result = false;
    int count = 0;
    auto minimum = m_play_lists.begin();
    auto maximum = m_play_lists.end();
    for (auto pci = minimum; pci != maximum; ++pci, ++count)
    {
        if (count == index)
        {
            if (m_show_on_stdout)
                show_list(pci->second);

            m_current_list = pci;
            if (selectsong)
                select_song_by_index(0);

            result = true;
        }
    }
    return result;
}

/**
 *  Selects a play-list with the given MIDI control value.
 *
 * \param index
 *      The MIDI control value of the play-list.  Generally should be
 *      restricted to the range of 0 to 127, to be suitable for MIDI control.
 *
 * \param selectsong
 *      If true, then the first (0th) song in the play-list is selected.
 *
 * \return
 *      Returns true if the selected play-list is valid.  If true, then the
 *      m_current_list iterator points to the current list.
 */

bool
playlist::select_list_by_midi (int ctrl, bool selectsong)
{
    bool result = false;
    int count = 0;
    auto minimum = m_play_lists.begin();
    auto maximum = m_play_lists.end();
    for (auto pci = minimum; pci != maximum; ++pci, ++count)
    {
        int midinumber = pci->second.ls_midi_number;
        if (midinumber == ctrl)
        {
            if (m_show_on_stdout)
                show_list(pci->second);

            m_current_list = pci;
            if (selectsong)
                select_song_by_index(0);

            result = true;
        }
    }
    return result;
}

/**
 *  Moves to the next play-list.  If the iterator reaches the end, this
 *  function wraps around to the beginning.  Also see the other return value
 *  conditions.
 *
 * \param selectsong
 *      If true (the default), the first song in the play-list is selected.
 *
 * \return
 *      Returns true if the play-list iterator was able to be moved, or if
 *      there was only one play-list, so that movement was unnecessary. If the
 *      there are no play-lists, then false is returned.
 */

bool
playlist::next_list (bool selectsong)
{
    bool result = m_play_lists.size() > 0;
    if (m_play_lists.size() > 1)
    {
        ++m_current_list;
        if (m_current_list == m_play_lists.end())
            m_current_list = m_play_lists.begin();

        if (m_show_on_stdout)
            show_list(m_current_list->second);

        if (selectsong)
            select_song_by_index(0);
    }
    return result;
}

/**
 *  Moves to the previous play-list.  If the iterator reaches the beginning,
 *  this function wraps around to the end.  Also see the other return value
 *  conditions.
 *
 * \param selectsong
 *      If true (the default), the first song in the play-list is selected.
 *
 * \return
 *      Returns true if the play-list iterator was able to be moved, or if
 *      there was only one play-list, so that movement was unnecessary. If the
 *      there are no play-lists, then false is returned.
 */

bool
playlist::previous_list (bool selectsong)
{
    bool result = m_play_lists.size() > 0;
    if (m_play_lists.size() > 1)
    {
        if (m_current_list == m_play_lists.begin())
            m_current_list = std::prev(m_play_lists.end());
        else
            --m_current_list;

        if (m_show_on_stdout)
            show_list(m_current_list->second);

        if (selectsong)
            select_song_by_index(0);
    }
    return result;
}

/**
 *  The following four functions are to be used for a playlist editor, though
 *  they can also be used for parsing/loading the playlist.  Kind of a low
 *  priority, but we are leaving room for the concept.
 *
 *  In usage, the playlist and its songs are ordered by MIDI control number.
 *  That is, the MIDI control number is the key, which allows for the fast
 *  lookup of the MIDI control number during a live performance involving MIDI
 *  control from the musician.
 *
 *  In editing, the playlist and its songs are ordered by MIDI control number.
 *  When a playlist is added, we are adding a row to the table, keyed by the
 *  control number.  We then have to re-order the playlist and repopulate the
 *  table.  The song-list for the new playlist is empty, and the playlist
 *  table must reflect that.
 *
 *  Adding a song to the new playlist, if it is indeed new, should be
 *  straightforward.
 *
 *  Adding a song to an existing playlist (which might be a new playlist)
 *  means inserting the song and re-ordering the playlist's songs as
 *  necessary).
 *
 *  The add_list() function adds a playlist and re-orders the list of
 *  playlists.  The playlist is initially empty, and an empty song-list is
 *  created.
 *
 * \todo
 *
 * \return
 *      Returns the return-value of the add_list() overload, which returns
 *      true if the count has changed.
 */

/**
 *  An overloaded function to encapsulate adding a playlist and make the
 *  callers simpler.  The inserted list has an empty song-list.  This function
 *  is intended for use by a playlist editor.
 *
 * \todo
 *      Add the ability to replace a play-list as well.
 *
 * \param index
 *      Provides the location of the active list in the table.  The actual
 *      stored value may change after reordering.
 */

bool
playlist::add_list
(
    int index,
    int midinumber,
    const std::string & name,
    const std::string & directory
)
{
    play_list_t plist;                  /* will be copied upon insertion    */
    plist.ls_index = index;             /* an ordinal value from list table */
    plist.ls_midi_number = midinumber;  /* MIDI control number to use       */
    plist.ls_list_name = name;
    plist.ls_file_directory = directory;
    plist.ls_song_count = 0;            /* no songs to start in new list    */

    /*
     * Song list is empty at first, created by the playlist default constructor.
     *
     *      plist.ls_song_list = slist;
     */

    bool result = add_list(plist);
    reorder_play_list();
    return result;
}

/**
 *  This function removes a playlist at the given index.  The index is an
 *  ordinal, not a key, therefore we have to iterate through the whole list
 *  until we encounter the desired index.  Useful for removing the selected
 *  playlist in a table.
 *
 *  This function works by iterating to the index'th element in the playlist
 *  and deleting it.
 *
 * \param index
 *      The ordinal value (not a key) of the desired table row.
 *
 * \return
 *      Returns true if the desired list was found and removed.
 */

bool
playlist::remove_list (int index)
{
    bool result = false;
    int count = 0;
    auto minimum = m_play_lists.begin();
    auto maximum = m_play_lists.end();
    for (auto pci = minimum; pci != maximum; /* ++pci, */ ++count)
    {
        if (count == index)
        {
            pci = m_play_lists.erase(pci);
            result = true;
            break;
        }
        else
            ++pci;
    }
    if (result)
        reorder_play_list();

    return result;
}

/**
 *  Moves through the play-list container in key (MIDI control number) order,
 *  modifying the index value of each playlist in the main list.
 */

void
playlist::reorder_play_list ()
{
    int index = 0;
    auto minimum = m_play_lists.begin();
    auto maximum = m_play_lists.end();
    for (auto pci = minimum; pci != maximum; ++pci, ++index)
    {
        play_list_t & p = pci->second;
        p.ls_index = index;
    }
}

/*
 *  Song-container functions.
 */

/**
 *  Obtains the current song index, which is a number starting at 0 that
 *  indicates the song's position in the list.  This value is useful to put
 *  the song information at the right place in a table, for example. If the
 *  current-list iterator is invalid, or the current-song iterator is invalid,
 *  then (-1) is returned.
 */

int
playlist::song_index () const
{
    int result = (-1);
    if (m_current_list != m_play_lists.end())
    {
        if (m_current_song != m_current_list->second.ls_song_list.end())
            result = m_current_song->second.ss_index;
    }
    return result;
}

/**
 *
 *  Used to return m_current_list->second.ls_file_directory.
 */

std::string
playlist::file_directory () const
{
    std::string result;
    if (m_current_list != m_play_lists.end())
        return m_current_list->second.ls_file_directory;

    return result;
}

/**
 *
 *  Used to return m_current_list->second.ls_file_directory.
 */

std::string
playlist::song_directory () const
{
    std::string result;
    if (m_current_list != m_play_lists.end())
    {
        if (m_current_song != m_current_list->second.ls_song_list.end())
            result = m_current_song->second.ss_song_directory;
    }
    return result;
}

/**
 *
 *  Used to return m_current_list->second.ls_file_directory.
 */

bool
playlist::is_own_song_directory () const
{
    bool result = false;
    if (m_current_list != m_play_lists.end())
    {
        if (m_current_song != m_current_list->second.ls_song_list.end())
            result = m_current_song->second.ss_embedded_song_directory;
    }
    return result;
}

/**
 *  Obtains the current song MIDI control number, which is a number ranging
 *  from 0 to 127 that indicates the control number that triggers the song. If
 *  the current-list iterator is invalid, or the current-song iterator is
 *  invalid, then (-1) is returned.
 */

int
playlist::song_midi_number () const
{
    int result = (-1);
    if (m_current_list != m_play_lists.end())
    {
        if (m_current_song != m_current_list->second.ls_song_list.end())
            result = m_current_song->second.ss_midi_number;
    }
    return result;
}

/**
 *
 */

std::string
playlist::song_filename () const
{
    std::string result;
    if (m_current_list != m_play_lists.end())
    {
        if (m_current_song != m_current_list->second.ls_song_list.end())
        {
            /*
             * This would return the full path for display, for those cases
             * where the directory is different from the play-list's
             * directory.  However, that is too long to display in some cases.
             * We need to think this through some more.
             *
            if (m_current_song->second.ss_embedded_song_directory)
                result = song_filepath(m_current_song->second);
            else
             *
             */

            result = m_current_song->second.ss_filename;
        }
    }
    return result;
}

/**
 *
 */

std::string
playlist::song_filepath (const song_spec_t & sinfo) const
{
    std::string result = clean_path(sinfo.ss_song_directory);
    result += sinfo.ss_filename;
    return result;
}

/**
 *  Gets the current song-specification from the current play-list, and, if
 *  valid concatenates the song's directory and file-name.
 *
 * \return
 *      Returns the song's directory and file-name as a full path
 *      specification.  However, if there's an error, then an empty string is
 *      returned.
 */

std::string
playlist::song_filepath () const
{
    std::string result;
    if (m_current_list != m_play_lists.end())
    {
        if (m_current_song != m_current_list->second.ls_song_list.end())
            result = song_filepath(m_current_song->second);
    }
    return result;
}

/**
 *  Provides a one-line description containing the current play-list name and
 *  song file.
 *
 * \return
 *      Returns the play-list name and song file-name.  If not in playlist
 *      mode, or an item cannot be found, then an empty string is returned.
 */

std::string
playlist::current_song () const
{
    std::string result;
    if (mode())
    {
        if (m_current_list != m_play_lists.end())
        {
            if (m_current_song != m_current_list->second.ls_song_list.end())
            {
                result = m_current_list->second.ls_list_name;
                result += ": ";
                result += m_current_song->second.ss_filename;
            }
        }
    }
    return result;
}

/**
 *  Selects a song with the given index (i.e. its ordinal position in the
 *  playlist or in a table in a GUI).
 *
 * \param index
 *      The index of the song re 0.
 *
 * \return
 *      Returns true if the current play-list and the current song are valid.
 *      If true, then the m_current_song iterator points to the current song.
 */

bool
playlist::select_song_by_index (int index)
{
    bool result = false;
    if (m_current_list != m_play_lists.end())
    {
        int count = 0;
        song_list & slist = m_current_list->second.ls_song_list;
        for (auto sci = slist.begin(); sci != slist.end(); ++sci, ++count)
        {
            if (count == index)
            {
                if (m_show_on_stdout)
                    show_song(sci->second);

                m_current_song = sci;
                result = true;
                break;
            }
        }
    }
    return result;
}

/**
 *  Selects a song with the given MIDI control value (the key of the map).
 *
 * \param index
 *      The MIDI control value.  Generally should be restricted to the
 *      range of 0 to 127, to be suitable for MIDI control.
 *
 * \return
 *      Returns true if the current play-list and the current song are valid.
 *      If true, then the m_current_song iterator points to the current song.
 */

bool
playlist::select_song_by_midi (int ctrl)
{
    bool result = false;
    if (m_current_list != m_play_lists.end())
    {
        int count = 0;
        song_list & slist = m_current_list->second.ls_song_list;
        for (auto sci = slist.begin(); sci != slist.end(); ++sci, ++count)
        {
            int midinumber = sci->second.ss_midi_number;
            if (midinumber == ctrl)
            {
                if (m_show_on_stdout)
                    show_song(sci->second);

                m_current_song = sci;
                result = true;
            }
        }
    }
    return result;
}

/**
 *  Moves to the next song in the current playlist, wrapping around to the
 *  beginning.
 *
 * \return
 *      Returns true if the next song was selected.
 */

bool
playlist::next_song ()
{
    bool result = false;
    if (m_current_list != m_play_lists.end())
    {
        ++m_current_song;
        if (m_current_song == m_current_list->second.ls_song_list.end())
            m_current_song = m_current_list->second.ls_song_list.begin();

        result = m_current_song != m_current_list->second.ls_song_list.end();
        if (result && m_show_on_stdout)
            show_song(m_current_song->second);
    }
    return result;
}

/**
 *
 */

bool
playlist::previous_song ()
{
    bool result = false;
    if (m_current_list != m_play_lists.end())
    {
        if (m_current_song == m_current_list->second.ls_song_list.begin())
            m_current_song = std::prev(m_current_list->second.ls_song_list.end());
        else
            --m_current_song;

        result = m_current_song != m_current_list->second.ls_song_list.end();
        if (result && m_show_on_stdout)
            show_song(m_current_song->second);
    }
    return result;
}

/**
 *  Adds a song to the current playlist, if available.  Calls the add_song()
 *  overload taking the current song-list and the provided song-specification.
 *
 * \param sspec
 *      Provides the infromation about the song to be added.
 *
 * \return
 *      Returns true if the song was added.
 */

bool
playlist::add_song (song_spec_t & sspec)
{
    bool result = m_current_list != m_play_lists.end();
    if (result)
        result = add_song(m_current_list->second.ls_song_list, sspec);

    return result;
}

/**
 *      Adds the given song to the given song-list.
 *
 * \param slist
 *      Provides the song-list to hold the new song.
 *
 * \param sspec
 *      Provides the infromation about the song to be added.
 *
 * \return
 *      Returns true if the song was added.  That is, if the size of the
 *      song-list increased by 1.
 */

bool
playlist::add_song (song_list & slist, song_spec_t & sspec)
{
    bool result = false;
    int count = int(slist.size());
    int songnumber = sspec.ss_midi_number;

    /*
     * std::pair<int, song_spec_t>
     */

    auto s = std::make_pair(songnumber, sspec);
    slist.insert(s);
    result = int(slist.size()) == count + 1;
    return result;
}

/**
 *      Adds the given song to the song-list of the given play-list.
 *
 * \param plist
 *      Provides the playlist whose song-list is to be updated.
 *
 * \param sspec
 *      Provides the infromation about the song to be added.
 *
 * \return
 *      Returns true if the song was added.  That is, the return value of the
 *      song-list, song-specification add_song() overload.
 */

bool
playlist::add_song (play_list_t & plist, song_spec_t & sspec)
{
    song_list & sl = plist.ls_song_list;
    bool result = add_song(sl, sspec);
    return result;
}

/**
 *  An overloaded function to encapsulate adding a song and make the
 *  callers simpler.  This function is intended for use by a playlist editor.
 *  It supports the replacement of existing songs.
 *
 * \param index
 *      Provides the location of the active item in the table.  The actual
 *      stored value may change after reordering.
 */

bool
playlist::add_song
(
    int index,
    int midinumber,
    const std::string & name,
    const std::string & directory
)
{
    song_spec_t sspec;                  /* will be copied upon insertion    */
    sspec.ss_index = index;             /* an ordinal value from song table */
    sspec.ss_midi_number = midinumber;  /* MIDI control number to use       */
    sspec.ss_song_directory = directory;
    sspec.ss_filename = name;

    /*
     * Song list is empty at first, created by the playlist default constructor.
     *
     *      plist.ls_song_list = sspec;
     */

    bool result = add_song(sspec);
    if (result)
    {
        reorder_song_list(m_current_list->second.ls_song_list);
    }
    else
    {
        /*
         * Remove the current entry and add this one.
         */

        if (remove_song_by_index(index))
        {
            result = add_song(sspec);
            reorder_song_list(m_current_list->second.ls_song_list);
        }
    }
    return result;
}

/**
 *  This function removes a song from the current playlist at the given index.
 *  The index is an ordinal, not a key, therefore we have to iterate through
 *  the whole list until we encounter the desired index.  Useful for removing
 *  the selected song in a table.
 *
 *  This function works by iterating to the index'th element in the song-list
 *  and deleting it.
 *
 * \param index
 *      The ordinal value (not a key) of the desired table row.
 *
 * \return
 *      Returns true if the desired song was found and removed.
 */

bool
playlist::remove_song_by_index (int index)
{
    bool result = false;
    if (m_current_list != m_play_lists.end())
    {
        int count = 0;
        song_list & slist = m_current_list->second.ls_song_list;
        for (auto sci = slist.begin(); sci != slist.end(); ++count)
        {
            if (count == index)
            {
                sci = slist.erase(sci);
                result = true;
                break;
            }
            else
                ++sci;
        }
        if (result)
            reorder_song_list(slist);
    }
    return result;
}

/**
 *  Moves through the song-list container in key (MIDI control number) order,
 *  modifying the index value of each song in the list.
 */

void
playlist::reorder_song_list (song_list & sl)
{
    int index = 0;
    for (auto sci = sl.begin(); sci != sl.end(); ++sci, ++index)
    {
        song_spec_t & s = sci->second;
        s.ss_index = index;
    }
}

/**
 *  Shows a summary of a playlist.
 *
 * \param pl
 *      The playlist structure to show.
 */

void
playlist::show_list (const play_list_t & pl) const
{
    std::cout
        << "    Playlist MIDI #" << pl.ls_midi_number
        << ", slot " << pl.ls_index
        << ": '" << pl.ls_list_name << "'"
        << std::endl
        << "    " << pl.ls_file_directory
        << " " << pl.ls_song_count << " songs"
        << std::endl
        ;
}

/**
 *  Shows a summary of a song. The directory of the song is currently not
 *  shown because it makes the summary more difficult to read in the CLI
 *  output.
 *
 * \param s
 *      The song-specification structure to show.
 */

void
playlist::show_song (const song_spec_t & s) const
{
    std::cout
        << "    Song MIDI #" << s.ss_midi_number << ", slot " << s.ss_index
        << ": " /* << s.ss_song_directory */ << s.ss_filename
        << std::endl
        ;
}


/**
 *  Performs a simple dump of the playlists, mostly for troubleshooting.
 */

void
playlist::show () const
{
    if (m_play_lists.empty())
    {
        printf("No items in playist.\n");
    }
    else
    {
        for (auto pci = m_play_lists.cbegin(); pci != m_play_lists.cend(); ++pci)
        {
            const play_list_t & pl = pci->second;
            show_list(pl);

            const song_list & sl = pl.ls_song_list;
            for (auto sci = sl.cbegin(); sci != sl.cend(); ++sci)
            {
                const song_spec_t & s = sci->second;
                show_song(s);
            }
        }
    }
}

/**
 *  A function for running tests of the play-list handling.  Normally
 *  not needed.
 */

void
playlist::test ()
{
    show();
    show_list(m_current_list->second);
    show_song(m_current_song->second);
    for (int i = 0; i < 8; ++i)
    {
        if (next_song())
        {
            std::cout << "Next song: ";
            show_song(m_current_song->second);
        }
        else
            break;
    }
    for (int i = 0; i < 8; ++i)
    {
        if (previous_song())
        {
            std::cout << "Prev song: ";
            show_song(m_current_song->second);
        }
        else
            break;
    }
    for (int i = 0; i < 8; ++i)
    {
        if (next_list())
        {
            std::cout << "Next list: ";
            show_list(m_current_list->second);
        }
        else
            break;
    }
    for (int i = 0; i < 8; ++i)
    {
        if (previous_list())
        {
            std::cout << "Prev list: ";
            show_list(m_current_list->second);
        }
        else
            break;
    }
    reset_list();
    write();
}

}           // namespace seq66

/*
 * playlist.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

