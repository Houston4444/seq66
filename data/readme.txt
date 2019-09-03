readme.txt for Seq66 0.96.1
Chris Ahlstrom
2015-09-10 to 2018-10-29

Seq66 is a reboot of seq66, extending it with new features and bug fixes.
It is a "live performance" sequencer, with the musician creating and
controlling a number of patter loops.

An extensive manual is found at:

    https://github.com/ahlstromcj/seq66-doc.git
    
Prebuilt Debian packages, Windows installers, and source tarballs are
available here:

    https://github.com/ahlstromcj/seq66-packages.git

Windows support:

    This version uses a Qt 5 user-interface based on Kepler34, but using the
    standard Seq66 libraries.  The user-interface works, and Windows
    built-in MIDI devices are detected, inaccessible devices are ignored, and
    playback (e.g. to the built-in wavetable synthesizer) work.

    However, the Qt 5 GUI is a little behind the Gtkmm 2.4 GUI for some
    features.  It is about 90% complete, but very useable. In the meantime,
    some configuration can be done manually in the "rc" and "usr" files.  See
    README.windows for more information.

See the INSTALL file for build-from-source instructions or using a
conventional source tarball.  This file is part of:

    https://github.com/ahlstromcj/seq66.git

# vim: sw=4 ts=4 wm=4 et ft=sh fileformat=dos
