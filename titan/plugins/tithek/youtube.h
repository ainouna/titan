#ifndef YOUTUBE_H
#define YOUTUBE_H

// flag 1 = getstreamurl

char* youtube(char* link, char* url, char* name, int flag)
{
	debug(99, "link(%d): %s", flag, link);
	char* ip = NULL, *pos = NULL, *path = NULL, *streamurl = NULL, *title = NULL, *tmpstr = NULL;
		
	ip = string_replace("http://", "", (char*)link, 0);

	if(ip != NULL)
		pos = strchr(ip, '/');
	if(pos != NULL)
	{
		pos[0] = '\0';
		path = pos + 1;
	}

	tmpstr = gethttp(ip, path, 80, NULL, NULL, NULL, 0);
		
	if(flag == 1)
	{
		tmpstr = string_decode(tmpstr, 0);
		tmpstr = string_decode(tmpstr, 0);
		tmpstr = string_decode(tmpstr, 0);

		if(ostrstr(tmpstr, "status=fail&") == NULL)
		{
			tmpstr = string_resub("&url_encoded_fmt_stream_map=", "endscreen_module", tmpstr, 0);
		
			while(ostrstr(tmpstr, ",url=") != NULL)
				tmpstr = string_replace(",url=", "\n", tmpstr, 1);
				
			struct menulist* mlist = NULL, *mbox = NULL;
			int count = 0;
			int i = 0;
			struct splitstr* ret1 = NULL;
			ret1 = strsplit(tmpstr, "\n", &count);
			if(ret1 != NULL)
			{
				int max = count;
				for(i = 0; i < max; i++)
				{
					printf("(%d) %s\n",i, (ret1[i]).part);
					if(ostrstr(ret1[i].part, "type=video/webm") == NULL)
					{
						streamurl = ostrcat((ret1[i]).part, NULL, 0, 0);
						if(ostrstr(ret1[i].part, "video/x-flv") != NULL)
						{
							ret1[i].part = string_replace("video/x-flv", "video/x-flv\n", ret1[i].part, 0);
							int count2 = 0;
							struct splitstr* ret2 = NULL;
							ret2 = strsplit(ret1[i].part, "\n", &count2);
							if(ret2 != NULL)
							{
								free(streamurl), streamurl = NULL;
								streamurl = ostrcat(ret2[0].part, NULL, 0, 0);
							}
							free(ret2); ret2 = NULL;					
						}
						else if(ostrstr(ret1[i].part, "+") != NULL)
						{
							int count2 = 0;
							struct splitstr* ret2 = NULL;
							ret2 = strsplit(ret1[i].part, "+", &count2);
							if(ret2 != NULL)
							{
								free(streamurl), streamurl = NULL;
								streamurl = ostrcat(ret2[0].part, NULL, 0, 0);
							}
							free(ret2); ret2 = NULL;					
						}
						
						if(ostrstr(ret1[i].part, "itag=85") != NULL)
							title = ostrcat("MP4 520p H.264 3D", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=84") != NULL)
							title = ostrcat("MP4 720p H.264 3D", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=83") != NULL)
							title = ostrcat("MP4 240p H.264 3D", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=82") != NULL)
							title = ostrcat("MP4 360p H.264 3D", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=38") != NULL)
							title = ostrcat("MP4 3072p H.264 High", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=37") != NULL)
							title = ostrcat("MP4 1080p H.264 High", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=22") != NULL)
							title = ostrcat("MP4 720p H.264 High", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=18") != NULL)
							title = ostrcat("MP4 360p H.264 Baseline", NULL, 0, 0);												
						else if(ostrstr(ret1[i].part, "itag=6") != NULL)
							title = ostrcat("FLV 270p Sorenson H.263", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=5") != NULL)
							title = ostrcat("FLV 240p Sorenson H.263", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=35") != NULL)
							title = ostrcat("FLV 480p H.264 Main", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=34") != NULL)
							title = ostrcat("FLV 360p H.264 Main", NULL, 0, 0);																		
						else if(ostrstr(ret1[i].part, "itag=36") != NULL)
							title = ostrcat("3GP 240p MPEG-4 Visual Simple", NULL, 0, 0);
						else if(ostrstr(ret1[i].part, "itag=17") != NULL)
							title = ostrcat("3GP 144p MPEG-4 Visual Simple", NULL, 0, 0);
						
						if(title == NULL)
							title = ostrcat(_("unknown"), NULL, 0, 0);
																																					
						addmenulist(&mlist, title, streamurl, NULL, 0, 0);
						free(title), title = NULL;					
						free(streamurl), streamurl = NULL;
					}
				}
				free(ret1); ret1 = NULL;			
			}
	
			if(mlist != NULL)
			{
				mbox = menulistbox(mlist, NULL, NULL, NULL, NULL, 1, 0);
				if(mbox != NULL)
				{
					free(streamurl), streamurl = NULL;
		
					debug(99, "mbox->name %s", mbox->name);
					debug(99, "mbox->text %s", mbox->text);
					streamurl = ostrcat(mbox->text, NULL, 0, 0);
		
				}
			}
		}
		else
		{	
			tmpstr = string_resub("&reason=", "&errordetail", tmpstr, 1);
			tmpstr = string_replace_all("+", " ", tmpstr, 1);
			tmpstr = string_replace_all(", ", "\n", tmpstr, 1);
			tmpstr = string_replace("wiedergegeben", "\nwiedergegeben ", tmpstr, 1);
			tmpstr = string_replace("<br/><u><a href='", "\n\n", tmpstr, 1);
			tmpstr = string_replace("' target='_blank'>", "\n", tmpstr, 1);
			tmpstr = string_replace("</a></u>", "\n", tmpstr, 1);			
			textbox(_("Message"), tmpstr, _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 1200, 400, 0, 0);
		}
	}

	free(tmpstr); tmpstr = NULL;
	free(ip), ip = NULL;
	free(title), title = NULL;

// segfault munmap_chunk(): invalid pointer
//	free(pos), pos = NULL;
//	free(path), path = NULL;

	debug(99, "streamurl2: %s", streamurl);	
	return streamurl;
}

void youtube_search(struct skin* grid, struct skin* listbox, struct skin* countlabel, struct skin* load, char* link, char* title)
{
	char* search = textinputhist("Search", "NULL", "searchhist");
	if(search != NULL)
	{
		drawscreen(load, 0, 0);
		search = stringreplacechar(search, ' ', '+');
		char* id = NULL;
		char* line = NULL;
		char* pic = NULL;
		char* title = NULL;
		char* menu = NULL;	
		char* ip = ostrcat("gdata.youtube.com", NULL, 0, 0);
		char* path = ostrcat("feeds/api/videos?q=", search, 0, 0);
		if(((struct tithek*)listbox->select->handle)->flag == 9)
			path = ostrcat(path, "&max-results=10", 1, 0);
		else if(((struct tithek*)listbox->select->handle)->flag == 10)
			path = ostrcat(path, "&max-results=25", 1, 0);
		else if(((struct tithek*)listbox->select->handle)->flag == 11)
			path = ostrcat(path, "&max-results=50", 1, 0);
					
		char* tmpstr = gethttp(ip, path, 80, NULL, NULL, NULL, 0);
		tmpstr = string_replace_all("media:thumbnail", "\nthumbnail", tmpstr, 1);

		int count = 0;
		int incount = 0;
		int i = 0;
		struct splitstr* ret1 = NULL;
		ret1 = strsplit(tmpstr, "\n", &count);

		if(ret1 != NULL)
		{
			int max = count;
			for(i = 0; i < max; i++)
			{
				if(ostrstr(ret1[i].part, "http://i.ytimg.com/vi/") != NULL)
				{
					pic = oregex(".*thumbnail url=\'(http://i.ytimg.com/vi/.*/0.jpg).*", ret1[i].part);
					id = oregex(".*thumbnail url=\'http://i.ytimg.com/vi/(.*)/0.jpg.*", ret1[i].part);

					if(id != NULL)
					{
						incount += 1;
						ip = ostrcat("www.youtube.com", NULL, 0, 0);
						path = ostrcat("watch?v=", id, 0, 0);
						title = gethttp(ip, path, 80, NULL, NULL, NULL, 0);
						title = string_resub("<meta name=\"title\" content=\"", "\">", title, 0);

						line = ostrcat(line, title, 1, 0);
//										line = ostrcat(line, "#http://www.youtube.com/watch?v=", 1, 0);
						line = ostrcat(line, "#http://www.youtube.com/get_video_info?&video_id=", 1, 0);
						line = ostrcat(line, id, 1, 0);
						line = ostrcat(line, "#", 1, 0);
						line = ostrcat(line, pic, 1, 0);
						line = ostrcat(line, "#youtube_search_", 1, 0);
						line = ostrcat(line, oitoa(incount + time(NULL)), 1, 0);
						line = ostrcat(line, ".jpg#YouTube - Search#4\n", 1, 0);
						free(ip), ip = NULL;
						free(path), path = NULL;
						free(title), title = NULL;
					}
				}
			}
			free(ret1), ret1 = NULL;

			if(line != NULL)
			{
				menu = ostrcat("/tmp/tithek/youtube.search.list", NULL, 0, 0);
				writesys(menu, line, 0);
				((struct tithek*)listbox->select->handle)->link = menu;
			}
		}
		free(tmpstr), tmpstr = NULL;
	}
	free(search), search = NULL;
}

#endif