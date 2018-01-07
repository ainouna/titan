//#ifndef SISPMCTL_H
//#define SISPMCTL_H



struct clist *myconfig[LISTHASHSIZE] = {NULL};
char* sispmctlconf = NULL;

int sispmctl_Anzeige()
{
	int anzmenu = 0;
	int ende = 0;
	int rcret = 0;
	int statb1 = 0;
	int statb2 = 0;
	int statb3 = 0;
	int statb4 = 0;
	
	char* zeile = NULL;
	char* cmd = NULL;
	
	char* red = NULL;
	char* yellow = NULL;
	char* green = NULL;
	char* menu = NULL;

	
	struct skin* sis_anzeige = getscreen("sis_anzeige");
	struct skin* buttonbar = getscreennode(sis_anzeige, "buttonbar");
	struct skin* b1 = getscreennode(sis_anzeige, "b1");
	struct skin* b2 = getscreennode(sis_anzeige, "b2");
	struct skin* b3 = getscreennode(sis_anzeige, "b3");
	struct skin* b4 = getscreennode(sis_anzeige, "b4");
	struct skin* m1 = getscreennode(sis_anzeige, "m1");
	
	struct skin* tmp = NULL;
	
	red = createpluginpath("/sispmctl/picons/key_red_22.png", 0);
	yellow = createpluginpath("/sispmctl/picons/key_yellow_22.png", 0);
	green = createpluginpath("/sispmctl/picons/key_green_22.png", 0);
	menu = createpluginpath("/sispmctl/picons/key_menu_70x40.png", 0);
	
	changetext(b1, getlist(myconfig, "g1_name", NULL));
	changetext(b2, getlist(myconfig, "g2_name", NULL));
	changetext(b3, getlist(myconfig, "g3_name", NULL));
	changetext(b4, getlist(myconfig, "g4_name", NULL));
	
	
	while(1)
	{	
		ende = 0;
		cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -g1 > /tmp/helpsis", 0, 0);
		system(cmd);
		free(cmd); cmd = NULL;
		zeile = readsys("/tmp/helpsis", 2);
		printf("->%s<-\n", zeile);
		if(zeile == NULL)
		{
			changepic(b1, yellow);
			changepic(b2, yellow);
			changepic(b3, yellow);
			changepic(b4, yellow);
			statb1 = 2;
			statb2 = 2;
			statb3 = 2;
			statb4 = 2;
		}
		else
		{
			if(ostrstr(zeile, "on") != NULL)
			{
				changepic(b1, green);
				statb1 = 1;
			}
			else
			{
				changepic(b1, red);
				statb1 = 0;
			}
			free(zeile); zeile = NULL;
		
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -g2 > /tmp/helpsis", 0, 0);
			system(cmd);
			free(cmd); cmd = NULL;
			zeile = readsys("/tmp/helpsis", 2);
			if(ostrstr(zeile, "on") != NULL)
			{
				changepic(b2, green);
				statb2 = 1;
			}
			else
			{
				changepic(b2, red);
				statb2 = 0;
			}
			free(zeile); zeile = NULL;
			
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -g3 > /tmp/helpsis", 0, 0);
			system(cmd);
			free(cmd); cmd = NULL;
			zeile = readsys("/tmp/helpsis", 2);
			if(ostrstr(zeile, "on") != NULL)
			{
				changepic(b3, green);
				statb3 = 1;
			}
			else
			{
				changepic(b3, red);
				statb3 = 0;
			}
			free(zeile); zeile = NULL;
		
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -g4 > /tmp/helpsis", 0, 0);
			system(cmd);
			free(cmd); cmd = NULL;
			zeile = readsys("/tmp/helpsis", 2);
			if(ostrstr(zeile, "on") != NULL)
			{
				changepic(b4, green);
				statb4 = 1;
			}
			else
			{
				changepic(b4, red);
				statb4 = 0;
			}
			free(zeile); zeile = NULL;
		}
		changepic(m1, menu);
		
	
	
		drawscreen(sis_anzeige, 0, 0);
		addscreenrc(sis_anzeige, buttonbar);
		tmp = buttonbar->select;
	
		while(1)
		{
			addscreenrc(sis_anzeige, tmp);
			rcret = waitrc(sis_anzeige, 1000, 0);
			tmp = buttonbar->select;
			if(rcret == getrcconfigint("rcok", NULL))
			{			
				if(buttonbar->select != NULL && ostrcmp(buttonbar->select->name, "b1") == 0)
				{
					if(statb1 == 0)	
						cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o1 > /tmp/helpsis", 0, 0);
					else
						cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -f1 > /tmp/helpsis", 0, 0);
					system(cmd);
					free(cmd); cmd = NULL;
					break;
				}
				if(buttonbar->select != NULL && ostrcmp(buttonbar->select->name, "b2") == 0)
				{
					if(statb2 == 0)	
						cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o2 > /tmp/helpsis", 0, 0);
					else
						cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -f2 > /tmp/helpsis", 0, 0);
					system(cmd);
					free(cmd); cmd = NULL;
					break;
				}
				if(buttonbar->select != NULL && ostrcmp(buttonbar->select->name, "b3") == 0)
				{
					if(statb3 == 0)	
						cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o3 > /tmp/helpsis", 0, 0);
					else
						cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -f3 > /tmp/helpsis", 0, 0);
					system(cmd);
					free(cmd); cmd = NULL;
					break;
				}
				if(buttonbar->select != NULL && ostrcmp(buttonbar->select->name, "b4") == 0)
				{
					if(statb4 == 0)	
						cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o4 > /tmp/helpsis", 0, 0);
					else
						cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -f4 > /tmp/helpsis", 0, 0);
					system(cmd);
					free(cmd); cmd = NULL;
					break;
				}
				if(buttonbar->select != NULL && ostrcmp(buttonbar->select->name, "m1") == 0)
				{
					ende = 1;
					anzmenu = 1;
					break;	
				}				
			}
			if(rcret == getrcconfigint("rcexit", NULL))
			{
				anzmenu = 0;
				ende = 1;
				break;
			}
			if(rcret == getrcconfigint("rcmenu", NULL))
			{
				ende = 1;
				anzmenu = 1;
				break;
			}			
		}
		delownerrc(sis_anzeige);
		clearscreen(sis_anzeige);
		if(ende == 1)
			break;
	}
	
	free(red);
	free(yellow);
	free(green);
	free(menu);
	
	return anzmenu;	
}
	
	

void sispmctl_settings()
{
	
	int rcret = 0;
	int rcode = 0;
	
	struct skin* sis_einstellungen = getscreen("sis_einstellungen");
	struct skin* listbox = getscreennode(sis_einstellungen, "listbox");
	
	struct skin* g1_name = getscreennode(sis_einstellungen, "g1_name");
	struct skin* g1_auto_on = getscreennode(sis_einstellungen, "g1_auto_on");
	struct skin* g1_timer = getscreennode(sis_einstellungen, "g1_timer");
	struct skin* g1_auto_off = getscreennode(sis_einstellungen, "g1_auto_off");
	struct skin* g1_delay = getscreennode(sis_einstellungen, "g1_delay");
	
	struct skin* g2_name = getscreennode(sis_einstellungen, "g2_name");
	struct skin* g2_auto_on = getscreennode(sis_einstellungen, "g2_auto_on");
	struct skin* g2_timer = getscreennode(sis_einstellungen, "g2_timer");
	struct skin* g2_auto_off = getscreennode(sis_einstellungen, "g2_auto_off");
	struct skin* g2_delay = getscreennode(sis_einstellungen, "g2_delay");
	
	struct skin* g3_name = getscreennode(sis_einstellungen, "g3_name");
	struct skin* g3_auto_on = getscreennode(sis_einstellungen, "g3_auto_on");
	struct skin* g3_timer = getscreennode(sis_einstellungen, "g3_timer");
	struct skin* g3_auto_off = getscreennode(sis_einstellungen, "g3_auto_off");
	struct skin* g3_delay = getscreennode(sis_einstellungen, "g3_delay");
	
	struct skin* g4_name = getscreennode(sis_einstellungen, "g4_name");
	struct skin* g4_auto_on = getscreennode(sis_einstellungen, "g4_auto_on");
	struct skin* g4_timer = getscreennode(sis_einstellungen, "g4_timer");
	struct skin* g4_auto_off = getscreennode(sis_einstellungen, "g4_auto_off");
	struct skin* g4_delay = getscreennode(sis_einstellungen, "g4_delay");
	
	struct skin* tmp = NULL;
	
	changemask(g1_name, "abcdefghijklmnopqrstuvwxyz");
  changeinput(g1_name, getlist(myconfig, "g1_name", NULL));
  
  addchoicebox(g1_auto_on, "off", _("off"));
	addchoicebox(g1_auto_on, "on", _("on"));
	setchoiceboxselection(g1_auto_on, getlist(myconfig, "g1_auto_on", NULL));
	
	addchoicebox(g1_timer, "yes", _("yes"));
	addchoicebox(g1_timer, "no", _("no"));
	setchoiceboxselection(g1_timer, getlist(myconfig, "g1_timer", NULL));
	
	addchoicebox(g1_auto_off, "off", _("off"));
	addchoicebox(g1_auto_off, "on", _("on"));
	setchoiceboxselection(g1_auto_off, getlist(myconfig, "g1_auto_off", NULL));
	
	addchoicebox(g1_delay, "0", "0");
	addchoicebox(g1_delay, "1", "1");
	addchoicebox(g1_delay, "2", "2");
	addchoicebox(g1_delay, "3", "3");
	addchoicebox(g1_delay, "4", "1");
	addchoicebox(g1_delay, "5", "1");
	setchoiceboxselection(g1_delay, getlist(myconfig, "g1_delay", NULL));
  
  changemask(g2_name, "abcdefghijklmnopqrstuvwxyz");
  changeinput(g2_name, getlist(myconfig, "g2_name", NULL));
  
  addchoicebox(g2_auto_on, "off", _("off"));
	addchoicebox(g2_auto_on, "on", _("on"));
	setchoiceboxselection(g2_auto_on, getlist(myconfig, "g2_auto_on", NULL));
	
	addchoicebox(g2_timer, "yes", _("yes"));
	addchoicebox(g2_timer, "no", _("no"));
	setchoiceboxselection(g2_timer, getlist(myconfig, "g2_timer", NULL));
	
	addchoicebox(g2_auto_off, "off", _("off"));
	addchoicebox(g2_auto_off, "on", _("on"));
	setchoiceboxselection(g2_auto_off, getlist(myconfig, "g2_auto_off", NULL));
	
	addchoicebox(g2_delay, "0", "0");
	addchoicebox(g2_delay, "1", "1");
	addchoicebox(g2_delay, "2", "2");
	addchoicebox(g2_delay, "3", "3");
	addchoicebox(g2_delay, "4", "1");
	addchoicebox(g2_delay, "5", "1");
	setchoiceboxselection(g2_delay, getlist(myconfig, "g2_delay", NULL));
	
	changemask(g3_name, "abcdefghijklmnopqrstuvwxyz");
  changeinput(g3_name, getlist(myconfig, "g3_name", NULL));
  
  addchoicebox(g3_auto_on, "off", _("off"));
	addchoicebox(g3_auto_on, "on", _("on"));
	setchoiceboxselection(g3_auto_on, getlist(myconfig, "g3_auto_on", NULL));
	
	addchoicebox(g3_timer, "yes", _("yes"));
	addchoicebox(g3_timer, "no", _("no"));
	setchoiceboxselection(g3_timer, getlist(myconfig, "g3_timer", NULL));
	
	addchoicebox(g3_auto_off, "off", _("off"));
	addchoicebox(g3_auto_off, "on", _("on"));
	setchoiceboxselection(g3_auto_off, getlist(myconfig, "g3_auto_off", NULL));
	
	addchoicebox(g3_delay, "0", "0");
	addchoicebox(g3_delay, "1", "1");
	addchoicebox(g3_delay, "2", "2");
	addchoicebox(g3_delay, "3", "3");
	addchoicebox(g3_delay, "4", "1");
	addchoicebox(g3_delay, "5", "1");
	setchoiceboxselection(g3_delay, getlist(myconfig, "g3_delay", NULL));
	
	changemask(g4_name, "abcdefghijklmnopqrstuvwxyz");
  changeinput(g4_name, getlist(myconfig, "g4_name", NULL));
  
  addchoicebox(g4_auto_on, "off", _("off"));
	addchoicebox(g4_auto_on, "on", _("on"));
	setchoiceboxselection(g4_auto_on, getlist(myconfig, "g4_auto_on", NULL));
	
	addchoicebox(g4_timer, "yes", _("yes"));
	addchoicebox(g4_timer, "no", _("no"));
	setchoiceboxselection(g4_timer, getlist(myconfig, "g4_timer", NULL));
	
	addchoicebox(g4_auto_off, "off", _("off"));
	addchoicebox(g4_auto_off, "on", _("on"));
	setchoiceboxselection(g4_auto_off, getlist(myconfig, "g4_auto_off", NULL));
	
	addchoicebox(g4_delay, "0", "0");
	addchoicebox(g4_delay, "1", "1");
	addchoicebox(g4_delay, "2", "2");
	addchoicebox(g4_delay, "3", "3");
	addchoicebox(g4_delay, "4", "1");
	addchoicebox(g4_delay, "5", "1");
	setchoiceboxselection(g4_delay, getlist(myconfig, "g4_delay", NULL));
	
  
	drawscreen(sis_einstellungen, 0, 0);
	addscreenrc(sis_einstellungen, listbox);
	tmp = listbox->select;
	
	while(1)
	{
		addscreenrc(sis_einstellungen, tmp);
		rcret = waitrc(sis_einstellungen, 1000, 0);
		tmp = listbox->select;
		
		if(rcret == getrcconfigint("rcexit", NULL)) break;
		if(rcret == getrcconfigint("rcgreen", NULL))
		{
			addlist(myconfig, "g1_name", g1_name->ret);
			addlist(myconfig, "g1_auto_on", g1_auto_on->ret);
			addlist(myconfig, "g1_timer", g1_timer->ret);
			addlist(myconfig, "g1_auto_off", g1_auto_off->ret);
			addlist(myconfig, "g1_delay", g1_delay->ret);

			addlist(myconfig, "g2_name", g2_name->ret);
			addlist(myconfig, "g2_auto_on", g2_auto_on->ret);
			addlist(myconfig, "g2_timer", g2_timer->ret);
			addlist(myconfig, "g2_auto_off", g2_auto_off->ret);
			addlist(myconfig, "g2_delay", g2_delay->ret);
			
			addlist(myconfig, "g3_name", g3_name->ret);
			addlist(myconfig, "g3_auto_on", g3_auto_on->ret);
			addlist(myconfig, "g3_timer", g3_timer->ret);
			addlist(myconfig, "g3_auto_off", g3_auto_off->ret);
			addlist(myconfig, "g3_delay", g3_delay->ret);
			
			addlist(myconfig, "g4_name", g4_name->ret);
			addlist(myconfig, "g4_auto_on", g4_auto_on->ret);
			addlist(myconfig, "g4_timer", g4_timer->ret);
			addlist(myconfig, "g4_auto_off", g4_auto_off->ret);
			addlist(myconfig, "g4_delay", g4_delay->ret);
			writelist(myconfig, sispmctlconf);
		}
	}
	delownerrc(sis_einstellungen);
	clearscreen(sis_einstellungen);
	return;	
}

void sispmctl_start()
{
	char* cmd = NULL;
	
	if(getwaswakuptimer() != 1)
	{
	
		if(ostrcmp(getlist(myconfig, "g1_auto_on", NULL), "on") == 0 )
		{
			if(ostrcmp(getlist(myconfig, "g1_delay", NULL), "0") == 0)
				cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o1 > /tmp/helpsis", 0, 0);
			else
			{
				cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -A1 --Aafter ", 0, 0);
				cmd = ostrcat(cmd, getlist(myconfig, "g1_delay", NULL), 1, 0);
				cmd = ostrcat(cmd, " --Ado on", 1, 0);
			}
			system(cmd);
			printf("%s\n", cmd);
			free(cmd); cmd = NULL;
		}
		if(ostrcmp(getlist(myconfig, "g2_auto_on", NULL), "on") == 0)
		{
			if(ostrcmp(getlist(myconfig, "g2_delay", NULL), "0") == 0)
				cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o2 > /tmp/helpsis", 0, 0);
			else
			{
				cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -A2 --Aafter ", 0, 0);
				cmd = ostrcat(cmd, getlist(myconfig, "g2_delay", NULL), 1, 0);
				cmd = ostrcat(cmd, " --Ado on", 1, 0);
			}
			system(cmd);
			printf("%s\n", cmd);
			free(cmd); cmd = NULL;
		}
		if(ostrcmp(getlist(myconfig, "g3_auto_on", NULL), "on") == 0)
		{
			if(ostrcmp(getlist(myconfig, "g3_delay", NULL), "0") == 0)
				cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o3 > /tmp/helpsis", 0, 0);
			else
			{
				cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -A3 --Aafter ", 0, 0);
				cmd = ostrcat(cmd, getlist(myconfig, "g3_delay", NULL), 1, 0);
				cmd = ostrcat(cmd, " --Ado on", 1, 0);
			}
			system(cmd);
			printf("%s\n", cmd);
			free(cmd); cmd = NULL;
		}
		if(ostrcmp(getlist(myconfig, "g4_auto_on", NULL), "on") == 0)
		{
			if(ostrcmp(getlist(myconfig, "g4_delay", NULL), "0") == 0)
				cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o4 > /tmp/helpsis", 0, 0);
			else
			{
				cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -A4 --Aafter ", 0, 0);
				cmd = ostrcat(cmd, getlist(myconfig, "g4_delay", NULL), 1, 0);
				cmd = ostrcat(cmd, " --Ado on", 1, 0);
			}
			system(cmd);
			printf("%s\n", cmd);
			free(cmd); cmd = NULL;
		}
	}
}

void sispmctl_stop()
{
	char* cmd = NULL;
	if(ostrcmp(getlist(myconfig, "g1_auto_off", NULL), "on") == 0)
	{
		if(ostrcmp(getlist(myconfig, "g1_delay", NULL), "0") == 0)
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o1 > /tmp/helpsis", 0, 0);
		else
		{
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -A1 --Aafter ", 0, 0);
			cmd = ostrcat(cmd, getlist(myconfig, "g1_delay", NULL), 1, 0);
			cmd = ostrcat(cmd, " --Ado off", 1, 0);
		}
		system(cmd);
		printf("%s\n", cmd);
		free(cmd); cmd = NULL;
	}
	if(ostrcmp(getlist(myconfig, "g2_auto_off", NULL), "on") == 0)
	{
		if(ostrcmp(getlist(myconfig, "g2_delay", NULL), "0") == 0)
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o2 > /tmp/helpsis", 0, 0);
		else
		{
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -A2 --Aafter ", 0, 0);
			cmd = ostrcat(cmd, getlist(myconfig, "g2_delay", NULL), 1, 0);
			cmd = ostrcat(cmd, " --Ado off", 1, 0);
		}
		system(cmd);
		printf("%s\n", cmd);
		free(cmd); cmd = NULL;
	}
	if(ostrcmp(getlist(myconfig, "g3_auto_off", NULL), "on") == 0)
	{
		if(ostrcmp(getlist(myconfig, "g3_delay", NULL), "0") == 0)
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o3 > /tmp/helpsis", 0, 0);
		else
		{
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -A3 --Aafter ", 0, 0);
			cmd = ostrcat(cmd, getlist(myconfig, "g3_delay", NULL), 1, 0);
			cmd = ostrcat(cmd, " --Ado off", 1, 0);
		}
		system(cmd);
		printf("%s\n", cmd);
		free(cmd); cmd = NULL;
	}
	if(ostrcmp(getlist(myconfig, "g4_auto_off", NULL), "on") == 0)
	{
		if(ostrcmp(getlist(myconfig, "g4_delay", NULL), "0") == 0)
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -o4 > /tmp/helpsis", 0, 0);
		else
		{
			cmd = ostrcat(createpluginpath("/sispmctl/bin/sispmctl", 0), " -A4 --Aafter ", 0, 0);
			cmd = ostrcat(cmd, getlist(myconfig, "g4_delay", NULL), 1, 0);
			cmd = ostrcat(cmd, " --Ado off", 1, 0);
		}
		system(cmd);
		printf("%s\n", cmd);
		free(cmd); cmd = NULL;
	}
}

void sispmctl_main()
{
	int rcret = 0;
	
	rcret = sispmctl_Anzeige();
	if(rcret == 1)
		sispmctl_settings();	

}