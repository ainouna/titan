#ifndef SCAN_H
#define SCAN_H

uint8_t changeservicetype(uint8_t type)
{
	int ret = type;

	switch(type)
	{
		case 0x01:
		case 0x11:
		case 0x19:
		case 0x9A:
		case 0x86:
		case 0xC3:
		case 0xC5:
		case 0xC6:
			ret = 0;
			break;
		case 0x02:
			ret = 1;
			break;
	}

	return ret;
}

int parsenit()
{
	int ret = 0, i;
	int secdone[255];
	int sectotal = -1;

	for(i = 0; i < 255; i++) secdone[i] = 0;

	unsigned char buf[MINMALLOC];

	// position in buffer
	unsigned short pos;
	unsigned short pos2;

	// network_information_section elements
	unsigned short seclen;
	unsigned short desclen;
	unsigned short tdesclen;
	unsigned short looplen;
	unsigned long transponderid;
	unsigned short onid;
	unsigned short nid;

	while(sectotal < buf[7])
	{
		seclen = ((buf[1] & 0x0F) << 8) + buf[2];
		nid = ((buf[3] << 8)| buf[4]);
		desclen = ((buf[8] & 0x0F) << 8) | buf[9];
		unsigned char secnr = buf[6];

//printf("[NIT] section %X last %X network_id 0x%x -> %s\n", secnum, buffer[7], network_id, secdone[secnum] ? "skip" : "use");

		if(secdone[secnr] == 1) continue;
		secdone[secnr] = 1;
		sectotal++;
		for(pos = 10; pos < desclen + 10; pos += buf[pos + 1] + 2)
		{
			looplen = ((buf[pos] & 0x0F) << 8) | buf[pos + 1];
			if (!looplen) continue;

			for(pos += 2; pos < seclen - 3; pos += tdesclen + 6)
			{
				transponderid = (buf[pos] << 8) | buf[pos + 1];
				onid = (buf[pos + 2] << 8) | buf[pos + 3];
				tdesclen = ((buf[pos + 4] & 0x0F) << 8) | buf[pos + 5];
				{
					for(pos2 = pos + 6; pos2 < pos + tdesclen + 6; pos2 += buf[pos2 + 1] + 2)
					{
						switch(buf[pos2])
						{
							case 0x41:
								//servicelistdesc(buffer + pos2, transponderid, onid);
								break;
							case 0x43:
								//if(satsystemdesc(buffer + pos2, transponderid, onid) < 0)
								{
									ret = -2;
									goto end;
								}
								break;
							case 0x44:
								//if(cablesystemdesc(buffer + pos2, transponderid, onid) < 0)
								{
									ret = -2;
									goto end;
								}
								break;
							case 0x5A:
								//if(terrestrialsystemdesc(buffer + pos2) < 0)
								{
									ret = -2;
									goto end;
								}
								break;
							case 0x62:
								//frequencylistdesc(buffer + pos2);
								break;

						}
					}
				}
			}
		}
	}

end:
	return ret;
}

int findchannel(unsigned char *buf, unsigned long transponderid, uint8_t* lastsecnr, struct skin* scan, struct skin* listbox)
{
	int ret = -1;
	struct skin* node = NULL;
	struct channel* chnode = NULL;

	// position in buffer
	unsigned short pos;
	unsigned short pos2;

	// service_description_section elements
	unsigned short seclen;
	uint8_t secnr;
	unsigned short onid, tid;
	unsigned short serviceid;
	unsigned short desclooplen;
	unsigned short running;
	int eitscheduleflag;
	int eitpresentfollowingflag;
	int camode;
	uint8_t servicetype = 0;
	uint8_t providerlen = 0;
	char* tmpstr = NULL, *tmpstr1 = NULL, *tmpstr2 = NULL;
	unsigned long* tmplong = NULL;

	seclen = ((buf[1] & 0x0F) << 8) | buf[2];
	secnr = buf[6];
	*lastsecnr = buf[7];
	tid = (buf[3] << 8) | buf[4];
	onid = (buf[8] << 8) | buf[9];
	
	transponderid = (onid << 16) | tid;
	debug(500, "SDT nr: %d, lastnr: %d, len: %d, tid: %d, onid: %d, transponderid: %lu", secnr, *lastsecnr, seclen, tid, onid, transponderid);

	for(pos = 11; pos < seclen - 1; pos += desclooplen + 5)
	{
		serviceid = (buf[pos] << 8) | buf[pos + 1];
		eitscheduleflag = buf[pos + 2] & 0x02;
		eitpresentfollowingflag = buf[pos + 2] & 0x01;
		running = buf[pos + 3] & 0xE0;
		camode = buf[pos + 3] & 0x10;
		desclooplen = ((buf[pos + 3] & 0x0F) << 8) | buf[pos + 4];
		if(scaninfo.onlyfree == 1 && camode > 0) continue;

		for(pos2 = pos + 5; pos2 < pos + desclooplen + 5; pos2 += buf[pos2 + 1] + 2)
		{
			switch(buf[pos2])
			{
				case 0x48:
					servicetype = buf[pos2 + 2];
					servicetype = changeservicetype(servicetype);
					providerlen = buf[pos2 + 3];
					tmpstr1 = strndup((char*)&(buf[pos2 + 4]), providerlen);
					tmpstr2 = strndup((char*)&(buf[pos2 + 4 + providerlen + 1]), (2 + buf[pos2 + 1]) - (4 + providerlen + 1));

					if(tmpstr1 == NULL || strlen(tmpstr1) == 0) tmpstr1 = ostrcat(tmpstr1, "unknown", 1, 0);
					if(tmpstr2 == NULL || strlen(tmpstr2) == 0) tmpstr2 = ostrcat(tmpstr2, "unknown", 1, 0);

					tmpstr1 = strutf8(tmpstr1, strlen(tmpstr1), 0, 0, 1, 0);
					tmpstr1 = stringreplacechar(tmpstr1, '#', '_');
					tmpstr2 = strutf8(tmpstr2, strlen(tmpstr2), 0, 0, 1, 1);
					tmpstr2 = stringreplacechar(tmpstr2, '#', '_');

					//add to listbox
					chnode = getchannel(serviceid, transponderid);
					node = addlistbox(scan, listbox, node, 1);
					if(node != NULL)
					{
						if(chnode != NULL)
							node->fontcol = convertcol("deaktivcol");
						switch(servicetype)
						{
							case 0:
								scaninfo.tvcount++;
								if(chnode == NULL) scaninfo.newtvcount++;
								break;
							case 1:
								scaninfo.radiocount++;
								if(chnode == NULL) scaninfo.newradiocount++;
								break;
							default:
								scaninfo.datacount++;
								if(chnode == NULL) scaninfo.newdatacount++;
								break;
						}
						tmpstr = ostrcat(tmpstr2, " (", 0, 0);
						tmpstr = ostrcat(tmpstr, tmpstr1, 0, 0);
						tmpstr = ostrcat(tmpstr, " - ", 0, 0);
						if(servicetype == 0)
							tmpstr = ostrcat(tmpstr, _("TV"), 0, 0);
						else if(servicetype == 1)
							tmpstr = ostrcat(tmpstr, _("Radio"), 0, 0);
						else
							tmpstr = ostrcat(tmpstr, _("Data"), 0, 0);
						tmpstr = ostrcat(tmpstr, ")", 0, 0);
						changetext(node, tmpstr);
						changeparam1(node, tmpstr1);
						changeparam2(node, tmpstr2);
						tmplong = (unsigned long*)malloc(sizeof(unsigned long) * 3);
						if(tmplong != NULL)
						{
							tmplong[0] = serviceid;
							tmplong[1] = transponderid;
							tmplong[2] = servicetype;
						}
						free(node->name);
						node->name = (char*)tmplong;
					}
					free(tmpstr); tmpstr = NULL;
					free(tmpstr1); tmpstr1 = NULL;
					free(tmpstr2); tmpstr2 = NULL;

					ret = 0;
					break;

			}
		}
	}

	return ret;
}

void doscan(struct stimerthread* timernode)
{
	int festatus = 0, secnr = 0;
	uint8_t lastsecnr = 0xff;
	unsigned char* buf = NULL;
	struct transponder* tpnode = NULL;
	struct dvbdev* fenode = NULL;
	struct channel* chnode = NULL;
	struct sat* satnode = sat;

	if(scaninfo.fenode == NULL || scaninfo.tpnode == NULL || timernode == NULL)
	{
		scaninfo.threadend = 1;
		return;
	}

	debug(500, "channel scan thread start");

	//sat loop
	satnode = sat;
	while(satnode != NULL && timernode->aktion == START)
	{
		if(satnode->scan == 0)
		{
			satnode = satnode->next;
			continue;
		}
		scaninfo.satname = satnode->name;

		if(scaninfo.scantype == 0)
			tpnode = scaninfo.tpnode;
		else if(scaninfo.scantype == 1)
			tpnode = transponder;
		else if(scaninfo.scantype == 2 || scaninfo.scantype == 3)
		{
			tpnode = transponder;
			scaninfo.orbitalpos = satnode->orbitalpos;
		}

		//transponder loop
		while(tpnode != NULL && timernode->aktion == START)
		{
			if(scaninfo.orbitalpos != tpnode->orbitalpos)
			{
				tpnode = tpnode->next;
				if(scaninfo.scantype == 0) break;
				continue;
			}
	
			fenode = fegetfree(tpnode, 0, scaninfo.fenode);
			if(fenode == NULL || (scaninfo.scantype != 3 && fenode != scaninfo.fenode))
			{
				scaninfo.tpcount++;
				tpnode = tpnode->next;
				debug(500, "Frontend for scan not free");
				if(scaninfo.scantype == 0) break;
				continue;
			}

			//frontend tune
			if(fenode->feinfo->type == FE_QPSK)
			{
				feset(fenode, tpnode);
				fetunedvbs(fenode, tpnode);
			}
			else if(fenode->feinfo->type == FE_QAM)
				fetunedvbc(fenode, tpnode);
			else if(fenode->feinfo->type == FE_OFDM)
				fetunedvbt(fenode, tpnode);
			else
			{
				scaninfo.tpcount++;
				tpnode = tpnode->next;
				debug(500, "Frontend type unknown");
				if(scaninfo.scantype == 0) break;
				continue;
			}

			festatus = fewait(fenode);
			if(debug_level == 200) fereadstatus(fenode);
			if(festatus != 0)
			{
				scaninfo.tpcount++;
				tpnode = tpnode->next;
				debug(500, "tuning failed");
				if(scaninfo.scantype == 0) break;
				continue;
			}

			//del channels from transponder if selected
			if(scaninfo.clear == 1)
			{
				chnode = channel;
				while(chnode != NULL)
				{
					if(chnode->transponder == tpnode)
						delchannel(chnode->serviceid, chnode->transponderid, 1);
					chnode = chnode->next;
				}
			}

			lastsecnr = 0xff;
			secnr = 0;
			while(secnr <= lastsecnr && secnr <= 256)
			{
				if(timernode->aktion != START) break;
				buf = dvbgetsdt(fenode, secnr, scaninfo.timeout);
				if(buf != NULL)
					findchannel(buf, tpnode->id, &lastsecnr, scaninfo.scanscreen, scaninfo.listbox);
				else
					break;
				free(buf); buf = NULL;
				secnr++;
			}
	
			scaninfo.tpcount++;
			if(scaninfo.scantype == 0) break;
			tpnode = tpnode->next;
		}
		if(scaninfo.scantype == 0 || scaninfo.scantype == 1) break;
		satnode = satnode->next;
	}
	scaninfo.threadend = 1;
	debug(500, "channel scan thread end");
}

void scanaddchannel(struct skin* node, int scantype, struct transponder* tp1)
{
	int serviceid = 0;
	unsigned long transponderid = 0;
	int servicetype = 0;
	int providerid = 0;
	struct provider* providernode = NULL;
	char* tmpstr = NULL;
	struct transponder* tp2 = NULL;

	if(node == NULL || node->name == NULL) return;

	serviceid = ((unsigned long*)node->name)[0];
	transponderid = ((unsigned long*)node->name)[1];
	servicetype = ((unsigned long*)node->name)[2];
	if(getchannel(serviceid, transponderid) == NULL)
	{
		//check if provider valid
		providernode = getproviderbyname(node->param1);
		if(providernode != NULL)
			providerid = providernode->providerid;
		else
		{
			providerid = getlastproviderid();
			tmpstr = ostrcat(tmpstr, oitoa(providerid), 1, 1);
			tmpstr = ostrcat(tmpstr, "#", 1, 0);
			tmpstr = ostrcat(tmpstr, node->param1, 1, 0);
			tmpstr = ostrcat(tmpstr, "#0", 1, 0);
			addprovider(tmpstr, 1, NULL);
			free(tmpstr); tmpstr = NULL;
		}

		//check if transponder valid
		if(scantype == 0)
		{
			tp2 = gettransponder(transponderid);
			if(tp2 == NULL) //create new transponder
				copytransponder(tp1, tp2, transponderid);
			else //change transponder
				copytransponder(tp1, tp2, 0);
		}

		if(servicetype != 0 && servicetype != 1)
			servicetype = 0;

		if(createchannel(node->param2, transponderid, providerid, serviceid, servicetype, 0, -1, -1, -1, -1, 0) != NULL)
			node->fontcol = convertcol("deaktivcol");
	}
	else
		node->fontcol = convertcol("deaktivcol");
}

void scansetallsat(int fetype)
{
	struct sat* satnode = sat;

	while(satnode != NULL)
	{
		if(fetype == satnode->fetype) satnode->scan = 1;
		satnode = satnode->next;
	}
}

void scansetmultisat(struct skin* scan)
{
	struct skin* node = scan;
	struct sat* satnode = NULL;

	while(node != NULL && node->name != NULL)
	{
		if(ostrcmp(node->ret, "1") == 0)
		{
			satnode = getsatbyorbitalpos(atoi(node->name));
			if(satnode != NULL)
				satnode->scan = 1;
		}
		node = node->next;
	}
}

void screenscan(struct skin* mscan, char* tuner, int scantype, int orbitalpos, int frequency, int inversion, int symbolrate, int polarization, int fec, int modulation, int rolloff, int pilot, int networkscan, int onlyfree, int clear, int system, int timeout)
{
	int rcret = 0, tpmax = 0, i = 0;
	struct skin* scan = getscreen("scan");
	struct skin* progress = getscreennode(scan, "progress");
	struct skin* listbox = getscreennode(scan, "listbox");
	struct skin* satname = getscreennode(scan, "satname");
	struct skin* tpcount = getscreennode(scan, "tpcount");
	struct skin* foundtv = getscreennode(scan, "foundtv");
	struct skin* foundradio = getscreennode(scan, "foundradio");
	struct skin* founddata = getscreennode(scan, "founddata");
	struct transponder* tpnode = NULL;
	struct dvbdev* fenode = NULL, *dvbnode = dvbdev;
	struct stimerthread* timernode = NULL;
	struct sat* satnode = NULL;
	char* tmpstr = NULL;

	listbox->aktline = 1;
	listbox->aktpage = -1;
	progress->progresssize = 0;
	memset(&scaninfo, 0, sizeof(struct scaninfo));

	addscreenrc(scan, listbox);
	if(scantype != 3)
	{
		while(dvbnode != NULL)
		{
			if(dvbnode->type == FRONTENDDEV && dvbnode->feinfo != NULL && dvbnode->felock < 1 && dvbnode->feinfo->type == FE_QPSK)
			{
				if(ostrcmp(dvbnode->feshortname, tuner) == 0)
				{
					fenode = dvbnode;
					break;
				}
			}
			dvbnode = dvbnode->next;
		}
		if(fenode == NULL) return;
	}
	else if(scantype == 3)
		fenode = dvbdev;

	if(scantype == 0) //single transponder
	{
		if(fenode != NULL && fenode->feinfo != NULL)
		{
			debug(500, "fetype=%d, orbitalpos=%d, frequ=%d, inverion=%d, symbolrate=%d, polarization=%d, fec=%d, modulation=%d, rolloff=%d, pilot=%d, system=%d", fenode->feinfo->type, orbitalpos, frequency, inversion, symbolrate, polarization, fec, modulation, rolloff, pilot, system);
			tpnode = createtransponder(0, fenode->feinfo->type, orbitalpos, frequency, inversion, symbolrate, polarization, fec, modulation, rolloff, pilot, system);
		}
		if(tpnode != NULL)
		{
			tpmax = 1;
			satnode = getsatbyorbitalpos(tpnode->orbitalpos);
			if(satnode != NULL) satnode->scan = 1;
		}
	}
	else if(scantype == 1) //single sat
	{
		tpnode = transponder;
		while(tpnode != NULL)
		{
			if(tpnode->orbitalpos == orbitalpos)
				tpmax++;
			tpnode = tpnode->next;
		}
		tpnode = transponder;
		satnode = getsatbyorbitalpos(orbitalpos);
		if(satnode != NULL) satnode->scan = 1;
	}
	else if(scantype == 2 || scantype == 3) //2: multi sat, 3: all
	{
		if(scantype == 2) scansetmultisat(mscan);
		if(scantype == 3) scansetallsat(FE_QPSK);
		satnode = sat;
		while(satnode != NULL)
		{
			if(satnode->scan != 0)
			{
				tpnode = transponder;
				while(tpnode != NULL)
				{
					if(tpnode->orbitalpos == satnode->orbitalpos)
						tpmax++;
					tpnode = tpnode->next;
				}
			}
			satnode = satnode->next;
		}
		tpnode = transponder;
	}

	scaninfo.fenode = fenode;
	scaninfo.tpnode = tpnode;
	scaninfo.scanscreen = scan;
	scaninfo.listbox = listbox;
	scaninfo.timeout = timeout;
	scaninfo.scantype = scantype;
	scaninfo.orbitalpos = orbitalpos;
	scaninfo.onlyfree = onlyfree;
	scaninfo.clear = clear;
	timernode = addtimer(&doscan, START, 1000, 1, NULL, NULL, NULL);

	while(1)
	{
		//satname
		tmpstr = ostrcat(tmpstr, _("Sat: "), 1, 0);
		tmpstr = ostrcat(tmpstr, scaninfo.satname, 1, 0);
		changetext(satname, tmpstr);
		free(tmpstr); tmpstr = NULL;

		//transpondercount
		tmpstr = ostrcat(tmpstr, _("Transponder: "), 1, 0);
		tmpstr = ostrcat(tmpstr, oitoa(scaninfo.tpcount), 1, 1);
		tmpstr = ostrcat(tmpstr, " / ", 1, 0);
		tmpstr = ostrcat(tmpstr, oitoa(tpmax), 1, 1);
		changetext(tpcount, tmpstr);
		free(tmpstr); tmpstr = NULL;

		//tvcount
		tmpstr = ostrcat(tmpstr, _("TV: "), 1, 0);
		tmpstr = ostrcat(tmpstr, oitoa(scaninfo.newtvcount), 1, 1);
		tmpstr = ostrcat(tmpstr, " / ", 1, 0);
		tmpstr = ostrcat(tmpstr, oitoa(scaninfo.tvcount), 1, 1);
		changetext(foundtv, tmpstr);
		free(tmpstr); tmpstr = NULL;

		//radiocount
		tmpstr = ostrcat(tmpstr, _("Radio: "), 1, 0);
		tmpstr = ostrcat(tmpstr, oitoa(scaninfo.newradiocount), 1, 1);
		tmpstr = ostrcat(tmpstr, " / ", 1, 0);
		tmpstr = ostrcat(tmpstr, oitoa(scaninfo.radiocount), 1, 1);
		changetext(foundradio, tmpstr);
		free(tmpstr); tmpstr = NULL;

		//datacount
		tmpstr = ostrcat(tmpstr, _("Data: "), 1, 0);
		tmpstr = ostrcat(tmpstr, oitoa(scaninfo.newdatacount), 1, 1);
		tmpstr = ostrcat(tmpstr, " / ", 1, 0);
		tmpstr = ostrcat(tmpstr, oitoa(scaninfo.datacount), 1, 1);
		changetext(founddata, tmpstr);
		free(tmpstr); tmpstr = NULL;

		if(tpmax == 0)
			progress->progresssize = 100;
		else
			progress->progresssize = (100 / (float)tpmax) * scaninfo.tpcount;

		drawscreen(scan, 0);
		rcret = waitrc(scan, 1000, 0);
		if(rcret == getrcconfigint("rcexit", NULL))
		{
			if(timernode != NULL && scaninfo.threadend == 0)
			{
				timernode->aktion = STOP;
				while(i < 3 && scaninfo.threadend == 0)
				{
					textbox(_("Message"), _("Wait for channel scan end"), NULL, 0, NULL, 0, NULL, 0, NULL, 0, 600, 200, 3, 0);
					i++;
				}
			}
			break;
		}
		if(rcret == getrcconfigint("rcok", NULL))
			scanaddchannel(listbox->select, scantype, tpnode);
		if(rcret == getrcconfigint("rcred", NULL))
		{
			struct skin* lnode = listbox;
			long deaktivcol = convertcol("deaktivcol");
			while(lnode != NULL)
			{
				if(lnode->fontcol != deaktivcol && lnode->del == 1)
					scanaddchannel(lnode, scantype, tpnode);
				lnode = lnode->next;
			}
		}
	}

	if(scantype == 0) deltransponder(0);
	if(clear == 1) delunusedbouquetchannels();
	delmarkedscreennodes(scan, 1);
	delownerrc(scan);
	clearscreen(scan);
	resetsatscan();
}

void changescantype(char* scantype, struct skin* scan, struct skin* listbox, struct skin* tuner, struct skin* satellite, struct skin* system, struct skin* frequency, struct skin* inversion, struct skin* symbolrate, struct skin* polarization, struct skin* fec, struct skin* modulation, struct skin* rolloff, struct skin* pilot)
{
	struct sat* satnode = sat;
	struct skin* tmp = NULL;
	char* tmpstr = NULL, *tmpstr1 = NULL, *tmpnr = NULL;
	char* feshortname = NULL;
	int i, orbitalpos = 0;

	if(tuner != NULL) feshortname = tuner->ret;

	tuner->hidden = NO;
	system->hidden = NO;
	frequency->hidden = NO;
	inversion->hidden = NO;
	symbolrate->hidden = NO;
	polarization->hidden = NO;
	fec->hidden = NO;
	modulation->hidden = NO;
	rolloff->hidden = NO;
	pilot->hidden = NO;
	satellite->hidden = NO;
	delmarkedscreennodes(scan, 1);

	if(ostrcmp(scantype, "1") == 0 || ostrcmp(scantype, "2") == 0 || ostrcmp(scantype, "3") == 0)
	{
		system->hidden = YES;
		frequency->hidden = YES;
		inversion->hidden = YES;
		symbolrate->hidden = YES;
		polarization->hidden = YES;
		fec->hidden = YES;
		modulation->hidden = YES;
		rolloff->hidden = YES;
		pilot->hidden = YES;
	}
	if(ostrcmp(scantype, "2") == 0 && feshortname != NULL)
	{
		satellite->hidden = YES;

		tmpstr = ostrcat(feshortname, "_sat", 0, 0);
		for(i = 1; i <= status.maxsat; i++)
		{
			tmpnr = oitoa(i);
			orbitalpos = getconfigint(tmpstr, tmpnr);
			free(tmpnr); tmpnr = NULL;

			satnode = getsatbyorbitalpos(orbitalpos);
			if(satnode != NULL && satnode->fetype == FE_QPSK)
			{
				tmp = addlistbox(scan, listbox, tmp, 1);
				if(tmp != NULL)
				{
					tmpstr1 = oitoa(satnode->orbitalpos);
					changename(tmp, tmpstr1);
					free(tmpstr1); tmpstr1 = NULL;
					changetext(tmp, satnode->name);
					tmp->type = CHOICEBOX;
					addchoicebox(tmp, "0", _("no"));
					addchoicebox(tmp, "1", _("yes"));
				}
			}
		}
		free(tmpstr); tmpstr = NULL;
	}
	if(ostrcmp(scantype, "3") == 0)
	{
		tuner->hidden = YES;
		satellite->hidden = YES;
	}
}

void scanchangesat(struct skin* sat, struct transponder* tpnode, char* feshortname)
{
	char* tmpstr = NULL;

	changeinput(sat, NULL);
	changechoiceboxvalue(sat, NULL);
	tmpstr = getsatstring(feshortname);
	changeinput(sat, tmpstr);
	free(tmpstr); tmpstr = NULL;
	tmpstr = getorbitalposstring(feshortname);
	changechoiceboxvalue(sat, tmpstr);
	free(tmpstr); tmpstr = NULL;
	if(tpnode != NULL)
	{
		tmpstr = oitoa(tpnode->orbitalpos);
		setchoiceboxselection(sat, tmpstr);
		free(tmpstr); tmpstr = NULL;
	}
}

//flag 0: manual scan
//flag 1: auto scan
void screenscanconfig(int flag)
{
	int rcret = 0;
	int iscantype = -1, isat = -1, ifrequency = -1;
	int iinversion = -1, isymbolrate = -1, ipolarization = -1;
	int ifec = -1, imodulation = -1, irolloff = -1, ipilot = -1;
	int isystem = -1, inetworkscan = -1, ionlyfree = -1, iclear = -1;
	int i = 0, treffer = 0, tunercount = 0;
	struct skin* scan = getscreen("manualscan");
	struct skin* listbox = getscreennode(scan, "listbox");
	struct skin* tuner = getscreennode(scan, "tuner");
	struct skin* scantype = getscreennode(scan, "scantype");
	struct skin* sat = getscreennode(scan, "sat");
	struct skin* system = getscreennode(scan, "system");
	struct skin* frequency = getscreennode(scan, "frequency");
	struct skin* inversion = getscreennode(scan, "inversion");
	struct skin* symbolrate = getscreennode(scan, "symbolrate");
	struct skin* polarization = getscreennode(scan, "polarization");
	struct skin* fec = getscreennode(scan, "fec");
	struct skin* modulation = getscreennode(scan, "modulation");
	struct skin* rolloff = getscreennode(scan, "rolloff");
	struct skin* pilot = getscreennode(scan, "pilot");
	struct skin* networkscan = getscreennode(scan, "networkscan");
	struct skin* clear = getscreennode(scan, "clear");
	struct skin* onlyfree = getscreennode(scan, "onlyfree");
	struct skin* tmp = NULL;
	char* tmpstr = NULL, *tmpnr = NULL, *feshortname = NULL;
	struct transponder* tpnode = NULL;
	struct dvbdev* dvbnode = dvbdev;

	if(status.recording > 0 || status.streaming > 0)
	{
		textbox(_("Message"), _("Scan is not allowed if record\nor stream is running !"), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, NULL, 0, 600, 200, 0, 0);
		return;
	}

	//del old values
	changeinput(tuner, NULL);
	changechoiceboxvalue(tuner, NULL);
	changeinput(scantype, NULL);
	changechoiceboxvalue(scantype, NULL);
	changeinput(sat, NULL);
	changechoiceboxvalue(sat, NULL);
	changeinput(system, NULL);
	changechoiceboxvalue(system, NULL);
	changeinput(inversion, NULL);
	changechoiceboxvalue(inversion, NULL);
	changeinput(polarization, NULL);
	changechoiceboxvalue(polarization, NULL);
	changeinput(fec, NULL);
	changechoiceboxvalue(fec, NULL);
	changeinput(modulation, NULL);
	changechoiceboxvalue(modulation, NULL);
	changeinput(rolloff, NULL);
	changechoiceboxvalue(rolloff, NULL);
	changeinput(pilot, NULL);
	changechoiceboxvalue(pilot, NULL);

	//tuner
	while(dvbnode != NULL)
	{
		if(dvbnode->type == FRONTENDDEV && dvbnode->feinfo != NULL && dvbnode->felock < 1 && dvbnode->feinfo->type == FE_QPSK)
		{
			treffer = 0;
			if(dvbnode->feshortname != NULL)
			{
				tmpstr = ostrcat(dvbnode->feshortname, "_sat", 0, 0);
				for(i = 1; i <= status.maxsat; i++)
				{
					tmpnr = oitoa(i);
					if(getconfigint(tmpstr, tmpnr) != 0)
					{
						treffer = 1;
						break;
					}
					free(tmpnr); tmpnr = NULL;
				}
				free(tmpnr); tmpnr = NULL;
				free(tmpstr); tmpstr = NULL;
			}

			if(treffer == 0)
			{
				dvbnode = dvbnode->next;
				continue;
			}

			tunercount++;
			if(feshortname == NULL) feshortname = dvbnode->feshortname;

			tmpstr = ostrcat(tmpstr, dvbnode->feinfo->name, 1, 0);
			tmpstr = ostrcat(tmpstr, " (", 1, 0);
			tmpstr = ostrcat(tmpstr, oitoa(dvbnode->adapter), 1, 1);
			tmpstr = ostrcat(tmpstr, "/", 1, 0);
			tmpstr = ostrcat(tmpstr, oitoa(dvbnode->devnr), 1, 1);
			tmpstr = ostrcat(tmpstr, ")", 1, 0);
			addchoicebox(tuner, dvbnode->feshortname, tmpstr);
			free(tmpstr); tmpstr = NULL;
		}
		dvbnode = dvbnode->next;
	}

	if(tunercount < 1)
	{
		textbox(_("Message"), _("No Tuner configured"), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, NULL, 0, 600, 200, 0, 0);
		return;
	}

	rcret = servicestop(status.aktservice, 1, 1);
        if(rcret == 1) return;

	if(status.aktservice->channel != NULL)
		tpnode = status.aktservice->channel->transponder;


	if(flag == 0)
	{
		addchoicebox(scantype, "0", _("Single Transponder"));
		addchoicebox(scantype, "1", _("Single Sat"));
		addchoicebox(scantype, "2", _("Multi Sat"));
		setchoiceboxselection(scantype, "0");
	}
	else
	{
		addchoicebox(scantype, "3", _("Auto Scan"));
		setchoiceboxselection(scantype, "3");
	}

	changescantype(scantype->ret, scan, listbox, tuner, sat, system, frequency, inversion, symbolrate, polarization, fec, modulation, rolloff, pilot);

	//sat
	scanchangesat(sat, tpnode, feshortname);

	//system
	tmpstr = transpondergetsystemstr(tpnode, 1);
	changeinput(system, tmpstr);
	free(tmpstr); tmpstr = NULL;
	tmpstr = transpondergetsystemstr(tpnode, 2);
	changechoiceboxvalue(system, tmpstr);
	free(tmpstr); tmpstr = NULL;
	if(tpnode != NULL)
	{
		tmpstr = oitoa(tpnode->system);
		setchoiceboxselection(system, tmpstr);
		free(tmpstr); tmpstr = NULL;
	}

	//frequency
	if(tpnode != NULL)
		tmpstr = oitoa(tpnode->frequency / 1000);
	else
		tmpstr = ostrcat(tmpstr, "00000", 1, 0);
	changemask(frequency, "00000");
	changeinput(frequency, tmpstr);
	free(tmpstr); tmpstr = NULL;

	//inversion
	tmpstr = transpondergetinversionstr(tpnode, 1);
	changeinput(inversion, tmpstr);
	free(tmpstr); tmpstr = NULL;
	tmpstr = transpondergetinversionstr(tpnode, 2);
	changechoiceboxvalue(inversion, tmpstr);
	free(tmpstr); tmpstr = NULL;
	if(tpnode != NULL)
	{
		tmpstr = oitoa(tpnode->inversion);
		setchoiceboxselection(inversion, tmpstr);
		free(tmpstr); tmpstr = NULL;
	}

	//symbolrate
	if(tpnode != NULL)
		tmpstr = oitoa(tpnode->symbolrate / 1000);
	else
		tmpstr = ostrcat(tmpstr, "00000", 1, 0);
	changemask(symbolrate, "00000");
	changeinput(symbolrate, tmpstr);
	free(tmpstr); tmpstr = NULL;

	//polarization
	tmpstr = transpondergetpolarizationstr(tpnode, 1);
	changeinput(polarization, tmpstr);
	free(tmpstr); tmpstr = NULL;
	tmpstr = transpondergetpolarizationstr(tpnode, 2);
	changechoiceboxvalue(polarization, tmpstr);
	free(tmpstr); tmpstr = NULL;
	if(tpnode != NULL)
	{
		tmpstr = oitoa(tpnode->polarization);
		setchoiceboxselection(polarization, tmpstr);
		free(tmpstr); tmpstr = NULL;
	}

	//fec
	tmpstr = transpondergetfecstr(tpnode, 1);
	changeinput(fec, tmpstr);
	free(tmpstr); tmpstr = NULL;
	tmpstr = transpondergetfecstr(tpnode, 2);
	changechoiceboxvalue(fec, tmpstr);
	free(tmpstr); tmpstr = NULL;
	if(tpnode != NULL)
	{
		tmpstr = oitoa(tpnode->fec);
		setchoiceboxselection(fec, tmpstr);
		free(tmpstr); tmpstr = NULL;
	}

	//modulation
	tmpstr = transpondergetmodulationstr(tpnode, 1);
	changeinput(modulation, tmpstr);
	free(tmpstr); tmpstr = NULL;
	tmpstr = transpondergetmodulationstr(tpnode, 2);
	changechoiceboxvalue(modulation, tmpstr);
	free(tmpstr); tmpstr = NULL;
	if(tpnode != NULL)
	{
		tmpstr = oitoa(tpnode->modulation);
		setchoiceboxselection(modulation, tmpstr);
		free(tmpstr); tmpstr = NULL;
	}

	//rolloff
	tmpstr = transpondergetrolloffstr(tpnode, 1);
	changeinput(rolloff, tmpstr);
	free(tmpstr); tmpstr = NULL;
	tmpstr = transpondergetrolloffstr(tpnode, 2);
	changechoiceboxvalue(rolloff, tmpstr);
	free(tmpstr); tmpstr = NULL;
	if(tpnode != NULL)
	{
		tmpstr = oitoa(tpnode->rolloff);
		setchoiceboxselection(rolloff, tmpstr);
		free(tmpstr); tmpstr = NULL;
	}

	//pilot
	tmpstr = transpondergetpilotstr(tpnode, 1);
	changeinput(pilot, tmpstr);
	free(tmpstr); tmpstr = NULL;
	tmpstr = transpondergetpilotstr(tpnode, 2);
	changechoiceboxvalue(pilot, tmpstr);
	free(tmpstr); tmpstr = NULL;
	if(tpnode != NULL)
	{
		tmpstr = oitoa(tpnode->pilot);
		setchoiceboxselection(pilot, tmpstr);
		free(tmpstr); tmpstr = NULL;
	}

	//networkscan
	addchoicebox(networkscan, "0", _("no"));
	addchoicebox(networkscan, "1", _("yes"));

	//clear
	addchoicebox(clear, "0", _("no"));
	addchoicebox(clear, "1", _("yes"));

	//only free
	addchoicebox(onlyfree, "0", _("no"));
	addchoicebox(onlyfree, "1", _("yes"));

	drawscreen(scan, 0);
	addscreenrc(scan, listbox);

	tmp = listbox->select;
	while(1)
	{
		addscreenrc(scan, tmp);
		rcret = waitrc(scan, 0, 0);
		tmp = listbox->select;

		if(scantype->ret != NULL) iscantype = atoi(scantype->ret);
		if(sat->ret != NULL) isat = atoi(sat->ret);
		if(system->ret != NULL) isystem = atoi(system->ret);
		if(frequency->ret != NULL) ifrequency = atoi(frequency->ret) * 1000;
		if(inversion->ret != NULL) iinversion = atoi(inversion->ret);
		if(symbolrate->ret != NULL) isymbolrate = atoi(symbolrate->ret) * 1000;
		if(polarization->ret != NULL) ipolarization = atoi(polarization->ret);
		if(fec->ret != NULL) ifec = atoi(fec->ret);
		if(modulation->ret != NULL) imodulation = atoi(modulation->ret);
		if(rolloff->ret != NULL) irolloff = atoi(rolloff->ret);
		if(pilot->ret != NULL) ipilot = atoi(pilot->ret);
		if(networkscan->ret != NULL) inetworkscan = atoi(networkscan->ret);
		if(onlyfree->ret != NULL) ionlyfree = atoi(onlyfree->ret);
		if(clear->ret != NULL) iclear = atoi(clear->ret);

		if(rcret == getrcconfigint("rcexit", NULL)) break;
		if(listbox->select != NULL && ostrcmp(listbox->select->name, "tuner") == 0)
		{
			scanchangesat(sat, tpnode, listbox->select->ret);
			changescantype(scantype->ret, scan, listbox, tuner, sat, system, frequency, inversion, symbolrate, polarization, fec, modulation, rolloff, pilot);
			drawscreen(scan, 0);
		}
		if(listbox->select != NULL && ostrcmp(listbox->select->name, "scantype") == 0)
		{
			changescantype(scantype->ret, scan, listbox, tuner, sat, system, frequency, inversion, symbolrate, polarization, fec, modulation, rolloff, pilot);
			drawscreen(scan, 0);

		}
		if(rcret == getrcconfigint("rcok", NULL))
		{
			clearscreen(scan);
			screenscan(scan->child, tuner->ret, iscantype, isat, ifrequency, iinversion, isymbolrate, ipolarization, ifec, imodulation, irolloff, ipilot, inetworkscan, ionlyfree, iclear, isystem, 5000000);
			drawscreen(scan, 0);
		}
		if(rcret == getrcconfigint("rcred", NULL) && tpnode != NULL && iscantype == 0)
		{
			struct transponder* tp1 = createtransponder(0, tpnode->fetype, isat, ifrequency, iinversion, isymbolrate, ipolarization, ifec, imodulation, irolloff, ipilot, isystem);
			copytransponder(tp1, tpnode, 0);
			deltransponder(0);
			textbox(_("Message"), _("Transponder changed"), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, NULL, 0, 600, 200, 5, 0);
			drawscreen(scan, 0);
		}
	}

	delmarkedscreennodes(scan, 1);
	delownerrc(scan);
	clearscreen(scan);
	resetsatscan();
	tmpstr = ostrcat(status.lastservice->channellist, NULL, 0, 0);
	servicecheckret(servicestart(status.lastservice->channel, tmpstr, NULL, 0), 0);
	free(tmpstr); tmpstr = NULL;
}

#endif
