#!/bin/sh
# $Id$
# ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version. The Blender
# Foundation also sells licenses for use in proprietary software under
# the Blender License.  See http://www.blender.org/BL/ for information
# about this.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# The Original Code is Copyright (C) 2002 by Hans Lambermont
# All rights reserved.
#
# The Original Code is: all of this file.
#
# Contributor(s): none yet.
#
# ***** END GPL/BL DUAL LICENSE BLOCK *****
#
# FreeBSD make kickstart script.
# install from ports: python, openal, jpeg, png, sdl, nspr

rm -f /tmp/.nanguess
MAKE=gmake
NANBLENDERHOME=`pwd`
MAKEFLAGS="-w -I $NANBLENDERHOME/source --no-print-directory"
HMAKE="$NANBLENDERHOME/source/tools/hmake/hmake"

export MAKE NANBLENDERHOME MAKEFLAGS HMAKE

$HMAKE -C extern/
if [ $? -eq 0 ]; then
	$HMAKE -C intern/
	if [ $? -eq 0 ]; then
		$HMAKE -C source/
	fi
fi

