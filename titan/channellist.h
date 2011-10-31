#ifndef CHANNELLIST_H
#define CHANNELLIST_H

int selectchannel(struct skin* listbox)
{
	struct skin* node = listbox;
	struct channel* chnode = NULL;
	listbox->aktpage = -1;
	listbox->aktline = 1;

	listbox->select = NULL;

	if(status.servicetype == 0)
		chnode = getchannel(getconfigint("serviceid", NULL), getconfigint("transponderid", NULL));
	else
		chnode = getchannel(getconfigint("rserviceid", NULL), getconfigint("rtransponderid", NULL));

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
		if(chnode == (struct channel*) node->handle)
			return 0;
		if(node->del == 1) listbox->aktline++;
		node = node->next;
	}
	listbox->aktline = 1;
	return 1;
}

void changechannelepg(struct channel* chnode, struct skin* node)
{
	struct epg* epgnode = NULL;
	char* tmpstr = NULL;

	epgnode = getepgakt(chnode);
	if(epgnode != NULL)
	{
		tmpstr = ostrcat(tmpstr, " - ", 1, 0);
		tmpstr = ostrcat(tmpstr, epgnode->title, 1, 0);
		changetext2(node, tmpstr);
		free(tmpstr); tmpstr = NULL;
	}
}

void showallchannel(struct skin* channellist, struct skin* listbox, int mode)
{
	struct skin* chnode = NULL;
	struct channel* tmpchannel = channel;
	long long deaktivcol = convertcol("deaktivcol");

	while(tmpchannel != NULL)
	{
		if(tmpchannel->servicetype == status.servicetype)
		{
			chnode = addlistbox(channellist, listbox, chnode, 1);
			if(chnode != NULL)
			{
				changechannelepg(tmpchannel, chnode);
				if(tmpchannel->protect > 0)
					chnode->fontcol = convertcol("protectcol");
				changetext(chnode, tmpchannel->name);
				chnode->handle = (char*) tmpchannel;
				chnode->handle1 = (char*) tmpchannel;
				if(mode == 0 && channelnottunable(tmpchannel) == 1)
					chnode->deaktivcol = deaktivcol;
			}
		}
		tmpchannel = tmpchannel->next;
	}
}

void showbouquetchannel(struct skin* channellist, struct skin* listbox, struct bouquet* firstbouquet, int mode)
{
	struct skin* chnode = NULL;
	struct bouquet* tmpbouquet = firstbouquet;
	char* tmpstr = NULL, *tmpnr = NULL;
	long long deaktivcol = convertcol("deaktivcol");

	while(tmpbouquet != NULL)
	{
		if(tmpbouquet->channel != NULL)
		{
			if(tmpbouquet->channel->servicetype == status.servicetype)
			{
				chnode = addlistbox(channellist, listbox, chnode, 1);
				if(chnode != NULL)
				{
					tmpnr = oitoa(tmpbouquet->nr);
					changeret(chnode, tmpnr);
					tmpstr = ostrcat(tmpstr, tmpnr, 1, 1);
					tmpstr = ostrcat(tmpstr, "  ", 1, 0);
					tmpstr = ostrcat(tmpstr, tmpbouquet->channel->name, 1, 0);
					changetext(chnode, tmpstr);
					free(tmpstr); tmpstr = NULL;
					if(tmpbouquet->channel->protect > 0)
						chnode->fontcol = convertcol("protectcol");

					changechannelepg(tmpbouquet->channel, chnode);
					chnode->handle = (char*) tmpbouquet->channel;
					chnode->handle1 = (char*) tmpbouquet;
					if(mode == 0 && channelnottunable(tmpbouquet->channel) == 1)
						chnode->deaktivcol = deaktivcol;
				}
			}
		}
		tmpbouquet = tmpbouquet->next;
	}
}

void showproviderchannel(struct skin* channellist, struct skin* listbox, struct provider* providernode, int mode)
{
	struct skin* chnode = NULL;
	struct channel* tmpchannel = channel;
	long long deaktivcol = convertcol("deaktivcol");

	while(tmpchannel != NULL)
	{
		if(tmpchannel->provider == providernode)
		{
			if(tmpchannel->servicetype == status.servicetype)
			{
				chnode = addlistbox(channellist, listbox, chnode, 1);
				if(chnode != NULL)
				{
					changechannelepg(tmpchannel, chnode);
					changetext(chnode, tmpchannel->name);
					if(tmpchannel->protect > 0)
						chnode->fontcol = convertcol("protectcol");
					chnode->handle = (char*) tmpchannel;
					chnode->handle1 = (char*) tmpchannel;
					if(mode == 0 && channelnottunable(tmpchannel) == 1)
						chnode->deaktivcol = deaktivcol;
				}
			}
		}
		tmpchannel = tmpchannel->next;
	}
}

void showprovider(struct skin* channellist, struct skin* listbox)
{
	struct provider *node = provider;
	struct skin* providernode = NULL;

	while(node != NULL)
	{
		providernode = addlistbox(channellist, listbox, providernode, 2);
		if(providernode != NULL)
		{
			changetext(providernode, node->name);
			changename(providernode, node->name);
			providernode->handle = (char*) node;
			providernode->handle1 = (char*) node;
		}
		node = node->next;
	}
}

void showsatchannel(struct skin* channellist, struct skin* listbox, struct sat* satnode, int mode)
{
	struct skin* chnode = NULL;
	struct channel* tmpchannel = channel;
	long long deaktivcol = convertcol("deaktivcol");

	if(satnode == NULL)
		return;

	while(tmpchannel != NULL)
	{
		if(tmpchannel->transponder != NULL && tmpchannel->transponder->orbitalpos == satnode->orbitalpos)
		{
			if(tmpchannel->servicetype == status.servicetype)
			{
				chnode = addlistbox(channellist, listbox, chnode, 1);
				if(chnode != NULL)
				{
					changechannelepg(tmpchannel, chnode);
					changetext(chnode, tmpchannel->name);
					if(tmpchannel->protect > 0)
						chnode->fontcol = convertcol("protectcol");
					chnode->handle = (char*) tmpchannel;
					chnode->handle1 = (char*) tmpchannel;
					if(mode == 0 && channelnottunable(tmpchannel) == 1)
						chnode->deaktivcol = deaktivcol;
				}
			}
		}
		tmpchannel = tmpchannel->next;
	}
}

void showsat(struct skin* channellist, struct skin* listbox)
{
	struct sat *node = sat;
	struct skin* satnode = NULL;

	while(node != NULL)
	{
		satnode = addlistbox(channellist, listbox, satnode, 2);
		if(satnode != NULL)
		{
			changetext(satnode, node->name);
			changename(satnode, node->name);
			satnode->handle = (char*) node;
			satnode->handle1 = (char*) node;
		}
		node = node->next;
	}
}

void showazchannel(struct skin* channellist, struct skin* listbox, int character, int mode)
{
	struct skin* chnode = NULL;
	struct channel* tmpchannel = channel;
	long long deaktivcol = convertcol("deaktivcol");

	while(tmpchannel != NULL)
	{
		if(tmpchannel->name != NULL && (tmpchannel->name[0] == character || tmpchannel->name[0] == character + 32))
		{
			if(tmpchannel->servicetype == status.servicetype)
			{
				chnode = addlistbox(channellist, listbox, chnode, 1);
				if(chnode != NULL)
				{
					changechannelepg(tmpchannel, chnode);
					changetext(chnode, tmpchannel->name);
					if(tmpchannel->protect > 0)
						chnode->fontcol = convertcol("protectcol");
					chnode->handle = (char*) tmpchannel;
					chnode->handle1 = (char*) tmpchannel;
					if(mode == 0 && channelnottunable(tmpchannel) == 1)
						chnode->deaktivcol = deaktivcol;
				}
			}
		}
		tmpchannel = tmpchannel->next;
	}
}

void showaz(struct skin* channellist, struct skin* listbox)
{
	struct skin* node = NULL;
	int i;
	char* tmpstr = NULL;

	tmpstr = malloc(2);
	if(tmpstr == NULL)
	{
		err("no memory");
		return;
	}

	for(i = 65; i < 91; i++)
	{
		node = addlistbox(channellist, listbox, node, 2);
		if(node != NULL)
		{
			snprintf(tmpstr, 2, "%c", i);
			changetext(node, tmpstr);
			changename(node, tmpstr);
			node->handle = (void*) i;
		}
	}

	free(tmpstr);
}

void showmainbouquet(struct skin* channellist, struct skin* listbox)
{
	struct mainbouquet *node = mainbouquet;
	struct skin* bouquetnode = NULL;

	while(node != NULL)
	{
		if(node->type == status.servicetype)
		{
			bouquetnode = addlistbox(channellist, listbox, bouquetnode, 2);
			if(bouquetnode != NULL)
			{
				changetext(bouquetnode, node->name);
				changename(bouquetnode, node->name);
				bouquetnode->handle = (char*) node->bouquet;
				bouquetnode->handle1 = (char*) node;
			}
		}
		node = node->next;
	}
}

void drawchannellist(struct skin* channellist, int list, struct skin* listbox)
{
	status.markedchannel = NULL;
	if(list == ALLCHANNEL || list == SATCHANNEL || list == PROVIDERCHANNEL || list == AZCHANNEL || list == BOUQUETCHANNEL)
	{
		if(listbox->select == NULL)
		{
			status.screencalc = 2;
			drawscreen(channellist, 0);
			status.screencalc = 0;
		}
		if(listbox->select != NULL)
			status.markedchannel = (struct channel*)listbox->select->handle;
	}
	drawscreen(channellist, 0);
}

void recalclist(int list, void* aktlist, int listmode, struct skin* channellist, struct skin* listbox)
{
	if(list == ALLCHANNEL)
		showallchannel(channellist, listbox, listmode);
	if(list == SATCHANNEL)
		showsatchannel(channellist, listbox, (struct sat*)aktlist, listmode);
	if(list == PROVIDERCHANNEL)
		showproviderchannel(channellist, listbox, (struct provider*)aktlist, listmode);
	if(list == AZCHANNEL)
		showazchannel(channellist, listbox, (int)aktlist, listmode);
	if(list == BOUQUETCHANNEL)
		showbouquetchannel(channellist, listbox, ((struct mainbouquet*)aktlist)->bouquet, listmode);
	if(list == MAINBOUQUETLIST)
		showmainbouquet(channellist, listbox);
	if(list == SATLIST)
		showsat(channellist, listbox);
	if(list == PROVIDERLIST)
		showprovider(channellist, listbox);
}

//flag 1: called from recordtimer screen
//flag 2: rcfav  (open bouquetlist)
int screenchannellist(struct channel** retchannel, char** retchannellist, int flag)
{
	struct skin* channellist = getscreen("channellist");
	struct skin* listbox = getscreennode(channellist, "listbox");
	struct skin* b7 = getscreennode(channellist, "b7");
	struct skin* tmpskin;
	int rcret, ret, listmode, newmodus, list;
	char* tmpstr = NULL, *tmpstr1 = NULL;
	void* movesel = NULL, *aktlist = NULL;
	
	status.channelswitch = 1;
	
start:
	rcret = 0, ret = -1, list = ALLCHANNEL, listmode = NOMODE, newmodus = 0;
	tmpstr = NULL, tmpstr1 = NULL, movesel = NULL, aktlist = NULL, tmpskin = NULL;

	if(status.servicetype == 0)
		tmpstr = getconfig("channellist", NULL);
	else
		tmpstr = getconfig("rchannellist", NULL);
	if(ostrncmp("(BOUQUET)-", tmpstr, 10) == 0 && strlen(tmpstr) > 10)
	{
		struct mainbouquet* mainbouquetnode = NULL;
		mainbouquetnode = getmainbouquet(tmpstr + 10);
		if(mainbouquetnode != NULL && mainbouquetnode->bouquet != NULL)
		{
			tmpstr1 = ostrcat(tmpstr1, _("Bouquets"), 0, 0);
			tmpstr1 = ostrcat(tmpstr1, " - ", 1, 0);
			tmpstr1 = ostrcat(tmpstr1, tmpstr + 10, 1, 0);
			changetitle(channellist, tmpstr1);
			free(tmpstr1); tmpstr1 = NULL;
			list = BOUQUETCHANNEL;
			aktlist = (void*)mainbouquetnode;
			showbouquetchannel(channellist, listbox, mainbouquetnode->bouquet, flag);
			selectchannel(listbox);
		}
	}
	else if(ostrncmp("(A-Z)-", tmpstr, 6) == 0 && strlen(tmpstr) > 6)
	{
		tmpstr1 = ostrcat(tmpstr1, _("Channel"), 0, 0);
		tmpstr1 = ostrcat(tmpstr1, " - ", 1, 0);
		tmpstr1 = ostrcat(tmpstr1, tmpstr + 6, 1, 0);
		changetitle(channellist, tmpstr1);
		free(tmpstr1); tmpstr1 = NULL;
		list = AZCHANNEL;
		aktlist = (void*)(int)tmpstr[6];
		showazchannel(channellist, listbox, (int)tmpstr[6], flag);
		selectchannel(listbox);
	}
	else if(ostrncmp("(SAT)-", tmpstr, 6) == 0 && strlen(tmpstr) > 6)
	{
		tmpstr1 = ostrcat(tmpstr1, _("Satellites"), 0, 0);
		tmpstr1 = ostrcat(tmpstr1, " - ", 1, 0);
		tmpstr1 = ostrcat(tmpstr1, tmpstr + 6, 1, 0);
		changetitle(channellist, tmpstr1);
		free(tmpstr1); tmpstr1 = NULL;
		struct sat* satnode = getsat(tmpstr + 6);
		list = SATCHANNEL;
		aktlist = (void*)satnode;
		showsatchannel(channellist, listbox, satnode, flag);
		selectchannel(listbox);
	}
	else if(ostrncmp("(PROVIDER)-", tmpstr, 11) == 0 && strlen(tmpstr) > 6)
	{
		tmpstr1 = ostrcat(tmpstr1, _("Provider"), 0, 0);
		tmpstr1 = ostrcat(tmpstr1, " - ", 1, 0);
		tmpstr1 = ostrcat(tmpstr1, tmpstr + 11, 1, 0);
		changetitle(channellist, tmpstr1);
		free(tmpstr1); tmpstr1 = NULL;
		struct provider* providernode = getproviderbyname(tmpstr + 11);
		list = PROVIDERCHANNEL;
		aktlist = (void*)providernode;
		showproviderchannel(channellist, listbox, providernode, flag);
		selectchannel(listbox);
	}
	else
	{
		changetitle(channellist, _("All Channels"));
		list = ALLCHANNEL;
		showallchannel(channellist, listbox, flag);
		selectchannel(listbox);
	}

	tmpstr = NULL;
	status.screencalc = 2;
	drawscreen(channellist, 0);
	status.screencalc = 0;
	addscreenrc(channellist, listbox);

	if(flag != 2) drawchannellist(channellist, list, listbox);

	while(1)
	{
		if(flag == 2)
		{
			rcret = getrcconfigint("rcblue", NULL);
			flag = 0;
		}
		else
		{
			status.screencalc = 2;
			rcret = waitrc(channellist, 10000, 0);
			status.screencalc = 0;
		}

		//read epg new
		if(listbox != NULL && listbox->select != NULL && (list == ALLCHANNEL || list == SATCHANNEL || list == PROVIDERCHANNEL || list == AZCHANNEL || list == BOUQUETCHANNEL))
		{
			tmpskin = listbox->select;
			while(tmpskin != NULL)
			{
				if(tmpskin->pagecount != listbox->aktpage) break;
				changechannelepg((struct channel*)tmpskin->handle, tmpskin);
				tmpskin = tmpskin->prev;
			}
			tmpskin = listbox->select;
			while(tmpskin != NULL)
			{
				if(tmpskin->pagecount != listbox->aktpage) break;
				changechannelepg((struct channel*)tmpskin->handle, tmpskin);
				tmpskin = tmpskin->next;
			}
		}

		if(rcret == getrcconfigint("rcup", NULL) || rcret == getrcconfigint("rcdown", NULL) || rcret == getrcconfigint("rcleft", NULL) || rcret == getrcconfigint("rcright", NULL))
		{
			if(listmode != MVMODE || (listmode == MVMODE && status.markmodus == 0))
				drawchannellist(channellist, list, listbox);
		}

		if(rcret == RCTIMEOUT)
			drawscreen(channellist, 0);

		if(rcret == getrcconfigint("rcexit", NULL)) break;

		if(flag == 0 && listmode > NOMODE)
		{
			if(rcret == getrcconfigint("rcmenu", NULL))
			{
				status.markmodus = 0;
				movesel = NULL;
				clearscreen(channellist);
				listmode = screenlistedit(list, NULL);
				if(listmode == MVMODE)
				{
					delrc(getrcconfigint("rcright", NULL), channellist, listbox);
					delrc(getrcconfigint("rcleft", NULL), channellist, listbox);
				}
				else
					addscreenrc(channellist, listbox);
				delmarkedscreennodes(channellist, 1);
				delmarkedscreennodes(channellist, 2);
				recalclist(list, aktlist, listmode, channellist, listbox);
				selectchannel(listbox);
				drawscreen(channellist, 0);
			}
			if(listmode == PROTECTMODE && listbox->select != NULL && listbox->select->handle != NULL && rcret == getrcconfigint("rcok", NULL))
			{
				if((list == ALLCHANNEL || list == SATCHANNEL || list == PROVIDERCHANNEL || list == AZCHANNEL || list == BOUQUETCHANNEL) && listbox->select->handle != NULL)
				{
					if(((struct channel*)listbox->select->handle)->protect == 0)
						((struct channel*)listbox->select->handle)->protect = 1;
					else
					{
						if(screenpincheck(1, NULL) == 0)
							((struct channel*)listbox->select->handle)->protect = 0;
					}
					status.writechannel = 1;
				}
				delmarkedscreennodes(channellist, 1);
				delmarkedscreennodes(channellist, 2);
				recalclist(list, aktlist, listmode, channellist, listbox);
				drawscreen(channellist, 0);
			}
			if(listmode == CPMODE && listbox->select != NULL && listbox->select->handle != NULL && rcret == getrcconfigint("rcok", NULL))
			{
				struct mainbouquet* mbouquet = screenmainbouquet();
				if(mbouquet != NULL && getbouquetbychannel(mbouquet->bouquet, ((struct channel*)listbox->select->handle)->serviceid, ((struct channel*)listbox->select->handle)->transponderid) == NULL)
				{
					tmpstr1 = oitoa(((struct channel*)listbox->select->handle)->serviceid);
					tmpstr = ostrcat(tmpstr, tmpstr1, 1, 1); tmpstr1 = NULL;
					tmpstr = ostrcat(tmpstr, "#", 1, 0);
					tmpstr1 = oitoa(((struct channel*)listbox->select->handle)->transponderid);
					tmpstr = ostrcat(tmpstr, tmpstr1, 1, 1); tmpstr1 = NULL;
					addbouquet(&mbouquet->bouquet, tmpstr, status.servicetype, 0, NULL);
					free(tmpstr); tmpstr = NULL;
					recalcbouquetnr();
					if(list == BOUQUETCHANNEL)
					{
						delmarkedscreennodes(channellist, 1);
						delmarkedscreennodes(channellist, 2);
						showbouquetchannel(channellist, listbox, ((struct mainbouquet*)aktlist)->bouquet, 1);
					}
				}
				drawscreen(channellist, 0);
			}
			if(listmode == RMMODE && listbox->select != NULL && listbox->select->handle1 != NULL && rcret == getrcconfigint("rcok", NULL))
			{
				if(list == ALLCHANNEL || list == SATCHANNEL || list == PROVIDERCHANNEL || list == AZCHANNEL)
				{
					if(delchannel(((struct channel*)listbox->select->handle1)->serviceid, ((struct channel*)listbox->select->handle1)->transponderid, 0) == 0)
					{
						listbox->aktline--;
						listbox->aktpage = -1;
					}
				}
				if(list == SATLIST)
				{
					delsat(((struct sat*)listbox->select->handle1)->name);
					listbox->aktline--;
					listbox->aktpage = -1;
				}
				if(list == PROVIDERLIST)
				{
					delprovider(((struct provider*)listbox->select->handle1)->providerid);
					listbox->aktline--;
					listbox->aktpage = -1;
				}
				if(list == MAINBOUQUETLIST)
				{
					delmainbouquet(((struct mainbouquet*)listbox->select->handle1)->name);
					recalcbouquetnr();
					listbox->aktline--;
					listbox->aktpage = -1;
				}
				if(list == BOUQUETCHANNEL)
				{
					delbouquet(((struct bouquet*)listbox->select->handle1)->serviceid, ((struct bouquet*)listbox->select->handle1)->transponderid, &((struct mainbouquet*)aktlist)->bouquet);
					recalcbouquetnr();
					listbox->aktline--;
					listbox->aktpage = -1;
				}
				delmarkedscreennodes(channellist, 1);
				delmarkedscreennodes(channellist, 2);
				status.markedchannel = NULL;
				recalclist(list, aktlist, listmode, channellist, listbox);
				drawscreen(channellist, 0);
			}
			if(listmode == MVMODE && listbox->select != NULL && rcret == getrcconfigint("rcok", NULL))
			{
				if(movesel == NULL)
				{
					status.markmodus = 1;
					movesel = listbox->select->handle1;
				}
				else
				{
					status.markmodus = 0;
					movesel = NULL;
				}
				drawscreen(channellist, 0);
			}
			if(listmode == MVMODE && listbox->select != NULL && movesel != NULL && (rcret == getrcconfigint("rcup", NULL) || rcret == getrcconfigint("rcdown", NULL)))
			{
				if(rcret == getrcconfigint("rcup", NULL))
				{
					if(list == ALLCHANNEL)
					{
						movechannelup(movesel);
						while(((struct channel*)movesel)->next != NULL && ((struct channel*)movesel)->next->servicetype != status.servicetype)
							movechannelup(movesel);
					}
					if(list == SATLIST)
						movesatup(movesel);
					if(list == PROVIDERLIST)
						moveproviderup(movesel);
					if(list == MAINBOUQUETLIST)
					{
						movemainbouquetup(movesel);
						while(((struct mainbouquet*)movesel)->next != NULL && ((struct mainbouquet*)movesel)->next->type != status.servicetype)
							movemainbouquetup(movesel);
						recalcbouquetnr();
					}
					if(list == BOUQUETCHANNEL)
					{
						movebouquetup(movesel);
						while(((struct bouquet*)movesel)->next != NULL && ((struct bouquet*)movesel)->next->channel != NULL && ((struct bouquet*)movesel)->next->channel->servicetype != status.servicetype)
							movebouquetup(movesel);
						recalcbouquetnr();
					}
				}
				if(rcret == getrcconfigint("rcdown", NULL))
				{
					if(list == ALLCHANNEL)
					{
						movechanneldown(movesel);
						while(((struct channel*)movesel)->prev != NULL && ((struct channel*)movesel)->prev->servicetype != status.servicetype)
							movechanneldown(movesel);
					}
					if(list == SATLIST)
						movesatdown(movesel);
					if(list == PROVIDERLIST)
						moveproviderdown(movesel);
					if(list == MAINBOUQUETLIST)
					{
						movemainbouquetdown(movesel);
						while(((struct mainbouquet*)movesel)->prev != NULL && ((struct mainbouquet*)movesel)->prev->type != status.servicetype)
							movemainbouquetdown(movesel);
						recalcbouquetnr();
					}
					if(list == BOUQUETCHANNEL)
					{
						movebouquetdown(movesel);
						while(((struct bouquet*)movesel)->prev != NULL && ((struct bouquet*)movesel)->prev->channel != NULL && ((struct bouquet*)movesel)->prev->channel->servicetype != status.servicetype)
							movebouquetdown(movesel);
						recalcbouquetnr();
					}
				}
				delmarkedscreennodes(channellist, 1);
				delmarkedscreennodes(channellist, 2);
				recalclist(list, aktlist, listmode, channellist, listbox);
				drawscreen(channellist, 0);
			}
			continue;
		}

		if(rcret == getrcconfigint("rcred", NULL))
		{
			list = ALLCHANNEL;
			if(status.servicetype == 0)
				addconfigtmp("channellist", "(ALL)");
			else
				addconfigtmp("rchannellist", "(ALL)");
			changetitle(channellist, _("All Channels"));
			delmarkedscreennodes(channellist, 1);
			delmarkedscreennodes(channellist, 2);
			showallchannel(channellist, listbox, flag);
			selectchannel(listbox);
			drawchannellist(channellist, list, listbox);
			continue;
		}
		if(rcret == getrcconfigint("rcblue", NULL))
		{
			list = MAINBOUQUETLIST;
			changetitle(channellist, _("Bouquets"));
			delmarkedscreennodes(channellist, 1);
			delmarkedscreennodes(channellist, 2);
			showmainbouquet(channellist, listbox);
			if(status.servicetype == 0)
			{
				delconfigtmp("channellist");
				tmpstr = getconfig("channellist", NULL);
			}
			else
			{
				delconfigtmp("rchannellist");
				tmpstr = getconfig("rchannellist", NULL);
			}
			if(ostrncmp("(BOUQUET)-", tmpstr, 10) != 0) tmpstr = NULL;
			if(tmpstr != NULL && strlen(tmpstr) > 10)
				setlistboxselection(listbox, tmpstr + 10);
			else
			{
				listbox->aktpage = -1;
				listbox->aktline = 1;
			}
			tmpstr = NULL;
			drawchannellist(channellist, list, listbox);
			continue;
		}
		if(rcret == getrcconfigint("rcgreen", NULL))
		{
			list = SATLIST;
			changetitle(channellist, _("Satellites"));
			delmarkedscreennodes(channellist, 1);
			delmarkedscreennodes(channellist, 2);
			showsat(channellist, listbox);
			if(status.servicetype == 0)
			{
				delconfigtmp("channellist");
				tmpstr = getconfig("channellist", NULL);
			}
			else
			{
				delconfigtmp("rchannellist");
				tmpstr = getconfig("rchannellist", NULL);
			}
			if(ostrncmp("(SAT)-", tmpstr, 6) != 0) tmpstr = NULL;
			if(tmpstr != NULL && strlen(tmpstr) > 6)
				setlistboxselection(listbox, tmpstr + 6);
			else
			{
				listbox->aktpage = -1;
				listbox->aktline = 1;
			}
			tmpstr = NULL;
			drawchannellist(channellist, list, listbox);
			continue;
		}
		if(rcret == getrcconfigint("rctext", NULL))
		{
			list = AZLIST;
			changetitle(channellist, _("Channels A-Z"));
			delmarkedscreennodes(channellist, 1);
			delmarkedscreennodes(channellist, 2);
			showaz(channellist, listbox);
			if(status.servicetype == 0)
			{
				delconfigtmp("channellist");
				tmpstr = getconfig("channellist", NULL);
			}
			else
			{
				delconfigtmp("rchannellist");
				tmpstr = getconfig("rchannellist", NULL);
			}
			if(ostrncmp("(A-Z)-", tmpstr, 6) != 0) tmpstr = NULL;
			if(tmpstr != NULL && strlen(tmpstr) > 6)
				setlistboxselection(listbox, &tmpstr[6]);
			else
			{
				listbox->aktpage = -1;
				listbox->aktline = 1;
			}
			tmpstr = NULL;
			drawchannellist(channellist, list, listbox);
			continue;
		}
		if(rcret == getrcconfigint("rcyellow", NULL))
		{
			list = PROVIDERLIST;
			changetitle(channellist, _("Provider"));
			delmarkedscreennodes(channellist, 1);
			delmarkedscreennodes(channellist, 2);
			showprovider(channellist, listbox);
			if(status.servicetype == 0)
			{
				delconfigtmp("channellist");
				tmpstr = getconfig("channellist", NULL);
			}
			else
			{
				delconfigtmp("rchannellist");
				tmpstr = getconfig("rchannellist", NULL);
			}
			if(ostrncmp("(PROVIDER)-", tmpstr, 11) != 0) tmpstr = NULL;
			if(tmpstr != NULL && strlen(tmpstr) > 11)
				setlistboxselection(listbox, tmpstr + 11);
			else
			{
				listbox->aktpage = -1;
				listbox->aktline = 1;
			}
			tmpstr = NULL;
			drawchannellist(channellist, list, listbox);
			continue;
		}
		if((list == ALLCHANNEL || list == SATCHANNEL || list == PROVIDERCHANNEL || list == AZCHANNEL || list == BOUQUETCHANNEL) && rcret == getrcconfigint("rcok", NULL))
		{ 
			if(listbox->select != NULL && listbox->select->handle != NULL)
			{
				if(flag == 1)
				{
					if(retchannel != NULL)
						*retchannel = (struct channel*)listbox->select->handle;
					if(retchannellist != NULL)
					{
						if(status.servicetype == 0)
							*retchannellist = ostrcat(getconfig("channellist", NULL), "", 0, 0);
						else
							*retchannellist = ostrcat(getconfig("rchannellist", NULL), "", 0, 0);
					}
					break;
				}

				clearscreen(channellist);
				drawscreen(skin, 0);
				if(status.servicetype == 0)
					ret = servicestart((struct channel*)listbox->select->handle, getconfig("channellist", NULL), NULL, 0);
				else
					ret = servicestart((struct channel*)listbox->select->handle, getconfig("rchannellist", NULL), NULL, 0);
				servicecheckret(ret, 0);
				break;
			}
			continue;
		}
		if(list == BOUQUETCHANNEL && (rcret == getrcconfigint("rcff", NULL) || rcret == getrcconfigint("rcfr", NULL))) 
		{
			struct mainbouquet* tmpaktlist = ((struct mainbouquet*)aktlist);

			while(tmpaktlist != NULL)
			{
				if(rcret == getrcconfigint("rcff", NULL))
					tmpaktlist = tmpaktlist->next;
				else if(rcret == getrcconfigint("rcfr", NULL))
					tmpaktlist = tmpaktlist->prev;
				if(tmpaktlist == NULL) break;
				if(tmpaktlist->type != status.servicetype) continue;

				delmarkedscreennodes(channellist, 1);
				struct skin* tmpnode = addlistbox(channellist, listbox, NULL, 2);
				if(tmpnode != NULL)
				{
					listbox->aktline = 1;
					listbox->aktpage = -1;

					status.screencalc = 2;
					drawscreen(channellist, 0);
					status.screencalc = 0;
					changetext(tmpnode, tmpaktlist->name);
					tmpnode->handle = (char*)tmpaktlist->bouquet;
					tmpnode->handle1 = (char*)tmpaktlist;
					rcret = getrcconfigint("rcok", NULL);
					list = MAINBOUQUETLIST;
					break;
				}
			}
		}
		if(list == MAINBOUQUETLIST && rcret == getrcconfigint("rcok", NULL))
		{
			if(listbox->select != NULL)
			{
				list = BOUQUETCHANNEL;
				tmpstr = ostrcat(tmpstr, _("Bouquets"), 0, 0);
				tmpstr = ostrcat(tmpstr, " - ", 1, 0);
				tmpstr = ostrcat(tmpstr, listbox->select->text, 1, 0);
				changetitle(channellist, tmpstr);
				free(tmpstr); tmpstr = NULL;
				tmpstr = ostrcat(tmpstr, "(BOUQUET)-", 0, 0);
				tmpstr = ostrcat(tmpstr, listbox->select->text, 1, 0);
				if(status.servicetype == 0)
					addconfigtmp("channellist", tmpstr);
				else
					addconfigtmp("rchannellist", tmpstr);
				free(tmpstr); tmpstr = NULL;
				aktlist = listbox->select->handle1;
				showbouquetchannel(channellist, listbox, (struct bouquet*)listbox->select->handle, flag);
				delmarkedscreennodes(channellist, 2);
				selectchannel(listbox);
				drawchannellist(channellist, list, listbox);
			}
			continue;
		}
		if(list == SATCHANNEL && (rcret == getrcconfigint("rcff", NULL) || rcret == getrcconfigint("rcfr", NULL))) 
		{
			struct sat* tmpaktlist = ((struct sat*)aktlist);

			while(tmpaktlist != NULL)
			{
				if(rcret == getrcconfigint("rcff", NULL))
					tmpaktlist = tmpaktlist->next;
				else if(rcret == getrcconfigint("rcfr", NULL))
					tmpaktlist = tmpaktlist->prev;
				if(tmpaktlist == NULL) break;

				delmarkedscreennodes(channellist, 1);
				struct skin* tmpnode = addlistbox(channellist, listbox, NULL, 2);
				if(tmpnode != NULL)
				{
					listbox->aktline = 1;
					listbox->aktpage = -1;

					status.screencalc = 2;
					drawscreen(channellist, 0);
					status.screencalc = 0;
					changetext(tmpnode, tmpaktlist->name);
					tmpnode->handle = (char*)tmpaktlist;
					tmpnode->handle1 = (char*)tmpaktlist;
					rcret = getrcconfigint("rcok", NULL);
					list = SATLIST;
					break;
				}
			}
		}
		if(list == SATLIST && rcret == getrcconfigint("rcok", NULL))
		{
			if(listbox->select != NULL)
			{
				list = SATCHANNEL;
				tmpstr = ostrcat(tmpstr, _("Satellites"), 0, 0);
				tmpstr = ostrcat(tmpstr, " - ", 1, 0);
				tmpstr = ostrcat(tmpstr, listbox->select->text, 1, 0);
				changetitle(channellist, tmpstr);
				free(tmpstr); tmpstr = NULL;
				tmpstr = ostrcat(tmpstr, "(SAT)-", 0, 0);
				tmpstr = ostrcat(tmpstr, listbox->select->text, 1, 0);
				if(status.servicetype == 0)
					addconfigtmp("channellist", tmpstr);
				else
					addconfigtmp("rchannellist", tmpstr);
				free(tmpstr); tmpstr = NULL;
				aktlist = listbox->select->handle;
				showsatchannel(channellist, listbox, (struct sat*)listbox->select->handle, flag);
				delmarkedscreennodes(channellist, 2);
				selectchannel(listbox);
				drawchannellist(channellist, list, listbox);
			}
			continue;
		}
		if(list == AZCHANNEL && (rcret == getrcconfigint("rcff", NULL) || rcret == getrcconfigint("rcfr", NULL))) 
		{
			int tmpaktlist = (int)aktlist;

			if(rcret == getrcconfigint("rcff", NULL))
				tmpaktlist++;
			else if(rcret == getrcconfigint("rcfr", NULL))
				tmpaktlist--;
			if(tmpaktlist < 65) tmpaktlist = 65;
			if(tmpaktlist > 90) tmpaktlist = 90;

			delmarkedscreennodes(channellist, 1);
			struct skin* tmpnode = addlistbox(channellist, listbox, NULL, 2);
			if(tmpnode != NULL)
			{
				listbox->aktline = 1;
				listbox->aktpage = -1;

				status.screencalc = 2;
				drawscreen(channellist, 0);
				status.screencalc = 0;
				tmpstr = malloc(2);
				if(tmpstr != NULL)
				{
					snprintf(tmpstr, 2, "%c", tmpaktlist);
					changetext(tmpnode, tmpstr);
					free(tmpstr); tmpstr = NULL;
				}
				tmpnode->handle = (char*)tmpaktlist;
				tmpnode->handle1 = (char*)tmpaktlist;
				rcret = getrcconfigint("rcok", NULL);
				list = AZLIST;
			}
		}
		if(list == AZLIST && rcret == getrcconfigint("rcok", NULL))
		{
			if(listbox->select != NULL)
			{
				list = AZCHANNEL;
				tmpstr = ostrcat(tmpstr, _("Channel"), 0, 0);
				tmpstr = ostrcat(tmpstr, " - ", 1, 0);
				tmpstr = ostrcat(tmpstr, listbox->select->text, 1, 0);
				changetitle(channellist, tmpstr);
				free(tmpstr); tmpstr = NULL;
				tmpstr = ostrcat(tmpstr, "(A-Z)-", 0, 0);
				tmpstr = ostrcat(tmpstr, listbox->select->text, 1, 0);
				if(status.servicetype == 0)
					addconfigtmp("channellist", tmpstr);
				else
					addconfigtmp("rchannellist", tmpstr);
				free(tmpstr); tmpstr = NULL;
				aktlist = listbox->select->handle;
				showazchannel(channellist, listbox, (int)listbox->select->handle, flag);
				delmarkedscreennodes(channellist, 2);
				selectchannel(listbox);
				drawchannellist(channellist, list, listbox);
			}
			continue;
		}
		if(list == PROVIDERCHANNEL && (rcret == getrcconfigint("rcff", NULL) || rcret == getrcconfigint("rcfr", NULL))) 
		{
			struct provider* tmpaktlist = ((struct provider*)aktlist);

			while(tmpaktlist != NULL)
			{
				if(rcret == getrcconfigint("rcff", NULL))
					tmpaktlist = tmpaktlist->next;
				else if(rcret == getrcconfigint("rcfr", NULL))
					tmpaktlist = tmpaktlist->prev;
				if(tmpaktlist == NULL) break;

				delmarkedscreennodes(channellist, 1);
				struct skin* tmpnode = addlistbox(channellist, listbox, NULL, 2);
				if(tmpnode != NULL)
				{
					listbox->aktline = 1;
					listbox->aktpage = -1;

					status.screencalc = 2;
					drawscreen(channellist, 0);
					status.screencalc = 0;
					changetext(tmpnode, tmpaktlist->name);
					tmpnode->handle = (char*)tmpaktlist;
					tmpnode->handle1 = (char*)tmpaktlist;
					rcret = getrcconfigint("rcok", NULL);
					list = PROVIDERLIST;
					break;
				}
			}
		}
		if(list == PROVIDERLIST && rcret == getrcconfigint("rcok", NULL))
		{
			if(listbox->select != NULL)
			{
				list = PROVIDERCHANNEL;
				tmpstr = ostrcat(tmpstr, _("Provider"), 0, 0);
				tmpstr = ostrcat(tmpstr, " - ", 1, 0);
				tmpstr = ostrcat(tmpstr, listbox->select->text, 1, 0);
				changetitle(channellist, tmpstr);
				free(tmpstr); tmpstr = NULL;
				tmpstr = ostrcat(tmpstr, "(PROVIDER)-", 0, 0);
				tmpstr = ostrcat(tmpstr, listbox->select->text, 1, 0);
				if(status.servicetype == 0)
					addconfigtmp("channellist", tmpstr);
				else
					addconfigtmp("rchannellist", tmpstr);
				free(tmpstr); tmpstr = NULL;
				aktlist = listbox->select->handle;
				showproviderchannel(channellist, listbox, (struct provider*) listbox->select->handle, flag);
				delmarkedscreennodes(channellist, 2);
				selectchannel(listbox);
				drawchannellist(channellist, list, listbox);
			}
			continue;
		}
		if(flag == 0 && rcret == getrcconfigint("rcmenu", NULL))
		{
			if(list == AZLIST) continue;
			clearscreen(channellist);
			if(listbox->select != NULL)
				listmode = screenlistedit(list, (struct channel*)listbox->select->handle);
			else
				listmode = screenlistedit(list, NULL);
                                
			if(listmode == MVMODE)
			{
				delrc(getrcconfigint("rcright", NULL), channellist, listbox);
				delrc(getrcconfigint("rcleft", NULL), channellist, listbox);
			}
			else
				addscreenrc(channellist, listbox);
			delmarkedscreennodes(channellist, 1);
			delmarkedscreennodes(channellist, 2);
			recalclist(list, aktlist, listmode, channellist, listbox);
			selectchannel(listbox);
			drawscreen(channellist, 0);
			continue;
		}
		if(flag == 0 && rcret == getrcconfigint("rcepg", NULL) && (list == ALLCHANNEL || list == SATCHANNEL || list == PROVIDERCHANNEL || list == AZCHANNEL || list == BOUQUETCHANNEL))
		{
			if(listbox->select != NULL)
			{
				clearscreen(channellist);
				epgchoice((struct channel*)listbox->select->handle);
				drawscreen(channellist, 0);
			}
			continue;
		}
		if(flag == 0 && rcret == getrcconfigint("rcinfo", NULL))
		{
			if(status.servicetype == 0)
			{
				status.servicetype = 1;
				changetext(b7, _("TV (Info)"));
			}
			else
			{
				status.servicetype = 0;
				changetext(b7, _("Radio (Info)"));
			}
			newmodus = 1;
			break;
		}
	}

	status.markedchannel = NULL;
	status.markmodus = 0;
	status.screencalc = 0;
	delmarkedscreennodes(channellist, 1);
	delmarkedscreennodes(channellist, 2);
	delownerrc(channellist);
	delconfigtmp("channellist");
	delconfigtmp("rchannellist");
	clearscreen(channellist);
	if(newmodus == 1) goto start;
	status.channelswitch = 0;
	return ret;
}

#endif
