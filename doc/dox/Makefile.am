#*******************************************************************************
# Makefile.am
#-------------------------------------------------------------------------------
##
# \file       	Makefile.am
# \library    	generic
# \author     	Chris Ahlstrom
# \date       	2015-09-10
# \update     	2020-10-02
# \version    	$Revision$
# \license    	$XPC_SUITE_GPL_LICENSE$
#
# 		Makefile for the doc/dox subdirectory of this package.  We had to do a
# 		little two-stepping to make sure that the documentation is built
# 		everytime we touch one of the doxygen files, and NOT when we touch one of
# 		the source-code files.  Tired of checking in the PDF file all the time!
#
#-------------------------------------------------------------------------------

#*****************************************************************************
# Packing/cleaning targets
#-----------------------------------------------------------------------------

AUTOMAKE_OPTIONS = foreign dist-zip dist-bzip2
MAINTAINERCLEANFILES = Makefile.in dox-warnings.log
CLEANFILES = dox-stamp

#******************************************************************************
#  EXTRA_DIST
#------------------------------------------------------------------------------
#
# We need to move some of this to a Makefile.am in the directory aboe.
#
#------------------------------------------------------------------------------

EXTRA_DIST = make_dox *.cfg *.dox *.pdf

#******************************************************************************
# Local project directories
#------------------------------------------------------------------------------

top_srcdir = @top_srcdir@
srcdir = @srcdir@

#******************************************************************************
# Install directories
#------------------------------------------------------------------------------

prefix = @prefix@
datadir = @datadir@
docdir = @docdir@
datarootdir = @datarootdir@
seq66docdir = @seq66docdir@

#------------------------------------------------------------------------------
# The $(srcdir) component points us to the correct place whether the build is
# in-source or out-of-source.  Can't use pushd/popd because they are bash
# builtins and are not available, except inside a script.
#------------------------------------------------------------------------------

all-local:
	$(srcdir)/make_dox $(srcdir) libseq66
	$(srcdir)/make_dox $(srcdir) libsessions
	$(srcdir)/make_dox $(srcdir) seq_portmidi
	$(srcdir)/make_dox $(srcdir) seq_rtmidi

clean-local:
	$(srcdir)/make_dox $(srcdir) libseq66 clean
	$(srcdir)/make_dox $(srcdir) libsessions clean
	$(srcdir)/make_dox $(srcdir) seq_portmidi clean
	$(srcdir)/make_dox $(srcdir) seq_rtmidi clean

#******************************************************************************
# Installing documentation
#------------------------------------------------------------------------------
#
# 	   We need to add an install-data-hook to copy the generated
# 	   documentation directories to the destination directory.  The normal
# 	   method doesn't work because /usr/bin/install will only install files,
# 	   and automake doesn't give it the switch needed to install directories.
#
# Don't install the html files:
#
# 		cp -r -p $(top_builddir)/doc/dox/html $(xpcdoxygendir)
#
#------------------------------------------------------------------------------

install-data-local:
	mkdir -p $(DESTDIR)$(seq66docdir)
	cp -r -p $(top_builddir)/doc/*.pdf $(DESTDIR)$(seq66docdir)

#*******************************************************************************
# uninstall-hook
#-------------------------------------------------------------------------------
#
#		Quick and dirty method used.
#
#-------------------------------------------------------------------------------

uninstall-hook:
	rm -rf $(DESTDIR)$(seq66docdir)

#******************************************************************************
# Makefile.am (dox)
#------------------------------------------------------------------------------
# vim: ts=3 sw=3 noet ft=automake
#------------------------------------------------------------------------------
