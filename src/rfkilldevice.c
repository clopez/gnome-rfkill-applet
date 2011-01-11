/*
 * RFKillApplet
 * An applet for Gnome that lets the user to inhibit the emission of radiation
 * of RF devices with a simple click on the applet’s icon.
 * The device manipulation is done using the special device /dev/rfkill
 *
 * Copyright (C) 2011 Carlos Alberto Lopez Perez <clopez@igalia.com>
 *
 * This code for the manipulation of the rfkill device is based on the
 * user space tool rfkill:
 * http://linuxwireless.org/en/users/Documentation/rfkill
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */




#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/time.h>

#include <glib.h>


#include "rfkill.h"

static const char *get_name(__u32 idx)
{
	static char name[128];
	ssize_t len;
	char *pos, filename[64];
	int fd;

	snprintf(filename, sizeof(filename) - 1,
				"/sys/class/rfkill/rfkill%u/name", idx);

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return NULL;

	memset(name, 0, sizeof(name));
	len = read(fd, name, sizeof(name) - 1);

	pos = strchr(name, '\n');
	if (pos)
		*pos = '\0';

	close(fd);

	return name;
}

static const char *type2string(enum rfkill_type type)
{
	switch (type) {
	case RFKILL_TYPE_ALL:
		return "All";
	case RFKILL_TYPE_WLAN:
		return "Wireless LAN";
	case RFKILL_TYPE_BLUETOOTH:
		return "Bluetooth";
	case RFKILL_TYPE_UWB:
		return "Ultra-Wideband";
	case RFKILL_TYPE_WIMAX:
		return "WiMAX";
	case RFKILL_TYPE_WWAN:
		return "Wireless WAN";
	case RFKILL_TYPE_GPS:
		return "GPS";
	case RFKILL_TYPE_FM:
		return "FM";
	case NUM_RFKILL_TYPES:
		return NULL;
	}

	return NULL;
}

struct rfkill_type_str {
	enum rfkill_type type;
	const char *name;
};
static const struct rfkill_type_str rfkill_type_strings[] = {
	{	.type = RFKILL_TYPE_ALL,		.name = "all"	},
	{	.type = RFKILL_TYPE_WLAN,		.name = "wifi"	},
	{	.type = RFKILL_TYPE_WLAN,		.name = "wlan"	}, /* alias */
	{	.type = RFKILL_TYPE_BLUETOOTH,	.name = "bluetooth"	},
	{	.type = RFKILL_TYPE_UWB,		.name = "uwb"	},
	{	.type = RFKILL_TYPE_UWB,		.name = "ultrawideband"	}, /* alias */
	{	.type = RFKILL_TYPE_WIMAX,		.name = "wimax"	},
	{	.type = RFKILL_TYPE_WWAN,		.name = "wwan"	},
	{	.type = RFKILL_TYPE_GPS,		.name = "gps"	},
	{	.type = RFKILL_TYPE_FM,			.name = "fm"	},
	{	.name = NULL }
};

struct rfkill_id {
	union {
		enum rfkill_type type;
		__u32 index;
	};
	enum {
		RFKILL_IS_INVALID,
		RFKILL_IS_TYPE,
		RFKILL_IS_INDEX,
	} result;
};

/**
 * rfkill_:
 *
 *
 * returns a ...
 **/

gboolean rfkill_get_status(RFKillApplet *applet)
{
	struct rfkill_id id = { .result = RFKILL_IS_INVALID };
	struct rfkill_event event;
	const char *name;
	ssize_t len;
	int fd;
	applet->status = RADIATION_UNKNOW;
	/* Clear the string */
	applet->tooltip = "\0";

	fd = open("/dev/rfkill", O_RDONLY);
	if (fd < 0) {
		applet->tooltip = "ERROR:\nCan't open RFKILL control device";
		return FALSE;
	}

	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
		close(fd);
		applet->tooltip = "ERROR:\nCan't set RFKILL control device to non-blocking";
		return FALSE;
	}

	while (1) {
		len = read(fd, &event, sizeof(event));
		if (len < 0) {
			if (errno == EAGAIN)
				break;
			applet->tooltip = "ERROR:\nReading of RFKILL events failed";
			applet->status = RADIATION_UNKNOW;
			break;
		}

		if (len != RFKILL_EVENT_SIZE_V1) {
			/* Wrong size of RFKILL event */
			continue;
		}

		if (event.op != RFKILL_OP_ADD)
			continue;

		/* filter out unwanted results */
		switch (id.result)
		{
		case RFKILL_IS_TYPE:
			if (event.type != id.type)
				continue;
			break;
		case RFKILL_IS_INDEX:
			if (event.idx != id.index)
				continue;
			break;
		case RFKILL_IS_INVALID:; /* must be last */
		}

		applet->tooltip = g_strconcat(
						applet->tooltip, type2string(event.type),
						" (", get_name(event.idx), ")\n",
						"\tSoft blocked: ", event.soft ? "yes" : "no", "\n"
						"\tHard blocked: ", event.hard ? "yes" : "no", "\n",
						NULL );


		/* If we detect only 1 device radiating then set status to EMITTING */
		if (!event.soft && !event.hard)
			applet->status = RADIATION_EMITTING;

		/* Set the device to KILLED when we detect the first device not radiating
		 * and the previous status was UNKNOW */
		if ( ( event.soft || event.hard ) && ( applet->status == RADIATION_UNKNOW ) )
			applet->status = RADIATION_KILLED;

	}

	close(fd);
	return TRUE;
}


/**
 * rfkill_change_status:
 * @applet: ...
 *
 * ...
 **/

gboolean rfkill_change_status (RFKillApplet *applet)
{

	struct rfkill_id id;
	struct rfkill_event event;
	ssize_t len;
	int fd;

	fd = open("/dev/rfkill", O_RDWR);
	if (fd < 0) {
		applet->tooltip = "Can't open RFKILL control device";
		return FALSE;
	}

	memset(&event, 0, sizeof(event));
	event.op = RFKILL_OP_CHANGE_ALL;
	/*
	 * event.soft = 1 (kill radiation)
	 * event.hard = 0 (emit radiation)
	 */
	event.soft = (applet->status == RADIATION_EMITTING)? 1 : 0;

	len = write(fd, &event, sizeof(event));

	if (len < 0) {
		applet->tooltip = "Failed to change RFKILL state";
		return FALSE;
	}

	close(fd);
	return TRUE;
}


