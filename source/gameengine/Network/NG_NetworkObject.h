/*
 * $Id$
 *
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * The contents of this file may be used under the terms of either the GNU
 * General Public License Version 2 or later (the "GPL", see
 * http://www.gnu.org/licenses/gpl.html ), or the Blender License 1.0 or
 * later (the "BL", see http://www.blender.org/BL/ ) which has to be
 * bought from the Blender Foundation to become active, in which case the
 * above mentioned GPL option does not apply.
 *
 * The Original Code is Copyright (C) 2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 * NetworkGame_NetworkObject generic Network Object class
 */
#ifndef NG_NETWORKOBJECT_H
#define NG_NETWORKOBJECT_H

#include "STR_String.h"

class NG_NetworkObject
{
	STR_String m_name;
public:
	NG_NetworkObject();
	~NG_NetworkObject();
	const STR_String& GetName();
};

#endif //NG_NETWORKOBJECT_H
