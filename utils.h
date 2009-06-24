/*
    utils.h is part of YouTube Downloader.

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

#ifndef UTILS_H
#define UTILS_H
	#include "common.h"
	void log(int no_nl, char *name, const char * str, ... );
	void bye(int ret);
	int strpos(char *haystack, char *needle );
	string url_decode (string url);
	string delete_strange_characters(string data);
	string get_file_name(string download_dir, string video_name, bool append_random);
	string get_video_id(string url);
	string trim(string str);
	string clean_string(string str);
	unsigned long get_ms_time();
	void print_format_from_number(int fmt);
	string get_ext_from_format(int fmt);
	string get_only_file_name(string fullpath);
	bool file_exists(const char *filename);
	int get_best_quality(int *array,int len);
#endif
