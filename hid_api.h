/******************************************************************************
 * Name:      hid_api.h
 * Purpose:   Header Selector for wxThread based USB HID API
 * Author:    Ingolf Tippner (info@tecno-world.com)
 * Created:   2017-08-08
 * Copyright (C) 2017  Ingolf Tippner (Tecno-World, Pitius Tec S.L.)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.

 * Changelog:

********************************************************************************/
#ifndef HID_API_H
#define HID_API_H

#if defined (__WINDOWS__)
 #include "hid_api_win.h"
#endif // defined
#if defined (__LINUX__)
 #include "hid_api_linux.h"
#endif

#endif // HID_API_H





