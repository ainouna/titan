#ifndef NOVAMOV_H
#define NOVAMOV_H

char* novamov(char* link)
{
	debug(99, "link: %s", link);
	int debuglevel = getconfigint("debuglevel", NULL);
	char* tmphost = NULL;
	char* tmppath = NULL;
	char* tmpstr = NULL;
	char* send = NULL;
	char* ip = NULL;
	char* streamlink = NULL;
	char* filekey = NULL;
	char* file = NULL;

	if(link == NULL) return NULL;

	unlink("/tmp/novamov1_get");
	unlink("/tmp/novamov2_get");
/////////////
	char* tmplink = NULL, *pos = NULL, *path = NULL;

	tmplink = ostrcat(link, NULL, 0, 0);

	if(ostrstr(link, "/Out/?s=") != NULL)
	{
		tmplink = string_replace("/Out/?s=", "", tmplink, 1);
		debug(99, "remove out string: %s", tmplink);
	}
	

	if(tmplink == NULL || ostrncmp("http://", tmplink, 7))
	{
		textbox(_("Message"), _("Hoster Url not http://") , _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 1200, 200, 0, 0);
		goto end;
	}

	tmphost = string_replace("http://", "", tmplink, 0);
	free(tmplink) , tmplink = NULL;

	if(tmphost != NULL)
		pos = strchr(tmphost, '/');
	if(pos != NULL)
	{
		pos[0] = '\0';
		path = pos + 1;
	}

	tmppath = ostrcat("/", path, 0, 0);
//	free(path), path = NULL;

/////////////


/*
	tmphost = ostrcat("www.", host, 0, 0);
	tmphost = ostrcat(tmphost, ".com", 1, 0);

	tmppath = ostrcat("/video/", file, 0, 0);
	
	debug(99, "tmphost: %s", tmphost);
	ip = get_ip(tmphost);
	debug(99, "ip: %s", ip);
	debug(99, "tmppath: %s", tmppath);
*/
	send = ostrcat(send, "GET ", 1, 0);
	send = ostrcat(send, tmppath, 1, 0);
	send = ostrcat(send, " HTTP/1.1\r\nHost: ", 1, 0);
	send = ostrcat(send, tmphost, 1, 0);
	send = ostrcat(send, "\r\nUser-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/13.0.782.99 Safari/535.1\r\nConnection: close\r\nAccept-Encoding: gzip\r\n\r\n", 1, 0);
	debug(99, "send: %s", send);

	tmpstr = gethttpreal(tmphost, tmppath, 80, NULL, NULL, NULL, 0, send, NULL, 5000, 1);
	debug(99, "tmpstr: %s", tmpstr);
	titheklog(debuglevel, "/tmp/novamov1_get", NULL, tmpstr);

	if(ostrstr(tmpstr, "The file is being transfered to our other servers. This may take few minutes.") != NULL)
	{
		textbox(_("Message"), _("The file is being transfered to our other servers. This may take few minutes.") , _("OK"), getrcconfigint("rcok", NULL), _("EXIT"), getrcconfigint("rcexit", NULL), NULL, 0, NULL, 0, 1200, 200, 0, 0);
		goto end;
	}

	file = string_resub("flashvars.file=\"", "\";", tmpstr, 0);
	filekey = string_resub("flashvars.filekey=\"", "\";", tmpstr, 0);

	if(filekey == NULL)
	{
		char* searchstr = string_resub("flashvars.filekey=", ";", tmpstr, 0);
		debug(99, "searchstr: %s", searchstr);
		searchstr = ostrcat(searchstr, "=\"", 1, 0);
		filekey = string_resub(searchstr, "\";", tmpstr, 0);
	}
	debug(99, "filekey: %s", filekey);


	free(tmppath), tmppath = NULL;
	tmppath = ostrcat("/api/player.api.php?file=", file, 0, 0);
	tmppath = ostrcat(tmppath, "&key=", 1, 0);
	tmppath = ostrcat(tmppath, filekey, 1, 0);

	free(send), send = NULL;
	send = ostrcat(send, "GET ", 1, 0);
	send = ostrcat(send, tmppath, 1, 0);
	send = ostrcat(send, " HTTP/1.1\r\nHost: ", 1, 0);
	send = ostrcat(send, tmphost, 1, 0);
	send = ostrcat(send, "\r\nUser-Agent: Mozilla/5.0 (X11; Linux i686) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/13.0.782.99 Safari/535.1\r\nConnection: close\r\nAccept-Encoding: gzip\r\n\r\n", 1, 0);
	debug(99, "send: %s", send);
	free(tmpstr), tmpstr = NULL;
	tmpstr = gethttpreal(tmphost, tmppath, 80, NULL, NULL, NULL, 0, send, NULL, 5000, 1);
	debug(99, "tmpstr: %s", tmpstr);
	titheklog(debuglevel, "/tmp/novamov2_get", NULL, tmpstr);

	streamlink = string_resub("url=", "&", tmpstr, 0);
	debug(99, "streamlink1: %s", streamlink);
	htmldecode(streamlink, streamlink);
	debug(99, "streamlink2: %s", streamlink);
	
end:
	free(tmphost); tmphost = NULL;
	free(tmppath); tmppath = NULL;
	free(tmpstr); tmpstr = NULL;
	free(send); send = NULL;
	free(filekey); filekey = NULL;
	free(ip); ip = NULL;

	return streamlink;
}

#endif
