/*
    common.h is part of YouTube Downloader.

    YouTube Downloader is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    YouTube Downloader is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with YouTube Downloader.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef COMMON_H
#define COMMON_H

	#include <cstdio>
	#include <cstdlib>
	#include <string>
	#include <iostream>
	#include <curl/curl.h>
	#include <stdarg.h>
	#include <string.h>
	
	#ifdef WIN32
		#include <windows.h>
	#endif
	
	int get_fmt();
	
	using namespace std;
#endif
