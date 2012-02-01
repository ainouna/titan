#ifndef PLAYER_H
#define PLAYER_H

#ifdef EPLAYER3
Context_t * player = NULL;
extern OutputHandler_t OutputHandler;
extern PlaybackHandler_t PlaybackHandler;
extern ContainerHandler_t ContainerHandler;
extern ManagerHandler_t ManagerHandler;
#endif

#ifdef EPLAYER4
GstElement *m_gst_playbin;
#endif

//titan player

//flag 0: from play
//flag 1: from timeshift
int playerstartts(char* file, int flag)
{
	int fd = -1, ret = 0;
	int16_t pmtpid = 0;
	int serviceid = 0;
	struct channel* chnode = NULL;
	struct service* snode = NULL;
	struct dvbdev* fenode = NULL;
	struct dvbdev* dvrnode = NULL;

	fd = open(file, O_RDONLY | O_LARGEFILE);
	if(fd < 0)
	{
		perr("open player file");
		return 1;
	}

	fenode = fegetdummy();
	dvrnode = dvropen(fenode);
	if(dvrnode == NULL)
	{
		err("find dvr dev");
		close(fd);
		return 1;
	}

	if(flag == 0)
	{
		ret = dvbfindpmtpid(fd, &pmtpid, &serviceid);
		if(ret == 1)
		{
			err("find sid/pmt pid");
			close(fd);
			dvrclose(dvrnode, -1);
			return 1;
		}
		chnode = createchannel("player", 0, 0, serviceid, 99, 0, -1, -1, -1, -1, 0);
		if(chnode != NULL) chnode->pmtpid = pmtpid;
	}
	else
		chnode = status.aktservice->channel;

	if(chnode == NULL)
	{
		err("create channel");
		close(fd);
		dvrclose(dvrnode, -1);
		return 1;
	}

	if(flag == 1)
	{
		ret = servicestart(chnode, NULL, NULL, 2);
		if(ret != 0)
		{
			err("start play");
			close(fd);
			dvrclose(dvrnode, -1);
			return 1;
		}
	}

	ret = recordstart(NULL, fd, dvrnode->fd, RECPLAY, 0, NULL);
	if(ret != 0)
	{
		err("start play thread");
		close(fd);
		dvrclose(dvrnode, -1);
		return 1;
	}

	if(flag == 0)
	{
		snode = getservice(RECORDPLAY, 0);
		if(snode != NULL) snode->recname = ostrcat(file, NULL, 0, 0);
		ret = servicestart(chnode, NULL, NULL, 1);
		if(ret != 0)
		{
			err("start play");
			if(snode != NULL) snode->recendtime = 1;
			close(fd);
			dvrclose(dvrnode, -1);
			return 1;
		}
	}

	return 0;
}

//flag 0: from play
//flag 1: from timeshift
//flag1 0: stop from rcstop
//flag1 1: stop from servicestop
void playerstopts(int flag, int flag1)
{
	int ret = 0;
	struct service* snode = NULL;
	struct channel* node = NULL;

	snode = getservice(RECORDPLAY, flag1);
	if(snode != NULL) snode->recendtime = 1;

	if(flag == 0)
	{
		playerffts(0);

		ret = servicestop(status.aktservice, 1, 1);
		if(ret == 1)
		{
			debug(150, "can't stop ts playback service");	
		}
		else
			status.aktservice->channel = NULL;

		node = gettmpchannel();
		if(node != NULL && ostrcmp(node->name, "player") == 0)
			delchannel(node->serviceid, node->transponderid, 1);
	}
}

void playercontinuets()
{
	videocontinue(status.aktservice->videodev);
	audioplay(status.aktservice->audiodev);
}

void playerpausets()
{
	videofreeze(status.aktservice->videodev);
	audiopause(status.aktservice->audiodev);
}

//flag 0: from play
//flag 1: from timeshift
int playerseekts(struct service* servicenode, int sekunden, int flag)
{
	off64_t offset;
	off64_t bufferoffset;
	off64_t endoffile;
	off64_t currentpos;
	int dupfd = -1;
	int ret = 0;
	unsigned long long pts = 0;
	unsigned long long bitrate = 0;
	struct service* snode = NULL;
	
	if(servicenode == NULL) return 1;

	if(servicenode->recsrcfd < 0)
	{
		err("source fd not ok");
		return 1;
	}
	if(flag == 0)
		snode = getservice(RECORDPLAY, 0);
	else
		snode = getservice(RECORDTIMESHIFT, 0);
	
	if(snode == NULL) return 1;
	
	dupfd = open(snode->recname, O_RDONLY | O_LARGEFILE);
	if(dupfd < 0)
	{
		err("copy source fd not ok");
		return 1;
	}
	
	usleep(500000);
	m_lock(&status.tsseekmutex, 15);
	usleep(500000);
	if(gettsinfo(dupfd, &pts, NULL, NULL, &bitrate) != 0)
	{
		err("cant read endpts/bitrate");
		m_unlock(&status.tsseekmutex, 15);
		return 1;
	}
	endoffile = lseek64(dupfd , -188 * 2, SEEK_END);
	close(dupfd);
	currentpos = lseek64(servicenode->recsrcfd, 0, SEEK_CUR);
	ret = videoclearbuffer(status.aktservice->videodev);
	ret = audioclearbuffer(status.aktservice->audiodev);

	if(sekunden >= 0)
	{
		offset = (bitrate / 8) * sekunden - 5000000;
		offset = offset - (offset % 188);
		if(currentpos + offset > endoffile)
		{
			offset = endoffile - currentpos;
			offset = offset - (offset % 188);
		}
	}
	else
	{
		sekunden = sekunden * -1;
		offset = (bitrate / 8) * sekunden + 5000000;
		offset = offset - (offset % 188);
		if(currentpos - offset < 0)
		{
			offset = currentpos - 188;
			offset = offset - (offset % 188);
			if(offset < 0)
				offset = 0; 
		}	
		offset = offset * -1;
	}
	currentpos = lseek64(servicenode->recsrcfd, offset, SEEK_CUR);
	m_unlock(&status.tsseekmutex, 15);
	usleep(500000);
	status.timeshiftseek = 0;
	return 0;
}

void playerffts(int speed)
{
		videofastforward(status.aktservice->videodev, speed / 2);
}

void playerfrts(int speed)
{
}

int playergetinfots(unsigned long long* lenpts, unsigned long long* startpts, unsigned long long* endpts, unsigned long long* aktpts, unsigned long long* bitrate)
{
	int dupfd = -1;
	struct service* snode = getservice(RECORDPLAY, 0);
	
	if(snode == NULL) return 1;
	
	dupfd = open(snode->recname, O_RDONLY | O_LARGEFILE);
	if(dupfd < 0)
	{
		err("copy source fd not ok");
		return 1;
	}

	if(gettsinfo(dupfd, lenpts, startpts, endpts, bitrate) != 0)
	{
		err("cant read endpts/bitrate");
		return 1;
	}
	
	close(dupfd);

	videogetpts(status.aktservice->videodev, aktpts);
	return 0;
}

void playerchangeaudiotrackts()
{
	screenaudiotrack();
}

void playerchangesubtitletrackts()
{
	screensubtitle();
}

int playerisplayingts()
{
	struct service* snode = getservice(RECORDPLAY, 0);

	if(snode == NULL)
		return 0;
	return 1;
}

void playerafterendts()
{
	playerstopts(0, 0);
}

//extern player

int playerstart(char* file)
{
	if(file != NULL)
	{
#ifdef EPLAYER3
		//use eplayer
		char * tmpfile = NULL;

		if(player != NULL)
		{
			debug(150, "eplayer allready running");
			playerstop();
		}

		player = malloc(sizeof(Context_t));

		if(player == NULL)
		{
			err("no mem");
			return 1;
		}

		if(strstr(file, "://") == NULL)
			tmpfile = ostrcat("file://", file, 0, 0);
		else
			tmpfile = ostrcat(file, "", 0, 0);

		if(tmpfile == NULL)
		{
			err("no mem");
			free(player); player = NULL;
			return 1;
		}

		player->playback = &PlaybackHandler;
		player->output = &OutputHandler;
		player->container = &ContainerHandler;
		player->manager = &ManagerHandler;

		debug(150, "eplayername = %s", player->output->Name);

		//Registrating output devices
		player->output->Command(player, OUTPUT_ADD, "audio");
		player->output->Command(player, OUTPUT_ADD, "video");
		player->output->Command(player, OUTPUT_ADD, "subtitle");

		if(player->playback->Command(player, PLAYBACK_OPEN, tmpfile) < 0)
		{
			free(player); player = NULL;
			free(tmpfile);
			return 1;
		}

		player->output->Command(player, OUTPUT_OPEN, NULL);
		player->playback->Command(player, PLAYBACK_PLAY, NULL);

		free(tmpfile);

		return 0;
#endif

#ifdef EPLAYER4
		int flags = 0x47; //(GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_NATIVE_VIDEO | GST_PLAY_FLAG_TEXT);
		gchar *uri;
		
		if(m_gst_playbin != NULL)
		{
			debug(150, "eplayer allready running");
			playerstop();
		}
		
		uri = g_filename_to_uri(file, NULL, NULL);
		m_gst_playbin = gst_element_factory_make("playbin2", "playbin");
		g_object_set(G_OBJECT (m_gst_playbin), "uri", uri, NULL);
		g_object_set(G_OBJECT (m_gst_playbin), "flags", flags, NULL);
		g_free(uri);
		
		if(m_gst_playbin)
			gst_element_set_state(m_gst_playbin, GST_STATE_PLAYING);
		
		return 0;
#endif
	}
	
	return 1;
}

void playerinit(int argc, char* argv[])
{
#ifdef EPLAYER4
	gst_init(&argc, &argv);
#endif
}

#ifdef EPLAYER4
int gstbuscall(GstBus *bus, GstMessage *msg)
{
	int ret = 1;
	if(!msg) return ret;
	if(!m_gst_playbin) return ret;

	gchar *sourceName = NULL;
	GstObject *source = GST_MESSAGE_SRC(msg);

	if(!GST_IS_OBJECT(source)) return ret;
	sourceName = gst_object_get_name(source);

	switch(GST_MESSAGE_TYPE(msg))
	{
		case GST_MESSAGE_EOS:
			debug(150, "gst player eof");
			ret = 0;
			break;
		case GST_MESSAGE_STATE_CHANGED:
			debug(150, "gst player state changed");
			break;
		case GST_MESSAGE_ERROR:
			debug(150, "gst player error");
			break;
		case GST_MESSAGE_INFO:
			debug(150, "gst player info");
			break;
		case GST_MESSAGE_TAG:
			debug(150, "gst player tag");
			break;
		//case GST_MESSAGE_ASYNC_DONE:
		//	debug(150, "gst player async done");
		//	break;
		case GST_MESSAGE_ELEMENT:
			debug(150, "gst player element");
			break;
		case GST_MESSAGE_BUFFERING:
			debug(150, "gst player buffering");
			break;
		case GST_MESSAGE_STREAM_STATUS:
			debug(150, "gst player stream status");
			break;
		default:
			debug(150, "gst player unknown message");
			break;
	}
	g_free(sourceName);
	return ret;
}
#endif

int playerisplaying()
{
#ifdef SIMULATE
	return 1;
#endif

#ifdef EPLAYER3
	if(player != NULL && player->playback != NULL && player->playback->isPlaying)
		return 1;
#endif

#ifdef EPLAYER4
	int ret = 1;

	if(m_gst_playbin)
	{
		GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_gst_playbin));
		GstMessage *message = NULL;
		while((message = gst_bus_pop(bus)))
		{
			ret = gstbuscall(bus, message);
			gst_message_unref(message);
		}
	}
	return ret;
#endif
	return 0;
}

void playerplay()
{
#ifdef EPLAYER3
	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_PLAY, NULL);
#endif
}

int playerstop()
{
#ifdef EPLAYER3
	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_STOP, NULL);
	if(player && player->container && player->container->selectedContainer)
		player->container->selectedContainer->Command(player, CONTAINER_STOP, NULL);
	if(player && player->output)
	{
		player->output->Command(player, OUTPUT_CLOSE, NULL);
		player->output->Command(player, OUTPUT_DEL, (void*)"audio");
		player->output->Command(player, OUTPUT_DEL, (void*)"video");
		player->output->Command(player, OUTPUT_DEL, (void*)"subtitle");
	}
	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_CLOSE, NULL);

	free(player);
	player = NULL;
#endif

#ifdef EPLAYER4
	if(m_gst_playbin)
	{
		gst_element_set_state(m_gst_playbin, GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(m_gst_playbin));
		m_gst_playbin = 0;
	}
#endif

	writesysint("/proc/sys/vm/drop_caches", 3, 0);
	return 0;
}

void playerafterend()
{
#ifdef EPLAYER3
	if(player != NULL && player->playback != NULL)
		playerstop();
#endif
}

void playerpause()
{
#ifdef EPLAYER3
	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_PAUSE, NULL);
#endif

#ifdef EPLAYER4
	if(m_gst_playbin)
		gst_element_set_state(m_gst_playbin, GST_STATE_PAUSED);
#endif
}

void playercontinue()
{
#ifdef EPLAYER3
	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_CONTINUE, NULL);
#endif

#ifdef EPLAYER4
	if(m_gst_playbin)
		gst_element_set_state(m_gst_playbin, GST_STATE_PLAYING);
#endif
}

void playerff(int speed)
{
#ifdef EPLAYER3
	int speedmap = 0;

	if (speed < 1) speed = 1;
	if (speed > 7) speed = 7;

	switch(speed)
	{
		case 1: speedmap = 1; break;
		case 2: speedmap = 3; break;
		case 3: speedmap = 7; break;
		case 4: speedmap = 15; break;
		case 5: speedmap = 31; break;
		case 6: speedmap = 63; break;
		case 7: speedmap = 127; break;
	}

	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_FASTFORWARD, &speedmap);
#endif
}

void playerfr(int speed)
{
#ifdef EPLAYER3
	int speedmap = 0;

	if (speed > -1) speed = -1;
	if (speed < -7) speed = -7;

	switch(speed)
	{
		case -1: speedmap = -5; break;
		case -2: speedmap = -10; break;
		case -3: speedmap = -20; break;
		case -4: speedmap = -40; break;
		case -5: speedmap = -80; break;
		case -6: speedmap = -160; break;
		case -7: speedmap = -320; break;
	}

	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_FASTBACKWARD, &speedmap);
#endif
}

void playerseek(float sec)
{
#ifdef EPLAYER3
	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_SEEK, (void*)&sec);
#endif

#ifdef EPLAYER4
	gint64 time_nanoseconds;
	gint64 pos;
	GstFormat fmt = GST_FORMAT_TIME;
		
	if(m_gst_playbin)
	{
		gst_element_query_position(m_gst_playbin, &fmt, &pos);
		time_nanoseconds = pos + (sec * 1000000000);
		if(time_nanoseconds < 0) time_nanoseconds = 0;
		gst_element_seek(m_gst_playbin, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, time_nanoseconds, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	}
#endif
}

void playerfreetracklist(char** TrackList)
{
	int i = 0;

	if(TrackList != NULL)
	{
		while(TrackList[i] != NULL)
		{
			free(TrackList[i]);
			free(TrackList[i + 1]);
			i += 2;
		}
	}
}

char** playergettracklist(int type)
{
	char ** TrackList = NULL;
#ifdef EPLAYER3
	if(player && player->manager)
	{
		switch(type)
		{
			case 1:
				if(player->manager->audio)
				{
					player->manager->audio->Command(player, MANAGER_LIST, &TrackList);
					debug(150, "Audio Track List");
				}
				break;
			case 2:
				if(player->manager->subtitle)
				{
					player->manager->subtitle->Command(player, MANAGER_LIST, &TrackList);
					debug(150, "Subtitle Track List");
				}
				break;
			default:
				if(player->manager->video)
				{
					player->manager->video->Command(player, MANAGER_LIST, &TrackList);
					debug(150, "Video Track List");
				}
		}
	
		if(TrackList != NULL)
		{
			debug(150, "Track List");
			int i = 0;
			while(TrackList[i] != NULL)
			{
				debug(150, "%s - %s", TrackList[i], TrackList[i + 1]);
				i += 2;
			}
		}
	}
#endif
	return TrackList;
}

//*CurTrackEncoding and *CurTrackName be freed
void playergetcurtrac(int type, int *CurTrackId, char** CurTrackEncoding, char** CurTrackName)
{
#ifdef EPLAYER3
	if(player && player->manager)
	{
		switch(type)
		{
			case 1:
				if(player->manager->audio)
				{
					player->manager->audio->Command(player, MANAGER_GET, CurTrackId);
					player->manager->audio->Command(player, MANAGER_GETENCODING, CurTrackEncoding);
					player->manager->audio->Command(player, MANAGER_GETNAME, CurTrackName);
				}
				break;
			case 2:
				if(player->manager->subtitle)
				{
					player->manager->subtitle->Command(player, MANAGER_GET, CurTrackId);
					player->manager->subtitle->Command(player, MANAGER_GETENCODING, CurTrackEncoding);
					player->manager->subtitle->Command(player, MANAGER_GETNAME, CurTrackName);
				}
				break;
			default:
				if(player->manager->video)
				{
					player->manager->video->Command(player, MANAGER_GET, CurTrackId);
					player->manager->video->Command(player, MANAGER_GETENCODING, CurTrackEncoding);
					player->manager->video->Command(player, MANAGER_GETNAME, CurTrackName);
				}
		}

		if(CurTrackId != NULL)
			debug(150, "Current Track ID: %d", *CurTrackId);
		if(*CurTrackEncoding != NULL)
			debug(150, "Current Track Enc: %s", *CurTrackEncoding);
		if(*CurTrackName != NULL)
			debug(150, "Current Track Name: %s", *CurTrackName);
	}
#endif

#ifdef EPLAYER4
	if(m_gst_playbin != NULL)
	{
		switch(type)
		{
			case 1:
				g_object_get(G_OBJECT(m_gst_playbin), "current-audio", CurTrackId, NULL);
				break;
		}
		
		if(CurTrackId != NULL)
			debug(150, "Current Track ID: %d", *CurTrackId);
	}
#endif
}

unsigned long long int playergetpts()
{
	unsigned long long int pts = 0;
	unsigned long long int sec = 0;

#ifdef EPLAYER3
	if(player && player->playback)
	{
		player->playback->Command(player, PLAYBACK_PTS, &pts);
		sec = pts / 90000;
		debug(150, "Pts = %02d:%02d:%02d (%llu.0000 sec)", (int)((sec / 60) / 60) % 60, (int)(sec / 60) % 60, (int)sec % 60, sec);
	}
#endif

#ifdef EPLAYER4
	GstFormat fmt = GST_FORMAT_TIME; //Returns time in nanosecs
	
	if(m_gst_playbin)
	{
		gst_element_query_position(m_gst_playbin, &fmt, (gint64*)&pts);
		sec = pts / 1000000000.0;
    debug(150, "Pts = %02d:%02d:%02d (%llu.0000 sec)", (int)((sec / 60) / 60) % 60, (int)(sec / 60) % 60, (int)sec % 60, sec);
	}
#endif

	return pts;
}

double playergetlength()
{
	double length = 0;

#ifdef EPLAYER3
	if(player && player->playback)
	{
		player->playback->Command(player, PLAYBACK_LENGTH, &length);
		if(length < 0) length = 0;
		debug(150, "Length = %02d:%02d:%02d (%.4f sec)", (int)((length / 60) / 60) % 60, (int)(length / 60) % 60, (int)length % 60, length);
	}
#endif

#ifdef EPLAYER4
	GstFormat fmt = GST_FORMAT_TIME; //Returns time in nanosecs
	gint64 len;

	if(m_gst_playbin)
	{
		gst_element_query_duration(m_gst_playbin, &fmt, &len);
		length = len / 1000000000.0;
		if(length < 0) length = 0;
		debug(150, "Length = %02d:%02d:%02d (%.4f sec)", (int)((length / 60) / 60) % 60, (int)(length / 60) % 60, (int)length % 60, length);
	}
#endif

	return length;
}

//TODO: implement
char* playergetinfo(char* tag)
{
	char *ret = NULL;

#ifdef EPLAYER3
	//char *tags[] = {"Title", "Artist", "Album", "Year", "Genre", "Comment", "Track", "Copyright", "TestLibEplayer", NULL};
	//int i = 0;

	if(player && player->playback)
	{
	//	while(tags[i] != NULL)
	//	{
	//		char* tag = tags[i];
			player->playback->Command(player, PLAYBACK_INFO, &ret);

			if(tag != NULL)
				printf("%s:%s",tag, ret);
			else
				printf("%s:NULL",tag);
			//i++;
		//}
	}
#endif
	return ret;
}

void playerchangeaudiotrack(int num)
{
#ifdef EPLAYER3
	if(player && player->playback)
	{
		if(num >= 0 && num <= 9)
			player->playback->Command(player, PLAYBACK_SWITCH_AUDIO, (void*)&num);
	}
#endif

#ifdef EPLAYER4
	if(m_gst_playbin != NULL)
		g_object_set(G_OBJECT(m_gst_playbin), "current-audio", num, NULL);	
#endif
}

void playerchangesubtitletrack(int num)
{
#ifdef EPLAYER3
	if(player && player->playback)
	{
		if(num >= 0 && num <= 9)
			player->playback->Command(player, PLAYBACK_SWITCH_SUBTITLE, (void*)&num);
	}
#endif
}

#endif
