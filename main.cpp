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


string buffer,token,title,download_dir;
bool down_in_hq = false, video_ok = false, autoquality = false;
int format_count, format_choosed, *fmt_map;

int get_fmt()
{
	return fmt_map[format_choosed];	
}

void print_logo()
{
    printf("*************************************\n");
    printf("** Youtube Video Downloader v0.03a **\n");
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
	
	int best_format = get_best_quality(fmt_map,format_count);	
	if(format_count > 1)
	{
		log(0,"FormatManager","Best format is:");
		printf("\t%d)",(best_format+1));
		print_format_from_number(fmt_map[best_format]);
	}

	if(autoquality || format_count == 1)
	{
		format_choosed = best_format;
		log(0,"FormatManager","Downloading in");
		print_format_from_number(fmt_map[format_choosed]);
	}
	else
	{
		bool ok = true;
		char temp_choose[30];
		do
		{
			if(ok == false)
				log(0, "FormatManager", "Wrong Format!");
				
			printf("Choose download format: ");
			fgets(temp_choose,25,stdin);
			sscanf(temp_choose,"%d",&format_choosed);
			format_choosed--;
			
			ok = false;
			if(format_choosed >=0 && format_choosed < format_count)
				ok = true;
		}while(ok == false);
	}
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
	
	char *error_buffer = (char *)malloc(CURL_ERROR_SIZE * sizeof(char));
	memset(error_buffer,0x00,sizeof(error_buffer));
	
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
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
	curl_easy_setopt(curl, CURLOPT_URL, pure_link.c_str());
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
	
	log(0,"VideoInfo","Getting infos");
	result = curl_easy_perform(curl);
	if(strlen(error_buffer)>0)
	{
		log(0,"curl","%s",error_buffer);
		free(error_buffer);
		bye(8);	
	}
	log(0,"VideoInfo","Decoding infos");
	curl_easy_cleanup(curl);
	string buf;
	buf = buffer;
	free(error_buffer);
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
	char format_video[30], *error_buffer = (char *)malloc(CURL_ERROR_SIZE * sizeof(char));
	memset(error_buffer,0x00,sizeof(error_buffer));
	if(!curl)
	{
		log(0,"curl","Starting failed");
		bye(5);	
	}
	string pure_link, fname;
	pure_link = "http://www.youtube.com/get_video?video_id=";
	pure_link.append(link);
	pure_link.append("&t=");
	pure_link.append(token);
	pure_link.append("&fmt=");
	sprintf(format_video,"%d",get_fmt());
	pure_link.append(format_video);
	
	fname = get_file_name(download_dir, title,false);

	printf("Downloading from %s\n",pure_link.c_str());
	log(0,"Download","Opening '%s'",fname.c_str());
	
	if(file_exists(fname.c_str()))
	{
		char choose;
		log(1,"Writer",	"%s already exists. Overwrite? [S\\n] ",get_only_file_name(fname).c_str());
		do
		{
			scanf("%c",&choose);
		}while(choose != 'n' && choose != 'N' && choose != 's' && choose != 'S');
		
		if(choose != 's' && choose != 'S')
		{
			log(0,"Writer", "Choosing file name...");
			do
			{
				fname = get_file_name(download_dir,title,true); //random name
			}while(file_exists(fname.c_str()));
			log(0,"Writer", "New filename: %s",get_only_file_name(fname).c_str());
		}
	}
	
	FILE *fp = fopen(fname.c_str(),"wb");
	if(!fp)
	{
		log(0,"Writer","Cannot open %s",fname.c_str());
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
	if(strlen(error_buffer)>0)
	{
		log(0,"curl","%s",error_buffer);
		free(error_buffer);
		bye(8);	
	}
	log(0,"Download","Download completed");
	curl_easy_cleanup(curl);
		free(error_buffer);
	fclose(fp);
}

void print_help(string exename)
{
	string realname = get_only_file_name(exename);
	
	printf(	"Usage: \n %s [<options>] <youtube link>\n"
		"    -a  --auto-quality\t\tDownload at highest quality available\n"
		"    -d  --download-dir <dir>\tDownload file in this directory\n"
		"    -h  --help\t\t\tPrint this help\n"
		"\n"
		,realname.c_str());
}

int main(int argc, char *argv[])
{
	print_logo();
    string url,video_id,info_page;
    
    if(argc<2)
    {
		print_help(argv[0]);
		bye(0);	
	}
	
	int c = 1;
	bool ok = false;
	while(c < argc)
	{
		ok = false;
		if((strcmp(argv[c],"--auto-quality") == 0) || (strcmp(argv[c],"-a") == 0))
		{
			autoquality = true;
			ok = true;
		}
		
		if((strcmp(argv[c],"--download-dir") == 0) || (strcmp(argv[c],"-d") == 0))
		{
			if(++c <= (argc - 1))
				download_dir = argv[c];
			else
			{
				log(0,"Engine","Error: %s option requires an input argument",argv[--c]);
				bye(1);
			}
			ok = true;
		}
		
		if((strcmp(argv[c],"--help") == 0) || (strcmp(argv[c],"-h") == 0))
		{
			print_help(argv[0]);
			bye(0);
			ok = true;
		}
		
		if(ok == false)
		{
			if(c == (argc - 1))
			{
				url.append(argv[c]);
			}
			else
			{
				log(0,"Engine","Error: unrecognized command %s",argv[c]);
				bye(1);
			}
		}
		c++;
	}
	
	if(url.length()==0)
	{
		log(0,"Engine","Youtube link not found");
		bye(2);
	}
	
	video_id = get_video_id(url);
	if(video_id.length()==0)
	{
		log(0,"Engine","Youtube link isn't valid");
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
