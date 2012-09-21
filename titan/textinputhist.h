#ifndef TEXTINPUTHIST_H
#define TEXTINPUTHIST_H

void savehistory(char* text, char* histname)
{
	int count = 0, i = 0;
	char* tmpstr = NULL, *tmpstr1 = NULL;
	struct splitstr* ret = NULL;

	if(histname == NULL || text == NULL || strlen(text) == 0) return;
	if(ostrstr(getconfig(histname, NULL), text) != NULL) return;

	tmpstr = ostrcat(text, "#", 0, 0);
	tmpstr = ostrcat(tmpstr, getconfig(histname, NULL), 1, 0);
	ret = strsplit(tmpstr, "#", &count);

	if(ret != NULL)
	{
		for(i = 0; i < count; i++)
		{
			tmpstr1 = ostrcat(tmpstr1, ret[i].part, 1, 0);
			tmpstr1 = ostrcat(tmpstr1, "#", 1, 0);
			if(i > 8) break;
		}
	}

	if(tmpstr1 != NULL && strlen(tmpstr1) > 0)
		tmpstr1[strlen(tmpstr1) - 1] = '\0';
	addconfig(histname, tmpstr1);

	free(ret); ret = NULL;
	free(tmpstr); tmpstr = NULL;
	free(tmpstr1); tmpstr1 = NULL;
}

void readhistory(struct skin* textinputhist, struct skin* listbox, char* histname)
{
	int count = 0, i = 0;
	char* tmpstr = NULL;
	struct skin* tmp = NULL;
	struct splitstr* ret = NULL;

	if(histname == NULL) return;

	listbox->aktline = 1;
	listbox->aktpage = -1;

	tmpstr = ostrcat(getconfig(histname, NULL), NULL, 0, 0);
	ret = strsplit(tmpstr, "#", &count);

	if(ret != NULL)
	{
		for(i = 0; i < count; i++)
		{
			tmp = addlistbox(textinputhist, listbox, tmp, 1);
			if(tmp != NULL)
			{
				changetext(tmp, ret[i].part);
				changename(tmp, ret[i].part);
			}
		}
	}

	free(ret); ret = NULL;
	free(tmpstr); tmpstr = NULL;
}

char* textinputhist(char* title, char* text, char* histname)
{
	debug(1000, "in");
	int rcret = -1, fromthread = 0, height = 0;
	struct skin* textinputhist = getscreen("textinputhist");
	struct skin* listbox = getscreennode(textinputhist, "listbox");
	struct skin* input = getscreennode(textinputhist, "input");
	struct skin* framebuffer = getscreen("framebuffer");
	char* ret = NULL, *bg = NULL;

	if(pthread_self() != status.mainthread)
	fromthread = 1;

	changetitle(textinputhist, title);
	height = textinputhist->height;
	if(title != NULL)
		textinputhist->height += textinputhist->fontsize + 6 + (textinputhist->bordersize * 2);
	changeinput(input, text);
	readhistory(textinputhist, listbox, histname);

	if(fromthread == 1)
	{
		m_lock(&status.drawingmutex, 0);
		m_lock(&status.rcmutex, 10);
		setnodeattr(textinputhist, framebuffer, 2);
		status.rcowner = textinputhist;
		bg = savescreen(textinputhist);
		drawscreen(textinputhist, 0, 2);
	}
	else
		drawscreen(textinputhist, 0, 0);


	addrc(getrcconfigint("rcup", NULL), listboxup, textinputhist, listbox);
	addrc(getrcconfigint("rcdown", NULL), listboxdown, textinputhist, listbox);
	addscreenrc(textinputhist, input);

	while(1)
	{
		rcret = waitrc(textinputhist, 0, 0);
		if(rcret == getrcconfigint("rcexit", NULL)) break;
		if(rcret == getrcconfigint("rcok", NULL))
		{
			ret = ostrcat(input->input, NULL, 0, 0);
			savehistory(ret, histname);
			break;
		}
		if(rcret == getrcconfigint("rcred", NULL) && listbox->select != NULL)
		{
			changeinput(input, listbox->select->name);
			if(fromthread == 1)
				drawscreen(textinputhist, 0, 2);
			else
				drawscreen(textinputhist, 0, 0);
		}
	}

	delownerrc(textinputhist);
	delrc(getrcconfigint("rcup", NULL), textinputhist, listbox);
	delrc(getrcconfigint("rcdown", NULL), textinputhist, listbox);

	if(fromthread == 1)
	{
		clearscreennolock(textinputhist);
		restorescreen(bg, textinputhist);
		blitfb(0);
		sleep(1);
		status.rcowner = NULL;
		m_unlock(&status.rcmutex, 3);
		m_unlock(&status.drawingmutex, 0);
	}
	else
	{
		clearscreen(textinputhist);
		drawscreen(skin, 0, 0);
	}

	textinputhist->height = height;
	delmarkedscreennodes(textinputhist, 1);
	debug(1000, "out");
	return ret;
}

#endif
