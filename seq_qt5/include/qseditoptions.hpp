#if ! defined SEQ66_QSEDITOPTIONS_HPP
#define SEQ66_QSEDITOPTIONS_HPP

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
 * \file          qseditoptions.hpp
 *
 *  The time bar shows markers and numbers for the measures of the song,
 *  and also depicts the left and right markers.
 *
 * \library       seq66 application
 * \author        Chris Ahlstrom
 * \date          2018-01-01
 * \updates       2018-09-26
 * \license       GNU GPLv2 or above
 *
 */

#include <QDialog>

/*
 *  Do not document the namespace, it breaks Doxygen.
 */

namespace Ui
{
    class qseditoptions;
}

/*
 *  Do not document the namespace, it breaks Doxygen.
 */

namespace seq66
{
    class performer;

/**
 *
 */

class qseditoptions final : public QDialog
{
    Q_OBJECT

public:

    qseditoptions
    (
        performer & perf,
        QWidget * parent = nullptr
    );
    virtual ~qseditoptions();

private:

    void syncWithInternals();   /* makes dialog reflect internal settings   */
    void backup();              /* backup preferences for cancel-changes    */

    const performer & perf () const
    {
        return m_perf;
    }

    performer & perf ()
    {
        return m_perf;
    }

private:

    void on_spinBoxClockStartModulo_valueChanged (int arg1);
    void on_plainTextEditTempoTrack_textChanged ();
    void on_pushButtonTempoTrack_clicked ();
    void on_checkBoxRecordByChannel_clicked (bool checked);
    void on_chkJackConditional_stateChanged (int arg1);

private slots:

    void slot_jack_mode (int buttonno);
    void update_jack_connect ();
    void update_jack_disconnect ();
    void update_master_cond ();
    void update_time_master ();
    void update_transport_support ();
    void update_jack_midi ();
    void okay ();
    void cancel ();
    void update_note_resume ();
    void update_key_height ();
    void update_ui_scaling (const QString &);
    void update_pattern_editor ();

private:

    Ui::qseditoptions * ui;
    performer & m_perf;
    bool m_is_initialized;

    /*
     * Backup variables for settings.  Not everything new is currently backed
     * up, though.
     */

    bool m_backup_JackTransport;
    bool m_backup_TimeMaster;
    bool m_backup_MasterCond;
    bool m_backup_NoteResume;
    bool m_backup_JackMidi;
    int m_backup_KeyHeight;

};          // class qseditoptions

}           // namespace seq66

#endif      // SEQ66_QSEDITOPTIONS_HPP

/*
 * qseditoptions.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

