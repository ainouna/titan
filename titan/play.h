#ifndef PLAY_H
#define PLAY_H

extern struct screensaver* screensaver;

void screenplaypolicy()
{
	if(checkbit(status.playercan, 0) == 0) return;

	int rcret = 0;
	struct skin* playpolicy = getscreen("playpolicy");
	char* tmpstr = NULL;

start:
	changepolicy();
	tmpstr = getpolicy();
	changetext(playpolicy, _(tmpstr));
	free(tmpstr); tmpstr = NULL;
	drawscreen(playpolicy, 0, 0);

	while(1)
	{
		rcret = waitrc(playpolicy, 1000, 0);
		if(rcret == getrcconfigint("rcok", NULL))
			goto start;
		break;
	}

	clearscreen(playpolicy);
}

//flag = 4 ---> timeshift
void screenplayinfobar(char* file, int mode, int playertype, int flag)
{
	if(checkbit(status.playercan, 14) == 0) return;

	if((flag == 2) || (flag == 3))
	{
		return;
	}
	struct skin* playinfobar = getscreen("playinfobar");
	struct skin* playinfobarpic = getscreen("playinfobarpic");
	if(mode == 1)
	{
		clearscreen(playinfobar);
		clearscreen(playinfobarpic);
		drawscreen(skin, 0, 0);
		return;
	}

	struct skin* title = getscreennode(playinfobar, "title");
	struct skin* spos = getscreennode(playinfobar, "pos");
	struct skin* slen = getscreennode(playinfobar, "len");
	struct skin* sreverse = getscreennode(playinfobar, "reverse");
	struct skin* sprogress = getscreennode(playinfobar, "progress");
	char* tmpstr = NULL;
	unsigned long long pos = 0, len = 0, reverse = 0;

// show thumb cover start
	struct skin* playinfobarcover = getscreen("playinfobarcover");
	struct skin* cover = getscreennode(playinfobarcover, "cover");

	if(file != NULL)
	{
		struct mediadb* node = NULL;
		char* dname = ostrcat(file, NULL, 0, 0);
		dname = dirname(dname);
		node = getmediadb(dname, basename(file), 0);
		free(dname); dname = NULL;
		
		if(node != NULL)
		{
			tmpstr = ostrcat(tmpstr, getconfig("mediadbpath", NULL), 1, 0);
			tmpstr = ostrcat(tmpstr, "/", 1, 0);
			tmpstr = ostrcat(tmpstr, node->id, 1, 0);
			tmpstr = ostrcat(tmpstr, "_poster.jpg", 0, 0);
			if(file_exist(tmpstr))
				changepic(cover, tmpstr);
			free(tmpstr), tmpstr = NULL;
		}
	}
// show thumb cover end

	tmpstr = ostrcat(file, NULL, 0, 0);
	if(tmpstr != NULL) changetext(title, basename(tmpstr));
	free(tmpstr); tmpstr = NULL;

	if(playertype == 1)
	{
		unsigned long long startpos = 0;
		if(flag == 4)
			playergetinfots(&len, &startpos, NULL, &pos, NULL, 1);
		else
			playergetinfots(&len, &startpos, NULL, &pos, NULL, 0);
		len = len / 90000;
		pos = (pos - startpos) / 90000;
	}
	else if(playertype == 2)
	{
		pos = dvdgetpts() / 90000;
		len = dvdgetlength();
	}
	else
	{
		pos = playergetpts() / 90000;
		len = playergetlength();
	}
	if(pos < 0) pos = 0;
	reverse = len - pos;

	if(len == 0)
		sprogress->progresssize = 0;
	else
		sprogress->progresssize = pos * 100 / len;

	tmpstr = convert_timesec(pos);
	changetext(spos, tmpstr);
	free(tmpstr); tmpstr = NULL;

	tmpstr = convert_timesec(len);
	changetext(slen, tmpstr);
	free(tmpstr); tmpstr = NULL;

	tmpstr = convert_timesec(reverse);
	changetext(sreverse, tmpstr);
	free(tmpstr); tmpstr = NULL;

	drawscreen(playinfobar, 0, 0);
	drawscreen(playinfobarpic, 0, 0);
	drawscreen(playinfobarcover, 0, 0);
}

void screenplaytracklist(int mode, int playertype, int flag)
{
	//mode 1 = audiotracks
	//mode 2 = subtitle tracks

	if(mode == 1 && checkbit(status.playercan, 1) == 0) return;
	if(mode == 2 && checkbit(status.playercan, 2) == 0) return;

	if(playertype == 1)
	{
		screenplayinfobar(NULL, 1, playertype, flag);
		if(mode == 1)
			playerchangeaudiotrackts();
		else if(mode == 2)
			playerchangesubtitletrackts();
		blitfb(0);
		return;
	}

	int i = 0;
	int rcret = 0, curtrackid = 0;
	struct skin* track = NULL;
	if(mode == 1)
		track = getscreen("audiotrack");
	else if(mode == 2)
		track = getscreen("subtitle");
	struct skin* listbox = getscreennode(track, "listbox");
	struct skin* tmp = NULL;
	char** tracklist = NULL;
	char* curtrackencoding = NULL, *curtrackname = NULL;
	char* tmpstr = NULL;

	playergetcurtrac(mode, &curtrackid, &curtrackencoding, &curtrackname);
	tracklist = playergettracklist(mode);

	if(tracklist != NULL)
	{
		while(tracklist[i] != NULL)
		{
			tmp = addlistbox(track, listbox, tmp, 1);
			if(tmp != NULL)
			{
				if(ostrcmp(tracklist[i], "und") == 0)
					tmpstr = ostrcat(tmpstr, _("undefined"), 1, 0);
				else
					tmpstr = ostrcat(tmpstr, _(tracklist[i]), 1, 0);
				tmpstr = ostrcat(tmpstr, " (", 1, 0);
				tmpstr = ostrcat(tmpstr, tracklist[i + 1], 1, 0);
				tmpstr = ostrcat(tmpstr, ")", 1, 0);
				changetext(tmp, tmpstr);
				free(tmpstr); tmpstr = NULL;
				tmp->type = CHOICEBOX;
				tmp->del = 1;
				tmp->handle = (char*)(i / 2);

				if(ostrcmp(curtrackname, tracklist[i]) == 0 && ostrcmp(curtrackencoding, tracklist[i + 1]) == 0)
				{
					tmp->handle1 = (char*)(i / 2);
					changeinput(tmp, _("running"));
				}
				else
					changeinput(tmp, "");
			}
			i += 2;
		}
	}

	free(curtrackencoding); curtrackencoding = NULL;
	free(curtrackname); curtrackname = NULL;
	playerfreetracklist(tracklist);
	tracklist = NULL;

	listbox->aktline = 1;
	listbox->aktpage = -1;

	screenplayinfobar(NULL, 1, playertype, flag);
	drawscreen(track, 0, 0);
	addscreenrc(track, listbox);

	while(1)
	{
		rcret = waitrc(track, 0, 0);

		if(rcret == getrcconfigint("rcexit", NULL)) break;
		if(rcret == getrcconfigint("rcok", NULL))
		{
			if(listbox->select != NULL)
			{
				if(mode == 1)
					playerchangeaudiotrack((int)listbox->select->handle);
				else if(mode == 2)
				{
					if(listbox->select->handle1 != NULL)
						playerstopsubtitletrack();
					else	
						playerchangesubtitletrack((int)listbox->select->handle);
				}
			}
			break;
		}
	}

	delmarkedscreennodes(track, 1);
	delownerrc(track);
	clearscreen(track);
	blitfb(0);
}

void playrcyellow(char* file, int playinfobarstatus, int playertype, int flag)
{
//	if(checkbit(status.playercan, 1) == 0) return;

	screenplaytracklist(1, playertype, flag);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrctext(char* file, int playinfobarstatus, int playertype, int flag)
{
//	if(checkbit(status.playercan, 2) == 0) return;

	screenplaytracklist(2, playertype, flag);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrcgreen(char* file, int playinfobarstatus, int playertype, int flag)
{
//	if(checkbit(status.playercan, 3) == 0) return;

	screenplayinfobar(file, 1, playertype, flag);
	if(playertype == 2)
		screenvideomode(2);
	else
		screenvideomode(1);
	drawscreen(skin, 0, 0);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrcblue(char* file, int playinfobarstatus, int playertype, int flag)
{
	if(checkbit(status.playercan, 4) == 0) return;

	screenplayinfobar(file, 1, playertype, flag);
	screenpowerofftimer();
	drawscreen(skin, 0, 0);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrcok(char* file, int playinfobarstatus, int playertype, int flag)
{
//	if(checkbit(status.playercan, 0) == 0) return;

	free(status.playfile); status.playfile = NULL;
	status.playfile = ostrcat(file, NULL, 0, 0);

	screenplaypolicy(file, 1);
	drawscreen(skin, 0, 0);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void id3tag_info(char* file)
{
	char* tmpstr = NULL;

	if(file == NULL) return;
	if(!filelistflt(".mp3 .flac .ogg .wma .ra .wav", file))
	{
		struct id3tag* id3tag = NULL;
		int hash = gethash(file);
		char* tmphash = olutoa(hash);
				
		id3tag = getid3(file, tmphash, 1);
		free(tmphash); tmphash = NULL;
	
		if(id3tag != NULL)
		{
			tmpstr = ostrcat(tmpstr, _("Title:"), 1, 0);
			tmpstr = ostrcat(tmpstr, " ", 1, 0);
			tmpstr = ostrcat(tmpstr, id3tag->title, 1, 0);
			tmpstr = ostrcat(tmpstr, "\n", 1, 0);
			
			tmpstr = ostrcat(tmpstr, _("Artist:"), 1, 0);
			tmpstr = ostrcat(tmpstr, " ", 1, 0);
			tmpstr = ostrcat(tmpstr, id3tag->artist, 1, 0);
			tmpstr = ostrcat(tmpstr, "\n", 1, 0);
			
			tmpstr = ostrcat(tmpstr, _("Album:"), 1, 0);
			tmpstr = ostrcat(tmpstr, " ", 1, 0);
			tmpstr = ostrcat(tmpstr, id3tag->album, 1, 0);
			tmpstr = ostrcat(tmpstr, "\n", 1, 0);
			
			tmpstr = ostrcat(tmpstr, _("Year:"), 1, 0);
			tmpstr = ostrcat(tmpstr, " ", 1, 0);
			tmpstr = ostrcat(tmpstr, id3tag->year, 1, 0);
			tmpstr = ostrcat(tmpstr, "\n", 1, 0);
		
			tmpstr = ostrcat(tmpstr, _("Genre:"), 1, 0);
			tmpstr = ostrcat(tmpstr, " ", 1, 0);
			tmpstr = ostrcat(tmpstr, id3tag->genretext, 1, 0);
			tmpstr = ostrcat(tmpstr, "\n", 1, 0);

			tmpstr = ostrcat(tmpstr, _("Comment:"), 1, 0);
			tmpstr = ostrcat(tmpstr, " ", 1, 0);
			tmpstr = ostrcat(tmpstr, id3tag->comment, 1, 0);
			tmpstr = ostrcat(tmpstr, "\n", 1, 0);
		}
		freeid3(id3tag); id3tag = NULL;		
	}
	else
	{
		tmpstr = ostrcat(tmpstr, _("Title:"), 1, 0);
		tmpstr = ostrcat(tmpstr, " ", 1, 0);
		tmpstr = ostrcat(tmpstr, playergetinfo("Title"), 1, 1);
		tmpstr = ostrcat(tmpstr, "\n", 1, 0);
		
		tmpstr = ostrcat(tmpstr, _("Artist:"), 1, 0);
		tmpstr = ostrcat(tmpstr, " ", 1, 0);
		tmpstr = ostrcat(tmpstr, playergetinfo("Artist"), 1, 1);
		tmpstr = ostrcat(tmpstr, "\n", 1, 0);
		
		tmpstr = ostrcat(tmpstr, _("Album:"), 1, 0);
		tmpstr = ostrcat(tmpstr, " ", 1, 0);
		tmpstr = ostrcat(tmpstr, playergetinfo("Album"), 1, 1);
		tmpstr = ostrcat(tmpstr, "\n", 1, 0);
		
		tmpstr = ostrcat(tmpstr, _("Year:"), 1, 0);
		tmpstr = ostrcat(tmpstr, " ", 1, 0);
		tmpstr = ostrcat(tmpstr, playergetinfo("Year"), 1, 1);
		tmpstr = ostrcat(tmpstr, "\n", 1, 0);
	
		tmpstr = ostrcat(tmpstr, _("Genre:"), 1, 0);
		tmpstr = ostrcat(tmpstr, " ", 1, 0);
		tmpstr = ostrcat(tmpstr, playergetinfo("Genre"), 1, 1);
		tmpstr = ostrcat(tmpstr, "\n", 1, 0);
		
		tmpstr = ostrcat(tmpstr, _("Comment:"), 1, 0);
		tmpstr = ostrcat(tmpstr, " ", 1, 0);
		tmpstr = ostrcat(tmpstr, playergetinfo("Comment"), 1, 1);
		tmpstr = ostrcat(tmpstr, "\n", 1, 0);
		
		tmpstr = ostrcat(tmpstr, _("Track:"), 1, 0);
		tmpstr = ostrcat(tmpstr, " ", 1, 0);
		tmpstr = ostrcat(tmpstr, playergetinfo("Track"), 1, 1);
		tmpstr = ostrcat(tmpstr, "\n", 1, 0);
		
		tmpstr = ostrcat(tmpstr, _("Copyright:"), 1, 0);
		tmpstr = ostrcat(tmpstr, " ", 1, 0);
		tmpstr = ostrcat(tmpstr, playergetinfo("Copyright"), 1, 1);
		tmpstr = ostrcat(tmpstr, "\n", 1, 0);
		
	//	tmpstr = ostrcat(tmpstr, _("TestLibEplayer:"), 1, 0);
	//	tmpstr = ostrcat(tmpstr, " ", 1, 0);
	//	tmpstr = ostrcat(tmpstr, playergetinfo("TestLibEplayer"), 1, 1);
	//	tmpstr = ostrcat(tmpstr, "\n", 1, 0);
	}
			
	textbox(_("iD3Tag"), tmpstr, _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 800, 500, 100, 0);
	free(tmpstr), tmpstr = NULL;
}

void imdb_submenu(char* file, int mode)
{
	struct skin* pluginnode = NULL;
	void (*startplugin)(char*, char*, int, char*, char*);
	
	if(mode == 0)
		pluginnode = getplugin("IMDb");
	else if(mode == 1)
		pluginnode = getplugin("IMDb-API");
	else if(mode == 2)
		pluginnode = getplugin("TMDb");
			
	if(pluginnode != NULL)
	{			
		if(mode == 0)
			startplugin = dlsym(pluginnode->pluginhandle, "screenimdb");
		else if(mode == 1)
			startplugin = dlsym(pluginnode->pluginhandle, "screenimdbapi");
		else if(mode == 2)
			startplugin = dlsym(pluginnode->pluginhandle, "screentmdb");

		if(startplugin != NULL)
		{
				debug(133, "file=%s", basename(file));
			if(file != NULL)
			{
				//create imdb search name

				char* dname = ostrcat(file, NULL, 0, 0);
				dname = dirname(dname);
			
				char* shortname = ostrcat(basename(file), NULL, 0, 0);
				string_tolower(shortname);
//				shortname = string_shortname(shortname, 1);
				shortname = string_shortname(shortname, 2);
				string_removechar(shortname);
				strstrip(shortname);

				debug(133, "inputfile=%s", file);
				debug(133, "shortname=%s", shortname);
				debug(133, "dname=%s", dname);
				debug(133, "file=%s", basename(file));

				startplugin(shortname, NULL, 2, dname, basename(file));

				free(shortname), shortname = NULL;
				free(dname), dname = NULL;
			}				
		}
	}
}

void get_mediadb_scan_info(int files)
{
	int videocount = 0, audiocount = 0, picturecount = 0;
	getmediadbcounts(&videocount, &audiocount, &picturecount);

	char* tmpstr = NULL;
	tmpstr = ostrcat(tmpstr, "scanning (", 1, 0);
	tmpstr = ostrcat(tmpstr, oitoa(videocount), 1, 1);
	tmpstr = ostrcat(tmpstr, "/", 1, 0);
	tmpstr = ostrcat(tmpstr, oitoa(files), 1, 1);
	tmpstr = ostrcat(tmpstr, ")", 1, 0);
	
	tmpstr = ostrcat(tmpstr, "\n\n ", 1, 0);
	tmpstr = ostrcat(tmpstr, _("MediaDB directory scan started in Background !"), 1, 0);
	tmpstr = ostrcat(tmpstr, "\n\n  ", 1, 0);
	tmpstr = ostrcat(tmpstr, _("Delete MediaDB before scan"), 1, 0);
	tmpstr = ostrcat(tmpstr, ": \t", 1, 0);
	if(ostrcmp(getconfig("mediadbscandelall", NULL), "1") == 0)
		tmpstr = ostrcat(tmpstr, _("yes"), 1, 0);
	else
		tmpstr = ostrcat(tmpstr, _("no"), 1, 0);
	tmpstr = ostrcat(tmpstr, "\n  ", 1, 0);			
	tmpstr = ostrcat(tmpstr, _("Delete unused entrys before scan"), 1, 0);
	tmpstr = ostrcat(tmpstr, ": \t", 1, 0);		
	if(ostrcmp(getconfig("mediadbscandelnotfound", NULL), "1") == 0)
		tmpstr = ostrcat(tmpstr, _("yes"), 1, 0);
	else
		tmpstr = ostrcat(tmpstr, _("no"), 1, 0);
	tmpstr = ostrcat(tmpstr, "\n  ", 1, 0);
	tmpstr = ostrcat(tmpstr, _("scan Directory:"), 1, 0);
	tmpstr = ostrcat(tmpstr, " \t\t\t", 1, 0);
	tmpstr = ostrcat(tmpstr, getconfig("mc_vp_path", NULL), 1, 0);
	tmpstr = ostrcat(tmpstr, "\n  ", 1, 0);		
	tmpstr = ostrcat(tmpstr, _("MediaDB place:"), 1, 0);
	tmpstr = ostrcat(tmpstr, " \t\t\t", 1, 0);				
	tmpstr = ostrcat(tmpstr, getconfig("mediadbpath", NULL), 1, 0);

	textbox(_("Message"), tmpstr, _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 1100, 500, 10, 0);
	free(tmpstr), tmpstr = NULL;
}

void mediadb_edit(char* file, int menuid)
{
	int rcret = 0, type = 0, i = 0;
	struct skin* tmcedit = getscreen("mediadbedit");
	struct skin* listbox = getscreennode(tmcedit, "listbox");
	struct skin* title = getscreennode(tmcedit, "title");
	struct skin* year = getscreennode(tmcedit, "year");
	struct skin* released = getscreennode(tmcedit, "released");
	struct skin* runtime = getscreennode(tmcedit, "runtime");
	struct skin* genre = getscreennode(tmcedit, "genre");
	struct skin* director = getscreennode(tmcedit, "director");
	struct skin* writer = getscreennode(tmcedit, "writer");
	struct skin* actors = getscreennode(tmcedit, "actors");
	struct skin* plot = getscreennode(tmcedit, "plot");
	struct skin* rating = getscreennode(tmcedit, "rating");
	struct skin* votes = getscreennode(tmcedit, "votes");
	struct skin* locked = getscreennode(tmcedit, "locked");
	struct skin* picture = getscreennode(tmcedit, "picture");
	struct skin* shortname = getscreennode(tmcedit, "shortname");
	struct skin* fileinfo = getscreennode(tmcedit, "fileinfo");
	struct skin* tmp = NULL;
	struct skin* load = getscreen("loading");
	char* tmpstr = NULL, *bg = NULL, *picret = NULL;
	struct mediadb* node = NULL;
	
	if(file != NULL)
	{
		char* dname = ostrcat(file, NULL, 0, 0);
		dname = dirname(dname);
		node = getmediadb(dname, basename(file), 0);
		free(dname); dname = NULL;
	}

	if(menuid == 3) type = 0; //video
	if(menuid == 4) type = 1; //audio
	if(menuid == 2) type = 2; //picture

	if(node != NULL)
	{
		changeinput(picture, NULL);
		changeinput(title, node->title);

		changemask(year, "0000");
		tmpstr = oitoa(node->year);
		changeinput(year, tmpstr);
		free(tmpstr); tmpstr = NULL;
		if(year->input != NULL)
		{
			int len = strlen(year->input);
			for(i = 0; i < 4 - len; i++)
				year->input = ostrcat("0", year->input, 0, 1);
		}

		changeinput(released, node->released);
		changeinput(runtime, node->runtime);
		changeinput(genre, node->genre);
		changeinput(director, node->director);
		changeinput(writer, node->writer);
		changeinput(actors, node->actors);
		changeinput(plot, node->plot);
		changeinput(shortname, node->shortname);
		changeinput(fileinfo, node->fileinfo);

		tmpstr = oitoa(node->rating);
		changeinput(rating, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10");
		setchoiceboxselection(rating, tmpstr);
		free(tmpstr); tmpstr = NULL;

		changemask(votes, "0000000");
		tmpstr = oitoa(node->votes);
		changeinput(votes, tmpstr);
		free(tmpstr); tmpstr = NULL;
		if(votes->input != NULL)
		{
			int len = strlen(votes->input);
			for(i = 0; i < 7 - len; i++)
				votes->input = ostrcat("0", votes->input, 0, 1);
		}

		addchoicebox(locked, "0", _("unlock -> lock it"));
		addchoicebox(locked, "1", _("unlock -> leaf unlock"));
		addchoicebox(locked, "2", _("locked -> unlock it"));
		addchoicebox(locked, "3", _("locked -> leaf locked"));

		if(checkbit(node->flag, 31) == 1)
			setchoiceboxselection(locked, "3");
		else
			setchoiceboxselection(locked, "0");

		drawscreen(tmcedit, 2, 0);
		bg = savescreen(tmcedit);

		addscreenrc(tmcedit, listbox);
		drawscreen(tmcedit, 0, 0);

		tmp = listbox->select;
		while(1)
		{
			addscreenrc(tmcedit, tmp);
			rcret = waitrcext(tmcedit, 0, 0, 1000);
			delownerrc(tmcedit);
			addscreenrc(tmcedit, listbox);
			tmp = listbox->select;

			if(rcret == getrcconfigint("rcexit", NULL)) break;
			if(rcret == getrcconfigint("rcok", NULL))
			{
				drawscreen(load, 0, 0);
				unsigned long hash = 0;

				if(node->id == NULL || strlen(node->id) == 0)
				{
					hash = gethash(node->file);
					tmpstr = olutoa(hash);
				}
				else
					tmpstr = ostrcat(node->id, NULL, 0, 0);

				if(ostrcmp(locked->ret, "1") == 0 || ostrcmp(locked->ret, "2") == 0)
					node->flag = clearbit(node->flag, 31);
				if(ostrcmp(locked->ret, "0") == 0 || ostrcmp(locked->ret, "3") == 0)
					node->flag = setbit(node->flag, 31);

				if(picret != NULL)
				{
					int channels = 0;
					unsigned long width = 0, height = 0, rowbytes = 0;
					char* thumb = NULL;
					unsigned char* buf = NULL;
					
					buf = loadjpg(picret, &width, &height, &rowbytes, &channels, 16);
					thumb = ostrcat(getconfig("mediadbpath", NULL), "/", 0, 0);
					thumb = ostrcat(thumb, tmpstr, 1, 0);
					thumb = ostrcat(thumb, "_thumb.jpg", 1, 0);
					buf = savejpg(thumb, width, height, channels, 91, 140, 70, buf);
					free(thumb); thumb = NULL;
					free(buf); buf = NULL;
          
					buf = loadjpg(picret, &width, &height, &rowbytes, &channels, 16);
					thumb = ostrcat(getconfig("mediadbpath", NULL), "/", 0, 0);
					thumb = ostrcat(thumb, tmpstr, 1, 0);
					thumb = ostrcat(thumb, "_cover.jpg", 1, 0);
					buf = savejpg(thumb, width, height, channels, 185, 264, 70, buf);
					free(thumb); thumb = NULL;
					free(buf); buf = NULL;

					buf = loadjpg(picret, &width, &height, &rowbytes, &channels, 16);
					thumb = ostrcat(getconfig("mediadbpath", NULL), "/", 0, 0);
					thumb = ostrcat(thumb, tmpstr, 1, 0);
					thumb = ostrcat(thumb, "_poster.jpg", 1, 0);
					buf = savejpg(thumb, width, height, channels, 400, 450, 70, buf);
					free(thumb); thumb = NULL;
					free(buf); buf = NULL;

					buf = loadjpg(picret, &width, &height, &rowbytes, &channels, 16);
					thumb = ostrcat(getconfig("mediadbpath", NULL), "/", 0, 0);
					thumb = ostrcat(thumb, tmpstr, 1, 0);
					thumb = ostrcat(thumb, "_postermid.jpg", 1, 0);
					buf = savejpg(thumb, width, height, channels, 400, 450, 70, buf);
					free(thumb); thumb = NULL;
					free(buf); buf = NULL;
/////////// start mvi
					char* log = NULL, *logdir = NULL, *logfile = NULL, *cmd = NULL, *mvi = NULL;

					logdir = ostrcat(getconfig("mediadbpath", NULL), "/.mediadb_log", 0, 0);
					if(!file_exist(logdir))
						mkdir(logdir, 0777);
					logfile = ostrcat(logdir, "/imdb-scan.log", 1, 0);

					if(getconfigint("mediadb_log", NULL) == 1)
					{
						log = ostrcat(log, "echo \"####################################################################\" >> ", 1, 0);
						log = ostrcat(log, logfile, 1, 0);
						system(log);
						free(log), log = NULL;
						log = ostrcat(log, "echo \"", 1, 0);
						log = ostrcat(log, node->id, 1, 0);
						log = ostrcat(log, "\" >> ", 1, 0);
						log = ostrcat(log, logfile, 1, 0);
						system(log);
						free(log), log = NULL;
						log = ostrcat(log, "echo \"####################################################################\" >> ", 1, 0);
						log = ostrcat(log, logfile, 1, 0);
						system(log);
						free(log), log = NULL;
					}

					mvi = ostrcat(getconfig("mediadbpath", NULL), "/", 0, 0);
					mvi = ostrcat(mvi, node->id, 1, 0);
					mvi = ostrcat(mvi, "_backdrop1.mvi", 1, 0);
					
					cmd = ostrcat(cmd, "ffmpeg -i ", 1, 0);
					cmd = ostrcat(cmd, picret, 1, 0);
					cmd = ostrcat(cmd, " > /tmp/mediadb.meta 2>&1", 1, 0);

					debug(133, "cmd %s", cmd);
					system(cmd);
					free(cmd); cmd = NULL;

					char* size = string_newline(command("cat /tmp/mediadb.meta | grep Stream | awk '{print $6}' | cut -d'x' -f1"));
					debug(133, "size %s", size);
					if(size != NULL)
					{
						debug(133, "size %d", atoi(size));
						int picsize = atoi(size);
	
						if(picsize < 2000)
						{
							debug(133, "size ok %d", picsize);
											
							cmd = ostrcat(cmd, "jpegtran -outfile /tmp/backdrop.resize.jpg -copy none ", 1, 0);
							cmd = ostrcat(cmd, picret, 1, 0);
			
							debug(133, "cmd %s", cmd);
							system(cmd);
							free(cmd); cmd = NULL;
		
							if(file_exist("/tmp/backdrop.resize.jpg"))
							{
								if(getconfigint("mediadb_log", NULL) == 1)
								{
									cmd = ostrcat(cmd, "echo \"#############\" >> ", 1, 0);
									cmd = ostrcat(cmd, logfile, 1, 0);
									system(cmd);
									free(cmd), cmd = NULL;
										
									cmd = ostrcat(cmd, "echo \"", 1, 0);
									cmd = ostrcat(cmd, picret, 1, 0);
									cmd = ostrcat(cmd, " size=(", 1, 0);
									cmd = ostrcat(cmd, size, 1, 0);
									cmd = ostrcat(cmd, ") (lokal file)\" >> ", 1, 0);
									cmd = ostrcat(cmd, logfile, 1, 0);
									system(cmd);
									free(cmd), cmd = NULL;

									cmd = ostrcat(cmd, "echo \"#############\" >> ", 1, 0);
									cmd = ostrcat(cmd, logfile, 1, 0);
									system(cmd);
									free(cmd), cmd = NULL;

									cmd = ostrcat(cmd, "ffmpeg -y -f image2 -i /tmp/backdrop.resize.jpg /tmp/backdrop.resize.mpg >> ", 1, 0);
									cmd = ostrcat(cmd, logfile, 1, 0);
									cmd = ostrcat(cmd, " 2>&1", 1, 0);
								}
								else
								{
									cmd = ostrcat(cmd, "ffmpeg -y -f image2 -i /tmp/backdrop.resize.jpg /tmp/backdrop.resize.mpg > /dev/null 2>&1", 1, 0);
								}

								debug(133, "cmd %s", cmd);
								system(cmd);
								free(cmd); cmd = NULL;
								if(file_exist("/tmp/backdrop.resize.mpg"))
								{					
									cmd = ostrcat(cmd, "mv -f /tmp/backdrop.resize.mpg ", 1, 0);
									cmd = ostrcat(cmd, mvi, 1, 0);
									debug(133, "cmd %s", cmd);
									system(cmd);
									free(cmd); cmd = NULL;
									
									writesysint("/proc/sys/vm/drop_caches", 3, 0);
									free(mvi), mvi = NULL;
									mvi = ostrcat(mvi, "1", 1, 0);
								}
								else
									free(mvi), mvi = NULL;
							}
						}
						else
						{
							debug(133, "ERROR Lokal Cover size to big skipped %d", picsize);
							cmd = ostrcat(cmd, "echo \"#############\" >> ", 1, 0);
							cmd = ostrcat(cmd, logfile, 1, 0);
							system(cmd);
							free(cmd), cmd = NULL;

							cmd = ostrcat(cmd, "echo \"ERROR Lokal Cover size to big skipped: ", 1, 0);
							cmd = ostrcat(cmd, picret, 1, 0);
							cmd = ostrcat(cmd, " size=(", 1, 0);
							cmd = ostrcat(cmd, size, 1, 0);
							cmd = ostrcat(cmd, ")\" >> ", 1, 0);
							cmd = ostrcat(cmd, logfile, 1, 0);
							system(cmd);
							free(cmd), cmd = NULL;

							cmd = ostrcat(cmd, "echo \"#############\" >> ", 1, 0);
							cmd = ostrcat(cmd, logfile, 1, 0);
							system(cmd);
							free(cmd), cmd = NULL;
							free(mvi), mvi = NULL;			
						}
					}
					else
					{
						debug(133, "ERROR size is NULL skipped %s", size);
						cmd = ostrcat(cmd, "echo \"#############\" >> ", 1, 0);
						cmd = ostrcat(cmd, logfile, 1, 0);
						system(cmd);
						free(cmd), cmd = NULL;

						cmd = ostrcat(cmd, "echo \"ERROR Lokal Cover size is NULL skipped: ", 1, 0);
						cmd = ostrcat(cmd, picret, 1, 0);
						cmd = ostrcat(cmd, " size=(", 1, 0);
						cmd = ostrcat(cmd, size, 1, 0);
						cmd = ostrcat(cmd, ")\" >> ", 1, 0);
						cmd = ostrcat(cmd, logfile, 1, 0);
						system(cmd);
						free(cmd), cmd = NULL;

						cmd = ostrcat(cmd, "echo \"#############\" >> ", 1, 0);
						cmd = ostrcat(cmd, logfile, 1, 0);
						system(cmd);
						free(cmd), cmd = NULL;
						free(mvi), mvi = NULL;
					}
					free(size), size = NULL;
					free(log), log = NULL;
					free(logdir), logdir = NULL;
					free(logfile), logfile = NULL;
					free(cmd), cmd = NULL;

					unlink("/tmp/mediadb.meta");
					unlink("/tmp/backdrop.resize.jpg");
					unlink("/tmp/backdrop.resize.mpg");

					node = createmediadb(node, tmpstr, type, title->ret, year->ret, released->ret, runtime->ret, genre->ret, director->ret, writer->ret, actors->ret, plot->ret, mvi, rating->ret, votes->ret, node->path, node->file, node->shortname, node->fileinfo, node->flag);
					free(mvi), mvi = NULL;
/////////// end mvi
				}
				else
					node = createmediadb(node, tmpstr, type, title->ret, year->ret, released->ret, runtime->ret, genre->ret, director->ret, writer->ret, actors->ret, plot->ret, node->poster, rating->ret, votes->ret, node->path, node->file, node->shortname, node->fileinfo, node->flag);

				free(tmpstr); tmpstr = NULL;
				clearscreen(load);
				break;
			}
			if(rcret == getrcconfigint("rcred", NULL))
			{
				free(picret); picret = NULL;
				picret = screendir(getconfig("mediadbpath", NULL), "*.jpg", NULL, NULL, NULL, NULL, 0, "SELECT", 0, NULL, 0, NULL, 0, tmcedit->width, tmcedit->prozwidth, tmcedit->height, tmcedit->prozheight, 0);
				if(picret != NULL)
					changeinput(picture, basename(picret));
				drawscreen(tmcedit, 0, 0);
			}
		}

		free(picret); picret = NULL;
		delownerrc(tmcedit);
		clearscreen(tmcedit);
		restorescreen(bg, tmcedit);
		blitfb(0);
	}
}

void playrcred(char* file, int playinfobarstatus, int playertype, int files, int flag)
{
//	if(checkbit(status.playercan, 5) == 0) return;
	if(status.play == 1)
		screenplayinfobar(file, 1, playertype, flag);

	struct skin* pluginnode = NULL;
	void (*startplugin)(void);
	struct skin* plugin = getscreen("plugin");
	struct skin* child = plugin->child;
	struct menulist* mlist = NULL, *mbox = NULL;
	char* skintitle = "Menu";

	if(status.play == 1)
	{
		addmenulist(&mlist, "Video Settings", NULL, NULL, 0, 0);
		addmenulist(&mlist, "AV Settings", NULL, NULL, 0, 0);
		addmenulist(&mlist, "iD3Tag Info", NULL, NULL, 0, 0);
	}

	addmenulist(&mlist, "MediaDB Edit", NULL, NULL, 0, 0);
		
	if(files > 0)
		addmenulist(&mlist, "MediaDB Scan Info", NULL, NULL, 0, 0);

	//add plugins
	while(child != NULL)
	{
		if(child->del == PLUGINDELMARK && (status.security == 1 || (status.security == 0 && checkpluginskip(child->name) == 0)))
		{
			if(ostrcmp(child->name, "Streaminfo") == 0 && status.play == 1)
				addmenulist(&mlist, child->name, NULL, child->pic, 0, 0);
			else if(ostrcmp(child->name, "IMDb") == 0)
				addmenulist(&mlist, child->name, NULL, child->pic, 0, 0);
			else if(ostrcmp(child->name, "IMDb-API") == 0)
				addmenulist(&mlist, child->name, NULL, child->pic, 0, 0);
			else if(ostrcmp(child->name, "TMDb") == 0)
				addmenulist(&mlist, child->name, NULL, child->pic, 0, 0);
		}
		child = child->next;
	}

	mbox = menulistbox(mlist, NULL, skintitle, NULL, NULL, 1, 0);
	if(mbox != NULL)
	{
		if(ostrcmp(mbox->name, "Video Settings") == 0)
			screenvideosettings();
		else if(ostrcmp(mbox->name, "AV Settings") == 0)
			screenavsettings(0);
		else if(ostrcmp(mbox->name, "IMDb") == 0)
			imdb_submenu(file, 0);
		else if(ostrcmp(mbox->name, "IMDb-API") == 0)
			imdb_submenu(file, 1);
		else if(ostrcmp(mbox->name, "TMDb") == 0)
			imdb_submenu(file, 2);
		else if(ostrcmp(mbox->name, "iD3Tag Info") == 0)
			id3tag_info(file);
		else if(ostrcmp(mbox->name, "MediaDB Scan Info") == 0)
			get_mediadb_scan_info(files);
		else if(ostrcmp(mbox->name, "MediaDB Edit") == 0)
			mediadb_edit(file, 0);
		else
		{
			pluginnode = getplugin(mbox->name);

			if(pluginnode != NULL)
			{
				startplugin = dlsym(pluginnode->pluginhandle, "start");
				if(startplugin != NULL)
					startplugin();
			}
		}
	}

	freemenulist(mlist, 1); mlist = NULL;
	drawscreen(skin, 0, 0);
	resettvpic();
	if(playinfobarstatus > 0 &&	status.play == 1)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrcinfo(char* file, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
	if(checkbit(status.playercan, 14) == 0) return;

	if(*playinfobarstatus == 0)
	{
		*playinfobarstatus = 1;
		*playinfobarcount = 0;
		screenplayinfobar(file, 0, playertype, flag);
	}
	else if(*playinfobarstatus == 1)
	{
		*playinfobarstatus = 0;
		screenplayinfobar(NULL, 1, playertype, flag);
	}
}

void playrcstop(int playertype, int flag)
{
	if(checkbit(status.playercan, 6) == 0) return;

	free(status.playfile); status.playfile = NULL;
	
	if(playertype == 1)
		playerstopts(0, 0);
	else if(playertype == 2)
		dvdstop();
	else
		playerstop();

	writevfd("Player");
	screenplayinfobar(NULL, 1, playertype, flag);
}

void playrcff(char* file, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
	if(checkbit(status.playercan, 7) == 0) return;

	if(status.pause == 0)
	{
		status.playspeed++;
		if(status.playspeed > 6) status.playspeed = 6;
		if(status.playspeed > 0)
		{
			status.play = 0;
			if(playertype == 1)
				playerffts((int)pow(2, status.playspeed));
			else if(playertype == 2)
				dvdff(status.playspeed);
			else	
				playerff(status.playspeed);
			*playinfobarstatus = 2;
			*playinfobarcount = 0;
			screenplayinfobar(file, 0, playertype, flag);
		}
		if(status.playspeed < 0)
		{
			status.play = 0;
			if(playertype == 1)
				playerfrts((int)(pow(2, status.playspeed * -1) * -1), 0);
			else if(playertype == 2)
				dvdfr(status.playspeed);
			else
				playerfr(status.playspeed);
			*playinfobarstatus = 2;
			*playinfobarcount = 0;
			screenplayinfobar(file, 0, playertype, flag);
		}
		if(status.playspeed == 0)
		{
			status.play = 1;
			if(playertype == 1)
			{
				playerpausets();
				playercontinuets();
			}
			else if(playertype == 2)
				dvdcontinue();
			else
				playercontinue();
			*playinfobarstatus = 1;
			*playinfobarcount = 0;
			screenplayinfobar(file, 0, playertype, flag);
		}
	}
}

void playrcfr(char* file, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
	if(checkbit(status.playercan, 8) == 0) return;

	if(status.pause == 0)
	{
		status.playspeed--;
		if(status.playspeed < -6) status.playspeed = -6;
		if(status.playspeed > 0)
		{
			status.play = 0;
			if(playertype == 1)
				playerffts((int)pow(2, status.playspeed));
			else if(playertype == 2)
				dvdff(status.playspeed);
			else
				playerff(status.playspeed);
			*playinfobarstatus = 2;
			*playinfobarcount = 0;
			screenplayinfobar(file, 0, playertype, flag);
		}
		if(status.playspeed < 0)
		{
			status.play = 0;
			if(playertype == 1)
				playerfrts((int)(pow(2, status.playspeed * -1) * -1), 0);
			else if(playertype == 2)
				dvdfr(status.playspeed);
			else
				playerfr(status.playspeed);
			*playinfobarstatus = 2;
			*playinfobarcount = 0;
			screenplayinfobar(file, 0, playertype, flag);
		}
		if(status.playspeed == 0)
		{
			status.play = 1;
			if(playertype == 1)
				playercontinuets();
			else if(playertype == 2)
				dvdcontinue();
			else
				playercontinue();
			*playinfobarstatus = 1;
			*playinfobarcount = 0;
			screenplayinfobar(file, 0, playertype, flag);
		}
	}
}

void playrcpause(char* file, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
	if(checkbit(status.playercan, 9) == 0) return;

	if(status.pause == 1)
	{
		status.playspeed = 0;
		status.play = 1;
		status.pause = 0;
		if(playertype == 1)
			playercontinuets();
		else if(playertype == 2)
			dvdcontinue();
		else
			playercontinue();
		*playinfobarstatus = 1;
		*playinfobarcount = 0;
		screenplayinfobar(file, 0, playertype, flag);
	}
	else
	{
		status.playspeed = 0;
		status.play = 0;
		status.pause = 1;
		if(playertype == 1)
			playerpausets();
		else if(playertype == 2)
			dvdpause();
		else
			playerpause();
		*playinfobarstatus = 2;
		*playinfobarcount = 0;
		screenplayinfobar(file, 0, playertype, flag);
	}
}

void playrcplay(char* file, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
	if(checkbit(status.playercan, 10) == 0) return;

	free(status.playfile); status.playfile = NULL;
	status.playfile = ostrcat(file, NULL, 0, 0);

	if(playertype == 1)
	{
		playerpausets();
		playercontinuets();
	}
	else if(playertype == 2)
		dvdcontinue();
	else
		playercontinue();
	status.playspeed = 0;
	status.pause = 0;
	status.play = 1;
	*playinfobarstatus = 1;
	*playinfobarcount = 0;
	screenplayinfobar(file, 0, playertype, flag);
}

void playrcjumpr(char* file, int sec, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
//	if(checkbit(status.playercan, 11) == 0) return;

	unsigned long long pos = 0;
	
	if(status.pause == 0 && status.playspeed == 0)
	{
		//a jump over the beginning of the
		//file, freez the player
		if(playertype == 1)
		{
			unsigned long long startpos = 0;
			playergetinfots(NULL, &startpos, NULL, &pos, NULL, 0);
			pos = (pos - startpos) / 90000;
		}
		else if(playertype == 2)
			pos = dvdgetpts() / 90000;
		else
			pos = playergetpts() / 90000;
	
		if(pos + 10 > sec)
		{
			if(playertype == 1)
				playerseekts(getservice(RECORDPLAY, 0), sec * -1, 0);
			else if(playertype == 2)
				dvdseek(sec * -1);
			else
				playerseek(sec * -1);
		}
		else
		{
			if(playertype == 1)
			{
				playerstopts(2, 0);
				playerstartts(file, 2);
			}
			else if(playertype == 2)
			{
				dvdstop();
				dvdstart(file);
			}
			else
			{
				playerstop();
				playerstart(file);
			}
		}

		*playinfobarstatus = 1;
		*playinfobarcount = 0;
		status.play = 0;		
		screenplayinfobar(file, 0, playertype, flag);
		status.play = 1;
		sleep(1);
	}
}

void playrcjumpf(char* file, int sec, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
//	if(checkbit(status.playercan, 12) == 0) return;

	if(status.pause == 0 && status.playspeed == 0)
	{
		if(playertype == 1)
			playerseekts(getservice(RECORDPLAY, 0), sec, 0);
		else if(playertype == 2)
			dvdseek(sec);
		else
			playerseek(sec);
		*playinfobarstatus = 1;
		*playinfobarcount = 0;
		status.play = 0;		
		screenplayinfobar(file, 0, playertype, flag);
		status.play = 1;
	}
}

void playchangecodec(int playertype)
{
	if(checkbit(status.playercan, 13) == 0) return;

	char** tracklist = NULL;

	if(getconfigint("av_ac3default", NULL) == 1)
	{
		int i = 0;
		
		tracklist = playergettracklist(1);
		if(tracklist != NULL)
		{
			while(tracklist[i] != NULL)
			{
				if(ostrcmp(tracklist[i + 1], "A_AC3") == 0)
				{
					playerchangeaudiotrack(i / 2);
				}
				i += 2;
			}
		}
		playerfreetracklist(tracklist);
		tracklist = NULL;
	}
}

int playcheckdirrcret(char* file, int dirrcret)
{
	int ret = 0;
	char* epgfilename = NULL, *tmpstr = NULL;

	if(dirrcret == 4)
	{
		int sort = screendirsort();
		addconfigint("dirstort", sort);
		ret = 1;
	}
	if(dirrcret == 3)
	{
		epgfilename = changefilenameext(file, ".epg");

		tmpstr = readfiletomem(epgfilename, 0);
		if(tmpstr != NULL)
			textbox(_("EPG Info"), tmpstr, _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 700, 600, 0, 2);
		else {
			free(epgfilename); epgfilename = NULL;
			epgfilename = changefilenameext(file, ".eit");
			tmpstr = readeittomem(epgfilename);
			if(tmpstr != NULL)
				textbox(_("EPG Info"), tmpstr, _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 700, 600, 0, 2);
		}
		free(epgfilename); epgfilename = NULL;
		free(tmpstr); tmpstr = NULL;
		ret = 1;
	}
	if(dirrcret == 1)
	{
		if(textbox(_("Realy Delete ?"), file, _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 600, 200, 0, 0) == 1)
		{
			unlink(file);
			epgfilename = changefilenameext(file, ".epg");
			unlink(epgfilename);
			free(epgfilename); epgfilename = NULL;
			epgfilename = changefilenameext(file, ".se");
			unlink(epgfilename);
			free(epgfilename); epgfilename = NULL;
		}
		ret = 1;
	}
	
	return ret;
}

void playwritevfd(char* file)
{
	char* tmpstr = NULL;

	tmpstr = ostrcat(file, NULL, 0, 0);
	if(tmpstr != NULL) writevfd(basename(tmpstr));
	free(tmpstr); tmpstr = NULL;
}


void playstartservice()
{
	char* tmpstr = NULL;

	if(status.aktservice->channel != NULL)
	{
		tmpstr = ostrcat(status.aktservice->channellist, NULL, 0, 0);
		servicecheckret(servicestart(status.aktservice->channel, tmpstr, NULL, 3), 0);
	}
	else
	{
		tmpstr = ostrcat(status.lastservice->channellist, NULL, 0, 0);
		servicecheckret(servicestart(status.lastservice->channel, tmpstr, NULL, 0), 0);
	}
	free(tmpstr); tmpstr = NULL;
}

// flag 0 = dirlist/playing/infobar
// flag 1 = playing/infobar
// flag 2 = playing
// flag 3 = not stop/start live service
// flag 4 = playing with screensaver
// startfolder 2 = do nothing with playstop/playstart
int screenplay(char* startfile, int startfolder, int flag)
{
	int rcret = 0, playertype = 0, ret = 0, rcwait = 1000, screensaver_delay = 0;
	char* file = NULL, *tmpstr = NULL;
	char* tmppolicy = NULL, *startdir = NULL;
	char* formats = NULL;
	struct skin* playinfobar = getscreen("playinfobar");
	struct skin* load = getscreen("loading");

	int oldsort = getconfigint("dirsort", NULL);
	int skip13 = getconfigint("skip13", NULL);
	int skip46 = getconfigint("skip46", NULL);
	int skip79 = getconfigint("skip79", NULL);
	
	if(startfolder == 0 && flag != 3)
	{
		rcret = servicestop(status.aktservice, 1, 1);
		if(rcret == 1) return ret;
	}

	if(status.webplayfile != NULL)
	{
		startfile = status.webplayfile;
		rcret = servicestop(status.aktservice, 1, 1);
		if(rcret == 1) return ret;
	}

	// allowed from atemio avi mkv mpg4 xvid mpg1 mpg2 jpeg png
	if(status.expertmodus > 0 && status.security == 1)
		formats = ostrcat(formats, ".flac .ogg .mp3 .avi .dat .divx .flv .mkv .m4v .mp4 .mov .mpg .mpeg .mts .m2ts .trp .ts .vdr .vob .wmv .rm", 1, 0);
	else
		formats = ostrcat(formats, ".ts .mts .m2ts", 1, 0);
	
	status.updatevfd = PAUSE;
	tmppolicy = getpolicy();

playerstart:
	if(startfolder == 0)
		startdir = getconfig("rec_moviepath", NULL);
	else
		startdir = getconfig("rec_path", NULL);

	status.playspeed = 0, status.play = 0, status.pause = 0;
	int playinfobarcount = 0, playinfobarstatus = 1, dirrcret = 0;

	if(startfile == NULL)
	{
		tmpstr = ostrcat(file, NULL, 1, 0); file = NULL;
		file = screendir(startdir, formats, basename(tmpstr), &dirrcret, ".epg", _("DEL"), getrcconfigint("rcred", NULL), _("SELECT"), 0, "EPG", getrcconfigint("rcyellow", NULL), "SORT", getrcconfigint("rcblue", NULL), 90, 1, 90, 1, 64);
		free(tmpstr); tmpstr = NULL;
	}
	else
		file = ostrcat(startfile, NULL, 0, 0);

	if(file == NULL)
	{
		if(playcheckdirrcret(file, dirrcret) == 1)
			goto playerstart;
	}

	if(file != NULL)
	{
		if(startfile == NULL)
		{
			if(getconfigint("playertype", NULL) == 1 && (cmpfilenameext(file, ".ts") == 0 || cmpfilenameext(file, ".mts") == 0 || cmpfilenameext(file, ".m2ts") == 0))
				playertype = 1;

			tmpstr = ostrcat(file, NULL, 0, 0);
			if(tmpstr != NULL && startfolder == 0) addconfig("rec_moviepath", dirname(tmpstr));
			free(tmpstr); tmpstr = NULL;
		
			if(playcheckdirrcret(file, dirrcret) == 1)
				goto playerstart;

			if(startfolder == 1 && flag != 3)
			{
				rcret = servicestop(status.aktservice, 1, 1);
				if(rcret == 1)
				{
					free(tmppolicy);
					free(file);
					free(formats);
					addconfigint("dirstort", oldsort);
					return ret;
				}
			}
		}

		drawscreen(skin, 0, 0);
		drawscreen(load, 0, 0);
		playwritevfd(file);
		if(playertype == 1)
			rcret = playerstartts(file, 0);
		else
			rcret = playerstart(file);
#ifndef SIMULATE
		if(rcret != 0)
		{
			textbox(_("Message"), _("Can't start playback !"), _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 600, 200, 7, 0);
			writevfd("Player");
			
			if(startfile == NULL)
			{
				if(startfolder == 1 && flag != 3) playstartservice();
				goto playerstart;
			}
			else
			{
				ret = 2;
				goto playerend;
			}
		}
#endif
		clearscreen(load);
		screenplayinfobar(file, 0, playertype, flag);

		if(flag == 4 && getconfigint("screensaver", NULL) == 1)
		{
			screensaver_delay = getconfigint("screensaver_delay", NULL);
			initscreensaver();
		}								
		
		//change codec if ac3default and video has ac3
		//deaktivate, freeze player, makes a seek -5
		//see eplayer container_ffmpeg_switch_audio
		//the seek to the beginning of the file freez
		//eplayer.
		//playchangecodec();
		free(status.playfile); status.playfile = NULL;
		status.playfile = ostrcat(file, NULL, 0, 0);
		status.play = 1;
		int count = 0;
		while(1)
		{
			while((playertype == 0 && playerisplaying()) || (playertype == 1 && playerisplayingts()))
			{
				rcret = waitrc(playinfobar, rcwait, 0);
				playinfobarcount++;
				if(playinfobarstatus > 0)
					screenplayinfobar(file, 0, playertype, flag);
				if(playinfobarstatus == 1 && playinfobarcount >= getconfigint("infobartimeout", NULL))
				{
					playinfobarstatus = 0;
					screenplayinfobar(NULL, 1, playertype, flag);
				}
				
				if(flag == 4)
				{
					if(status.play == 1 && screensaver != NULL)
						count++;
	
					if(count > screensaver_delay && screensaver != NULL)
					{
						showscreensaver();
						rcwait = screensaver->speed;
					}
				}
		
				if(rcret == getrcconfigint("rcyellow", NULL))
					playrcyellow(file, playinfobarstatus, playertype, flag);
				
				if(rcret == getrcconfigint("rctext", NULL) || rcret == getrcconfigint("rcsubtitel", NULL))
					playrctext(file, playinfobarstatus, playertype, flag);
					
				if(rcret == getrcconfigint("rcgreen", NULL))
					playrcgreen(file, playinfobarstatus, playertype, flag);
					
				if(rcret == getrcconfigint("rcblue", NULL))
					playrcblue(file, playinfobarstatus, playertype, flag);
					
				if(rcret == getrcconfigint("rcok", NULL))
					playrcok(file, playinfobarstatus, playertype, flag);
				
				if(rcret == getrcconfigint("rcred", NULL))
					playrcred(file, playinfobarstatus, playertype, 0, flag);

				if(rcret == getrcconfigint("rcinfo", NULL))
					playrcinfo(file, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rcstop", NULL) || rcret == getrcconfigint("rcexit", NULL))
				{
					playrcstop(playertype, flag);
					if(startfile == NULL)
					{						
						if(startfolder == 1 && flag != 3) playstartservice();
						goto playerstart;
					}
					else
					{
						ret = 1;
						goto playerend;
					}
				}

				if(rcret == getrcconfigint("rcff", NULL))
					playrcff(file, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rcfr", NULL))
					playrcfr(file, &playinfobarstatus, &playinfobarcount, playertype, flag);

				if(rcret == getrcconfigint("rcpause", NULL))
					playrcpause(file, &playinfobarstatus, &playinfobarcount, playertype, flag);

				if(rcret == getrcconfigint("rcplay", NULL))
					playrcplay(file, &playinfobarstatus, &playinfobarcount, playertype, flag);

				if(rcret == getrcconfigint("rcleft", NULL))
					playrcjumpr(file, 60, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rc1", NULL))
					playrcjumpr(file, skip13, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rc4", NULL))
					playrcjumpr(file, skip46, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rc7", NULL))
					playrcjumpr(file, skip79, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rcright", NULL))
					playrcjumpf(file, 60, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rc3", NULL))
					playrcjumpf(file, skip13, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rc6", NULL))
					playrcjumpf(file, skip46, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rc9", NULL))
					playrcjumpf(file, skip79, &playinfobarstatus, &playinfobarcount, playertype, flag);

				if(rcret == getrcconfigint("rcdown", NULL))
					playrcjumpr(file, 300, &playinfobarstatus, &playinfobarcount, playertype, flag);

				if(rcret == getrcconfigint("rcup", NULL))
					playrcjumpf(file, 300, &playinfobarstatus, &playinfobarcount, playertype, flag);

			}
			//don't change this sleep, without this
			//the player stops to fast, and a last seek can
			//produce a segfault
playerend:
			sleep(1);
			if(playertype == 1)
				playerafterendts();
			else
				playerafterend();

			writevfd("Player");
			screenplayinfobar(file, 1, playertype, flag);

			if(startfile == NULL)
			{
				if(startfolder == 1 && flag != 3) playstartservice();
				goto playerstart;
			}
			else
				break;
		}
	}
	if(startfolder == 0 && flag != 3) playstartservice();
	status.updatevfd = START;
	
	if(status.webplayfile != NULL)
	{
		playstartservice();
		free(status.webplayfile); status.webplayfile = NULL;
	}

	if(tmppolicy != NULL)
	{
		setpolicy(tmppolicy);
		free(tmppolicy);
	}
	
	if(flag == 4)
		deinitscreensaver();

	addconfigint("dirstort", oldsort);
	free(status.playfile); status.playfile = NULL; 
	status.playspeed = 0;
	status.pause = 0;
	status.play = 0;
	free(file);
	free(formats);

	return ret;
}

#endif
