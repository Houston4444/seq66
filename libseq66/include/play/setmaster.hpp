#if ! defined SEQ66_SETMASTER_HPP
#define SEQ66_SETMASTER_HPP

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
 * \file          setmaster.hpp
 *
 *  This module declares a small manager for a set of sets.
 *
 * \library       seq66 application
 * \author        Chris Ahlstrom
 * \date          2020-08-10
 * \updates       2020-08-10
 * \license       GNU GPLv2 or above
 *
 *  The setmaster class is meant to encapsulate the sets and their layout,
 *  without performing any functions related to patterns.  This new class was
 *  created because we found some confusion, in the setmapper, between the
 *  size of a set versus the size of the set of sets.
 *
 *  The size of a pattern set can vary widely based on user preferences, but
 *  the size of the set of sets managed by the setmaster is hard-wired to 4 x
 *  8.
 */

#include <map>                          /* std::map<>                       */

#include "play/screenset.hpp"           /* seq66::screenset and seq         */

/*
 *  This namespace is not documented because it screws up the document
 *  processing done by Doxygen.
 */

namespace seq66
{

/**
 *  Provides a class for managing screensets and mutegroups.  Much of the
 *  action will occur in the selected play-screen.
 */

class setmaster
{
    friend class performer;             /* a very good friend to have   */
    friend class setmapper;

private:

    using container = std::map<screenset::number, screenset>;

    /**
     *  Storage for the number of rows in the layout of the set-master
     *  (defaults to 4 rows).
     *
     *  Removed the const qualifier to avoid issues with containers.
     */

    int m_rows;

    /**
     *  Storage for the number of columns in the layout of the set-master
     *  (defaults to 8 rows).
     *
     *  Removed the const qualifier to avoid issues with containers.
     */

    int m_columns;

    /**
     *  The maximum number of sets supported.  The main purpose for this value
     *  is as a sanity check for set lookup, not necessarily for limiting the
     *  number of sets.
     */

    int m_set_count;

    /**
     *  Holds a vector of screenset objects.  This container starts out empty.
     */

    container m_container;

    /**
     *  Indicates which set is now in view and available for playback.
     *  We guarantee this to be a valid value or a value (-1) that will be
     *  ignored.  We're not fans of throwing things.
     */

    screenset::number m_playscreen;

    /**
     *  To save set lookup during a number of operations, this pointer, owned by
     *  no one, really, stores a painter to the playing screen-set (play-screen)
     *  in the containter.
     */

    screenset * m_playscreen_pointer;

    /**
     *  Indicates if the m_saved_armed_statuses[] values are the saved state
     *  of the sequences, and can be restored.
     */

    bool m_armed_saved;

    /**
     *  Holds the status of the current play-screen.
     */

    midibooleans m_tracks_mute_state;

private:

    static const int Rows = 4;
    static const int Columns = 8;

public:

    setmaster ();

    /*
     * The move and copy constructors, the move and copy assignment operators,
     * and the destructors are all compiler generated.
     */

    setmaster (const setmaster &) = default;
    setmaster & operator = (const setmaster &) = default;
    setmaster (setmaster &&) = default;
    setmaster & operator = (setmaster &&) = default;
    ~setmaster () = default;

private:

    screenset::number calculate_set (int row, int column) const;

    void clear ()
    {
        m_container.clear();                    /* unconditional zappage!   */
    }

    int rows () const
    {
        return m_rows;
    }

    int columns () const
    {
        return m_columns;
    }

    void reset ();

    /*
     * set_function(s) executes a set-handler for each set.
     * set_function(s,p) runs a set-handler and a slot-handler for each set.
     * set_function(p) runs the slot-handler for all patterns in all sets.
     */

    bool set_function (screenset::sethandler s);
    bool set_function (screenset::sethandler s, screenset::slothandler p);
    bool set_function (screenset::slothandler p);

    bool slot_function (screenset::slothandler p)
    {
        return play_screen()->slot_function(p);
    }

public:

    /**
     *  Using std::map::operator [] is too problematic.  So we use at() and
     *  avoid using illegal values.
     */

    screenset * play_screen ()
    {
        return m_playscreen_pointer;
    }

    const screenset * play_screen () const
    {
        return m_playscreen_pointer;
    }

    std::string sets_to_string (bool showseqs = true) const;
    void show (bool showseqs = true) const;
    bool set_playscreen (screenset::number setno);

    const std::string & name (screenset::number setno) const
    {
        return m_container.find(setno) != m_container.end() ?
            m_container.at(setno).name() : dummy_screenset().name() ;
    }

    bool name (const std::string & nm)
    {
        return play_screen()->name(nm);
    }

    bool name (screenset::number setno, const std::string & nm)
    {
        return m_container.find(setno) != m_container.end() ?
            m_container.at(setno).name(nm) : false ;
    }

    bool is_screenset_active (screenset::number setno) const
    {
        return is_screenset_available(setno) ?
            m_container.at(setno).active() : false ;
    }

    bool is_screenset_available (screenset::number setno) const
    {
        return m_container.find(setno) != m_container.end();
    }

    bool is_screenset_valid (screenset::number setno) const
    {
        return setno >= 0 && setno < m_set_count;
    }

    int screenset_count () const
    {
        return int(m_container.size()) - 1;     /* ignore the dummy set */
    }

    int screenset_max () const
    {
        return m_set_count;
    }

    int screenset_index (screenset::number setno) const;
    bool swap_sets (seq::number set0, seq::number set1);

private:

    container::iterator add_set (screenset::number setno);
    container::iterator find_by_value (screenset::number setno);

    bool remove_set (screenset::number setno)
    {
        container::size_type count = m_container.erase(setno);
        return count > 0;
    }

    /**
     *  Clamps a screenset number to the range of 0 to one less than
     *  m_set_count.
     */

    screenset::number clamp (screenset::number offset) const
    {
        if (offset < 0)
            return 0;
        else if (offset >= m_set_count)
            return m_set_count - 1;

        return offset;
    }

    screenset & dummy_screenset ()
    {
        return m_container.at(screenset::limit());
    }

    const screenset & dummy_screenset () const
    {
        return m_container.at(screenset::limit());
    }

    container & set_container ()        /* meant only for setmapper */
    {
        return m_container;
    }

};              // class setmaster

}               // namespace seq66

#endif          // SEQ66_SETMASTER_HPP

/*
 * setmaster.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
