/* $Id$
-----------------------------------------------------------------------------
This source file is part of VideoTexture library

Copyright (c) 2007 The Zdeno Ash Miklas

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#if !defined IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H


#include "Common.h"

#include "ImageBase.h"


/// class for viewport access
class ImageViewport : public ImageBase
{
public:
	/// constructor
	ImageViewport (void);

	/// destructor
	virtual ~ImageViewport (void);

	/// is whole buffer used
	bool getWhole (void) { return m_whole; }
	/// set whole buffer use
	void setWhole (bool whole);
	/// get capture size in viewport
	short * getCaptureSize (void) { return m_capSize; }
	/// set capture size in viewport
	void setCaptureSize (short * size = NULL);

	/// get position in viewport
	int * getPosition (void) { return m_position; }
	/// set position in viewport
	void setPosition (int * pos = NULL);

protected:
	/// frame buffer rectangle
	int m_viewport[4];

	/// size of captured area
	short m_capSize[2];
	/// use whole viewport
	bool m_whole;

	/// position of capture rectangle in viewport
	int m_position[2];
	/// upper left point for capturing
	int m_upLeft[2];

	/// buffer to copy viewport
	BYTE * m_viewportImage;
	/// texture is initialized
	bool m_texInit;

	/// capture image from viewport
	virtual void calcImage (unsigned int texId);

	/// get viewport size
	int * getViewportSize (void) { return m_viewport + 2; }
};


#endif

