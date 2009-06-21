/*
    main.cpp is part of YouTube Downloader.

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
#include "common.h"
#include "utils.h"


string buffer,token,title;
bool down_in_hq = false;
bool video_ok = false;
int format_count, *fmt_map,format_choosed;

int get_fmt()
{
	return fmt_map[format_choosed];	
}

void print_logo()
{
    printf("*************************************\n");
    printf("** Youtube Video Downloader v0.01b **\n");
    printf("**        Copyleft(C) 2009 r0b0t82 **\n");
    printf("*************************************\n\n");
}

void print_choose()
{
	log(0,"FormatManager","Available formats:");
	for(int i=0;i<format_count;i++)
	{
		printf("\t%d)",i+1);
		print_format_from_number(fmt_map[i]);
	}
	
	bool ok = true;
	do
	{
		if(ok == false)
			log(0, "FormatManager", "Wrong Format!");
			
		printf("Choose download format: ");
		scanf("%d",&format_choosed);
		format_choosed--;
		
		ok = false;
		if(format_choosed >=0 && format_choosed < format_count)
			ok = true;
	}while(ok == false);
}

void analyze_info(string data)
{
	string key,value;
	int eqpos = data.find("=");
	if(eqpos<0)
		return;
	
	key = data.substr(0,eqpos);
	value = data.substr(eqpos + 1);
	
	key = clean_string(key);
	
	if(key=="status" && value!="ok")
	{
		log(0,"InfoAnalyzer","Url isn't valid");
		bye(7);
	}	
	if(key=="token")
	{
		token = value;
		log(0,"InfoAnalyzer","Found token: %s",token.c_str());
	}
	
	if(key=="title")
	{
		title = clean_string(url_decode(value));
		if(title.length()>30)
			log(0,"InfoAnalyzer","Title: '%s...'",title.substr(0,30).c_str());
		else
			log(0,"InfoAnalyzer","Title: '%s'",title.c_str());
	}

	if(key=="author")
	{
			log(0,"InfoAnalyzer","Author: '%s'",url_decode(value).c_str());
	}

	if(key=="avg_rating")
	{
		log(1,"InfoAnalyzer","Video rating: ");

		for(int i=0;i<atof(value.c_str());i++)
			printf("*");
		printf("\n");
	}
	if(key=="length_seconds")
	{
		log(0,"InfoAnalyzer","Video length: %s seconds",value.c_str());
	}
	
	if(key=="fmt_map")
	{
		value = trim(url_decode(value));
		if(value.length()<1)
		{
			log(0,"InfoAnalyzer","No available formats");
			bye(8);	
		}
		
		format_count = 0;
		int comma_pos = 0;
		int slash_pos = 0;
		int temp_comma = 0;
		string fmt_val;
		do
		{
			slash_pos = value.find("/",comma_pos);
			
			fmt_val = value.substr(comma_pos,(slash_pos-comma_pos));
			format_count++;
			fmt_map = (int *) realloc(fmt_map,format_count * sizeof(int));
			fmt_map[format_count-1] = atoi(fmt_val.c_str());
			log(0,"InfoAnalyzer","Format found: %d",fmt_map[format_count-1]);
			
			comma_pos = value.find(",",comma_pos+1)+1;
		}while(comma_pos-1 != -1);
		
		log(0,"InfoAnalyzer","Found format map");
	}	
}

void get_info(string page)
{
	char *value;
	
	value = strtok ((char *)page.c_str(),"&");
	while(value != NULL)
	{
		analyze_info(value);
		
		value = strtok(NULL,"&");
	}
}

int writer(char *data, size_t size, size_t nmemb, string *buffer)
{  
	int result = 0;

	if (buffer != NULL)
	{  
		buffer->append(data, size * nmemb); 

		result = size * nmemb;
	}
	return result;  
}

int file_writer(char *data, size_t size, size_t nmemb, FILE *fp)
{  
	int result = 0;
	
	if (fp != NULL)
	{  
		fwrite(data,1,size * nmemb,fp);
		result = size * nmemb;
	}
	return result;  
}

string get_info_page(string video_id)
{
	CURL *curl;
	CURLcode result;
	curl = curl_easy_init();
	if(!curl)
	{
		log(0,"curl","Starting failed");
		bye(3);	
	}
	log(0,"VideoInfo","Configuring curl");
	
	string pure_link;
    pure_link = "http://www.youtube.com/get_video_info?video_id=";
    pure_link.append(video_id);

	curl_easy_setopt(curl, CURLOPT_URL, pure_link.c_str());
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
	log(0,"VideoInfo","Getting infos");
	result = curl_easy_perform(curl);
	log(0,"VideoInfo","Decoding infos");
	curl_easy_cleanup(curl);
	string buf;
	buf = buffer;
	buffer.clear();
	return buf;
}
int p_bar (void *clientp, double t, double d, double ultotal, double ulnow)
{
	if(((int)(d*100/t)) < 0)
		log(1,"Download","Downloading video (0%%)\r");
	else
		log(1,"Download","Downloading video (%d%%)\r",(int)(d*100/t));
	return 0;
}

void download_video(string link, string token)
{
	CURL *curl;
	CURLcode result;
	curl = curl_easy_init();
	char format_video[30];
	if(!curl)
	{
		log(0,"curl","Starting failed");
		bye(5);	
	}
	string pure_link;
	pure_link = "http://www.youtube.com/get_video?video_id=";
	pure_link.append(link);
	pure_link.append("&t=");
	pure_link.append(token);
	pure_link.append("&fmt=");
	sprintf(format_video,"%d",get_fmt());
	pure_link.append(format_video);
	printf("Downloading from %s\n",pure_link.c_str());
	log(0,"Download","Opening '%s'",get_file_name(title).c_str());
	FILE *fp = fopen(get_file_name(title).c_str(),"wb");
	if(!fp)
	{
		log(0,"Writer","Cannot open %s",get_file_name(title).c_str());
		bye(6);
	}
	log(0,"Download","Configuring curl");
	curl_easy_setopt(curl, CURLOPT_URL, pure_link.c_str());
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, file_writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, p_bar);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, 0);
	result = curl_easy_perform(curl);	
	printf("\n");
	log(0,"Download","Download completed");
	curl_easy_cleanup(curl);
	fclose(fp);
}

void print_help(char *exename)
{
	#ifdef WIN32
		char *usage = (char *) malloc((strlen(exename) + 300) * sizeof(char));
		sprintf(usage, "Call this program from command prompt with this syntax:\n %s <youtube link>\n",exename);
		MessageBox(0,usage,"Usage",MB_OK);
		free(usage);
	#else
		printf("Usage: %s <youtube link>\n",exename);
	#endif		
}

int main(int argc, char *argv[])
{
	print_logo();
    string url,video_id,info_page;
    if(argc<2)
    {
		print_help(argv[0]);
		bye(1);	
	}
	
    video_id = get_video_id(argv[1]);
    if(video_id.length()==0)
    {
        log(0,"Engine","Url isn't valid");
        bye(2);
    }
    
    info_page = get_info_page(video_id);
	
	log(0,"Engine","Getting info");
	get_info(info_page);
	
	
	if(token.length()==0)
	{
		log(0,"Engine","Url isn't valid");
		bye(4);
	}
	
	print_choose();
	
	download_video(video_id,token);
	bye(0);	
    return 0;
}
