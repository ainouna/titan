#ifndef GMULTIEPG_H
#define GMULTIEPG_H

time_t calcprimetime(time_t akttime)
{
	struct tm *loctime = NULL;
	time_t ret = 0;

	loctime = olocaltime(&akttime);
	if(loctime != NULL)
	{
		loctime->tm_hour = 20;
		loctime->tm_min = 15;
		ret = mktime(loctime);
		if(ret < akttime) // add 1 day
		{
			free(loctime);
			ret += 86400;
			loctime = olocaltime(&ret);
			if(loctime != NULL)
				ret = mktime(loctime);
			else
				ret = akttime;
		}
	}

	free(loctime);
	return ret;
}

void createtimeline(struct skin* gmultiepg, struct skin* timeline, time_t akttime, int zoom)
{
	struct skin* node = NULL;
	struct tm *loctime = NULL;
	int posx = 0;
	char* buf = NULL;

	buf = malloc(6);
	if(buf == NULL)
	{
		err("no mem");
		return;
	}

	delmarkedscreennodes(gmultiepg, 3);

	while(posx < timeline->iwidth)
	{
		node = addlistbox(gmultiepg, timeline, node, 3);
		if(node != NULL)
		{

			loctime = olocaltime(&akttime);
			if(loctime != NULL)
				strftime(buf, 6, "%H:%M", loctime);
			else
				snprintf(buf, 6, "00:00");
			free(loctime); loctime = NULL;

			akttime += 30 * 60;
			changetext(node, buf);
			node->prozwidth = 0;
			node->posx = posx;
			node->width = 30 * zoom; //30 min * zoom pixel
			node->bordersize = 1;
			posx += node->width;
			if(posx > timeline->iwidth)
				node->width -= (posx - timeline->iwidth);
		}
	}

	free(buf);
}

int selectchannelgmepg(struct skin* listbox)
{
	struct skin* node = listbox;
	struct channel* chnode = NULL;
	listbox->aktpage = -1;
	listbox->aktline = 1;

	listbox->select = NULL;

	if(status.servicetype == 0)
		chnode = getchannel(getconfigint("serviceid", NULL), getconfiglu("transponderid", NULL));
	else
		chnode = getchannel(getconfigint("rserviceid", NULL), getconfiglu("rtransponderid", NULL));

	if(chnode == NULL)
	{
		debug(1000, "out -> NULL detect");
		return 1;
	}

	while(node != NULL)
	{
		if(node->deaktivcol > -1)
		{
			node = node->next;
			continue;
		}
		if(chnode == (struct channel*)node->handle)
			return 0;

		if(node->del == 1) listbox->aktline++;
		node = node->next;
	}
	listbox->aktline = 1;
	return 1;
}


int calcgmultiepg(struct channel* tmpchannel, struct skin* gmultiepg, struct skin* channellistbox, struct skin* listbox, int zoom, time_t akttime, struct channel* aktchannel, int linecol1, int linecol2, int linecol3, int* aktline, struct skin** pchnode, struct skin** pchnode1, int height, int picheight)
{
	int treffer = 0, gridbr = 0, aktcol = 0, nottuneable = 0;
	struct epg* epgnode = NULL;
	struct skin* chnode = NULL, *chnode1 = NULL;
	char* tmpstr = NULL;

	int epgpicon = getconfigint("epgpicon", NULL);

	if(tmpchannel != NULL && tmpchannel->servicetype == status.servicetype)
	{
		if(channelnottunable(tmpchannel) == 1) nottuneable = 1;
		*pchnode = addlistbox(gmultiepg, channellistbox, *pchnode, 1);
		chnode = *pchnode;
		if(chnode != NULL)
		{
			chnode->bgspace = 1;
			tmpstr = createpiconpath(tmpchannel, 0);
			if(epgpicon == 1 && tmpstr != NULL && strstr(tmpstr, "default.png") == NULL)
			{
				changepic(chnode, tmpstr);
				chnode->height = picheight;
			}
			else
			{
				changetext(chnode, tmpchannel->name);
				chnode->height = height;
			}
			free(tmpstr); tmpstr = NULL;

			epgnode = getepgakt(tmpchannel);

			while(epgnode != NULL)
			{
				if(epgnode->endtime <= akttime)
				{
					epgnode = epgnode->next;
					continue;
				}
				if(epgnode->starttime >= akttime + ((listbox->iwidth * 60) / zoom))
					break;

				*pchnode1 = addlistbox(gmultiepg, listbox, *pchnode1, 1);
				chnode1 = *pchnode1;
				if(chnode1 != NULL)
				{
					(*aktline)++;
					if(gridbr == 0)
					{
						chnode1->type = TEXTBOXGRIDBR;
						if(tmpchannel == aktchannel)
						{
							listbox->aktline = *aktline;
							listbox->gridcol = 0;
						}
					}
					else
						chnode1->type = TEXTBOX;
					chnode1->wrap = YES;
					gridbr = 1;
					treffer = 1;
					chnode1->height = chnode->height;
					chnode1->width = ((epgnode->endtime - epgnode->starttime) / 60) * zoom;
					chnode1->posx = ((epgnode->starttime - akttime) / 60) * zoom;
					if(chnode1->posx < 0)
					{
						chnode1->width += chnode1->posx;
						chnode1->posx = 0;
					}
					chnode1->prozwidth = 0;
					if(aktcol == linecol1)
						aktcol = linecol2;
					else
						aktcol = linecol1;
					if(akttime >= epgnode->starttime && akttime < epgnode->endtime)
						aktcol = linecol3;
					chnode1->bgcol = aktcol;
					chnode1->bgspace = 1;
					chnode1->vspace = 2;
					chnode1->hspace = 2;
					if(nottuneable == 1)
						chnode1->deaktivcol = convertcol("deaktivcol");
					
					//TODO: record timeline
					//chnode1->type = MULTIPROGRESSBAR;
					//chnode1->progresscol = listbox->progresscol;
					//chnode1->epgrecord = getepgrecord(tmpchannel, epgnode);
		
					changetext(chnode1, epgnode->title);
					chnode1->handle = (char*)tmpchannel;
					chnode1->handle1 = (char*)epgnode;
					if(chnode1->posx + chnode1->width >= listbox->iwidth)
					{
						chnode1->width = listbox->iwidth - chnode1->posx;
						break;
					}
				}
				epgnode = epgnode->next;
			}
			if(gridbr == 0)
			{
				*pchnode1 = addlistbox(gmultiepg, listbox, *pchnode1, 1);
				chnode1 = *pchnode1;
				if(chnode1 != NULL)
				{
					(*aktline)++;
					if(tmpchannel == aktchannel)
						listbox->aktline = *aktline;
					if(gridbr == 0) chnode1->type = GRIDBR;
					gridbr = 1;
					chnode1->height = chnode->height;
					chnode1->handle = (char*)tmpchannel;
					chnode1->bgspace = 1;
					if(nottuneable == 1)
						chnode1->deaktivcol = convertcol("deaktivcol");
				}
			}

			chnode->handle = (char*)tmpchannel;
			if(nottuneable == 1)
				chnode->deaktivcol = convertcol("deaktivcol");
		}
	}
	return treffer;
}

int showallgmepgchannel(struct skin* gmultiepg, struct skin* channellistbox, struct skin* listbox, int zoom, time_t akttime, struct channel* aktchannel)
{
	int treffer = 0, aktline = 0;
	struct skin* chnode = NULL, *chnode1 = NULL;
	struct channel* tmpchannel = channel;

	int linecol1 = convertcol("epgcol1");
	int linecol2 = convertcol("epgcol2");
	int linecol3 = convertcol("epgcol3");
	int height = getskinconfigint("epgheight", NULL);
	int picheight = getskinconfigint("epgpicheight", NULL);

	if(height < 1) height = 35;
	if(picheight < 1) picheight = 35;

	if(gmultiepg != NULL) delmarkedscreennodes(gmultiepg, 1);
	if(listbox != NULL) listbox->aktline = 1;

	while(tmpchannel != NULL)
	{ 
		if(calcgmultiepg(tmpchannel, gmultiepg, channellistbox, listbox, zoom, akttime, aktchannel, linecol1, linecol2, linecol3, &aktline, &chnode, &chnode1, height, picheight) == 1)
			treffer = 1;
		tmpchannel = tmpchannel->next;
	}
	return treffer;
}

int showbouquetgmepgchannel(struct skin* gmultiepg, struct skin* channellistbox, struct skin* listbox, struct bouquet* firstbouquet, int zoom, time_t akttime, struct channel* aktchannel)
{
	int treffer = 0, aktline = 0;
	struct skin* chnode = NULL, *chnode1 = NULL;
	struct bouquet* tmpbouquet = firstbouquet;

	int linecol1 = convertcol("epgcol1");
	int linecol2 = convertcol("epgcol2");
	int linecol3 = convertcol("epgcol3");
	int height = getskinconfigint("epgheight", NULL);
	int picheight = getskinconfigint("epgpicheight", NULL);

	if(height < 1) height = 35;
	if(picheight < 1) picheight = 35;

	if(gmultiepg != NULL) delmarkedscreennodes(gmultiepg, 1);
	if(listbox != NULL) listbox->aktline = 1;

	while(tmpbouquet != NULL)
	{
		if(calcgmultiepg(tmpbouquet->channel, gmultiepg, channellistbox, listbox, zoom, akttime, aktchannel, linecol1, linecol2, linecol3, &aktline, &chnode, &chnode1, height, picheight) == 1)
			treffer = 1;
		tmpbouquet = tmpbouquet->next;
	}
	return treffer;
}

int showprovidergmepgchannel(struct skin* gmultiepg, struct skin* channellistbox, struct skin* listbox, struct provider* providernode, int zoom, time_t akttime, struct channel* aktchannel)
{
	int treffer = 0, aktline = 0;
	struct skin* chnode = NULL, *chnode1 = NULL;
	struct channel* tmpchannel = channel;

	int linecol1 = convertcol("epgcol1");
	int linecol2 = convertcol("epgcol2");
	int linecol3 = convertcol("epgcol3");
	int height = getskinconfigint("epgheight", NULL);
	int picheight = getskinconfigint("epgpicheight", NULL);

	if(height < 1) height = 35;
	if(picheight < 1) picheight = 35;

	if(gmultiepg != NULL) delmarkedscreennodes(gmultiepg, 1);

	while(tmpchannel != NULL)
	{
		if(tmpchannel->provider == providernode)
		{
			if(calcgmultiepg(tmpchannel, gmultiepg, channellistbox, listbox, zoom, akttime, aktchannel, linecol1, linecol2, linecol3, &aktline, &chnode, &chnode1, height, picheight) == 1)
				treffer = 1;
		}
		tmpchannel = tmpchannel->next;
	}
	return treffer;
}

int showsatgmepgchannel(struct skin* gmultiepg, struct skin* channellistbox, struct skin* listbox, struct sat* satnode, int zoom, time_t akttime, struct channel* aktchannel)
{
	int treffer = 0, aktline = 0;
	struct skin* chnode = NULL, *chnode1 = NULL;
	struct channel* tmpchannel = channel;
	
	if(satnode == NULL) return 1;

	int linecol1 = convertcol("epgcol1");
	int linecol2 = convertcol("epgcol2");
	int linecol3 = convertcol("epgcol3");
	int height = getskinconfigint("epgheight", NULL);
	int picheight = getskinconfigint("epgpicheight", NULL);

	if(height < 1) height = 35;
	if(picheight < 1) picheight = 35;

	if(gmultiepg != NULL) delmarkedscreennodes(gmultiepg, 1);

	while(tmpchannel != NULL)
	{
		if(tmpchannel->transponder != NULL && tmpchannel->transponder->orbitalpos == satnode->orbitalpos)
		{
			if(calcgmultiepg(tmpchannel, gmultiepg, channellistbox, listbox, zoom, akttime, aktchannel, linecol1, linecol2, linecol3, &aktline, &chnode, &chnode1, height, picheight) == 1)
				treffer = 1;
		}
		tmpchannel = tmpchannel->next;
	}
	return treffer;
}

int showazgmepgchannel(struct skin* gmultiepg, struct skin* channellistbox, struct skin* listbox, int character, int zoom, time_t akttime, struct channel* aktchannel)
{
	int treffer = 0, aktline = 0;
	struct skin* chnode = NULL, *chnode1 = NULL;
	struct channel* tmpchannel = channel;

	int linecol1 = convertcol("epgcol1");
	int linecol2 = convertcol("epgcol2");
	int linecol3 = convertcol("epgcol3");
	int height = getskinconfigint("epgheight", NULL);
	int picheight = getskinconfigint("epgpicheight", NULL);

	if(height < 1) height = 35;
	if(picheight < 1) picheight = 35;

	if(gmultiepg != NULL) delmarkedscreennodes(gmultiepg, 1);

	while(tmpchannel != NULL)
	{
		if(tmpchannel->name != NULL && (tmpchannel->name[0] == character || tmpchannel->name[0] == character + 32))
		{
			if(calcgmultiepg(tmpchannel, gmultiepg, channellistbox, listbox, zoom, akttime, aktchannel, linecol1, linecol2, linecol3, &aktline, &chnode, &chnode1, height, picheight) == 1)
				treffer = 1;
		}
		tmpchannel = tmpchannel->next;
	}
	return treffer;
}

void drawchannellistgmepg(struct skin* gmultiepg, int list, struct skin* listbox)
{
	status.markedchannel = NULL;

	if(list == ALLCHANNEL || list == SATCHANNEL || list == PROVIDERCHANNEL || list == AZCHANNEL || list == BOUQUETCHANNEL)
	{
		if(listbox->select == NULL)
		{
			status.screencalc = 2;
			drawscreen(gmultiepg, 0);
			status.screencalc = 0;
		}
		if(listbox->select != NULL)
			status.markedchannel = (struct channel*)listbox->select->handle;
	}
	drawscreen(gmultiepg, 0);
}

void screengmultiepg(struct channel* chnode, struct epg* epgnode, int flag)
{
	int rcret = 0, ret = 0, epgscreenconf = 0;
	struct skin* gmultiepg = getscreen("gmultiepg");
	struct skin* listbox = getscreennode(gmultiepg, "listbox");
	struct skin* channellistbox = getscreennode(gmultiepg, "channellistbox");
	struct skin* epgdesc = getscreennode(gmultiepg, "epgdesc");
	struct skin* timeline = getscreennode(gmultiepg, "timeline");
	struct skin* b2 = getscreennode(gmultiepg, "b2");
	struct skin* b3 = getscreennode(gmultiepg, "b3");
	struct skin* b4 = getscreennode(gmultiepg, "b4");
	
	int list = ALLCHANNEL;
	char* tmpstr = NULL, *tmpstr1 = NULL, *tmpstr2 = NULL;
	void* aktlist = NULL;
	int character = 0;
	struct sat* satnode = NULL;
	struct provider* providernode = NULL;
	struct channel* aktchannel = NULL;
#ifdef SIMULATE
	time_t akttime = 1307871000;
	//akttime = 1315614900;
	//akttime = 1317926400;
#else
	time_t akttime = time(NULL);
#endif
	int zoom = getconfigint("gmultiepgzoom", NULL);
	if(zoom < 1) zoom = 4;
	int nochanneltitle = getskinconfigint("nochanneltitle", NULL);

	akttime -= (((akttime / 60) % 15) * 60);
	akttime -= (((akttime) % 60));
	time_t starttime = akttime;

	if(chnode == NULL) chnode = status.aktservice->channel;
	if(epgnode == NULL) epgnode = getepgakt(chnode);
	tmpstr2 = epgdescunzip(epgnode);
	changetext(epgdesc, tmpstr2);
	free(tmpstr2); tmpstr2 = NULL;

	//chalc screen, so we have all infos
	status.screencalc = 2;
	drawscreen(gmultiepg, 0);
	status.screencalc = 0;

	time_t addtime = (listbox->iwidth / zoom) * 60;
	addtime -= (((addtime / 60) % 15) * 60);
	addtime -= (((addtime) % 60));
	epgscreenconf = getconfigint("epg_screen", NULL);

	if(status.servicetype == 0)
	{
		tmpstr = getconfig("channellist", NULL);
		aktchannel = getchannel(getconfigint("serviceid", NULL), getconfiglu("transponderid", NULL));
	}
	else
	{
		tmpstr = getconfig("rchannellist", NULL);
		aktchannel = getchannel(getconfigint("rserviceid", NULL), getconfiglu("rtransponderid", NULL));
	}

	if(ostrncmp("(BOUQUET)-", tmpstr, 10) == 0 && strlen(tmpstr) > 10)
	{
		struct mainbouquet* mainbouquetnode = NULL;
		mainbouquetnode = getmainbouquet(tmpstr + 10);
		if(mainbouquetnode != NULL && mainbouquetnode->bouquet != NULL)
		{
			tmpstr1 = ostrcat(tmpstr1, _("GRAPHIC MULTI EPG - Bouquets"), 0, 0);
			tmpstr1 = ostrcat(tmpstr1, " - ", 1, 0);
			tmpstr1 = ostrcat(tmpstr1, tmpstr + 10, 1, 0);
			if(nochanneltitle == 0) changetitle(gmultiepg, tmpstr1);
			free(tmpstr1); tmpstr1 = NULL;
			list = BOUQUETCHANNEL;
			aktlist = (void*)mainbouquetnode;
			showbouquetgmepgchannel(gmultiepg, channellistbox, listbox, mainbouquetnode->bouquet, zoom, akttime, aktchannel);
			selectchannelgmepg(channellistbox);
		}
	}
	else if(ostrncmp("(A-Z)-", tmpstr, 6) == 0 && strlen(tmpstr) > 6)
	{
		tmpstr1 = ostrcat(tmpstr1, _("GRAPHIC MULTI EPG - Channel"), 0, 0);
		tmpstr1 = ostrcat(tmpstr1, " - ", 1, 0);
		tmpstr1 = ostrcat(tmpstr1, tmpstr + 6, 1, 0);
		if(nochanneltitle == 0) changetitle(gmultiepg, tmpstr1);
		free(tmpstr1); tmpstr1 = NULL;
		list = AZCHANNEL;
		character = (int)tmpstr[6];
		aktlist = (void*)(int)tmpstr[6];
		showazgmepgchannel(gmultiepg, channellistbox, listbox, character, zoom, akttime, aktchannel);
		selectchannelgmepg(channellistbox);
	}
	else if(ostrncmp("(SAT)-", tmpstr, 6) == 0 && strlen(tmpstr) > 6)
	{
		tmpstr1 = ostrcat(tmpstr1, _("GRAPHIC MULTI EPG - Satellites"), 0, 0);
		tmpstr1 = ostrcat(tmpstr1, " - ", 1, 0);
		tmpstr1 = ostrcat(tmpstr1, tmpstr + 6, 1, 0);
		if(nochanneltitle == 0) changetitle(gmultiepg, tmpstr1);
		free(tmpstr1); tmpstr1 = NULL;
		satnode = getsat(tmpstr + 6);
		list = SATCHANNEL;
		aktlist = (void*)satnode;
		showsatgmepgchannel(gmultiepg, channellistbox, listbox, satnode, zoom, akttime, aktchannel);
		selectchannelgmepg(channellistbox);
	}
	else if(ostrncmp("(PROVIDER)-", tmpstr, 11) == 0 && strlen(tmpstr) > 6)
	{
		tmpstr1 = ostrcat(tmpstr1, _("GRAPHIC MULTI EPG - Provider"), 0, 0);
		tmpstr1 = ostrcat(tmpstr1, " - ", 1, 0);
		tmpstr1 = ostrcat(tmpstr1, tmpstr + 11, 1, 0);
		if(nochanneltitle == 0) changetitle(gmultiepg, tmpstr1);
		free(tmpstr1); tmpstr1 = NULL;
		providernode = getproviderbyname(tmpstr + 11);
		list = PROVIDERCHANNEL;
		aktlist = (void*)providernode;
		showprovidergmepgchannel(gmultiepg, channellistbox, listbox, providernode, zoom, akttime, aktchannel);
		selectchannelgmepg(channellistbox);
	}
	else
	{
		if(nochanneltitle == 0) changetitle(gmultiepg, _("GRAPHIC MULTI EPG - All Channels"));
		list = ALLCHANNEL;
		showallgmepgchannel(gmultiepg, channellistbox, listbox, zoom, akttime, aktchannel);
		selectchannelgmepg(channellistbox);
	}

	if(flag == 0 && epgscreenconf == 3)
	{
		b2->hidden = NO;
		b3->hidden = NO;
		b4->hidden = NO;
	}
	else
	{
		b2->hidden = YES;
		b3->hidden = YES;
		b4->hidden = YES;
	}

	tmpstr = NULL;
	status.screencalc = 2;
	drawscreen(gmultiepg, 0);
	status.screencalc = 0;
	addscreenrc(gmultiepg, listbox);
	listbox->aktpage = channellistbox->aktpage;

	createtimeline(gmultiepg, timeline, akttime, zoom);

	drawchannellistgmepg(gmultiepg, list, listbox);

	while(1)
	{
		status.screencalc = 2;
		rcret = waitrc(gmultiepg, 0, 0);
		status.screencalc = 0;

		channellistbox->aktpage = listbox->aktpage;

		if((rcret == getrcconfigint("rcexit", NULL)) || (rcret == getrcconfigint("rcepg", NULL))) break;
		//if(rcret == getrcconfigint("rcinfo", NULL)) break;
		if(rcret == getrcconfigint("rcok", NULL))
		{
			servicecheckret(servicestart((struct channel*)listbox->select->handle, NULL, NULL, 0), 0);
			break;
		}
		if(rcret == getrcconfigint("rcinfo", NULL))
		{
			if(listbox->select != NULL)
			{
				clearscreen(gmultiepg);
				screenepg((struct channel*)listbox->select->handle, (struct epg*)listbox->select->handle1, 1);
				drawscreen(gmultiepg, 0);
			}
		}
		
		if(flag == 0 && epgscreenconf == 3 && rcret == getrcconfigint("rcgreen", NULL))
		{
			if(listbox->select != NULL)
			{
				clearscreen(gmultiepg);
				screenepg((struct channel*)listbox->select->handle, (struct epg*)listbox->select->handle1, 0);
				//drawscreen(gmultiepg, 0);
				break;
			}
		}
		if(flag == 0 && epgscreenconf == 3 && rcret == getrcconfigint("rcyellow", NULL))
		{
			if(listbox->select != NULL)
			{
				clearscreen(gmultiepg);
				screensingleepg((struct channel*)listbox->select->handle, NULL, 0);
				//drawscreen(gmultiepg, 0);
				break;
			}
		}
		if(flag == 0 && epgscreenconf == 3 && rcret == getrcconfigint("rcblue", NULL))
		{
			if(listbox->select != NULL)
			{
				clearscreen(gmultiepg);
				screenmultiepg((struct channel*)listbox->select->handle, NULL, 0);
				//drawscreen(gmultiepg, 0);
				break;
			}
		}

		if(rcret == getrcconfigint("rcff", NULL) || rcret == getrcconfigint("rcfav", NULL))
		{
			time_t tmptime = 0;

			if(rcret == getrcconfigint("rcfav", NULL))
			{
				tmptime = calcprimetime(akttime);
				if(tmptime != 0)
				{
					akttime = tmptime;
					akttime -= addtime;
				}
			}
			akttime += addtime;

			if(list == BOUQUETCHANNEL)
			{
				if(showbouquetgmepgchannel(gmultiepg, channellistbox, listbox, ((struct mainbouquet*)aktlist)->bouquet, zoom, akttime, aktchannel) == 0)
				{
					if(tmptime == 0)
						akttime -= addtime;
					else
						akttime = starttime;
					if(akttime < starttime)
						akttime = starttime;
					else
						showbouquetgmepgchannel(gmultiepg, channellistbox, listbox, ((struct mainbouquet*)aktlist)->bouquet, zoom, akttime, aktchannel);
				}
			}
			else if(list == ALLCHANNEL)
			{
				if(showallgmepgchannel(gmultiepg, channellistbox, listbox, zoom, akttime, aktchannel) == 0)
				{
					if(tmptime == 0)
						akttime -= addtime;
					else
						akttime = starttime;
					if(akttime < starttime)
						akttime = starttime;
					else
						showallgmepgchannel(gmultiepg, channellistbox, listbox, zoom, akttime, aktchannel);
				}
			}
			else if(list == AZCHANNEL)
			{
				if(showazgmepgchannel(gmultiepg, channellistbox, listbox, character, zoom, akttime, aktchannel) == 0)
				{
					if(tmptime == 0)
						akttime -= addtime;
					else
						akttime = starttime;
					if(akttime < starttime)
						akttime = starttime;
					else
						showazgmepgchannel(gmultiepg, channellistbox, listbox, character, zoom, akttime, aktchannel);
				}
			}
			else if(list == SATCHANNEL)
			{
				if(showsatgmepgchannel(gmultiepg, channellistbox, listbox, satnode, zoom, akttime, aktchannel) == 0)
				{
					if(tmptime == 0)
						akttime -= addtime;
					else
						akttime = starttime;
					if(akttime < starttime)
						akttime = starttime;
					else
						showsatgmepgchannel(gmultiepg, channellistbox, listbox, satnode, zoom, akttime, aktchannel);
				}
			}
			else if(list == PROVIDERCHANNEL)
			{
				if(showprovidergmepgchannel(gmultiepg, channellistbox, listbox, providernode, zoom, akttime, aktchannel) == 0)
				{
					if(tmptime == 0)
						akttime -= addtime;
					else
						akttime = starttime;
					if(akttime < starttime)
						akttime = starttime;
					else
						showprovidergmepgchannel(gmultiepg, channellistbox, listbox, providernode, zoom, akttime, aktchannel);
				}
			}
			createtimeline(gmultiepg, timeline, akttime, zoom);
			drawscreen(gmultiepg, 0);
			if(listbox->select != NULL)
				aktchannel = (struct channel*)listbox->select->handle;
			continue;
		}

		if(rcret == getrcconfigint("rcfr", NULL))
		{
			akttime -= addtime;
			if(akttime < starttime) akttime = starttime;

			if(list == BOUQUETCHANNEL)
				showbouquetgmepgchannel(gmultiepg, channellistbox, listbox, ((struct mainbouquet*)aktlist)->bouquet, zoom, akttime, aktchannel);
			else if(list == ALLCHANNEL)
				showallgmepgchannel(gmultiepg, channellistbox, listbox, zoom, akttime, aktchannel);
			else if(list == AZCHANNEL)
				showazgmepgchannel(gmultiepg, channellistbox, listbox, character, zoom, akttime, aktchannel);
			else if(list == SATCHANNEL)
				showsatgmepgchannel(gmultiepg, channellistbox, listbox, satnode, zoom, akttime, aktchannel);
			else if(list == PROVIDERCHANNEL)
				showprovidergmepgchannel(gmultiepg, channellistbox, listbox, providernode, zoom, akttime, aktchannel);
			createtimeline(gmultiepg, timeline, akttime, zoom);
			drawscreen(gmultiepg, 0);
			if(listbox->select != NULL)
				aktchannel = (struct channel*)listbox->select->handle;
			continue;
		}

		if(rcret == getrcconfigint("rcred", NULL) && listbox->select != NULL)
		{
			clearscreen(gmultiepg);
			ret = addrecepg((struct channel*)listbox->select->handle, (struct epg*)listbox->select->handle1, NULL);
			drawscreen(gmultiepg, 0);
			continue;
		}

		if(listbox->select != NULL)
		{
			tmpstr2 = epgdescunzip((struct epg*)listbox->select->handle1);
			changetext(epgdesc, tmpstr2);
			free(tmpstr2); tmpstr2 = NULL;

			aktchannel = (struct channel*)listbox->select->handle;
		}
		drawscreen(gmultiepg, 0);
	}

	status.markedchannel = NULL;
	status.markmodus = 0;
	status.screencalc = 0;
	delmarkedscreennodes(gmultiepg, 1);
	delmarkedscreennodes(gmultiepg, 3);
	delownerrc(gmultiepg);
	clearscreen(gmultiepg);
}

#endif
