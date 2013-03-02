#ifndef PLAYER_H
#define PLAYER_H

// playercan bits:
// 0 policy
// 1 auditraklist
// 2 subtitle
// 3 videomode
// 4 powerofftimer
// 5 videosettings
// 6 stop
// 7 ff
// 8 fr
// 9 pause
// 10 play
// 11 jump/seek reverse
// 12 jump/seek forward
// 13 changecodec
// 14 infobar
// 15 slowmotion

#ifdef EPLAYER3
Context_t * player = NULL;
extern OutputHandler_t OutputHandler;
extern PlaybackHandler_t PlaybackHandler;
extern ContainerHandler_t ContainerHandler;
extern ManagerHandler_t ManagerHandler;
#endif

#ifdef EPLAYER4
GstElement *m_gst_playbin = NULL;
unsigned long long m_gst_startpts = 0;
#endif

//titan player

//flag 0: from play
//flag 1: from timeshift
//flag 2: from playrcjumpr
int playerstartts(char* file, int flag)
{
	int fd = -1, ret = 0, tssize = 188;
	int16_t pmtpid = 0;
	int serviceid = 0;
	int supermagic = -1;
	struct channel* chnode = NULL;
	struct service* snode = NULL;
	struct dvbdev* fenode = NULL;
	struct dvbdev* dvrnode = NULL;

	//supermagic = getsupermagic(file);

	if(supermagic == NFS_SUPER_MAGIC || supermagic == SMB_SUPER_MAGIC)
	{
		debug(150, "use O_DIRECT to open file %s", file);
		fd = open(file, O_RDONLY | O_LARGEFILE | O_NONBLOCK | O_DIRECT);
	}
	else
		fd = open(file, O_RDONLY | O_LARGEFILE | O_NONBLOCK);

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

	if(flag == 0 || flag == 2)
	{
		//TODO: funktion to get tssize from file content
		if(cmpfilenameext(file, ".mts") == 0) tssize = 192;
		if(cmpfilenameext(file, ".m2ts") == 0) tssize = 192;
		
		ret = dvbfindpmtpid(fd, &pmtpid, &serviceid, tssize);
		if(ret == 1)
		{
			err("find sid/pmt pid");
			close(fd);
			dvrclose(dvrnode, -1);
			return 1;
		}
		if(flag == 0)
		{
			delmarkernode(-1);
			char* filemarker = changefilenameext(file, ".ma");
			getmarker(filemarker);
			free(filemarker); filemarker=NULL;
		}

		delchannel(serviceid, 0, 1);
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

	ret = recordstartreal(NULL, fd, dvrnode->fd, RECPLAY, 0, NULL, tssize);
	if(ret != 0)
	{
		err("start play thread");
		close(fd);
		dvrclose(dvrnode, -1);
		return 1;
	}

	snode = getservice(RECORDPLAY, 0);
	if(snode != NULL)
	{
		gettsinfo(snode->recsrcfd, &snode->lenpts, &snode->startpts, &snode->endpts, &snode->bitrate, snode->tssize);
		if(flag == 1)
		{
			snode->lenpts = 0;
			snode->endpts = 0;
		}
		else
		{
			snode->endoffile = lseek64(snode->recsrcfd , 0, SEEK_END);
			lseek64(snode->recsrcfd , 0, SEEK_SET);
			if(flag == 0 && getconfigint("showlastpos", NULL) == 1)
			{ 
				char* fileseek = changefilenameext(file, ".se");
				FILE* fbseek = fopen(fileseek, "r");
				if(fbseek != NULL)
				{
					ret = textbox(_("Message"), _("Start at last position ?"), _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 600, 200, 10, 0);
					if(ret == 0 || ret == 1)
					{
						char* skip1 = calloc(1, 20);
						if(skip1 != NULL)
						{
							fscanf(fbseek, "%s", skip1);
							lseek64(snode->recsrcfd, atoll(skip1), SEEK_SET);
						}
						free(skip1); skip1 = NULL;
					}
					fclose(fbseek);
				}
				free(fileseek); fileseek = NULL;
			}
		}
	}

	if(flag == 0 || flag == 2)
	{
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
		//status.playercan = 0x7EFF;
		status.playercan = 0xFFFF;	
	}

	return 0;
}

//flag 0: from play
//flag 1: from timeshift
//flag 2: from playrcjumpr/playerafterendts
//flag1 0: stop from rcstop
//flag1 1: stop from servicestop
void playerstopts(int flag, int flag1)
{
	int ret = 0;
	struct service* snode = NULL;
	struct channel* node = NULL;

	snode = getservice(RECORDPLAY, flag1);
	if(snode != NULL) snode->recendtime = 1;

	if(flag == 0 || flag == 2)
	{
		playerffts(0);

		if(snode != NULL && snode->recsrcfd >= 0 && flag == 0)
		{
			char* fileseek = changefilenameext(snode->recname, ".se");
			FILE* fbseek = fopen(fileseek, "w");
			if(fbseek != NULL)
			{
				off64_t pos = lseek64(snode->recsrcfd, 0, SEEK_CUR);
				fprintf(fbseek,"%lld", pos);
				fclose(fbseek);
			}
			free(fileseek); fileseek=NULL;
			char* filemarker = changefilenameext(snode->recname, ".ma");
			ret = putmarker(filemarker);
			free(filemarker); filemarker=NULL;
			delmarkernode(-1);
		}
		
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
	off64_t offset = 0;
	off64_t endoffile = 0;
	off64_t currentpos = 0;
	//off64_t fdptspos = 0;
	int ret = 0;
	unsigned long long lenpts = 0;
	unsigned long long startpts = 0;
	unsigned long long endpts = 0;
	unsigned long long bitrate = 0;
	//unsigned long long aktpts = 0;
	//unsigned long long fdpts = 0;
	//int aktsekunden = 0;
	int sekundenoff = 0;
	
	if(servicenode == NULL) return 1;

	if(servicenode->recsrcfd < 0)
	{
		err("source fd not ok");
		return 1;
	}
	
	m_lock(&status.tsseekmutex, 15);

/*
	ret = videogetpts(status.aktservice->videodev, &aktpts);
	if(ret == 0)
	{
		aktsekunden = aktpts / 90000;
	}
	else
		aktsekunden = 0;
	ret = getpts(servicenode->recsrcfd, 0, 0, 256 * 1024, &fdpts, &fdptspos, -1, servicenode->tssize);
	if(ret == 0 && aktsekunden != 0)
	{
		sekundenoff = fdpts / 90000 - aktsekunden ;
		//currentpos = lseek64(servicenode->recsrcfd, fdptspos, SEEK_SET);
	}
	else
		sekundenoff = 0;
*/
	
	currentpos = lseek64(servicenode->recsrcfd, 0, SEEK_CUR);

	lenpts = servicenode->lenpts;
	startpts = servicenode->startpts;
	endpts = servicenode->endpts;
	bitrate = servicenode->bitrate;
	if(gettsinfo(servicenode->recsrcfd, &lenpts, &startpts, &endpts, &bitrate, servicenode->tssize) != 0)
	{
		err("cant read ts info");
		m_unlock(&status.tsseekmutex, 15);
		return 1;
	}
	if(servicenode->endoffile > 0)
		endoffile = servicenode->endoffile - (servicenode->tssize * 2);
	else
		endoffile = lseek64(servicenode->recsrcfd , -servicenode->tssize * 2, SEEK_END);

/*
	ret = videoclearbuffer(status.aktservice->videodev);
	ret = audioclearbuffer(status.aktservice->audiodev);
	ret = videodiscontinuityskip(status.aktservice->videodev, 0);
*/

	if(sekunden >= 0)
	{
		if(sekundenoff != 0)
			offset = (bitrate / 8) * (sekunden - sekundenoff);
		else
			offset = (bitrate / 8) * sekunden - 5000000;
		offset = offset - (offset % servicenode->tssize);
		if(currentpos + offset > endoffile)
		{
			offset = endoffile - currentpos;
			offset = offset - (offset % servicenode->tssize);
		}
	}
	else
	{
		sekunden = sekunden * -1;
		if(sekundenoff != 0)
			offset = (bitrate / 8) * (sekunden + sekundenoff);
		else
			offset = (bitrate / 8) * sekunden + 5000000;
		offset = offset - (offset % servicenode->tssize);
		if(currentpos - offset < 0)
		{
			offset = currentpos - servicenode->tssize;
			offset = offset - (offset % servicenode->tssize);
			if(offset < 0)
				offset = 0; 
		}	
		offset = offset * -1;
	}
	currentpos = lseek64(servicenode->recsrcfd, offset, SEEK_CUR);

	audiostop(status.aktservice->audiodev);
	videostop(status.aktservice->videodev, 0);
	videoplay(status.aktservice->videodev);
	audioplay(status.aktservice->audiodev);

	m_unlock(&status.tsseekmutex, 15);
	return 0;
}

void playerffts(int speed)
{
	videofastforward(status.aktservice->videodev, speed);
}

void playerslowts(int speed)
{
	videoslowmotion(status.aktservice->videodev, speed);
}

//flag = 0 --> recordplay
//flag = 1 --> timeshift
void playerfrts(int speed, int flag)
{
	if(flag == 1)
		videocontinue(status.aktservice->videodev);
	if(speed == -2)
	{
		videoclearbuffer(status.aktservice->videodev);
		audioclearbuffer(status.aktservice->audiodev);
	}
	speed *= -1;
	videofastforward(status.aktservice->videodev, speed);
}
	

//flag = 0 --> play ts
//flag = 1 --> timeshift
int playergetinfots(unsigned long long* lenpts, unsigned long long* startpts, unsigned long long* endpts, unsigned long long* aktpts, unsigned long long* bitrate, int flag)
{
	int ret = 0, dupfd = -1;
	struct service* snode = NULL;
	unsigned long long lenpts1 = 0;
	unsigned long long startpts1 = 0;
	unsigned long long endpts1 = 0;
	unsigned long long bitrate1 = 0;
	
	if(flag == 0)
		snode = getservice(RECORDPLAY, 0);
	else if(flag == 1)
		snode = getservice(RECORDTIMESHIFT, 0);
		
	if(snode == NULL) return 1;

	if(snode->lenpts > 0 && snode->startpts > 0 && snode->endpts > 0 && snode->bitrate > 0)
	{
		if(lenpts != NULL) *lenpts = snode->lenpts;
		if(startpts != NULL) *startpts = snode->startpts;
		if(endpts != NULL) *endpts = snode->endpts;
		if(bitrate != NULL) *bitrate = snode->bitrate;

		ret = videogetpts(status.aktservice->videodev, aktpts);
		return ret;
	}
	
	dupfd = open(snode->recname, O_RDONLY | O_LARGEFILE);
	if(dupfd < 0)
	{
		err("copy source fd not ok");
		return 1;
	}

	lenpts1 = snode->lenpts;
	startpts1 = snode->startpts;
	endpts1 = snode->endpts;
	bitrate1 = snode->bitrate;
	if(gettsinfo(dupfd, &lenpts1, &startpts1, &endpts1, &bitrate1, snode->tssize) != 0)
	{
		err("cant read ts info");
		return 1;
	}

	if(lenpts != NULL) *lenpts = lenpts1;
	if(startpts != NULL) *startpts = startpts1;
	if(endpts != NULL) *endpts = endpts1;
	if(bitrate != NULL) *bitrate = bitrate1;
	
	close(dupfd);

	ret = videogetpts(status.aktservice->videodev, aktpts);
	return ret;
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
	playerstopts(2, 0);
}

//extern player

int playerstart(char* file)
{
	char * tmpfile = NULL;
	
	if(file != NULL)
	{
#ifdef EPLAYER3
		//use eplayer

		if(player != NULL)
		{
			debug(150, "eplayer allready running");
			playerstop();
		}

		player = calloc(1, sizeof(Context_t));

		if(player == NULL)
		{
			err("no mem");
			return 1;
		}

		if(ostrstr(file, "://") == NULL)
			tmpfile = ostrcat("file://", file, 0, 0);
		else
			tmpfile = ostrcat(file, NULL, 0, 0);

		if(tmpfile == NULL)
		{
			err("no mem");
			free(player); player = NULL;
			return 1;
		}

		if(ostrstr(tmpfile, "file://") == NULL)
			status.playercan = 0x4650;
		else
			status.playercan = 0xFFFF;
		
		player->playback = &PlaybackHandler;
		player->output = &OutputHandler;
		player->container = &ContainerHandler;
		player->manager = &ManagerHandler;
		
		//add container befor open, so we can set buffer size
		char* ext = getfilenameext(tmpfile);
		if(ext != NULL)
		{
			player->container->Command(player, CONTAINER_ADD, ext);
			free(ext); ext = NULL;
		}

		//select container_ffmpeg, if we does not found a container with extensions
		if(player->container->selectedContainer == NULL)
			player->container->Command(player, CONTAINER_ADD, "mp3");
		
		if(player && player->container && player->container->selectedContainer)
		{
			int size = getconfigint("playerbuffersize", NULL);
			int seektime = getconfigint("playerbufferseektime", NULL);
			player->container->selectedContainer->Command(player, CONTAINER_SET_BUFFER_SIZE, (void*)&size);
			player->container->selectedContainer->Command(player, CONTAINER_SET_BUFFER_SEEK_TIME, (void*)&seektime);
		}
		
		debug(150, "eplayername = %s", player->output->Name);

		//Registrating output devices
		player->output->Command(player, OUTPUT_ADD, "audio");
		player->output->Command(player, OUTPUT_ADD, "video");
		player->output->Command(player, OUTPUT_ADD, "subtitle");
		
		//for subtitle
		SubtitleOutputDef_t subout;

		subout.screen_width = fb->width;
		subout.screen_height = fb->height;
		subout.framebufferFD = fb->fd;
		subout.destination = fb->fb;
		subout.destStride = fb->pitch;
		subout.shareFramebuffer = 1;

		player->output->subtitle->Command(player, (OutputCmd_t)OUTPUT_SET_SUBTITLE_OUTPUT, (void*)&subout);
		
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
		
		if(m_gst_playbin != NULL)
		{
			debug(150, "eplayer allready running");
			playerstop();
		}
		
		if(ostrstr(file, "://") == NULL)
			tmpfile = ostrcat("file://", file, 0, 0);
		else
			tmpfile = ostrcat(file, NULL, 0, 0);

		if(tmpfile == NULL)
		{
			err("no mem");
			free(m_gst_playbin); m_gst_playbin = NULL;
			return 1;
		}

		if(ostrstr(tmpfile, "file://") == NULL)
			status.playercan = 0x7E7F;
		else
			status.playercan = 0x7E7F;
		
		m_gst_playbin = gst_element_factory_make("playbin2", "playbin");
		g_object_set(G_OBJECT (m_gst_playbin), "uri", tmpfile, NULL);
		g_object_set(G_OBJECT (m_gst_playbin), "flags", flags, NULL);
		free(tmpfile); tmpfile = NULL;
		
		if(m_gst_playbin)
			gst_element_set_state(m_gst_playbin, GST_STATE_PLAYING);
		
		int count = 0;
		m_gst_startpts = 0;
		while(m_gst_startpts == 0 && count < 5)
		{
			count++;
			sleep(1);
			m_gst_startpts = playergetpts();
		}
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
	if(!m_gst_playbin) return 0;
	if(!msg) return ret;

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
			if(GST_MESSAGE_SRC(msg) != GST_OBJECT(m_gst_playbin))
				break;

			GstState old_state, new_state;
			gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
		
			if(old_state == new_state) break;
	
			debug(150, "gst state change %s -> %s", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
	
			GstStateChange transition = (GstStateChange)GST_STATE_TRANSITION(old_state, new_state);
	
			switch(transition)
			{
				case GST_STATE_CHANGE_NULL_TO_READY:
					break;
				case GST_STATE_CHANGE_READY_TO_PAUSED:
/*
					GstElement *appsink = gst_bin_get_by_name(GST_BIN(m_gst_playbin), "subtitle_sink");
 					if(appsink)
 					{
 						g_object_set(G_OBJECT(appsink), "max-buffers", 2, NULL);
 						g_object_set(G_OBJECT(appsink), "sync", FALSE, NULL);
 						g_object_set(G_OBJECT(appsink), "emit-signals", TRUE, NULL);
 						gst_object_unref(appsink);
 					}
*/
					break;
				case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
					//if(m_sourceinfo.is_streaming && m_streamingsrc_timeout )
						//m_streamingsrc_timeout->stop();
					break;
				case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
					break;
				case GST_STATE_CHANGE_PAUSED_TO_READY:
					break;
				case GST_STATE_CHANGE_READY_TO_NULL:
					ret = 0;
					break;
			}
			break;
		case GST_MESSAGE_ERROR:
			debug(150, "gst player error");

			gchar *gdebug1;
			GError *err;

			gst_message_parse_error(msg, &err, &gdebug1);
			g_free(gdebug1);

			debug(150, "gst error: %s (%i) from %s", err->message, err->code, sourceName);
			if(err->domain == GST_STREAM_ERROR)
			{
				if(err->code == GST_STREAM_ERROR_CODEC_NOT_FOUND )
				{
					//if(g_strrstr(sourceName, "videosink"))
					//	m_event((iPlayableService*)this, evUser+11);
					//else if ( g_strrstr(sourceName, "audiosink") )
					//	m_event((iPlayableService*)this, evUser+10);
				}
			}
			g_error_free(err);
			break;
		case GST_MESSAGE_INFO:
			debug(150, "gst player info");

/*
			gchar *gdebug2;
			GError *inf;
	
			gst_message_parse_info(msg, &inf, &gdebug2);
			g_free(gdebug2);
			if(inf->domain == GST_STREAM_ERROR && inf->code == GST_STREAM_ERROR_DECODE )
			{
				//if(g_strrstr(sourceName, "videosink"))
				//	m_event((iPlayableService*)this, evUser+14);
			}
			g_error_free(inf);
*/
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

/*
			GstBufferingMode mode;
			gst_message_parse_buffering(msg, &(m_bufferInfo.bufferPercent));
			gst_message_parse_buffering_stats(msg, &mode, &(m_bufferInfo.avgInRate), &(m_bufferInfo.avgOutRate), &(m_bufferInfo.bufferingLeft));
			//m_event((iPlayableService*)this, evBuffering);
*/
			break;
		case GST_MESSAGE_STREAM_STATUS:
			debug(150, "gst player stream status");

/*
			GstStreamStatusType type;
			GstElement *owner;

			gst_message_parse_stream_status(msg, &type, &owner);
			if(type == GST_STREAM_STATUS_TYPE_CREATE && m_sourceinfo.is_streaming)
			{
				if(GST_IS_PAD(source))
					owner = gst_pad_get_parent_element(GST_PAD(source));
				else if(GST_IS_ELEMENT(source))
					owner = GST_ELEMENT(source);
				else
					owner = NULL;
				if(owner)
				{
					GstElementFactory *factory = gst_element_get_factory(GST_ELEMENT(owner));
					const gchar *name = gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory));
					if (!strcmp(name, "souphttpsrc"))
					{
						//m_streamingsrc_timeout->start(10 * 1000, true);
						g_object_set(G_OBJECT(owner), "timeout", 10, NULL);
					}
					
				}
				if(GST_IS_PAD(source))
					gst_object_unref(owner);
			}
*/
			break;
		default:
			debug(150, "gst player unknown message");
			break;
	}
	g_free(sourceName);
	return ret;
}
#endif

int playergetbuffersize()
{
	int ret = 0;

#ifdef EPLAYER3
	if(player && player->container && player->container->selectedContainer)
		player->container->selectedContainer->Command(player, CONTAINER_GET_BUFFER_SIZE, (void*)&ret);
#endif

	return ret;
}

int playergetbufferstatus()
{
	int ret = 0;

#ifdef EPLAYER3
	if(player && player->container && player->container->selectedContainer)
		player->container->selectedContainer->Command(player, CONTAINER_GET_BUFFER_STATUS, (void*)&ret);
#endif

	return ret;
}

int playerstopbuffer()
{
	int ret = 0;

#ifdef EPLAYER3
	if(player && player->container && player->container->selectedContainer)
		player->container->selectedContainer->Command(player, CONTAINER_STOP_BUFFER, NULL);
#endif

	return ret;
}

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
	else
		ret = 0;

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

#ifdef EPLAYER4
	if(m_gst_playbin)
		gst_element_set_state(m_gst_playbin, GST_STATE_PLAYING);
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
		m_gst_playbin = NULL;
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

#ifdef EPLAYER4
	if(m_gst_playbin)
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

void playerslow(int speed)
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
		player->playback->Command(player, PLAYBACK_SLOWMOTION, &speedmap);
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
	gint64 nanos_pts = 0, nanos_len = 0;
	gint64 pts = 0, len = 0;
	//GstFormat fmt = GST_FORMAT_TIME;
		
	if(m_gst_playbin)
	{
		len = playergetlength();
		nanos_len = len * 1000000000;
		if(nanos_len < 0) nanos_len = 0;

		pts = playergetpts();
		nanos_pts = pts * 11111;
		nanos_pts = nanos_pts + (sec * 1000000000);
		if(nanos_pts < 0) nanos_pts = 0;

		if(nanos_pts >= nanos_len)
			playerstop();
		else
			gst_element_seek(m_gst_playbin, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, nanos_pts, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
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

unsigned long long playergetpts()
{
	unsigned long long pts = 0;
	unsigned long long sec = 0;

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
	
/*
	if(m_gst_playbin)
	{
		gst_element_query_position(m_gst_playbin, &fmt, (gint64*)&pts);
		sec = pts / 1000000000;
		pts = sec * 90000;
		debug(150, "Pts = %02d:%02d:%02d (%llu.0000 sec)", (int)((sec / 60) / 60) % 60, (int)(sec / 60) % 60, (int)sec % 60, sec);
	}
*/

	if(m_gst_playbin)
	{
		gint64 pos;
		GstElement *sink;
		pts = 0;

		g_object_get(G_OBJECT (m_gst_playbin), "audio-sink", &sink, NULL);

		if(!sink) g_object_get (G_OBJECT (m_gst_playbin), "video-sink", &sink, NULL);
		if(!sink) return 0;

		gchar *name = gst_element_get_name(sink);
		gboolean use_get_decoder_time = ostrstr(name, "dvbaudiosink") || ostrstr(name, "dvbvideosink");
		g_free(name);

		if(use_get_decoder_time) g_signal_emit_by_name(sink, "get-decoder-time", &pos);

		gst_object_unref(sink);

		if(!use_get_decoder_time && !gst_element_query_position(m_gst_playbin, &fmt, &pos))
			return 0;

		/* pos is in nanoseconds. we have 90 000 pts per second. */
		pts = pos / 11111;
		pts = pts - m_gst_startpts;
		sec = pts / 90000;
		debug(150, "StartPTS = %llu Pts = %02d:%02d:%02d (%llu.0000 sec)", m_gst_startpts, (int)((sec / 60) / 60) % 60, (int)(sec / 60) % 60, (int)sec % 60, sec);
	}
#endif

	if(pts < 0) pts = 0;
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
		length = len / 1000000000;
		if(length < 0) length = 0;
		debug(150, "Length = %02d:%02d:%02d (%.4f sec)", (int)((length / 60) / 60) % 60, (int)(length / 60) % 60, (int)length % 60, length);
	}
#endif

	return length;
}

char* playergetinfo(char* tag)
{
	char* ret = NULL;

#ifdef EPLAYER3
	char *tags[] = {"Title", "Artist", "Album", "Year", "Genre", "Comment", "Track", "Copyright", "TestLibEplayer", NULL};
	int i = 0;

	if(player && player->playback)
	{
		while(tags[i] != NULL)
		{
			ret = tags[i];
			if(ostrcmp(tag, ret) == 0)
			{
				player->playback->Command(player, PLAYBACK_INFO, &ret);
				break;
			}

			i++;
		}
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

void playerstopsubtitletrack()
{
#ifdef EPLAYER3
	if(player && player->output && player->output->subtitle)
		player->output->subtitle->Command(player, (OutputCmd_t)OUTPUT_STOP, NULL);

#endif
}

int playerjumpts(struct service* servicenode, int sekunden, int *startpts, off64_t *poslastpts, off64_t *bitrate, int vpid, int tssize)
{
	int adaptation = 0;
	int payload = 0;
	int pes = 0;
	int tspid = 0;
	
	off64_t pts  = 0;
	uint64_t aktpts = 0;
	long long unsigned int lenpts = 0;
	long long unsigned int startpts1 = 0;
	long long unsigned int endpts = 0;
	long long unsigned int aktbitrate = 0;
	off64_t ziehlpts = 0;

	off64_t curpos = 0;
	off64_t newpos = 0;
	off64_t jump = 0;

	int kleiner = 0;
	int groesser = 0;
	int gleich = 0;
	int len = 0;
	int i = 0;
	int ret = 0;

	if(servicenode == NULL) return -1;

	int buflen = tssize * 15000;
	char *buf = malloc(buflen);
	if(buf == NULL)
		return -1;
	
	curpos = lseek64(servicenode->recsrcfd, 0, SEEK_CUR);	
	int dupfd = open(servicenode->recname, O_RDONLY | O_LARGEFILE);
	newpos = lseek64(dupfd, curpos, SEEK_SET);
	
	if (*startpts == 0)
	{
		if(videogetpts(status.aktservice->videodev, &aktpts) == 0)
		{
				ziehlpts = (aktpts / 90000) + sekunden;
		}
		else
			return 1;
	}
	else
	{
		ziehlpts = *startpts + sekunden;
	}
	*startpts = ziehlpts;

	if(*bitrate == 0)
	{
		lenpts = servicenode->lenpts;
		startpts1 = servicenode->startpts;
		endpts = servicenode->endpts;
		aktbitrate = servicenode->bitrate;
		ret = gettsinfo(dupfd, &lenpts, &startpts1, &endpts, &aktbitrate, servicenode->tssize);
		if(ret != 0) 
		{
			err("cant read ts info");
		}
		else
			*bitrate = aktbitrate;
		newpos = lseek64(dupfd, curpos, SEEK_SET);
	}
	else
		aktbitrate = *bitrate;
		
	if(*poslastpts == 0)
		*poslastpts = curpos;
	
	if(sekunden > 0)
	{
		err("not implemented");
		return 1;
	}	
	else if(sekunden < 0)
	{
		sekunden = sekunden * -1;
		if(aktbitrate != 0)
		{
			jump = (aktbitrate / 8) * sekunden;
			jump = jump + (curpos - *poslastpts);
			jump = jump + (jump % servicenode->tssize);
			newpos = lseek64(dupfd, -jump, SEEK_CUR);
		}
		else
			newpos = lseek64(dupfd, - buflen, SEEK_CUR);
		if(newpos < 0)
			newpos = lseek64(dupfd, tssize, SEEK_SET);
	}
	len = read(dupfd, buf, buflen);
	for(i = 0; i < len; i = i + 1)
	{
		if (buf[i] == 0x47 && buf[i+tssize] == 0x47)
		{
			newpos = lseek64(dupfd, newpos + i, SEEK_SET);
			break;
		}
	}
	if(i >= len)
	{
		newpos = lseek64(dupfd, curpos, SEEK_SET);	
		return 1;
	} 
	while(1)
	{
	len = read(dupfd, buf, buflen);

		if(len > 0)
		{
			for(i = 0; i <= len-tssize; i = i + tssize)
			{
				payload = 0;

				tspid = (buf[i+1] & 0x1F) << 8;
				tspid = tspid + (buf[i+2] & 0xFF);
				pes = buf[i+1] & 0x40;

				if(tspid == vpid)
				{	
					adaptation = buf[i+3] & 0x30;
					if(adaptation == 16)
					{
						payload = 4;
					}
					if(adaptation == 32)
					{
						//printf("adaptation field only\n");
					}
					if(adaptation == 48)
					{
						payload = buf[i+4] & 0xFF;
						payload = payload + 5;
					}
					if(payload != 0)
					{
						if(pes == 64)
						{
							if(buf[i+payload+7] & 0x80) //PTS
							{
								pts = ((unsigned long long)(buf[i+payload+9] & 0xE)) << 29;
								pts |= ((unsigned long long)(buf[i+payload+10] & 0xFF)) << 22;
								pts |= ((unsigned long long)(buf[i+payload+11] & 0xFE)) << 14;
								pts |= ((unsigned long long)(buf[i+payload+12] & 0xFF)) << 7;
								pts |= ((unsigned long long)(buf[i+payload+13] & 0xFE)) >> 1;
								
								if(pts / 90000 == ziehlpts)
								{
									gleich = newpos + i;
									break;
								}
								else if(pts / 90000 > ziehlpts)
								{									
									groesser = newpos + i;
									break;
								}
								else
								{
									kleiner = newpos + i;
								}
							}
						}
					}
				}
			}
			if(gleich != 0)
			{
				close(dupfd);
				free(buf);buf = NULL;
				*poslastpts = lseek64(servicenode->recsrcfd, gleich, SEEK_SET);
				return 0;
			}
			else if(groesser != 0 && kleiner != 0)
			{
				close(dupfd);
				free(buf);buf = NULL;
				*poslastpts = lseek64(servicenode->recsrcfd, kleiner, SEEK_SET);
				return 0;
			}
			else if(groesser != 0)
			{
				if((newpos - buflen)  < 0)
				{
					close(dupfd);
					free(buf);buf = NULL;
					*poslastpts = 0;
					return -1	;
				}
				else
				{
					newpos = lseek64(dupfd, -(buflen * 2), SEEK_CUR);
				}
			}
		}
		else
		{
			if(kleiner == 0)
			{
				close(dupfd);
				free(buf);buf = NULL;
				newpos = lseek64(servicenode->recsrcfd, curpos, SEEK_SET);
				*poslastpts = 0;
				return -1;
			}
			else
			{
				close(dupfd);
				free(buf);buf = NULL;
				*poslastpts = lseek64(servicenode->recsrcfd, kleiner, SEEK_SET);
				return 0;
			}
		}
	}
}


#endif
