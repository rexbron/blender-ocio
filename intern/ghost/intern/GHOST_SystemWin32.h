/**
 * $Id$
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

/**

 * $Id$
 * Copyright (C) 2001 NaN Technologies B.V.
 * @author	Maarten Gribnau
 * @date	May 10, 2001
 */

#ifndef _GHOST_SYSTEM_WIN32_H_
#define _GHOST_SYSTEM_WIN32_H_

#ifndef WIN32
#error WIN32 only!
#endif // WIN32

#include <windows.h>

#include "GHOST_System.h"

#if defined(__CYGWIN32__)
#	define __int64 long long
#endif


class GHOST_EventButton;
class GHOST_EventCursor;
class GHOST_EventKey;
class GHOST_EventWindow;

/**
 * WIN32 Implementation of GHOST_System class.
 * @see GHOST_System.
 * @author	Maarten Gribnau
 * @date	May 10, 2001
 */
class GHOST_SystemWin32 : public GHOST_System {
public:
	/**
	 * Constructor.
	 */
	GHOST_SystemWin32();

	/**
	 * Destructor.
	 */
	virtual ~GHOST_SystemWin32();

	/***************************************************************************************
	 ** Time(r) functionality
	 ***************************************************************************************/

	/**
	 * Returns the system time.
	 * Returns the number of milliseconds since the start of the system process.
	 * This overloaded method uses the high frequency timer if available.
	 * @return The number of milliseconds.
	 */
	virtual GHOST_TUns64 getMilliSeconds() const;

	/***************************************************************************************
	 ** Display/window management functionality
	 ***************************************************************************************/

	/**
	 * Returns the number of displays on this system.
	 * @return The number of displays.
	 */
	virtual	GHOST_TUns8 getNumDisplays() const;

	/**
	 * Returns the dimensions of the main display on this system.
	 * @return The dimension of the main display.
	 */
	virtual void getMainDisplayDimensions(GHOST_TUns32& width, GHOST_TUns32& height) const;
	
	/**
	 * Create a new window.
	 * The new window is added to the list of windows managed. 
	 * Never explicitly delete the window, use disposeWindow() instead.
	 * @param	title	The name of the window (displayed in the title bar of the window if the OS supports it).
	 * @param	left	The coordinate of the left edge of the window.
	 * @param	top		The coordinate of the top edge of the window.
	 * @param	width	The width the window.
	 * @param	height	The height the window.
	 * @param	state	The state of the window when opened.
	 * @param	type	The type of drawing context installed in this window.
	 * @return	The new window (or 0 if creation failed).
	 */
	virtual GHOST_IWindow* createWindow(
		const STR_String& title,
		GHOST_TInt32 left, GHOST_TInt32 top, GHOST_TUns32 width, GHOST_TUns32 height,
		GHOST_TWindowState state, GHOST_TDrawingContextType type,
		const bool stereoVisual);

	/***************************************************************************************
	 ** Event management functionality
	 ***************************************************************************************/

	/**
	 * Gets events from the system and stores them in the queue.
	 * @param waitForEvent Flag to wait for an event (or return immediately).
	 * @return Indication of the presence of events.
	 */
	virtual bool processEvents(bool waitForEvent);
	

	/***************************************************************************************
	 ** Cursor management functionality
	 ***************************************************************************************/

	/**
	 * Returns the current location of the cursor (location in screen coordinates)
	 * @param x			The x-coordinate of the cursor.
	 * @param y			The y-coordinate of the cursor.
	 * @return			Indication of success.
	 */
	virtual GHOST_TSuccess getCursorPosition(GHOST_TInt32& x, GHOST_TInt32& y) const;

	/**
	 * Updates the location of the cursor (location in screen coordinates).
	 * @param x			The x-coordinate of the cursor.
	 * @param y			The y-coordinate of the cursor.
	 * @return			Indication of success.
	 */
	virtual GHOST_TSuccess setCursorPosition(GHOST_TInt32 x, GHOST_TInt32 y) const;

	/***************************************************************************************
	 ** Access to mouse button and keyboard states.
	 ***************************************************************************************/

	/**
	 * Returns the state of all modifier keys.
	 * @param keys	The state of all modifier keys (true == pressed).
	 * @return		Indication of success.
	 */
	virtual GHOST_TSuccess getModifierKeys(GHOST_ModifierKeys& keys) const;

	/**
	 * Returns the state of the mouse buttons (ouside the message queue).
	 * @param buttons	The state of the buttons.
	 * @return			Indication of success.
	 */
	virtual GHOST_TSuccess getButtons(GHOST_Buttons& buttons) const;

protected:
	/**
	 * Initializes the system.
	 * For now, it justs registers the window class (WNDCLASS).
	 * @return A success value.
	 */
	virtual GHOST_TSuccess init();

	/**
	 * Closes the system down.
	 * @return A success value.
	 */
	virtual GHOST_TSuccess exit();
	
	/**
	 * Converts raw WIN32 key codes from the wndproc to GHOST keys.
	 * @param wParam	The wParam from the wndproc
	 * @param lParam	The lParam from the wndproc
	 * @return The GHOST key (GHOST_kKeyUnknown if no match).
	 */
	virtual GHOST_TKey convertKey(WPARAM wParam, LPARAM lParam) const;

	/**
	 * Creates modifier key event(s) and updates the key data stored locally (m_modifierKeys).
	 * With the modifier keys, we want to distinguish left and right keys.
	 * Sometimes this is not possible (Windows ME for instance). Then, we want
	 * events generated for both keys.
	 */
	void processModifierKeys(GHOST_IWindow *window);

	/**
	 * Creates mouse button event.
	 * @param type	The type of event to create.
	 * @param type	The button mask of this event.
	 * @return The event created.
	 */
	static GHOST_EventButton* processButtonEvent(GHOST_TEventType type, GHOST_IWindow *window, GHOST_TButtonMask mask);

	/**
	 * Creates cursor event.
	 * @param type	The type of event to create.
	 * @return The event created.
	 */
	static GHOST_EventCursor* processCursorEvent(GHOST_TEventType type, GHOST_IWindow *window);

	/**
	 * Creates a key event and updates the key data stored locally (m_modifierKeys).
	 * In most cases this is a straightforward conversion of key codes.
	 * For the modifier keys however, we want to distinguish left and right keys.
	 */
	static GHOST_EventKey* processKeyEvent(GHOST_IWindow *window, bool keyDown, WPARAM wParam, LPARAM lParam);

	/** 
	 * Creates a window event.
	 * @param type		The type of event to create.
	 * @param window	The window receiving the event.
	 * @return The event created.
	 */
	static GHOST_Event* processWindowEvent(GHOST_TEventType type, GHOST_IWindow* window);

	/**
	 * Returns the local state of the modifier keys (from the message queue).
	 * @param keys The state of the keys.
	 */
	inline virtual void retrieveModifierKeys(GHOST_ModifierKeys& keys) const;

	/**
	 * Stores the state of the modifier keys locally.
	 * For internal use only!
	 * @param keys The new state of the modifier keys.
	 */
	inline virtual void storeModifierKeys(const GHOST_ModifierKeys& keys);

	/**
	 * Windows call back routine for our window class.
	 */
	static LRESULT WINAPI s_wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/** The current state of the modifier keys. */
	GHOST_ModifierKeys m_modifierKeys;
	/** State variable set at initialization. */
	bool m_hasPerformanceCounter;
	/** High frequency timer variable. */
	__int64 m_freq;
	/** High frequency timer variable. */
	__int64 m_start;
	/** Stores the capability of this system to distinguish left and right modifier keys. */
	bool m_seperateLeftRight;
	/** Stores the initialization state of the member m_leftRightDistinguishable. */
	bool m_seperateLeftRightInitialized;
};

inline void GHOST_SystemWin32::retrieveModifierKeys(GHOST_ModifierKeys& keys) const
{
	keys = m_modifierKeys;
}

inline void GHOST_SystemWin32::storeModifierKeys(const GHOST_ModifierKeys& keys)
{
	m_modifierKeys = keys;
}

#endif // _GHOST_SYSTEM_WIN32_H_

