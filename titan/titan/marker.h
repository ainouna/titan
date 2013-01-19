#ifndef MARKER_H
#define MARKER_H

struct marker* addmarkernode(off64_t pos)
{
	
	struct marker *newnode = NULL;
	struct marker *oldnode = NULL;
	struct marker *prev = NULL;
	
	newnode = (struct marker*)calloc(1, sizeof(struct marker));
	if(newnode == NULL)
	{
		err("no memory");
		return NULL;
	}
	newnode->next = NULL;
	newnode->pos = pos;
	
	if(status.playmarker == NULL)
	{
		newnode->prev = NULL;
		status.playmarker = newnode;
		return newnode;
	}
	
	oldnode = status.playmarker;
	if(pos == -1)
	{
		while(oldnode->next != NULL)
		{
			oldnode = oldnode->next;
		}
		oldnode->next = newnode;
		newnode->prev = oldnode;
	}
	else
	{
		prev = oldnode;
		while(oldnode->next != NULL && oldnode->pos < pos)
		{
			prev = oldnode;
			oldnode = oldnode->next;
		}
		
		if(oldnode->prev == NULL)
		{
			if(oldnode->pos > pos)
			{
				newnode->next = oldnode;
				newnode->prev = NULL;
				oldnode->prev = newnode;
				status.playmarker = newnode;
				return newnode;
			}
		}
		if(oldnode->next == NULL)
		{
			if(oldnode->pos < pos)
			{
				newnode->next = NULL;
				newnode->prev = oldnode;
				oldnode->next = newnode;
				return newnode;
			}
		}
		newnode->next = prev->next;
		newnode->prev = prev;
		if(prev->next != NULL)
				prev->next->prev = newnode;
		prev->next = newnode;
	}
	
	return newnode;
}


int delmarkernode(off64_t pos)
{
	if(status.playmarker == NULL)
		return -1;
	
	struct marker* delnode;
	struct marker* node = status.playmarker;
	
	delnode = node;
	while(delnode != NULL)
	{
		node = delnode->next;
		if(pos == -1)
		{
			free(delnode->timetext);
			free(delnode);
			status.playmarker = NULL;
		}
		else if(delnode->pos == pos)
		{
			if(delnode->next != NULL)
				delnode->next->prev = delnode->prev;
			if(delnode->prev == NULL)
				status.playmarker = delnode->next;
			else
				delnode->prev->next = delnode->next;
			free(delnode->timetext);
			free(delnode);
			break;
		}
		delnode = node;
	}
	return 0;
}

int getmarker(char* dateiname)
{
	off64_t pos;
	off64_t time;
	struct marker *node = NULL;
	
	FILE* datei = fopen(dateiname, "r");
	if(datei!= NULL)
	{
		while (!feof(datei))
		{
			fscanf(datei, "%lld,%lld", &pos,&time);
			node = addmarkernode(pos);
			if(node == NULL)
				return -1;
			node->time = time;
			node->pos = pos;
			node->timetext = convert_timesec(time);
		}
		fclose(datei);
	}
	else 
		return -1;
		
	return 0;
}

int putmarker(char* dateiname)
{
	struct marker *node = NULL;
	
	if(status.playmarker == NULL)
		return -1;	
	else
		node = status.playmarker;
	
	FILE* datei = fopen(dateiname, "w");
	if(datei!= NULL)
	{
		while(node->next != NULL)
		{
			fprintf(datei, "%lld,%lld,", node->pos, node->time);
			node = node->next;
		}
		fprintf(datei, "%lld,%lld", node->pos, node->time);	
		fclose(datei);
	}
	else 
		return -1;

	return 0;
}

int setmarker()
{
	unsigned long long atime = 0, len = 0, startpos = 0;
	struct marker *node = NULL;
	
	struct service* snode = getservice(RECORDPLAY, 0);
	off64_t pos = lseek64(snode->recsrcfd, 0, SEEK_CUR);
	node = addmarkernode(pos);
	if(node != NULL)
	{
		node->pos = pos;
		playergetinfots(&len, &startpos, NULL, &atime, NULL, 0);
		atime = (atime - startpos) / 90000;
		node->time = atime;
		node->timetext = convert_timesec(atime);
	}
	else
		return -1;
	
	return 0;
}

void screenmarker()
{
	int rcret;
	
	struct skin* screen1 = getscreen("marker");
	struct skin* listbox = getscreennode(screen1, "listbox");
	struct skin* dummy = getscreennode(screen1, "dummy");
	struct skin* tmp = NULL;
	
	struct marker *marker = NULL;
	
	marker = status.playmarker;
	while(marker != NULL)
	{
		tmp = addlistbox(screen1, listbox, tmp, 1);
		if(tmp != NULL)
		{
			tmp->textposx = dummy->textposx;
			tmp->prozposx = dummy->prozposx;
			tmp->prozposy = dummy->prozposy;
			tmp->height = dummy->height;
			tmp->valign = dummy->valign;
			tmp->hspace = dummy->hspace;
			
			changename(tmp, marker->timetext);
			changetext(tmp, marker->timetext);
		}
		marker = marker->next;
	}

	drawscreen(screen1, 0, 0);
	addscreenrc(screen1, listbox);
	while (1)
	{
		rcret = waitrc(screen1, 0, 0);
		if(rcret==getrcconfigint("rcexit",NULL)) break;
	}
	delownerrc(screen1);
	delmarkedscreennodes(screen1, 1);
	clearscreen(screen1);
}

#endif
