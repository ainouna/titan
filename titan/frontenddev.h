#ifndef FRONTENDDEV_H
#define FRONTENDDEV_H

int calclof(struct dvbdev* node, struct transponder* tpnode, char* feaktlnb, int flag)
{
	debug(1000, "in");
	int loftype = 0;
	int lofl, lofh, lofthreshold;

	if(node == NULL || tpnode == NULL)
	{
		debug(1000, "out-> NULL detect");
		return -1;
	}

	if(node->feinfo->type != FE_QPSK)
		return 0;

	int frequency = tpnode->frequency;

	if(feaktlnb == NULL) feaktlnb = node->feaktlnb;

	loftype = getconfigint("lnb_loftype", feaktlnb);
        switch(loftype)
        {
		case 1: //c-band
			lofl = 5150 * 1000;
			lofh = 5150 * 1000;
			lofthreshold = 5150 * 1000;
			break;
		case 2: //user
			lofl = getconfigint("lnb_lofl", feaktlnb) * 1000;
			lofh = getconfigint("lnb_lofh", feaktlnb) * 1000;
			lofthreshold = getconfigint("lnb_lofthreshold", feaktlnb) * 1000;
			break;
		default: //standard lnb
			lofl = 9750 * 1000;
			lofh = 10600 * 1000;
			lofthreshold = 11700 * 1000;
        }

	if(lofthreshold && lofh && frequency >= lofthreshold)
	{
		if(flag == 1) return 1;
		node->feaktband = 1;
	}
	else
	{
		if(flag == 1) return 0;
		node->feaktband = 0;
	}

	if(node->feaktband)
		node->feloffrequency = frequency - lofh;
	else
	{
		if(frequency < lofl)
			node->feloffrequency = lofl - frequency;
		else
			node->feloffrequency = frequency - lofl;
	}

	debug(200, "tuning to freq %d (befor lof %d), band=%d", node->feloffrequency, frequency, node->feaktband);
	return node->feaktband;
}

char* fegettypestr(struct dvbdev* dvbnode)
{
	char* text = NULL;

	if(dvbnode == NULL)
	{
		debug(1000, "out -> NULL detect");
		return NULL;
	}

	switch(dvbnode->feinfo->type)
	{
		case FE_QPSK: text = ostrcat(text, "DVB-S", 1, 0); break;
		case FE_QAM: text = ostrcat(text, "DVB-C", 1, 0); break;
		case FE_OFDM: text = ostrcat(text, "DVB-T", 1, 0); break;
		default: text = ostrcat(text, "unknown", 1, 0);
	}

	return text;
}

struct dvbdev* fegetbyshortname(char* feshortname)
{
	struct dvbdev* dvbnode = dvbdev;
	
	while(dvbnode != NULL)
	{
		if(dvbnode->type == FRONTENDDEV && ostrcmp(dvbnode->feshortname, feshortname) == 0)
			return dvbnode;
		dvbnode = dvbnode->next;
	}
	return NULL;
}

void fegetconfig(struct dvbdev *dvbnode, struct transponder *tpnode, char** aktlnb, char** aktdiseqc, char* tmpnr)
{
	char* tmpstr = NULL;

	if(dvbnode == NULL || tpnode == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}

	tmpstr = ostrcat(dvbnode->feshortname, "_lnb", 0, 0);
	*aktlnb = getconfig(tmpstr, tmpnr);

	free(tmpstr); tmpstr = NULL;
	tmpstr = ostrcat(dvbnode->feshortname, "_diseqc", 0, 0);
	*aktdiseqc = getconfig(tmpstr, tmpnr);
	free(tmpstr); tmpstr = NULL;
}

struct dvbdev* fegetdummy()
{
	struct dvbdev* dvbnode = dvbdev;

	while(dvbnode != NULL)
	{
		if(dvbnode->type == FRONTENDDEVDUMMY)
			return dvbnode;
		dvbnode = dvbnode->next;
	}
	return NULL;
}

//flag 0 = normal
//flag 1 = check only
//flag 2 = from record
struct dvbdev* fegetfree(struct transponder* tpnode, int flag, struct dvbdev* dvbfirst)
{
	debug(1000, "in");
	struct dvbdev* dvbnode = NULL;
	struct dvbdev* tmpdvbnode = NULL;
	char* tmpstr = NULL, *tmpnr = NULL, *aktlnb = NULL, *aktdiseqc = NULL;
	int i, orbitalpos = 0, band = 0;

	if(dvbfirst != NULL)
		dvbnode = dvbfirst;
	else
		dvbnode = dvbdev;

	if(tpnode == NULL)
	{
		debug(1000, "out -> NULL detect");
		return NULL;
	}

	//suche tuner der auf der gleichen orbitalpos/frequency/pol/band ist
	while(dvbnode != NULL)
	{
		//FRONTENDDEV first in the list
		if(dvbnode->type != FRONTENDDEV) break;
		if(dvbnode->type == FRONTENDDEV && dvbnode->feinfo->type == tpnode->fetype)
		{
			if(dvbnode->feakttransponder != NULL && dvbnode->feakttransponder->orbitalpos == tpnode->orbitalpos && dvbnode->feakttransponder->frequency == tpnode->frequency && dvbnode->feaktpolarization == tpnode->polarization)
			{
				band = calclof(dvbnode, tpnode, dvbnode->feaktlnb, 1);
				if(dvbnode->feaktband != band)
				{
					dvbnode = dvbnode->next;
					continue;
				}
				dvbnode->felasttransponder = dvbnode->feakttransponder;
				dvbnode->feakttransponder = tpnode;
				if(flag != 1) debug(200, "found tuner with same orbitalpos/frequency/pol/band %s", dvbnode->feshortname);
				return(dvbnode);
			}
		}
		dvbnode = dvbnode->next;
	}
	if(dvbfirst != NULL)
		dvbnode = dvbfirst;
	else
		dvbnode = dvbdev;

	//suche tuner der die gewuenschte orbitalpos kann und nicht belegt ist
	while(dvbnode != NULL)
	{
		//FRONTENDDEV first in the list
		if(dvbnode->type != FRONTENDDEV) break;
		if(dvbnode->type == FRONTENDDEV && dvbnode->feinfo->type == tpnode->fetype && dvbnode->felock == 0)
		{
			if(flag == 2 && status.aktservice->fedev == dvbnode)
			{
				dvbnode = dvbnode->next;
				continue;
			}

			//check if tuner is loop and looptuner is locked
			tmpstr = getconfigbyval(dvbnode->feshortname, NULL);
			if(tmpstr != NULL) //found loop tuner
			{
				tmpdvbnode = fegetbyshortname(tmpstr);
				if(tmpdvbnode != NULL && tmpdvbnode->feakttransponder != NULL && tmpdvbnode->feaktpolarization != tpnode->polarization && (tmpdvbnode->felock != 0 || (flag == 2 && tmpdvbnode->felock == 0)))
				{
					dvbnode = dvbnode->next;
					continue;
				}
			}

			tmpstr = ostrcat(dvbnode->feshortname, "_sat", 0, 0);
			for(i = 1; i <= status.maxsat; i++)
			{
				tmpnr = oitoa(i);

				orbitalpos = getconfigint(tmpstr, tmpnr);
				if(orbitalpos == tpnode->orbitalpos)
				{
					fegetconfig(dvbnode, tpnode, &aktlnb, &aktdiseqc, tmpnr);
					band = calclof(dvbnode, tpnode, aktlnb, 1);
					if(tmpdvbnode != NULL && tmpdvbnode->feaktband != band && (tmpdvbnode->felock != 0 || (flag == 2 && tmpdvbnode->felock == 0)))
					{
						free(tmpnr); tmpnr = NULL;
						continue;
					}
					if(flag == 1)
					{
						free(tmpstr); tmpstr = NULL;
						free(tmpnr); tmpnr = NULL;
						return dvbnode;
					}
					if(tmpdvbnode != NULL)
					{
						tmpdvbnode->feaktband = band;
						tmpdvbnode->feaktpolarization = tpnode->polarization;
					}
					dvbnode->felasttransponder = dvbnode->feakttransponder;
					dvbnode->feakttransponder = tpnode;
					dvbnode->feaktpolarization = tpnode->polarization;
					free(dvbnode->feaktlnb);
					if(aktlnb != NULL)
						dvbnode->feaktlnb = ostrcat(aktlnb, "", 0, 0);
					else
						dvbnode->feaktlnb = NULL;
					free(dvbnode->feaktdiseqc);
					if(aktdiseqc != NULL)
						dvbnode->feaktdiseqc = ostrcat(aktdiseqc, "", 0, 0);
					else
						dvbnode->feaktdiseqc = NULL;

					free(tmpstr); tmpstr = NULL;
					free(tmpnr); tmpnr = NULL;
					if(flag != 1) debug(200, "found free tuner witch same orbitalpos %s", dvbnode->feshortname);
					return dvbnode;
				}
				free(tmpnr); tmpnr = NULL;
			}
			free(tmpstr); tmpstr = NULL;
		}
		dvbnode = dvbnode->next;
	}
	if(dvbfirst != NULL)
		dvbnode = dvbfirst;
	else
		dvbnode = dvbdev;

	//suche loop tuner, wo der haupttuner 
	//die gewuenschte orbitalpos kann, nicht belegt ist
	//und auf der gleichen poarization/band ist, wo wir hintunen wollen
	while(dvbnode != NULL)
	{
		//FRONTENDDEV first in the list
		if(dvbnode->type != FRONTENDDEV) break;
		if(dvbnode->type == FRONTENDDEV && dvbnode->feinfo->type == tpnode->fetype && dvbnode->felock == 0)
		{
			if(flag == 2 && status.aktservice->fedev == dvbnode)
			{
				dvbnode = dvbnode->next;
				continue;
			}

			//check if tuner is loop an looptuner is locked
			tmpstr = getconfig(dvbnode->feshortname, NULL);
			if(tmpstr != NULL) //found loop tuner
			{
				tmpdvbnode = fegetbyshortname(tmpstr);
				if(tmpdvbnode != NULL && tmpdvbnode->feakttransponder != NULL && tmpdvbnode->feaktpolarization != tpnode->polarization && (tmpdvbnode->felock != 0 || (flag == 2 && tmpdvbnode->felock == 0)))
				{
					dvbnode = dvbnode->next;
					continue;
				}
			}
			else
			{
				dvbnode = dvbnode->next;
				continue;
			}

			tmpstr = ostrcat(tmpdvbnode->feshortname, "_sat", 0, 0);
			for(i = 1; i <= status.maxsat; i++)
			{
				tmpnr = oitoa(i);
				orbitalpos = getconfigint(tmpstr, tmpnr);
				if(orbitalpos == tpnode->orbitalpos)
				{ 
					fegetconfig(tmpdvbnode, tpnode, &aktlnb, &aktdiseqc, tmpnr);
					band = calclof(dvbnode, tpnode, aktlnb, 1);
					if(tmpdvbnode != NULL && tmpdvbnode->feaktband != band && (tmpdvbnode->felock != 0 || (flag == 2 && tmpdvbnode->felock == 0)))
					{
						free(tmpnr); tmpnr = NULL;
						continue;
					}
					if(flag == 1)
					{
						free(tmpstr); tmpstr = NULL;
						free(tmpnr); tmpnr = NULL;
						return dvbnode;
					}
					if(tmpdvbnode != NULL)
					{
						tmpdvbnode->feaktband = band;
						tmpdvbnode->feaktpolarization = tpnode->polarization;
					}
					dvbnode->felasttransponder = dvbnode->feakttransponder;
					dvbnode->feakttransponder = tpnode;
					dvbnode->feaktpolarization = tpnode->polarization;
					free(dvbnode->feaktlnb);
					if(aktlnb != NULL)
						dvbnode->feaktlnb = ostrcat(aktlnb, "", 0, 0);
					else
						dvbnode->feaktlnb = NULL;
					free(dvbnode->feaktdiseqc);
					if(aktdiseqc != NULL)
						dvbnode->feaktdiseqc = ostrcat(aktdiseqc, "", 0, 0);
					else
						dvbnode->feaktdiseqc = NULL;
					free(tmpstr); tmpstr = NULL;
					free(tmpnr); tmpnr = NULL;
					if(flag != 1) debug(200, "found free looptuner witch same orbitalpos/polarization/band %s", dvbnode->feshortname);
					return dvbnode;
				}
				free(tmpnr); tmpnr = NULL;
			}
			free(tmpstr); tmpstr = NULL;
		}
		dvbnode = dvbnode->next;
	}

	debug(1000, "out");
	return NULL;
}

int feopen(struct dvbdev* node, char *fedev)
{
	debug(1000, "in");
	int fd = -1;

	if(node != NULL)
	{	
		if((fd = open(node->dev, O_RDWR | O_NONBLOCK)) < 0)
			debug(200, "open frontend failed %s", node->dev);
		node->fd = fd;
	}
	else
	{
		if((fd = open(fedev, O_RDWR | O_NONBLOCK)) < 0)
			debug(200, "open frontend failed %s", fedev);
	}

	closeonexec(fd);
	debug(1000, "out");
	return fd;
}

void feclose(struct dvbdev* node, int fd)
{
	debug(1000, "in");

	if(node != NULL)
	{
		close(node->fd);
		node->fd = -1;
	}
	else
		close(fd);

	debug(1000, "out");
}

int fegetunlock(struct dvbdev* node)
{
	debug(1000, "in");
	fe_status_t status;

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return 1;
	}

#ifdef SIMULATE
	return 0;
#endif

	if(ioctl(node->fd, FE_READ_STATUS, &status) == -1)
		perr("FE_READ_STATUS");

	debug(1000, "out");
        if(status & FE_HAS_LOCK)
		return 0;
	else
		return 1;
}

int fewait(struct dvbdev* node)
{
	debug(1000, "in");
	//struct dvb_frontend_event ev;
	fe_status_t status;

	int count = 0;

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return 1;
	}

#ifdef SIMULATE
	return 0;
#endif

	//wait for tuner ready
	debug(200, "wait for tuner");
	while(count <= 200)
	{
		count++;
		//ioctl(node->fd, FE_GET_EVENT, &ev);
		//if(ev.status & FE_HAS_LOCK)
		//	return 0;
        	ioctl(node->fd, FE_READ_STATUS, &status);
		if(status & FE_HAS_LOCK)
			return 0;
		usleep(1000);
	}

	debug(1000, "out");
	//if(ev.status & FE_HAS_LOCK)
	//	return 0;
	if(status & FE_HAS_LOCK)
		return 0;
	else
		return 1;
}

void fegetfrontend(struct dvbdev* node)
{
	debug(1000, "in");

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}

#if DVB_API_VERSION >= 5
	struct dtv_property p[8];
	struct dtv_properties cmdseq;
	cmdseq.props = p;

	p[0].cmd = DTV_DELIVERY_SYSTEM;
	p[1].cmd = DTV_FREQUENCY;
	p[2].cmd = DTV_MODULATION;
	p[3].cmd = DTV_SYMBOL_RATE;
	p[4].cmd = DTV_INNER_FEC;
	p[5].cmd = DTV_INVERSION;
	p[6].cmd = DTV_ROLLOFF;
	p[7].cmd = DTV_PILOT;
	cmdseq.num = 8;
	
	if(ioctl(node->fd, FE_GET_PROPERTY, &cmdseq) < 0)
	{
		perr("FE_GET_PROPERTY");
	}
	else
	{
		debug(200, "frontend akt delivery system = %d", p[0].u.data);
		debug(200, "frontend akt frequency = %d", p[1].u.data);
		debug(200, "frontend akt inversion = %d", p[5].u.data);
		debug(200, "frontend akt symbol_rate = %d", p[3].u.data);
		debug(200, "frontend akt fec_inner = %d", p[4].u.data);
		debug(200, "frontend akt modulation = %d", p[2].u.data);
		debug(200, "frontend akt rolloff = %d", p[6].u.data);
		debug(200, "frontend akt pilot = %d", p[7].u.data);
	}
#else
	struct dvb_frontend_parameters fe_param;

	if(ioctl(node->fd, FE_GET_FRONTEND, &fe_param) < 0)
	{
		perr("FE_GET_FRONTEND");
	}
	else
	{
		debug(200, "frontend akt frequency = %d", fe_param.frequency);
		debug(200, "frontend akt inversion = %d", fe_param.inversion);
		debug(200, "frontend akt u.qpsk.symbol_rate = %d", fe_param.u.qpsk.symbol_rate);
		debug(200, "frontend akt u.qam.symbol_rate = %d", fe_param.u.qam.symbol_rate);
		debug(200, "frontend akt u.qpsk.fec_inner = %d", fe_param.u.qpsk.fec_inner);
		debug(200, "frontend akt u.qam.fec_inner = %d", fe_param.u.qam.fec_inner);
		debug(200, "frontend akt u.qam.modulation = %d", fe_param.u.qam.modulation);
	}
#endif

	debug(1000, "out");
}

void fesettone(struct dvbdev* node, fe_sec_tone_mode_t tone, int wait)
{
	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}

	debug(200, "FE_SET_TONE: %d", tone);
	if(ioctl(node->fd, FE_SET_TONE, tone) == -1)
		perr("FE_SET_TONE");
	usleep(wait * 1000);
	debug(1000, "out");
}

void fesetvoltage(struct dvbdev* node, fe_sec_voltage_t volt, int wait)
{
	debug(1000, "in");
	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}

	debug(200, "FE_SET_VOLT: %d", volt);
	if(ioctl(node->fd, FE_SET_VOLTAGE, volt) == -1)
		perr("FE_SET_VOLTAGE");
	usleep(wait * 1000);
	debug(1000, "out");
}

void fediseqcsendburst(struct dvbdev* node, fe_sec_mini_cmd_t burst, int wait)
{
	debug(1000, "in");
	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}

	debug(200, "FE_DISEQC_SEND_BURST: %d", burst);
	if(ioctl(node->fd, FE_DISEQC_SEND_BURST, burst) == -1)
		perr("FE_DISEQC_SEND_BURST");
	usleep(wait * 1000);
	debug(1000, "out");
}

void fediseqcsendmastercmd(struct dvbdev* node, struct dvb_diseqc_master_cmd *cmd, int wait)
{
	debug(1000, "in");
	int i, repeat = 0; 

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}

	repeat = getconfigint("diseqc_repeat", node->feaktdiseqc); 
	if(repeat < 1) repeat = 1;

	for(i = 0; i < repeat; i++)
	{
		if(ioctl(node->fd, FE_DISEQC_SEND_MASTER_CMD, cmd) == -1)
		{
			perr("FE_DISEQC_SEND_MASTER_CMD");
		}
		usleep(wait * 1000);
	}
	debug(1000, "out");
}

void fesdiseqcpoweron(struct dvbdev* node)
{
	debug(1000, "in");
	struct dvb_diseqc_master_cmd cmd = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0};

	cmd.msg[0] = 0xE0;
	cmd.msg[1] = 0x00;
	cmd.msg[2] = 0x03;
	cmd.msg_len = 3;

	debug(200, "DISEQC Power on");
	fediseqcsendmastercmd(node, &cmd, 100);
	debug(1000, "out");
}

void fesdiseqcreset(struct dvbdev* node)
{
	debug(1000, "in");
	struct dvb_diseqc_master_cmd cmd = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0};

	cmd.msg[0] = 0xE0;
	cmd.msg[1] = 0x00;
	cmd.msg[2] = 0x00;
	cmd.msg_len = 3;

	debug(200, "DISEQC Reset");
	fediseqcsendmastercmd(node, &cmd, 100);
	debug(1000, "out");
}

void fesdiseqcstandby(struct dvbdev* node)
{
	debug(1000, "in");
	struct dvb_diseqc_master_cmd cmd = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0};
	
	cmd.msg[0] = 0xE0;
	cmd.msg[1] = 0x00;
	cmd.msg[2] = 0x02;
	cmd.msg_len = 3;

	debug(200, "DISEQC Standby");
	fediseqcsendmastercmd(node, &cmd, 100);
	debug(1000, "out");
}

float ferotorangle(int pos)
{
	int val = 0, orbitalpos = 0;
	char* tmpnr = NULL;
	float angle = -1;

	tmpnr = oitoa(pos);
	val = getconfigint("rotorpos", tmpnr);
	free(tmpnr); tmpnr = NULL;

	if(val != 0)
	{
		orbitalpos = val & 0xffff;
		if(((val >> 24) & 0xff) == 0) //west
			angle = 360 - orbitalpos / 10;
		else //east
			angle = orbitalpos / 10;
	}

	return angle;
}

void fediseqcrotor(struct dvbdev* node, int pos, int oldpos, int flag)
{
	debug(1000, "in");
	struct dvb_diseqc_master_cmd cmd = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0};
	//float speed13V = 1.5;
	float speed18V = 2.4;
	float degreesmov, a1, a2, waittime;
	int i;
	
	switch(flag)
	{
		case 0: //stop move
			cmd.msg[0] = 0xE0; cmd.msg[1] = 0x31; cmd.msg[2] = 0x60; cmd.msg_len = 3;
			debug(200, "DISEQC Rotorpos (stop move)");
			break;
		case 1: //disable limits
			cmd.msg[0] = 0xE0; cmd.msg[1] = 0x31; cmd.msg[2] = 0x63; cmd.msg_len = 3;
			debug(200, "DISEQC Rotorpos (disable limits)");
			break;
		case 2: //enable limits
			cmd.msg[0] = 0xE0; cmd.msg[1] = 0x31; cmd.msg[2] = 0x6A; cmd.msg[3] = 0x00; cmd.msg_len = 4;
			debug(200, "DISEQC Rotorpos (enable limits)");
			break;
		case 3: //set east limit
			cmd.msg[0] = 0xE0; cmd.msg[1] = 0x31; cmd.msg[2] = 0x66; cmd.msg_len = 3;
			debug(200, "DISEQC Rotorpos (set east limit)");
			break;
		case 4: //set west limit
			cmd.msg[0] = 0xE0; cmd.msg[1] = 0x31; cmd.msg[2] = 0x67; cmd.msg_len = 3;
			debug(200, "DISEQC Rotorpos (set west limit)");
			break;
		case 5: //move east cont.
			cmd.msg[0] = 0xE0; cmd.msg[1] = 0x31; cmd.msg[2] = 0x68; cmd.msg[3] = 0x00; cmd.msg_len = 4;
			debug(200, "DISEQC Rotorpos (move east cont.)");
			break;
		case 6: //move west cont.
			cmd.msg[0] = 0xE0; cmd.msg[1] = 0x31; cmd.msg[2] = 0x69; cmd.msg[3] = 0x00; cmd.msg_len = 4;
			debug(200, "DISEQC Rotorpos (move west cont.)");
			break;
		case 7: //store pos
			cmd.msg[0] = 0xE0; cmd.msg[1] = 0x31; cmd.msg[2] = 0x6A; cmd.msg[3] = pos; cmd.msg_len = 4;
			debug(200, "DISEQC Rotorpos (store pos)");
			break;
		case 8: //goto pos
			cmd.msg[0] = 0xE0; cmd.msg[1] = 0x31; cmd.msg[2] = 0x6B; cmd.msg[3] = pos; cmd.msg_len = 4;
			debug(200, "DISEQC Rotorpos (goto pos)");
			break;
	}

	if(flag >= 0 && flag < 7)
	{
		fesettone(node, SEC_VOLTAGE_18, 15);
		fesettone(node, SEC_TONE_OFF, 15);
		fediseqcsendmastercmd(node, &cmd, 100);
	}

	if(flag == 7 && pos != 0)
	{
		fesettone(node, SEC_VOLTAGE_18, 15);
		fesettone(node, SEC_TONE_OFF, 15);
		fediseqcsendmastercmd(node, &cmd, 100);
	}

	if(flag == 8 && pos != 0 && pos != oldpos)
	{
		fesettone(node, SEC_VOLTAGE_18, 15);
		fesettone(node, SEC_TONE_OFF, 15);
		fediseqcsendmastercmd(node, &cmd, 100);

		if(oldpos == 0)
			waittime = 15;
		else
		{
			a1 = ferotorangle(pos);
			a2 = ferotorangle(oldpos);
			degreesmov = abs(a1 - a2);
			if(degreesmov > 180) degreesmov = 360 - degreesmov;
			waittime = degreesmov / speed18V;
		}

		for(i = 0; i < 10; i++)
			usleep(waittime * 100000);
	}
	debug(1000, "out");
}

//TODO
#if 0
void CFrontend::sendMotorCommand(uint8_t cmdtype, uint8_t address, uint8_t command, uint8_t num_parameters, uint8_t parameter1, uint8_t parameter2, int repeat)
{
	struct dvb_diseqc_master_cmd cmd;
	int i;

	printf("[fe%d] sendMotorCommand: cmdtype   = %x, address = %x, cmd   = %x\n", fenumber, cmdtype, address, command);
	printf("[fe%d] sendMotorCommand: num_parms = %d, parm1   = %x, parm2 = %x\n", fenumber, num_parameters, parameter1, parameter2);

	cmd.msg[0] = cmdtype;	//command type
	cmd.msg[1] = address;	//address
	cmd.msg[2] = command;	//command
	cmd.msg[3] = parameter1;
	cmd.msg[4] = parameter2;
	cmd.msg_len = 3 + num_parameters;
	secSetVoltage(SEC_VOLTAGE_13, 15);
	secSetTone(SEC_TONE_OFF, 25);

	for(i = 0; i <= repeat; i++)
		sendDiseqcCommand(&cmd, 50);

	printf("[fe%d] motor command sent.\n", fenumber);

}

void CFrontend::sendDiseqcSmatvRemoteTuningCommand(const uint32_t frequency)
{
	/* [0] from master, no reply, 1st transmission
	 * [1] intelligent slave interface for multi-master bus
	 * [2] write channel frequency
	 * [3] frequency
	 * [4] frequency
	 * [5] frequency */

	struct dvb_diseqc_master_cmd cmd = {
		{0xe0, 0x71, 0x58, 0x00, 0x00, 0x00}, 6
	};

	cmd.msg[3] = (((frequency / 10000000) << 4) & 0xF0) | ((frequency / 1000000) & 0x0F);
	cmd.msg[4] = (((frequency / 100000) << 4) & 0xF0) | ((frequency / 10000) & 0x0F);
	cmd.msg[5] = (((frequency / 1000) << 4) & 0xF0) | ((frequency / 100) & 0x0F);

	sendDiseqcCommand(&cmd, 15);
}

int CFrontend::driveToSatellitePosition(t_satellite_position satellitePosition, bool from_scan)
{
	int waitForMotor = 0;
	int new_position = 0, old_position = 0;
	bool use_usals = 0;

	//if(diseqcType == DISEQC_ADVANCED) //FIXME testing
	{
		printf("[fe0] SatellitePosition %d -> %d\n", currentSatellitePosition, satellitePosition);
		bool moved = false;

		sat_iterator_t sit = satellitePositions.find(satellitePosition);
		if (sit == satellitePositions.end()) {
			printf("[fe0] satellite position %d not found!\n", satellitePosition);
			return 0;
		} else {
			new_position = sit->second.motor_position;
			use_usals = sit->second.use_usals;
		}
		sit = satellitePositions.find(currentSatellitePosition);
		if (sit != satellitePositions.end())
			old_position = sit->second.motor_position;

		printf("[fe0] motorPosition %d -> %d usals %s\n", old_position, new_position, use_usals ? "on" : "off");

		if (currentSatellitePosition == satellitePosition)
			return 0;

		if (use_usals) {
			gotoXX(satellitePosition);
			moved = true;
		} else {
			if (new_position > 0) {
				positionMotor(new_position);
				moved = true;
			}
		}

		if (from_scan || (new_position > 0 && old_position > 0)) {
			waitForMotor = motorRotationSpeed ? 2 + abs(satellitePosition - currentSatellitePosition) / motorRotationSpeed : 0;
		}
		if (moved) {
			//currentSatellitePosition = satellitePosition;
			waitForMotor = motorRotationSpeed ? 2 + abs(satellitePosition - currentSatellitePosition) / motorRotationSpeed : 0;
			currentSatellitePosition = satellitePosition;
		}
	}
	//currentSatellitePosition = satellitePosition;

	return waitForMotor;
}
#endif

void fediseqcset(struct dvbdev* node, struct transponder* tpnode)
{
	debug(1000, "in");
	int toneburst = 0, cmdorder = 0, aktlnb = 1, input = 0, uinput = 0, diseqmode = 0;
	fe_sec_mini_cmd_t mini = -1;
	struct dvb_diseqc_master_cmd cmd = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0};
	struct dvb_diseqc_master_cmd ucmd = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0};
	
	if(node == NULL) return;
	
	input = getconfigint("diseqc_committedcmd", node->feaktdiseqc);
	uinput = getconfigint("diseqc_uncommittedcmd", node->feaktdiseqc);
	diseqmode = getconfigint("diseqc_mode", node->feaktdiseqc);
	cmdorder = getconfigint("diseqc_cmdorder", node->feaktdiseqc);
	toneburst = getconfigint("diseqc_toneburst", node->feaktdiseqc);

	if(node->feaktlnb != NULL)
		aktlnb = atoi(node->feaktlnb);

	debug(200, "set diseqc: number=%d, band=%d, pol=%d", aktlnb, node->feaktband, node->feaktpolarization);
	debug(200, "set diseqc: diseqmode=%d, input=%d, uinput=%d, cmdorder=%d, toneburst=%d", diseqmode, input, uinput, cmdorder, toneburst);
	 
	switch(toneburst)
	{
		case 1: mini = SEC_MINI_A; break;
		case 2: mini = SEC_MINI_B; break;
	}
	
	if(diseqmode == 100) // Tonburst A/B
	{
		debug(200, "set diseqc: Tonburst A/B");
		if(mini == -1)
			mini = (aktlnb - 1) % 2 ? SEC_MINI_B : SEC_MINI_A;
		fediseqcsendburst(node, mini, 15);
		return;
	}
		
	if(diseqmode == 0 || diseqmode == 1) // Diseqc 1.0 + 1.1
	{
		debug(200, "set committed switch");
		cmd.msg[0] = 0xE0;
		cmd.msg[1] = 0x10;
		cmd.msg[2] = 0x38;

		if(input == 0)
			cmd.msg[3] = 0xF0 | ((((aktlnb - 1) * 4) & 0x0F) | (node->feaktband ? 1 : 0) | (node->feaktpolarization ? 0 : 2));
		else
			cmd.msg[3] = 0xF0 + ((input - 1) & 0x0F);

		cmd.msg_len = 4;
	}

	if(diseqmode == 1) // Diseqc 1.1
	{
		if(uinput > 0)
		{
			debug(200, "set uncommitted switch");
			ucmd.msg[0] = 0xE0;
			ucmd.msg[1] = 0x10;
			ucmd.msg[2] = 0x39;
			ucmd.msg[3] = 0xF0 + ((uinput - 1) & 0x0F);
			ucmd.msg_len = 4;
		}
	}
		 
	switch(cmdorder)
	{
		case 1:
			if(mini != -1) fediseqcsendburst(node, mini, 15);
			fediseqcsendmastercmd(node, &cmd, 100);
			break;
		case 2:
			fediseqcsendmastercmd(node, &cmd, 100);
			if(uinput > 0) fediseqcsendmastercmd(node, &ucmd, 100);
			if(mini != -1) fediseqcsendburst(node, mini, 15);
			break;
		case 3:
			if(mini != -1) fediseqcsendburst(node, mini, 15);
			fediseqcsendmastercmd(node, &cmd, 100);
			if(uinput > 0) fediseqcsendmastercmd(node, &ucmd, 100);
			break;
		case 4:
			if(uinput > 0) fediseqcsendmastercmd(node, &ucmd, 100);
			fediseqcsendmastercmd(node, &cmd, 100);
			if(mini != -1) fediseqcsendburst(node, mini, 15);
			break;
		case 5:
			if(mini != -1) fediseqcsendburst(node, mini, 15);
			if(uinput > 0) fediseqcsendmastercmd(node, &ucmd, 100);
			fediseqcsendmastercmd(node, &cmd, 100);
			break;
		default:
			fediseqcsendmastercmd(node, &cmd, 100);
			if(mini != -1) fediseqcsendburst(node, mini, 15);
			break;
	}

	debug(1000, "out");
}

void feset(struct dvbdev* node, struct transponder* tpnode)
{
	debug(1000, "in");
	int voltagemode = 0, tonemode = 0;
	fe_sec_tone_mode_t tone;
	fe_sec_voltage_t volt;
	struct dvbdev* dvbnode = dvbdev;

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}

	// set volage off from other unused frontend
	while(dvbnode != NULL)
	{
		if(dvbnode->type != FRONTENDDEV) break;
		if(dvbnode->type == FRONTENDDEV && dvbnode != node && dvbnode->felock == 0 && dvbnode != status.aktservice->fedev)
		{
			fesetvoltage(dvbnode, SEC_VOLTAGE_OFF, 15);
		}
		dvbnode = dvbnode->next;
	}

	calclof(node, tpnode, NULL, 0);

	voltagemode = getconfigint("lnb_voltagemode", node->feaktlnb); 
	switch(voltagemode)
	{
		case 1: volt = SEC_VOLTAGE_13; break;
		case 2: volt = SEC_VOLTAGE_18; break;
		default: volt = node->feaktpolarization ? SEC_VOLTAGE_13 : SEC_VOLTAGE_18;
	}
	fesetvoltage(node, volt, 15);

	if(node->feaktdiseqc == NULL)
	{
		debug(200, "don't use diseqc");
	}
	else
	{
		fesettone(node, SEC_TONE_OFF, 15);
		fediseqcset(node, tpnode);
	}

	tonemode = getconfigint("lnb_tonemode", node->feaktlnb); 
	switch(tonemode)
	{
		case 1: tone = SEC_TONE_ON; break;
		case 2: tone = SEC_TONE_OFF; break;
		default: tone = node->feaktband ? SEC_TONE_ON : SEC_TONE_OFF;
	}
	fesettone(node, tone, 15);
	debug(1000, "out");
}

void fediscard(struct dvbdev* node)
{
	debug(1000, "in");
	struct dvb_frontend_event ev;
	int count = 0;

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}

	/* discard stale QPSK events */
	while(count < 20)
	{
		count++;
		if(ioctl(node->fd, FE_GET_EVENT, &ev) == -1)
			break;
	}

	debug(1000, "out");
}

uint16_t fereadsnr(struct dvbdev* node)
{
	debug(1000, "in");
	uint16_t snr = 0;

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return 0;
	}

	ioctl(node->fd, FE_READ_SNR, &snr);
	debug(200, "frontend snr = %02x", (snr * 100) / 0xffff);
	debug(1000, "out");
	return snr;
}

uint16_t fereadsignalstrength(struct dvbdev* node)
{
	debug(1000, "in");
	uint16_t signal = 0;

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return 0;
	}

	ioctl(node->fd, FE_READ_SIGNAL_STRENGTH, &signal);
	debug(200, "frontend signal = %02x", (signal * 100) / 0xffff);
	debug(1000, "out");
	return signal;
}

uint32_t fereadber(struct dvbdev* node)
{
	debug(1000, "in");
	uint32_t ber = 0;

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return 0;
	}

	ioctl(node->fd, FE_READ_BER, &ber);
	debug(200, "frontend ber = %02x", ber);
	debug(1000, "out");
	return ber;
}

uint32_t fereaduncorrectedblocks(struct dvbdev* node)
{
	debug(1000, "in");
	uint32_t unc = 0;

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return 0;
	}

	ioctl(node->fd, FE_READ_UNCORRECTED_BLOCKS, &unc);
	debug(200, "frontend unc = %02x", unc);
	debug(1000, "out");
	return unc;
}

fe_status_t fereadstatus(struct dvbdev* node)
{
	debug(1000, "in");
	fe_status_t status;

	if(node == NULL)
	{
		debug(1000, "out-> NULL detect");
		return -1;
	}

	if(ioctl(node->fd, FE_READ_STATUS, &status) == -1)
		perr("FE_READ_STATUS");

	debug(200, "frontend status = %02x", status);
	if(status & FE_HAS_LOCK) debug(200, "frontend = FE_HAS_LOCK");
	if(status & FE_HAS_SIGNAL) debug(200, "frontend = FE_HAS_SIGNAL");
	if(status & FE_HAS_CARRIER) debug(200, "frontend = FE_HAS_CARRIER");
	if(status & FE_HAS_VITERBI) debug(200, "frontend = FE_HAS_VITERBI");
	if(status & FE_HAS_SYNC) debug(200, "frontend = FE_HAS_SYNC");
	if(status & FE_TIMEDOUT) debug(200, "frontend = FE_TIMEDOUT");
	if(status & FE_REINIT) debug(200, "frontend = FE_REINIT");

	debug(1000, "out");
	return status;
}

void fetunedvbs(struct dvbdev* node, struct transponder* tpnode)
{
	debug(1000, "in");
	if(node == NULL || tpnode == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}

#if DVB_API_VERSION >= 5
	struct dtv_property p[10];
	struct dtv_properties cmdseq;
	cmdseq.props = p;

	//convert transponderlist for dvbapi5
	int system = tpnode->system;
	switch(system)
	{
		case 0: system = SYS_DVBS; break;
		case 1: system = SYS_DVBS2; break;
		default: system = SYS_DVBS; break;
	}

	int fec = tpnode->fec;
	switch(fec)
	{
		case 0: fec = FEC_AUTO; break;
		case 1: fec = FEC_1_2; break;
		case 2: fec = FEC_2_3; break;
		case 3: fec = FEC_3_4; break;
		case 4: fec = FEC_5_6; break;
		case 5: fec = FEC_7_8; break;
		case 6: fec = FEC_8_9; break;
		case 7: fec = FEC_3_5; break;
		case 8: fec = FEC_4_5; break;
		case 9: fec = FEC_9_10; break;
		case 15: fec = FEC_NONE; break;
		default: fec = FEC_AUTO; break;
	}
	
	int pilot = tpnode->pilot;
	switch(pilot)
	{
		case 0: pilot = PILOT_OFF; break;
		case 1: pilot = PILOT_ON; break;
		case 2: pilot = PILOT_AUTO; break;
		default: pilot = PILOT_AUTO; break;
	}

	int rolloff = tpnode->rolloff;
	switch(rolloff)
	{
		case 0: rolloff = ROLLOFF_35; break;
		case 1: rolloff = ROLLOFF_25; break;
		case 2: rolloff = ROLLOFF_20; break;
		default: rolloff = ROLLOFF_35; break;
	}

	int modulation = tpnode->modulation;
	switch(modulation)
	{
		case 0: modulation = QPSK; break;
		case 1: modulation = QPSK; break;
		case 2: modulation = PSK_8; break;
		case 3: modulation = QAM_16; break;
		default: modulation = QPSK; break;
	}

	p[0].cmd = DTV_CLEAR;
	p[1].cmd = DTV_DELIVERY_SYSTEM,	p[1].u.data = system;
	p[2].cmd = DTV_FREQUENCY,	p[2].u.data = node->feloffrequency;
	p[3].cmd = DTV_MODULATION,	p[3].u.data = modulation;
	p[4].cmd = DTV_SYMBOL_RATE,	p[4].u.data = tpnode->symbolrate;
	p[5].cmd = DTV_INNER_FEC,	p[5].u.data = fec;
	p[6].cmd = DTV_INVERSION,	p[6].u.data = (fe_spectral_inversion_t) tpnode->inversion;
	if(system == SYS_DVBS2)
	{
		p[7].cmd = DTV_ROLLOFF,		p[7].u.data = rolloff;
		p[8].cmd = DTV_PILOT,		p[8].u.data = pilot;
		p[9].cmd = DTV_TUNE;
		cmdseq.num = 10;
	}
	else
	{
		p[7].cmd = DTV_TUNE;
		cmdseq.num = 8;
	}

	debug(200, "frequ=%d, inversion=%d, pilot=%d, rolloff=%d, fec=%d, sr=%d, modulation=%d, system=%d", node->feloffrequency, tpnode->inversion, pilot, rolloff, fec, tpnode->symbolrate, modulation, system);
#else
	struct dvb_frontend_parameters tuneto;
	fe_spectral_inversion_t inversion = tpnode->inversion;

	//convert transponderlist for dvbapi3
	int fec = tpnode->fec;
	if(tpnode->system == 1)
	{
		if(tpnode->modulation == 1) fec = fec + 9;
		if(tpnode->modulation == 2) fec = fec + 18;
	}

	inversion |= (tpnode->rolloff << 2) | inversion; // use bit 2..3 of inversion for rolloff
	inversion |= (tpnode->pilot << 4) | inversion; // use bit 4..5 of inversion for pilot

	tuneto.frequency = node->feloffrequency;
	tuneto.inversion = inversion;
	tuneto.u.qpsk.symbol_rate = tpnode->symbolrate;
	tuneto.u.qpsk.fec_inner = fec;

	debug(200, "frequ=%d, inversion=%d, pilot=%d, rolloff=%d, fec=%d, sr=%d modulation=%d, system=%d", node->feloffrequency, inversion, tpnode->pilot, tpnode->rolloff, fec, tpnode->symbolrate, tpnode->modulation, tpnode->system);
#endif

	fediscard(node);

#if DVB_API_VERSION >= 5
	if((ioctl(node->fd, FE_SET_PROPERTY, &cmdseq)) == -1)
	{
		perr("FE_SET_PROPERTY");
	}
#else
	if(ioctl(node->fd, FE_SET_FRONTEND, &tuneto) == -1)
	{
		perr("FE_SET_FRONTEND");
	}
#endif
	debug(1000, "out");
}

void fetunedvbc(struct dvbdev* node, struct transponder* tpnode)
{
	debug(1000, "in");

	if(node == NULL || tpnode == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}
	
	int fec = tpnode->fec;
	switch(fec)
	{
		case 0: fec = FEC_AUTO; break;
		case 1: fec = FEC_1_2; break;
		case 2: fec = FEC_2_3; break;
		case 3: fec = FEC_3_4; break;
		case 4: fec = FEC_5_6; break;
		case 5: fec = FEC_7_8; break;
		case 6: fec = FEC_8_9; break;
		case 15: fec = FEC_NONE; break;
		default: fec = FEC_AUTO; break;
	}

	int modulation = tpnode->modulation;
	switch(modulation)
	{
		case 0: modulation = QAM_AUTO; break;
		case 1: modulation = QAM_16; break;
		case 2: modulation = QAM_32; break;
		case 3: modulation = QAM_64; break;
		case 4: modulation = QAM_128; break;
		case 5: modulation = QAM_256; break;
		default: modulation = QAM_AUTO; break;
	}

#if DVB_API_VERSION >= 5
	struct dtv_property p[8];
	struct dtv_properties cmdseq;
	cmdseq.props = p;

	p[0].cmd = DTV_CLEAR;
	p[1].cmd = DTV_DELIVERY_SYSTEM, p[1].u.data = tpnode->system;
	p[2].cmd = DTV_FREQUENCY,	p[2].u.data = tpnode->frequency;
	p[3].cmd = DTV_MODULATION,	p[3].u.data = modulation;
	p[4].cmd = DTV_SYMBOL_RATE,	p[4].u.data = tpnode->symbolrate;
	p[5].cmd = DTV_INVERSION,	p[5].u.data = (fe_spectral_inversion_t) tpnode->inversion;
	p[6].cmd = DTV_INNER_FEC,	p[6].u.data = fec;
	p[7].cmd = DTV_TUNE;
	cmdseq.num = 8;

	debug(200, "frequ=%d, inversion=%d, fec=%d, sr=%d, modulation=%d, system=%d", tpnode->frequency, tpnode->inversion, fec, tpnode->symbolrate, modulation, tpnode->system);
#else
	struct dvb_frontend_parameters tuneto;

	tuneto.frequency = tpnode->frequency;
	tuneto.inversion = tpnode->inversion;
	tuneto.u.qam.symbol_rate = tpnode->symbolrate;
	tuneto.u.qam.fec_inner = tpnode->fec;
	tuneto.u.qam.modulation = tpnode->modulation;

	debug(200, "frequ=%d, inversion=%d, fec=%d, sr=%d, modulation=%d", tpnode->frequency, tpnode->inversion, fec, tpnode->symbolrate, modulation);
#endif

	fediscard(node);

#if DVB_API_VERSION >= 5
	if((ioctl(node->fd, FE_SET_PROPERTY, &cmdseq)) == -1)
	{
		perr("FE_SET_PROPERTY");
	}
#else
	if(ioctl(node->fd, FE_SET_FRONTEND, &tuneto) == -1)
	{
		perr("FE_SET_FRONTEND");
	}
#endif
	debug(1000, "out");
}

void fetunedvbt(struct dvbdev* node, struct transponder* tpnode)
{
	debug(1000, "in");
	struct dvb_frontend_parameters tuneto;

	if(node == NULL || tpnode == NULL)
	{
		debug(1000, "out-> NULL detect");
		return;
	}
	
	int fec = tpnode->fec;
	switch(fec)
	{
		case 0: fec = FEC_1_2; break;
		case 1: fec = FEC_2_3; break;
		case 2: fec = FEC_3_4; break;
		case 3: fec = FEC_5_6; break;
		case 4: fec = FEC_7_8; break;
		case 5: fec = FEC_AUTO; break;
		default: fec = FEC_AUTO; break;
	}
	
	int modulation = tpnode->modulation;
	switch(modulation)
	{
		case 0: modulation = QPSK; break;
		case 1: modulation = QAM_16; break;
		case 2: modulation = QAM_64; break;
		case 3: modulation = QAM_AUTO; break;
		default: modulation = QAM_AUTO; break;
	}
	
/*
	int bandwidth = tpnode->bandwidth;
	switch(bandwidth)
	{
		case 0: bandwidth = BANDWIDTH_8MHZ; break;
		case 1: bandwidth = BANDWIDTH_7MHZ; break;
		case 2: bandwidth = BANDWIDTH_6MHZ; break;
		case 3: bandwidth = BANDWIDTH_AUTO; break;
		default: modulation = BANDWIDTH_AUTO; break;
	}
	
	int transmission = tpnode->transmission;
	switch(transmission)
	{
		case 0: transmission = TRANSMISSIONMODE_2K; break;
		case 1: transmission = TRANSMISSIONMODE_8K; break;
		case 2: transmission = TRANSMISSIONMODE_AUTO; break;
		default: transmission = TRANSMISSIONMODE_AUTO; break;
	}
	
	int guardinterval = tpnode->guardinterval;
	switch(guardinterval)
	{
		case 0: guardinterval = GUARDINTERVAL_1_32; break;
		case 1: guardinterval = GUARDINTERVAL_1_16; break;
		case 2: guardinterval = GUARDINTERVAL_1_8; break;
		case 3: guardinterval = GUARDINTERVAL_1_4; break;
		case 4: guardinterval = GUARDINTERVAL_AUTO; break;
		default: guardinterval = GUARDINTERVAL_AUTO; break;
	}
	
	int hierarchy = tpnode->hierarchy;
	switch(guardinterval)
	{
		case 0: hierarchy = HIERARCHY_NONE;
		case 1: hierarchy = HIERARCHY_1;
		case 2: hierarchy = HIERARCHY_2;
		case 3: hierarchy = HIERARCHY_4;
		case 4: hierarchy = HIERARCHY_AUTO;
		default: hierarchy = HIERARCHY_AUTO; break;
	}
*/

	tuneto.frequency = tpnode->frequency;
	tuneto.inversion = tpnode->inversion;
	//tuneto.u.ofdm.bandwidth = bandwidth;
	tuneto.u.ofdm.code_rate_HP = 0;
	tuneto.u.ofdm.code_rate_LP = 0;
	tuneto.u.ofdm.constellation = 0;
	//tuneto.u.ofdm.transmission_mode = transmission;
	//tuneto.u.ofdm.guard_interval = guardinterval;
	//tuneto.u.ofdm.hierarchy_information = hierarchy;

	fediscard(node);

	if(ioctl(node->fd, FE_SET_FRONTEND, &tuneto) == -1)
	{
		perr("FE_SET_FRONTEND");
	}
	debug(1000, "out");
}

#ifdef SIMULATE
int tunercount = 0;
#endif
struct dvb_frontend_info* fegetinfo(struct dvbdev* node, int fd)
{
	debug(1000, "in");
	struct dvb_frontend_info* feinfo = NULL;
	int tmpfd = -1;

	if(node != NULL)
		tmpfd = node->fd;
	else
		tmpfd = fd;

	feinfo = (struct dvb_frontend_info*)malloc(sizeof(struct dvb_frontend_info));
	if(feinfo == NULL)
	{
		err("no mem");
		return NULL;
	}

#ifndef SIMULATE
	if(ioctl(tmpfd, FE_GET_INFO, feinfo) < 0)
	{
		perr("FE_GET_INFO");
		free(feinfo);
		return NULL;
	}
#else
	tunercount++;
	if(tunercount == 1)
	{
		sprintf(feinfo->name, "%s", "Conax 7500 DVB-C");
		feinfo->type = FE_QAM;
	}
	else
	{
		sprintf(feinfo->name, "%s", "Conax 7500 DVB-S");
		feinfo->type = FE_QPSK;
		//feinfo->type = FE_QAM;
	}
#endif
	debug(1000, "out");
	return feinfo;
}

int fegetdev()
{
	debug(1000, "in");
	int i, y, fd = -1, count = 0;
	char *buf = NULL, *frontenddev = NULL;
	struct dvb_frontend_info* feinfo = NULL;
	struct dvbdev* dvbnode = NULL;

	frontenddev = getconfig("frontenddev", NULL);
	if(frontenddev == NULL)
	{
		debug(1000, "out -> NULL detect");
		return count;
	}

	buf = malloc(MINMALLOC);
	if(buf == NULL)
	{
		err("no memory");
		return count;
	}

	for(i = 0; i < MAXDVBADAPTER; i++)
	{
		for(y = 0; y < MAXFRONTENDDEV; y++)
		{
			sprintf(buf, frontenddev, i, y);
			fd = feopen(NULL, buf);
			if(fd >= 0)
			{
				feinfo = fegetinfo(NULL, fd);
				if(feinfo != NULL)
				{
					count++;
					dvbnode = adddvbdev(buf, i, y, fd, FRONTENDDEV, feinfo, NULL);
					if(dvbnode->feinfo->type == FE_QPSK)
						fesetvoltage(dvbnode, SEC_VOLTAGE_OFF, 15);
				}
			}
		}
	}

	free(buf);
	debug(1000, "out");
	return count;
}

int fecreatedummy()
{
	//create dummy frontend for playback
	char *buf = NULL, *frontenddev = NULL;
	struct dvbdev* dvbnode = NULL;

	frontenddev = getconfig("frontenddev", NULL);
	if(frontenddev == NULL)
	{
		debug(1000, "out -> NULL detect");
		return 1;
	}

	buf = malloc(MINMALLOC);
	if(buf == NULL)
	{
		err("no memory");
		return 1;
	}

	dvbnode = dmxgetlast(0);
	if(dvbnode != NULL)
	{
		sprintf(buf, frontenddev, 0, dvbnode->devnr);
		adddvbdev(buf, 0, dvbnode->devnr, -1, FRONTENDDEVDUMMY, NULL, NULL);
	}

	free(buf);
	return 0;
}

#endif
