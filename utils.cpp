/*
    utils.cpp is part of YouTube Downloader.

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

#include "utils.h"

#ifdef WIN32
	#include <windows.h>
	#include <mmsystem.h>
#else
	#include <sys/timeb.h>
#endif

unsigned long get_ms_time()
{
    unsigned long time_in_ms = 0;
	#ifdef WIN32
    	time_in_ms = timeGetTime();
	#else
    	struct timeb tp;
    	ftime(&tp);

    	time_in_ms = tp.time * 1000 + tp.millitm;
	#endif

    return time_in_ms;
}

void log(int no_nl, char *name, const char * str, ... )
{
	va_list ap;
	char buf[32768];
    printf("[%s] ",name);
	va_start(ap, str);
	vprintf(str, ap);
	va_end(ap);
	if(no_nl==0)
		printf("\n");
}

void bye(int ret)
{
    system("pause");
    exit(ret);    
}

int strpos(char *haystack, char *needle )
{
    if(strlen(haystack)==0)
        return -1;
	char *pDest = (char *)malloc((strlen(haystack) + 1) * sizeof(char));
	int position;

	pDest = strstr( haystack, needle );

	if(pDest)
		position = pDest - haystack;
	else
	{
        free(pDest);
		return -1;
    }
    
    free(pDest);
	return position;
}

string url_decode (string url)
{
	string ret;
	char hex_val[1];
	int dec_val;
	for(int i=0;i<url.length();i++)
	{
		if(url[i]=='%')
		{
			if((i+2) <= url.length())
			{
				hex_val[0] = url[++i];
				hex_val[1] = url[++i];
				sscanf(hex_val,"%2X",&dec_val);
				ret += dec_val;
			}	
		}
		else if(url[i]=='+')
			ret += ' ';
		else
			ret += url[i];
	}
	return ret;
}

string delete_strange_characters(string data)
{
	string ret;
	for(int i=0;i<data.length();i++)	
		if((data[i]<0x20) || (data[i]>0x7E))
			ret += '?';
		else
			ret += data[i];
			
	return trim(ret);
}

string get_file_name(string download_dir, string video_name, bool append_random)
{
	string ret;
	char pos;
	char strange_char[] = "?\\/*<>\"|:#+%";
	char rnd[300];
	
	if(download_dir.length()>0)
	{
		if( (download_dir.rfind("/")==(download_dir.length()-1)) || (download_dir.rfind("\\")==(download_dir.length()-1)) )
			download_dir = download_dir.substr(0,download_dir.length() - 1);
		#ifdef WIN32
			download_dir.append("\\");
		#else
			download_dir.append("/");
		#endif
		ret = download_dir;
	}
	
	video_name = delete_strange_characters(video_name);
	if(video_name.length()==0)
	{
		video_name = "video";
		append_random = true;
	}
	
	if(append_random)
	{
		ltoa(get_ms_time(),rnd,10);
		sprintf(rnd,"%d",get_ms_time());
		video_name.append(rnd);
	}

	for(int i=0;i<video_name.length();i++)
	{
		
		for(int x=0;x<strlen(strange_char);x++)
			if (video_name[i]==strange_char[x])
				video_name[i]='_';	
			
		ret += video_name[i];
	}
	

	ret = trim(ret);

	ret.append(get_ext_from_format(get_fmt()));
	
	return ret;
}

string get_video_id(string url)
{
    string tot;
    
    int start = url.find("v=") + 2;
    if (start<2)
        return (string)"";
    
    int end = url.find("&",start);
    
    if(end!=-1)
        tot = url.substr(start,end-start);
    else
        tot = url.substr(start);

    
    return tot;
}

string trim(string str) 
{ 
    string ret;
    bool ok = false;
    int pos=0,bp=0;
    
    //skip all spaced-char(part 1 of ltrim)
	for(pos=0;(str[pos]==' ' || str[pos]=='\t') && str[pos]!='\0';pos++);
	
	//copy all char started from first non-spaced-char(part 2 of ltrim)
	ret = str.substr(pos);

	//delete spaced-char at the end (rtrim)
	for(pos=ret.length()-1;ret[pos]=='\t' || ret[pos]==' ';pos--);
	
	ret[++pos]=0x00;
		
	return ret;
}
string clean_string(string str)
{ 
	return trim(delete_strange_characters(str));
}

void print_format_from_number(int fmt)
{
	switch(fmt)
	{
		case 5:
			printf ("\t[FLV]\t"
					"Audio: [h263] Low Quality (MONO)\n\t\t"
					"Video: [FLV]  Low Quality (320x240)\n\n");
			break;
		case 6:
			printf ("\t[FLV]\t"
					"Audio: [???] High Quality (MONO)\n\t\t"
					"Video: [FLV] Low Quality  (320x240)\n\n");
			break;
		case 18:
			printf ("\t[MP4]\t"
					"Audio: [MP4] High Quality (STEREO)\n\t\t"
					"Video: [AVC] Low Quality  (320x240)\n\n");
			break;
		case 22:
			printf ("\t[MP4]\t"
					"Audio: [MP4] High Quality (STEREO)\n\t\t"
					"Video: [AVC] High Quality (HD 720p)\n\n");
			break;
		case 34:
			printf ("\t[FLV]\t"
					"Audio: [AAC]  High Quality (STEREO)\n\t\t"
					"Video: [h264] Low Quality  (320x240)\n\n");
			break;
		case 35:
			printf ("\t[MP4]\t"
					"Audio: [MP4]  High Quality (STEREO)\n\t\t"
					"Video: [h264] Low Quality  (320x240)\n\n");
			break;
	}	
}

int get_best_quality(int *array,int len)
{
	int order[] = {22,18,35,34,6,5};
	for(int i=0;i<sizeof(order);i++)
		for(int k=0;k<len;k++)
			if(order[i]==array[k])
				return k;
	return 0;
}

string get_ext_from_format(int fmt)
{
	switch(fmt)
	{
		case 5:
			return ".flv";
			break;
		case 6:
			return ".flv";
			break;
		case 18:
			return ".mp4";
			break;
		case 22:
			return ".mp4";
			break;
		case 34:
			return ".flv";
			break;
		case 35:
			return ".mp4";
			break;
	}
	
}

string get_only_file_name(string fullpath)
{
	#ifdef WIN32
		int pos = fullpath.rfind("\\");
	#else
		int pos = fullpath.rfind("/");
	#endif
		return fullpath.substr(++pos);	
}

bool file_exists(const char *filename)
{
	FILE *fp = fopen(filename,"r");
	
	if(fp == NULL)
		return false;
	
	return true;	
}
