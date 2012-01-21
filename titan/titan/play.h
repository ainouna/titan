#ifndef PLAY_H
#define PLAY_H

void screenplaypolicy()
{
	int rcret = 0;
	struct skin* playpolicy = getscreen("playpolicy");
	char* tmpstr = NULL;

start:
	changepolicy();
	tmpstr = getpolicy();
	changetext(playpolicy, _(tmpstr));
	free(tmpstr); tmpstr = NULL;
	drawscreen(playpolicy, 0);

	while(1)
	{
		rcret = waitrc(playpolicy, 1000, 0);
		if(rcret == getrcconfigint("rcok", NULL))
			goto start;
		break;
	}

	clearscreen(playpolicy);
}

void screenplayinfobar(char* file, int mode, int playertype, int flag)
{
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
		drawscreen(skin, 0);
		return;
	}

	struct skin* title = getscreennode(playinfobar, "title");
	struct skin* spos = getscreennode(playinfobar, "pos");
	struct skin* slen = getscreennode(playinfobar, "len");
	struct skin* sreverse = getscreennode(playinfobar, "reverse");
	struct skin* sprogress = getscreennode(playinfobar, "progress");
	char* tmpstr = NULL;
	unsigned long long int pos = 0, len = 0, reverse = 0;

	tmpstr = ostrcat(file, "", 0, 0);
	changetext(title, basename(tmpstr));
	free(tmpstr); tmpstr = NULL;

	if(playertype == 1)
	{
		pos = playergetptsts() / 90000;
		len = playergetlengthts();
	}
	else
	{
		pos = playergetpts() / 90000;
		len = playergetlength();
	}
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

	drawscreen(playinfobar, 0);
	drawscreen(playinfobarpic, 0);
}

void screenplaytracklist(int mode, int playertype, int flag)
{
	//mode 1 = audiotracks
	//mode 2 = subtitle tracks
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

	if(playertype == 1)
	{
		playergetcurtracts();
		tracklist = playergettracklistts();
	}
	else
	{
		playergetcurtrac(mode, &curtrackid, &curtrackencoding, &curtrackname);
		tracklist = playergettracklist(mode);
	}
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
	if(playertype == 1)
		playerfreetracklistts();
	else
		playerfreetracklist(tracklist);
	tracklist = NULL;

	listbox->aktline = 1;
	listbox->aktpage = -1;

	screenplayinfobar(NULL, 1, playertype, flag);
	drawscreen(track, 0);
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
				{
					if(playertype == 1)
						playerchangeaudiotrackts();
					else
						playerchangeaudiotrack((int)listbox->select->handle);
				}
				else if(mode == 2)
				{
					if(listbox->select->handle1 != NULL)
					{
						if(playertype == 1)
							playerchangesubtitletrackts();
						else
							playerchangesubtitletrack(-1);
					}
					else	
					{
						if(playertype == 1)
							playerchangesubtitletrackts();
						else
							playerchangesubtitletrack((int)listbox->select->handle);
					}
				}
			}
			break;
		}
	}

	delmarkedscreennodes(track, 1);
	delownerrc(track);
	clearscreen(track);
	blitfb();
}

void playrcyellow(char* file, int playinfobarstatus, int playertype, int flag)
{
	screenplaytracklist(1, playertype, flag);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrctext(char* file, int playinfobarstatus, int playertype, int flag)
{
	screenplaytracklist(2, playertype, flag);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrcgreen(char* file, int playinfobarstatus, int playertype, int flag)
{
	screenplayinfobar(file, 1, playertype, flag);
	screenvideomode();
	drawscreen(skin, 0);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrcblue(char* file, int playinfobarstatus, int playertype, int flag)
{
	screenplayinfobar(file, 1, playertype, flag);
	screenpowerofftimer();
	drawscreen(skin, 0);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrcok(char* file, int playinfobarstatus, int playertype, int flag)
{
	screenplaypolicy(file, 1);
	drawscreen(skin, 0);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrcred(char* file, int playinfobarstatus, int playertype, int flag)
{
	screenplayinfobar(file, 1, playertype, flag);
	screenvideosettings();
	drawscreen(skin, 0);
	if(playinfobarstatus > 0)
		screenplayinfobar(file, 0, playertype, flag);
}

void playrcinfo(char* file, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
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
	if(playertype == 1)
		playerstopts(0, 0);
	else
		playerstop();

	writevfd("Player");
	screenplayinfobar(NULL, 1, playertype, flag);
}

void playrcff(char* file, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
	if(status.pause == 0)
	{
		status.playspeed++;
		if(status.playspeed > 6) status.playspeed = 6;
		if(status.playspeed > 0)
		{
			status.play = 0;
			if(playertype == 1)
				playerffts(status.playspeed);
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
				playerfrts(status.playspeed);
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
	if(status.pause == 0)
	{
		status.playspeed--;
		if(status.playspeed < -6) status.playspeed = -6;
		if(status.playspeed > 0)
		{
			status.play = 0;
			if(playertype == 1)
				playerffts(status.playspeed);
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
				playerfrts(status.playspeed);
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
	if(status.pause == 1)
	{
		status.playspeed = 0;
		status.play = 1;
		status.pause = 0;
		if(playertype == 1)
			playercontinuets();
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
		else
			playerpause();
		*playinfobarstatus = 2;
		*playinfobarcount = 0;
		screenplayinfobar(file, 0, playertype, flag);
	}
}

void playrcplay(char* file, int* playinfobarstatus, int* playinfobarcount, int playertype, int flag)
{
	if(playertype == 1)
		playercontinuets();
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
	unsigned long long int pos = 0;
	
	if(status.pause == 0 && status.playspeed == 0)
	{
		//a jump over the beginning of the
		//file, freez the player
		if(playertype == 1)
			pos = playergetptsts() / 90000;
		else
			pos = playergetpts() / 90000;
	
		if(pos + 10 > sec)
		{
			if(playertype == 1)
				playerseekts(getservice(RECORDPLAY, 0), sec * -1);
			else
				playerseek(sec * -1);
		}
		else
		{
			if(playertype == 1)
			{
				playerstopts(0, 0);
				playerstartts(file, 0);
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
	if(status.pause == 0 && status.playspeed == 0)
	{
		if(playertype == 1)
			playerseekts(getservice(RECORDPLAY, 0), sec);
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
	char** tracklist = NULL;

	if(getconfigint("av_ac3default", NULL) == 1)
	{
		int i = 0;
		if(playertype == 1)
			tracklist = playergettracklistts();
		else
			tracklist = playergettracklist(1);
		if(tracklist != NULL)
		{
			while(tracklist[i] != NULL)
			{
				if(ostrcmp(tracklist[i + 1], "A_AC3") == 0)
				{
					if(playertype == 1)
						playerchangeaudiotrackts();
					else
						playerchangeaudiotrack(i / 2);
				}
				i += 2;
			}
		}
		if(playertype == 1)
			playerfreetracklistts();
		else
			playerfreetracklist(tracklist);
		tracklist = NULL;
	}
}

int playcheckdirrcret(char* file, int dirrcret)
{
	int ret = 0;
	char* epgfilename = NULL, *tmpstr = NULL;

	if(dirrcret == 3)
	{
		epgfilename = changefilenameext(file, ".epg");

		tmpstr = readfiletomem(epgfilename, 0);
		if(tmpstr != NULL)
			textbox(_("EPG Info"), tmpstr, _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 700, 600, 0, 2);

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
		}
		ret = 1;
	}
	
	return ret;
}

void playwritevfd(char* file)
{
	char* tmpstr = NULL;

	tmpstr = ostrcat(file, "", 0, 0);
	if(tmpstr != NULL) writevfd(basename(tmpstr));
	free(tmpstr); tmpstr = NULL;
}

// flag 0 = dirlist/playing/infobar
// flag 1 = playing/infobar
// flag 2 = playing
void screenplay(int startfolder, int flag)
{
	int rcret = 0, playertype = 0;
	char* file = NULL, *tmpstr = NULL;
	char* tmppolicy = NULL, *startdir = NULL;
	char* formats = NULL;
	struct skin* playinfobar = getscreen("playinfobar");
	
	// allowed from atemio avi mkv mpg4 xvid mpg1 mpg2 jpeg png
	if(status.expertmodus > 0)
		formats = ostrcat(formats, "*.flac *.ogg *.mp3 *.avi *.dat *.divx *.flv *.mkv *.m4v *.mp4 *.mov *.mpg *.mpeg *.mts *.m2ts *.trp *.ts *.vdr *.vob *.wmv *.rm", 1, 0);
	else
		formats = ostrcat(formats, "*.ts", 1, 0);

	int skip13 = getconfigint("skip13", NULL);
	int skip46 = getconfigint("skip46", NULL);
	int skip79 = getconfigint("skip79", NULL);
	
	if(startfolder == 0)
	{
		rcret = servicestop(status.aktservice, 1, 1);
		if(rcret == 1) return;
	}
	status.updatevfd = PAUSE;
	tmppolicy = getpolicy();

playerstart:
	if(startfolder == 0)
		startdir = getconfig("rec_moviepath", NULL);
	else
		startdir = getconfig("rec_path", NULL);

	status.playspeed = 0, status.play = 0, status.pause = 0;
	int playinfobarcount = 0, playinfobarstatus = 1, dirrcret = 0;

	tmpstr = ostrcat(file, "", 1, 0); file = NULL;
	file = screendir(startdir, formats, basename(tmpstr), &dirrcret, ".epg", _("DEL"), getrcconfigint("rcred", NULL), _("SELECT"), 0, "EPG", getrcconfigint("rcyellow", NULL), NULL, 0, 90, 1, 90, 1, 0);
	free(tmpstr); tmpstr = NULL;

	if(file != NULL)
	{
		if(getconfigint("playertype", NULL) == 1)
			if(cmpfilenameext(file, ".ts") == 0) playertype = 1;
		tmpstr = ostrcat(file, "", 0, 0);
		if(tmpstr != NULL && startfolder == 0) addconfig("rec_moviepath", dirname(tmpstr));
		free(tmpstr); tmpstr = NULL;
		
		if(playcheckdirrcret(file, dirrcret) == 1)
			goto playerstart;

		if(startfolder == 1)
		{
			rcret = servicestop(status.aktservice, 1, 1);
			if(rcret == 1) return;
		}

		drawscreen(skin, 0);
		playwritevfd(file);
		if(playertype == 1)
			rcret = playerstartts(file, 0);
		else
			rcret = playerstart(file);
#ifndef SIMULATE
		if(rcret != 0)
		{
			textbox(_("Message"), _("Can't start playback !"), _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 600, 200, 0, 0);
			writevfd("Player");
			goto playerstart;
		}
#endif
		screenplayinfobar(file, 0, playertype, flag);

		//change codec if ac3default and video has ac3
		//deaktivate, freeze player, makes a seek -5
		//see eplayer container_ffmpeg_switch_audio
		//the seek to the beginning of the file freez
		//eplayer.
		//playchangecodec();

		status.play = 1;
		while(1)
		{
			while((playertype == 0 && playerisplaying()) || (playertype == 1 && playerisplayingts()))
			{
				rcret = waitrc(playinfobar, 1000, 0);
				playinfobarcount++;
				if(playinfobarstatus > 0)
					screenplayinfobar(file, 0, playertype, flag);
				if(playinfobarstatus == 1 && playinfobarcount >= getconfigint("infobartimeout", NULL))
				{
					playinfobarstatus = 0;
					screenplayinfobar(NULL, 1, playertype, flag);
				}

				if(rcret == getrcconfigint("rcyellow", NULL))
					playrcyellow(file, playinfobarstatus, playertype, flag);
				
				if(rcret == getrcconfigint("rctext", NULL))
					playrctext(file, playinfobarstatus, playertype, flag);
					
				if(rcret == getrcconfigint("rcgreen", NULL))
					playrcgreen(file, playinfobarstatus, playertype, flag);
					
				if(rcret == getrcconfigint("rcblue", NULL))
					playrcblue(file, playinfobarstatus, playertype, flag);
					
				if(rcret == getrcconfigint("rcok", NULL))
					playrcok(file, playinfobarstatus, playertype, flag);
				
				if(rcret == getrcconfigint("rcred", NULL))
					playrcred(file, playinfobarstatus, playertype, flag);

				if(rcret == getrcconfigint("rcinfo", NULL))
					playrcinfo(file, &playinfobarstatus, &playinfobarcount, playertype, flag);
				
				if(rcret == getrcconfigint("rcstop", NULL))
				{
					playrcstop(playertype, flag);
					if(startfolder == 1)
					{
						tmpstr = ostrcat(status.lastservice->channellist, NULL, 0, 0);
						servicecheckret(servicestart(status.lastservice->channel, tmpstr, NULL, 0), 0);
						free(tmpstr); tmpstr = NULL;
					}
					goto playerstart;
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
			}
			//don't change this sleep, without this
			//the player stops to fast, and a last seek can
			//produce a segfault
			sleep(1);
			if(playertype == 1)
				playerafterendts();
			else
				playerafterend();
			writevfd("Player");
			screenplayinfobar(file, 1, playertype, flag);
			goto playerstart;
		}
	}

	if(startfolder == 0)
	{
		tmpstr = ostrcat(status.lastservice->channellist, NULL, 0, 0);
		servicecheckret(servicestart(status.lastservice->channel, tmpstr, NULL, 0), 0);
		free(tmpstr); tmpstr = NULL;
	}
	status.updatevfd = START;

	if(tmppolicy != NULL)
	{
		setpolicy(tmppolicy);
		free(tmppolicy);
	}

	status.playspeed = 0;
	status.pause = 0;
	status.play = 0;
	free(file);
	free(formats);
}

#endif
