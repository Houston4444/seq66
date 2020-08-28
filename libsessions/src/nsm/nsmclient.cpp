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
 * \file          nsmclient.cpp
 *
 *  This module defines some informative functions that are actually
 *  better off as functions.
 *
 * \library       seq66 application
 * \author        Chris Ahlstrom
 * \date          2020-03-01
 * \updates       2020-08-28
 * \license       GNU GPLv2 or above
 *
 *  nsmclient is an Non Session Manager (NSM) OSC client agent.  The NSM API
 *  comprises a simple Open Sound Control (OSC) based protocol.
 *
 *  The Non project contains a daemon, nsmd, which is an implementation of the
 *  server side of the NSM API. nsmd is controlled by the non-session-manager
 *  GUI. The same server-side API can also be implemented by other session
 *  managers (such as LADISH).  The only dependency for client implementations
 *  is liblo (the OSC library) and the nsm.h header file.
 *
 *  Session Manager Startup:
 *
 *      To start the Non Session Manager and the GUI:
 *
 *      $ non-session-manager [ -- --session-root path ] &
 *
 *      The default path is "$HOME/NSM Sessions".  Inside this directory there
 *      will ultimately exist one directory per session, each with the name of
 *      the session as given by the user.  Inside this session directory is a
 *      file called "session.nsm".  Inside this file is a list of session
 *      clients in the following format:
 *
 *          appname:exename:nXXXX       (example:  qseq66:qseq66:nYUSM)
 *
 *      where XXXX is a set of four random uppercase ASCII letters.  The
 *      string "nXXXX" is used to look up clients.
 *
 *      At startup, the environment variable NSM_URL is added to the
 *      environment.  It's format is described below. Commands are handled as
 *      per the "Commands handling by the server" section at the top of the
 *      nsmmessageex.cpp module. A new session (e.g. "MySession") can be added
 *      using the "New" button.  It will be stored in "$HOME / New Sessions /
 *      MySession / session.nsm", which starts out empty.
 *
 *      Once running, an executable can be added as a client.  However, the
 *      executable must be in the path.  The command "/nsm/server/add" will
 *      fail with the error "Absolute paths are not permitted. Clients must be
 *      in $PATH".  Once added properly, NSM spawns the executable, and the
 *      executable inherits the environment (i.e. NSM_URL).
 *
 *      After closing/saving the session, the session.nsm file contains
 *      only a line such as "qseq66:qseq66:nMTRJ", as noted above.
 *
 *      Once qseq66 is part of the session, clicking on the session name will
 *      launch qseq66.
 *
 *  Process:
 *
 *      -#  Find out if NSM_URL is defined in the host environment, and
 *          create the nsmclient object on the heap if so. The
 *          create_nsmclient() should be used; it returns a "unique pointer".
 *          (We may need to provide specific factory functions for Qt, Gtkmm,
 *          and command-line versions of the application.)
 *      -#  If NSM_URL is valid and reachable, call the nsmbase::announce()
 *          function.
 *      -#  Connect up callbacks (e.g. signals in Qt) for the following events:
 *          -   Open NSM session. The caller should first see if this nsmclient
 *              is active.  If so, close the session, which checks the
 *              dirty-count in order to ask the user if changes need to be saved.
 *          -   Save NSM session.
 *          -   Show NSM session.
 *          -   Hide NSM session.
 *      -#  Applications must not register JACK clients until receiving an
 *          open message, which provides a unique client name prefix suitable
 *          for passing to JACK.
 *      -#  Call nsmclient::announce(APP_TITLE, ":switch:dirty:optional-gui:") if
 *          using a GUI.
 *
 *  NSM_URL:
 *
 *      The NSM_URL environment variable is used to inform clients of how to
 *      reach the nsmd daemon, which can be started as follows:
 *
 *          nsmd [--osc-port portnum] [--session-root path] [--detach]
 *
 *      In the following setting, 18440 is the 'portnum'.  127.0.0.1 is the
 *      local host (the computer on which all NSM-related apps are running.
 *
 *          NSM_URL=osc.udp://127.0.0.1:18440/
 *          NSM_URL=osc.udp://mlsleno:15325/        (on developer laptop)
 *          NSM_URL=osc.udp://mlsasus.mls:12325/    (on another laptop)
 *
 *      Note that, if running an nsm_proxy client, this variable may need to be
 *      passed on the command-line (in typical bash fashion).
 *
 *      Also see the file contrib/non/nsmopen.sh for examples, and "oscsend
 *      --help".
 *
 *  New session:
 *
 *      TODO
 *
 *  Detecting NSM session actions:
 *
 *      In a Qt-based application, we can provide an extended NSM client that
 *      will respond to signals propagated by the "emit" operator. In a
 *      command-line application, we can set flags that are detected in a polling
 *      loop and cause actions to occur.  In Gtkmm applications, we can set
 *      callbacks to be executed.  It would be nice to use the same (callback?)
 *      system for all of them.
 *
 * Shutdown:
 *
 *      When an NSM client shuts down, NSM detects its PID and detects if the
 *      client aborted or stopped normally. A normal stopped occurs when the
 *      client receives a KILL or QUIT command.  If a QUIT command, the server
 *      sends:
 *
 *          "/nsm/gui/client/status" + Client-ID + "removed" status
 *
 *      Otherwise, it sends two messages:
 *
 *          "/nsm/gui/client/label" + Client-ID + optional error message
 *          "/nsm/gui/client/status" + Client-ID + "stopped"
 *
 *  Notes for the future:
 *
 *      There's the general NSM osc protocol which allows basic session
 *      management like adding programs, listing the current sessions and
 *      starting one. This would be sufficient for cadence.
 *
 *      Detection of NSM is by checking validity of NSM_URL environment variable
 *      and getting a response from it so a simple "/nsm/session/list" to
 *      NSM_URL would be sufficient to both populate the list and detect whether
 *      it's running
 *
 *      nsm-proxy integration, however, might be more difficult due to nsmd only
 *      providing details of launching programs to the non-session-manager gui.
 *
 *      Also nsmd will only bind to a single GUI, so this should be the
 *      frontend. (An alternative would be to modify nsmd to support multiple
 *      GUIs running at the same time.)
 *
 *      New tool that implements the /nsm/gui/xxx OSC endpoint to receive more
 *      details about the session which allow it to give a view of the current
 *      session.
 *
 *      INVESTIGATE the NSM replacement, RaySend!!!
 */

#include "util/basic_macros.hpp"        /* not_nullptr(), pathprint()       */
#include "nsm/nsmclient.hpp"            /* seq66::nsmclient class           */
#include "nsm/nsmmessagesex.hpp"        /* seq66::nsm message functions     */

#if defined SEQ66_PLATFORM_DEBUG
#include "util/strfunctions.hpp"        /* seq66::bool_to_string()          */
#endif

/*
 *  Do not document a namespace; it breaks Doxygen.
 */

namespace seq66
{

/**
 *  The typical type signature of this callback is "ssss".
 */

static int
osc_nsm_announce_reply
(
    const char * path,
    const char * types,
    lo_arg ** argv,
    int /* argc */,
    lo_message /* msg */,
    void * user_data
)
{
    nsmclient * pnsmc = static_cast<nsmclient *>(user_data);
    if (is_nullptr(pnsmc) || (! nsm::is_announce(&argv[0]->s)))
        return -1;

    nsm::incoming_msg("osc_nsm_announce_reply", path, types);
    pnsmc->announce_reply(&argv[1]->s, &argv[2]->s, &argv[3]->s);
    return 0;
}

static int
osc_nsm_open
(
    const char * path,
    const char * types,
    lo_arg ** argv,
    int /* argc */,
    lo_message /* msg */,
    void * user_data
)
{
    nsmclient * pnsmc = static_cast<nsmclient *>(user_data);
    if (is_nullptr(pnsmc))
        return -1;

    nsm::incoming_msg("osc_nsm_open", path, types);
    pnsmc->open(&argv[0]->s, &argv[1]->s, &argv[2]->s);
    return 0;
}

static int
osc_nsm_save
(
    const char * path,
    const char * types,
    lo_arg ** /* argv */,
    int /* argc */,
    lo_message /* msg */,
    void * user_data
)
{
    nsmclient * pnsmc = static_cast<nsmclient *>(user_data);
    if (is_nullptr(pnsmc))
        return -1;

    nsm::incoming_msg("osc_nsm_save", path, types);
    pnsmc->save();                  /* a virtual function   */
    return 0;
}

static int
osc_nsm_session_loaded
(
    const char * path,
    const char * types,
    lo_arg ** /* argv */,
    int /* argc */,
    lo_message /* msg */,
    void * user_data
)
{
    nsmclient * pnsmc = static_cast<nsmclient *>(user_data);
    if (is_nullptr(pnsmc))
        return -1;

    nsm::incoming_msg("osc_nsm_session_loaded", path, types);
    pnsmc->loaded();
    return 0;
}

static int
osc_nsm_label
(
    const char * path,
    const char * types,
    lo_arg ** argv,
    int /* argc */,
    lo_message /* msg */,
    void * user_data
)
{
    nsmclient * pnsmc = static_cast<nsmclient *>(user_data);
    if (is_nullptr(pnsmc))
        return -1;

    nsm::incoming_msg("osc_nsm_label", path, types);
    pnsmc->label(std::string(&argv[0]->s));         /* a virtual function */
    return 0;
}

/**
 *
 *  This function could also be called osc_show_gui().  See the nsm-proxy code.
 */

static int
osc_nsm_show
(
    const char * path,
    const char * types,
    lo_arg ** /* argv */,
    int /* argc */,
    lo_message /* msg */,
    void * user_data
)
{
    nsmclient * pnsmc = static_cast<nsmclient *>(user_data);
    if (is_nullptr(pnsmc))
        return -1;

    nsm::incoming_msg("osc_nsm_show", path, types);
    pnsmc->show(path);                  /* a virtual function   */
    return 0;
}

/**
 *  This function could also be called osc_hide_gui().  See the nsm-proxy
 *  code.
 */

static int
osc_nsm_hide
(
    const char * path,
    const char * types,
    lo_arg ** /* argv */,
    int /* argc */,
    lo_message /* msg */,
    void * user_data
)
{
    nsmclient * pnsmc = static_cast<nsmclient *>(user_data);
    if (pnsmc == NULL)
        return -1;

    nsm::incoming_msg("osc_nsm_hide", path, types);
    pnsmc->hide(path);
    return 0;
}

/**
 *
 */

static int
osc_nsm_broadcast
(
    const char * path,
    const char * types,
    lo_arg ** argv,
    int argc,
    lo_message /*msg*/,
    void * user_data
)
{
    nsmclient * pnsmc = static_cast<nsmclient *>(user_data);
    if (pnsmc == NULL)
        return -1;

    std::vector<std::string> arguments = nsm::convert_lo_args(types, argc, argv);
    nsm::incoming_msg("osc_nsm_broadcast", path, types);
    pnsmc->broadcast(path, types, arguments);
    return 0;
}

/* ------------------------------------------------------------------------ */

/**
 *
 */

nsmclient::nsmclient
(
    const std::string & nsmurl,
    const std::string & nsmfile,
    const std::string & nsmext
) :
    nsmbase         (nsmurl, nsmfile, nsmext)
{
    // no code so far
}

/**
 *
 */

nsmclient::~nsmclient ()
{
    // no code so far
}

/**
 *
 */

bool
nsmclient::initialize ()
{
    bool result = nsmbase::initialize();
    if (result)
    {
        add_client_method(nsm::tag::replyex, osc_nsm_announce_reply);
        add_client_method(nsm::tag::open, osc_nsm_open);
        add_client_method(nsm::tag::save, osc_nsm_save);
        add_client_method(nsm::tag::loaded, osc_nsm_session_loaded);
        add_client_method(nsm::tag::label, osc_nsm_label);
        add_client_method(nsm::tag::show, osc_nsm_show);
        add_client_method(nsm::tag::hide, osc_nsm_hide);
        add_client_method(nsm::tag::null, osc_nsm_broadcast);
        start_thread();
    }
    return result;
}

/*
 * Server announce reply.
 */

void
nsmclient::announce_reply
(
    const std::string & mesg,
    const std::string & mgr,
    const std::string & caps
)
{
    is_active(true);
    manager(mgr);
    capabilities(caps);
    // emit active(true);

    nsm::incoming_msg("announce_reply", mgr, caps + " " + mesg);
}

/**
 *  Client open callback. Compare to the "open" code in nsm-proxy.
 */

void
nsmclient::open
(
    const std::string & pathname,
    const std::string & displayname,
    const std::string & clientid
)
{
    path_name(pathname);
    display_name(displayname);
    client_id(clientid);
    // emit open();

    nsm::incoming_msg("open", pathname, clientid + "" + displayname);
}

/*
 * Client save callback.
 */

void
nsmclient::save ()
{
    nsm_debug("save");

    // Here, zyn gets a character message and an error code, and replies with
    // either a reply or an error-reply.
    //
    // emit save();

    if (save_session())
    {
        (void) save_reply(nsm::reply::ok);       // A FAKE ANSWER FOR NOW
    }

}

void
nsmclient::loaded ()
{
    nsm_debug("loaded");
    // emit loaded();
}

void
nsmclient::label (const std::string & label)
{
    std::string tag("label: '");
    tag += label;
    tag += "'";
    nsm_debug(tag); // no code
}

/**
 *  Client show optional GUI.  The derived class must provide this
 *  functionality.
 */

void
nsmclient::show (const std::string & path)
{
    nsm_debug("show");
    // emit show();
    send_from_client(nsm::tag::reply, path, "OK");
}

/*
 * Client hide optional GUI.
 */

void
nsmclient::hide (const std::string & path)
{
    nsm_debug("hide");
    send_from_client(nsm::tag::hidden, path, "OK");
    send_from_client(nsm::tag::reply, path, "OK");  // ss
    // emit hide();
}

/**
 *  Receives a broadcast and figures out what to do with it.  It handles:
 *
 *      -   /reply
 *      -   /nsm/server/announce
 *
 *  WEIRD.  NO LONGER BEING CALLED.  Instead announce_reply() and open() are
 *  being called.  WEIRD.
 *
 *  This one is for *sending* broadcasts, not yet implemented:
 *
 *      nsmbase::broadcast (const std::string & path, lo_message msg)
 */

void
nsmclient::broadcast
(
    const std::string & message,
    const std::string & pattern,
    const std::vector<std::string> & argv
)
{
    if (lo_is_valid())
    {
        int argc = int(argv.size());
        for (int i = 0; i < argc; ++i)
        {
            printf("   [%d] '%s'\n", i, argv[i].c_str());
        }

#if 0
        std::string message;
        std::string pattern;
        bool ok = nsm::server_msg(nsm::tag::broadcast, message, pattern);
        if (ok)
        {
            // nsm::incoming_msg("broadcast", message, tag);
        }
#endif

        nsm::tag t = nsm::client_tag(message, pattern);
        if (t == nsm::tag::reply)                       /* "ss"   */
        {
printf("BROADCAST /reply received\n");
        }
        else if (t == nsm::tag::replyex)                /* "ssss" */
        {
            /*
             *  Check if argv[0] is "/nsm/server/announce".  If so, then the
             *  rest of the arguments are "Howdy...", "Non Session Manager", and
             *  the capabilities string of the server.
             */

printf("BROADCAST /replyex received\n");
            nsm::tag t0 = nsm::server_tag(message);
            if (t0 == nsm::tag::announce)
            {
                // TODO: pass the values to Tab_Session in qsmainwnd.
printf("BROADCAST /nsm/server/announce received\n");
            }

        }
        else if (t == nsm::tag::open)
        {
            // TODO
printf("BROADCAST /nsm/client/open received\n");
        }
    }
    // emit broadcast();
}

/**
 *  Provides a client-announce function.
 *
 *  If NSM_URL is valid and reachable, call this function to send the following
 *  "sssiii" message to the provided address as soon as ready to respond to the
 *  /nsm/client/open event.  api_version_major and api_version_minor must be
 *  the two parts of the version number of the NSM API.  If registering JACK
 *  clients, application_name must be passed to jack_client_open.  capabilities
 *  is a string containing a list of the capabilities the client possesses,
 *  e.g.  :dirty:switch:progress: executable_name must be the executable name
 *  that launched the program (e.g argv[0]).
 *
\verbatim
    /nsm/server/announce s:application_name s:capabilities s:executable_name
         i:api_version_major i:api_version_minor i:pid
\endverbatim
 *
 */

bool
nsmclient::announce
(
    const std::string & appname,        /* actually a package name, "Seq66" */
    const std::string & exename,        /* comes from argv[0]               */
    const std::string & capabilities    /* e.g. ":switch:dirty:"            */
)
{
    bool result = send_announcement(appname, exename, capabilities);
    if (result)
    {
        msg_check();                    /* wait for the response    */
    }
    return result;
}

/*
 * Prospective caller helpers a la qtractorMainForm.
 */

/**
 *  After calling this function and checking the return value,
 *  the caller should close out any "open" items and set up a new "session".
 *
 *  After that, call open_reply() with the boolean result of the new session
 *  setup.
 */

bool
nsmclient::open_session ()
{
    bool result = nsmbase::open_session();
    if (result)
    {
        // what else?
    }
    return result;
}

/**
 *  Provides a factory function to create an nsmclient, and then to call its
 *  virtual initialization function (so that we don't have to call it in the
 *  constructor).
 *
 *  Note that this bare pointer should be assigned immediately to a smart
 *  pointer, such as std::unique_ptr<>.  See seq_qt5/src/qt5nsmanager.cpp for
 *  an example.
 */

nsmclient *
create_nsmclient
(
    const std::string & nsmfile,
    const std::string & nsmext
)
{
    nsmclient * result = nullptr;
    std::string url = nsm::get_url();
    if (! url.empty())
    {
        result = new (std::nothrow) nsmclient(url, nsmfile, nsmext);
        if (not_nullptr(result))
            (void) result->initialize();
    }
    return result;
}

}           // namespace seq66

/*
 * nsmclient.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

