/*
 * fsdevice.c - File system device.
 *
 * Written by
 *  Teemu Rantanen      (tvr@cs.hut.fi)
 *  Jarkko Sonninen     (sonninen@lut.fi)
 *  Jouko Valta         (jopi@stekt.oulu.fi)
 *  Olaf Seibert        (rhialto@mbfys.kun.nl)
 *  Andr� Fachat        (a.fachat@physik.tu-chemnitz.de)
 *  Ettore Perazzoli    (ettore@comm2000.it)
 *  Martin Pottendorfer (Martin.Pottendorfer@aut.alcatel.at)
 *  Andreas Boose       (boose@unixserv.rz.fh-hannover.de)
 *
 * Patches by
 *  Dan Miner           (dminer@nyx10.cs.du.edu)
 *  Germano Caronni     (caronni@tik.ethz.ch)
 *  Daniel Fandrich     (dan@fch.wimsey.bc.ca)  /DF/
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <assert.h>
#include <errno.h>

#include "resources.h"
#include "drive.h"
#include "file.h"
#include "charsets.h"
#include "tape.h"
#include "fsdevice.h"

enum fsmode {
    Write, Read, Directory
};

struct fs_buffer_info {
    FILE *fd;
    DIR *dp;
    enum fsmode mode;
    char dir[MAXPATHLEN];
    BYTE name[MAXPATHLEN + 5];
    int buflen;
    BYTE *bufp;
    int eof;
    int dirmpos;
    int reclen;
} fs_info[16];

/* this should somehow go into the fs_info struct... */

static char fs_errorl[MAXPATHLEN];
static unsigned int fs_elen, fs_eptr;
static char fs_cmdbuf[MAXPATHLEN];
static unsigned int fs_cptr = 0;

static char fs_dirmask[MAXPATHLEN];

static FILE *fs_find_pc64_name(char *name, int length);
static void fs_test_pc64_name(char *rname);

/* FIXME: ugly.  */
extern errortext_t floppy_error_messages;

/* ------------------------------------------------------------------------- */

static int fsdevice_convert_p00_enabled;

static int set_fsdevice_convert_p00(resource_value_t v)
{
    fsdevice_convert_p00_enabled = (int) v;
    return 0;
}

static resource_t resources[] = {
    { "FSDeviceConvertP00", RES_INTEGER, (resource_value_t) 1,
      (resource_value_t *) & fsdevice_convert_p00_enabled,
      set_fsdevice_convert_p00 },
    { NULL }
};

int fsdevice_init_resources(void)
{
    return resources_register(resources);
}

/* ------------------------------------------------------------------------- */

int attach_fsdevice(int device, char *var, char *name)
{
    if (serial_attach_device(device, (char *) var, (char *) name,
			     read_fs, write_fs, open_fs, close_fs, flush_fs))
	return 1;
    fs_error(IPE_DOS_VERSION);
    return 0;
}

void fs_error(int code)
{
    static int last_code;
    char *message;

    /* Only set an error once per command */
    if (code != IPE_OK && last_code != IPE_OK && last_code != IPE_DOS_VERSION)
	return;

    last_code = code;

    if (code == IPE_DOS_VERSION) {
	message = "UNIX FS DRIVER V1.X";
    } else {
	errortext_t *e;
	e = &floppy_error_messages;
	while (e->nr >= 0 && e->nr != code)
	    e++;
	if (e->nr >= 0)
	    message = e->text;
	else
	    message = "UNKNOWN ERROR NUMBER";
    }

    sprintf(fs_errorl, "%02d,%s,00,00\015", code, message);

    fs_elen = strlen(fs_errorl);
    fs_eptr = 0;

    if (code && code != IPE_DOS_VERSION)
	fprintf(stderr, "UnixFS: ERR = %02d, %s\n", code, message);
}

void flush_fs(void *flp, int secondary)
{
    char *cmd, *arg, *arg2 = NULL;
    int er = IPE_SYNTAX;

    if (secondary != 15 || !fs_cptr)
	return;

    /* remove trailing cr */
    while (fs_cptr && (fs_cmdbuf[fs_cptr - 1] == 13))
	fs_cptr--;
    fs_cmdbuf[fs_cptr] = 0;
    petconvstring(fs_cmdbuf, 1);	/* CBM name to FSname */

    cmd = fs_cmdbuf;
    while (*cmd == ' ')
	cmd++;

    arg = strchr(fs_cmdbuf, ':');
    if (arg) {
	*arg++ = '\0';
    }
#ifdef DEBUG_FS
    printf("Flush_FS: command='%s', cmd='%s'\n", cmd, arg);
#endif
    if (!strcmp(cmd, "cd")) {
	er = IPE_OK;
	if (chdir(arg)) {
	    er = IPE_NOT_FOUND;
	    if (errno == EPERM)
		er = IPE_PERMISSION;
	}
    } else if (!strcmp(cmd, "md")) {
	er = IPE_OK;
	if (mkdir(arg, /*S_IFDIR | */ 0770)) {
	    er = IPE_INVAL;
	    if (errno == EEXIST)
		er = IPE_FILE_EXISTS;
	    if (errno == EACCES)
		er = IPE_PERMISSION;
	    if (errno == ENOENT)
		er = IPE_NOT_FOUND;
	}
    } else if (!strcmp(cmd, "rd")) {
	er = IPE_OK;
	if (rmdir(arg)) {
	    er = IPE_NOT_EMPTY;
	    if (errno == EPERM)
		er = IPE_PERMISSION;
	}
    } else if (*cmd == 's') {
	er = IPE_DELETED;
	if (unlink(arg)) {
	    er = IPE_NOT_FOUND;
	    if (errno == EPERM)
		er = IPE_PERMISSION;
	}
    } else if (*cmd == 'r') {
	if ((arg2 = strchr(arg, '='))) {
	    er = IPE_OK;
	    *arg2++ = 0;
	    if (rename(arg2, arg)) {
		er = IPE_NOT_FOUND;
		if (errno == EPERM)
		    er = IPE_PERMISSION;
	    }
	}
    }
    fs_error(er);
    fs_cptr = 0;
}

int write_fs(void *flp, BYTE data, int secondary)
{
    if (secondary == 15) {
#ifdef DEBUG_FS_
	printf("Write_FS(secadr=15, data=%02x, '%c')\n", data, data);
#endif
	if (fs_cptr < MAXPATHLEN - 1) {		/* keep place for nullbyte */
	    fs_cmdbuf[fs_cptr++] = data;
	    return SERIAL_OK;
	} else {
	    fs_error(IPE_LONG_LINE);
	    return SERIAL_ERROR;
	}
    }
    if (fs_info[secondary].mode != Write)
	return FLOPPY_ERROR;

    if (fs_info[secondary].fd) {
	fputc(data, fs_info[secondary].fd);
	return FLOPPY_COMMAND_OK;
    };

    return FLOPPY_ERROR;
}


int read_fs(void *flp, BYTE * data, int secondary)
{
    int i, l, f;
    unsigned short blocks;
    struct dirent *dirp;	/* defined in /usr/include/sys/dirent.h */
    struct stat statbuf;
    struct fs_buffer_info *info = &fs_info[secondary];
    char rname[256];

    if (secondary == 15) {
	if (!fs_elen)
	    fs_error(IPE_OK);
	if (fs_eptr < fs_elen) {
	    *data = fs_errorl[fs_eptr++];
#ifdef DEBUG_FS
	    printf("Read_FS(secadr=15) reads '%c'\n", *data);
#endif
	    return SERIAL_OK;
	} else {
	    fs_error(IPE_OK);
	    *data = 0xc7;
	    return SERIAL_EOF;
	}
    }
    switch (info->mode) {
      case Write:
	  return FLOPPY_ERROR;

      case Read:
	  if (info->fd) {
	      i = fgetc(info->fd);
	      if (ferror(info->fd))
		  return FLOPPY_ERROR;
	      if (feof(info->fd)) {
		  *data = 0xc7;
		  return SERIAL_EOF;
	      }
	      *data = i;
	      return SERIAL_OK;
	  }
	  break;

      case Directory:
	  if (info->dp) {
	      if (info->buflen <= 0) {
		  char buf[MAXPATHLEN];

#ifdef DEBUG_FSDRIVE
		  printf("reading\n");
#endif

		  info->bufp = info->name;

		  if (info->eof) {
		      *data = 0xc7;
		      return SERIAL_EOF;
		  }
		  /*
		   * Find the next directory entry and return it as a CBM
		   * directory line.
		   */

		  /* first test if dirmask is needed - maybe this should be
		     replaced by some regex functions... */
#ifdef DEBUG_FS
		  printf("FS_ReadDir: mask ='%s'\n", fs_dirmask);
#endif
		  f = 1;
		  do {
		      char *p;
		      dirp = readdir(info->dp);
		      if (!dirp)
			  break;
#ifdef DEBUG_FS
		      printf("FS_ReadDir: testing file '%s'\n", dirp->d_name);
#endif
		      strcpy(rname, dirp->d_name);
		      if (fsdevice_convert_p00_enabled)
			  fs_test_pc64_name(rname);
		      if (!*fs_dirmask)
			  break;
		      l = strlen(fs_dirmask);
		      for (p = rname, i = 0; *p && fs_dirmask[i] && i < l; i++) {
			  if (fs_dirmask[i] == '?') {
			      p++;
			  } else if (fs_dirmask[i] == '*') {
			      if (!fs_dirmask[i + 1]) {
				  f = 0;
				  break;
			      }	/* end mask */
			      while (*p && (*p != fs_dirmask[i + 1]))
				  p++;
			  } else {
			      if (*p != fs_dirmask[i])
				  break;
			      p++;
			  }
			  if ((!*p) && (!fs_dirmask[i + 1])) {
			      f = 0;
			      break;
			  }
		      }
		  }
		  while (f);

#ifdef DEBUG_FS
		  printf("FS_ReadDir: printing file '%s'\n",
			 dirp ? rname : NULL);
#endif

		  if (dirp) {
		      BYTE *p = info->name;
		      char *tp;

		      strcpy(buf, info->dir);
		      strcat(buf, "/");
		      tp = buf + strlen(buf);
		      strcat(buf, dirp->d_name);

		      /* Line link, Length and spaces */

		      p += 2;	/* skip link addr, fill in later */

		      if (stat(buf, &statbuf) < 0)
			  blocks = 0;	/* this file can't be opened */
		      else
			  blocks = (unsigned short) ((statbuf.st_size + 253) / 254);

		      SET_LO_HI(p, blocks);

		      if (blocks < 10)
			  *p++ = ' ';
		      if (blocks < 100)
			  *p++ = ' ';
		      if (blocks < 1000)
			  *p++ = ' ';

		      *p++ = ' ';


		      /*
		       * Filename
		       * Any extension is not used.
		       */

		      *p++ = '"';

		      /* hide the extensions ",prg" and ",P" */
		      if ((strlen(tp) >= 4) && !strcmp(tp + strlen(tp) - 4, ",prg"))
			  tp[strlen(tp) - 4] = 0;
		      if ((strlen(tp) >= 2) && !strcmp(tp + strlen(tp) - 2, ",P"))
			  tp[strlen(tp) - 2] = 0;

		      if (strcmp(rname, dirp->d_name)) {
			  for (i = 0; rname[i] && (*p = rname[i]); ++i, ++p);
		      } else {
			  for (i = 0; tp[i] /*i < dirp->d_namlen */ &&
			       (*p = p_topetcii(tp[i] /*dirp->d_name[i] */ )); ++i, ++p);
		      }

		      *p++ = '"';
		      for (; i < 17; i++)
			  *p++ = ' ';

		      if (S_ISDIR(statbuf.st_mode)) {
			  *p++ = 'D';
			  *p++ = 'I';
			  *p++ = 'R';
		      } else {
			  *p++ = 'P';
			  *p++ = 'R';
			  *p++ = 'G';
		      }

		      *p = '\0';	/* to allow strlen */

		      /* some (really very) old programs rely on the directory
		         entry to be 32 Bytes in total (incl. nullbyte) */
		      l = strlen((char *) (info->name + 4)) + 4;
		      while (l < 31) {
			  *p++ = ' ';
			  l++;
		      }

		      *p++ = '\0';

		      info->dirmpos += p - info->name;
		      *info->name = info->dirmpos & 0xff;
		      *(info->name + 1) = (info->dirmpos >> 8) & 0xff;

		      info->buflen = (int) (p - info->name);

#ifdef DEBUG_FSDRIVE
		      printf("found %4d>%s< (%d/%d)  buf:>%s< (%d)\n",
			     blocks, dirp->d_name, i, dirp->d_namlen,
			     info->name + 4, info->buflen);
#endif
		  } else {

		      /* EOF => End file */

		      memset(info->name, 0, 2);
		      info->buflen = 2;
		      info->eof++;
		  }
	      }			/* info->buflen */
	      *data = *info->bufp++;
	      info->buflen--;
	      return SERIAL_OK;

	  }			/* info->dp */
	  break;
    }

    return FLOPPY_ERROR;
}


int open_fs(void *flp, char *name, int length, int secondary)
{
    FILE *fd;
    DIR *dp;
    BYTE *p, *linkp;
    char fsname[MAXPATHLEN];
    char fsname2[MAXPATHLEN];
#ifdef 0			/* Old P00 support.  */
    char realname[32];
#endif
    char *mask;
    int status = 0, i, reallength, readmode, filetype, rl;

    if (fs_info[secondary].fd)
	return FLOPPY_ERROR;

    memcpy(fsname2, name, length);
    fsname2[length] = 0;

    if (secondary == 15) {
#ifdef DEBUG_FS
	printf("Open_FS(secadr=15, name='%s'\n", fsname2);
#endif
	for (i = 0; i < length; i++) {
	    status = write_fs(flp, name[i], 15);
	}
	return status;
    }
    /*
     * Support only load / save - why? (AF, 12jan1997)
     */
#if 0
    if (secondary < 0 || secondary >= 2) {
	fs_error(IPE_NO_CHANNEL);
	return FLOPPY_ERROR;
    }
#endif

    /* Directory read */

    if (secondary == 1)
	readmode = FAM_WRITE;
    else
	readmode = FAM_READ;

    filetype = 0;
    rl = 0;

    /* This is a hack to allow "*,prg" files. The lower case letters
       are give errors in floppy_parse_name when trying to find the
       file type.

       if((strlen(fsname2)>=4) && !strcmp(",prg",fsname2+strlen(fsname2)-4))
       fsname2[strlen(fsname2)-4]=0;

     */

    if (floppy_parse_name(fsname2, length, fsname, &reallength, &readmode,
			  &filetype, &rl) != SERIAL_OK)
	return SERIAL_ERROR;

    petconvstring(fsname, 1);	/* CBM name to FSname */

    /* avoid append */
    if (readmode != FAM_READ && readmode != FAM_WRITE) {
	fs_error(IPE_NO_CHANNEL);
	return FLOPPY_ERROR;
    }
    fsname[reallength] = 0;

    fs_info[secondary].mode = (readmode == FAM_WRITE) ? Write : Read;

    if (*name == '$') {
	if ((secondary != 0) || (fs_info[secondary].mode != Read)) {
	    fs_error(IPE_NOT_WRITE);
	    return FLOPPY_ERROR;
	}
	/* test on wildcards */
	if (!(mask = strrchr(fsname, '/')))
	    mask = fsname;
	if (strchr(mask, '*') || strchr(mask, '?')) {
	    if (*mask == '/') {
		strcpy(fs_dirmask, mask + 1);
		*mask++ = 0;
	    } else {
		strcpy(fs_dirmask, mask);
		strcpy(fsname, ".");
	    }
	} else {
	    *fs_dirmask = 0;
	    if (!*fsname)
		strcpy(fsname, ".");
	}
#ifdef DEBUG_FS
	printf("Opening Dir with dir='%s', mask='%s')\n", fsname, fs_dirmask);
#endif
	/* trying to open */
	if (!(dp = opendir((char *) fsname))) {
	    for (p = (BYTE *) fsname; *p; p++)
		if (isupper(*p))
		    *p = tolower(*p);
	    if (!(dp = opendir((char *) fsname))) {
		fs_error(IPE_NOT_FOUND);
		return FLOPPY_ERROR;
	    }
	}
	strcpy(fs_info[secondary].dir, fsname);

	/*
	 * Start Address, Line Link and Line number 0
	 */

	p = fs_info[secondary].name;

	*p++ = 1;
	*p++ = 4;

	linkp = p;
	p += 2;

	*p++ = 0;
	*p++ = 0;

	*p++ = (BYTE) 0x12;	/* Reverse on */

	*p++ = '"';
	strcpy((char *) p, fs_info[secondary].dir);	/* Dir name */
	petconvstring((char *) p, 0);	/* ASCII name to PETSCII */
	i = 0;
	while (*p) {
	    ++p;
	    i++;
	}
	while (i < 16) {
	    *p++ = ' ';
	    i++;
	}
	*p++ = '"';
	while (i < 22) {
	    *p++ = ' ';
	    i++;
	}
	*p++ = 0;

	i = 0x0401 + p - linkp;
	*linkp = i & 0xff;
	*(linkp + 1) = (i >> 8) & 0xff;

	fs_info[secondary].buflen = p - fs_info[secondary].name;
	fs_info[secondary].bufp = fs_info[secondary].name;
	fs_info[secondary].mode = Directory;
	fs_info[secondary].dp = dp;
	fs_info[secondary].eof = 0;
	fs_info[secondary].dirmpos = i;		/* start address of next line */

#ifdef DEBUG_FSDRIVE
	printf("opened directory\n");
#endif


    } else {			/* Normal file, not directory ("$") */

	fs_info[secondary].mode = (secondary == 1 ? Write : Read);


	/* Wildcards FIXME: support them here */

	if (strchr(fsname, '*') || strchr(fsname, '?')) {
	    if (fs_info[secondary].mode == Write) {
		fs_error(IPE_BAD_NAME);
		return FLOPPY_ERROR;
	    }
	}
	fd = fopen(fsname, fs_info[secondary].mode ? READ : APPEND);

	if (!fd) {		/* lets test some variants... */
	    int l = strlen(fsname);
	    strcat(fsname, ",P");
	    fd = fopen(fsname, fs_info[secondary].mode ? READ : APPEND);
	    if (!fd) {
		fsname[l] = '\0';
		strcat(fsname, ",prg");
		fd = fopen(fsname, fs_info[secondary].mode ? READ : APPEND);
		if (!fd) {
		    fsname[l] = '\0';
		    for (p = (BYTE *) fsname; *p; p++)
			if (isupper(*p))
			    *p = tolower(*p);

		    fd = fopen(fsname, fs_info[secondary].mode ? READ : APPEND);

		    if (!fd) {
			if (fsdevice_convert_p00_enabled)
			    fd = fs_find_pc64_name(name, length);
			if (!fd) {
			    fs_error(IPE_NOT_FOUND);
			    return FLOPPY_ERROR;
			}
		    }
		}
	    }
	}
	fs_info[secondary].fd = fd;

#ifdef 0			/* Old P00 support.  */
	/* P00 header */

	if (fs_info[secondary].mode == Read) {
	    int realtype;

	    /* Check if P00 header and read the actual filename and type */

	    if ((realtype = is_pc64name(fsname)) >= 0 &&
		read_pc64header(fd, realname, &fs_info[secondary].reclen) == FD_OK) {
		;
	    } else
		rewind(fd);	/* There is no P00 header */

	} else {

	    /* Write P00 header ? */

	    if (is_pc64name(fsname) >= 0) {
		printf("writing PC64 header.\n");
		write_pc64header(fd, realname, 0);
	    }
	}			/* P00 */
#endif
    }
    fs_error(IPE_OK);
    return FLOPPY_COMMAND_OK;
}


int close_fs(void *flp, int secondary)
{
    if (secondary == 15) {
#ifdef DEBUG_FS
	printf("Close_FS(secadr=15)\n");
#endif
	fs_error(IPE_OK);
	return FLOPPY_COMMAND_OK;
    }
    switch (fs_info[secondary].mode) {
      case Write:
      case Read:
	  if (!fs_info[secondary].fd)
	      return FLOPPY_ERROR;

	  fclose(fs_info[secondary].fd);
	  fs_info[secondary].fd = NULL;
	  break;

      case Directory:
	  if (!fs_info[secondary].dp)
	      return FLOPPY_ERROR;

	  closedir(fs_info[secondary].dp);
	  fs_info[secondary].dp = NULL;
	  break;
    }

    return FLOPPY_COMMAND_OK;
}

void fs_test_pc64_name(char *rname)
{
    char p00id[8];
    char p00name[17];
    FILE *fd;

    if (is_pc64name(rname) >= 0) {
	fd = fopen(rname, "r");
	if (!fd)
	    return;

	fread((char *) p00id, 8, 1, fd);
	if (ferror(fd)) {
	    fclose(fd);
	    return;
	}
	p00id[7] = '\0';
	if (!strncmp(p00id, "C64File", 7)) {
	    fread((char *) p00name, 16, 1, fd);
	    if (ferror(fd)) {
		fclose(fd);
		return;
	    }
	    p00name[16] = '\0';
	    strcpy(rname, p00name);
	    fclose(fd);
	    return;
	}
	fclose(fd);
    }
}


FILE *fs_find_pc64_name(char *name, int length)
{
    struct dirent *dirp;
    char *p;
    DIR *dp;
    char p00id[8];
    char p00name[17];
    char p00dummy[2];
    FILE *fd;

    name[length] = '\0';
    dp = opendir(".");
    do {
	dirp = readdir(dp);
	if (dirp != NULL) {
	    p = dirp->d_name;
	    if (is_pc64name(p) >= 0) {
		fd = fopen(p, "r");
		if (!fd)
		    continue;
		fread((char *) p00id, 8, 1, fd);
		if (ferror(fd)) {
		    fclose(fd);
		    continue;
		}
		p00id[7] = '\0';
		if (!strncmp(p00id, "C64File", 7)) {
		    fread((char *) p00name, 16, 1, fd);
		    if (ferror(fd)) {
			fclose(fd);
			continue;
		    }
		    p00name[16] = '\0';
		    if (strcmp(name, p00name) == 0) {
			fread((char *) p00dummy, 2, 1, fd);
			if (ferror(fd)) {
			    fclose(fd);
			    continue;
			}
			return fd;
		    }
		}
		fclose(fd);
	    }
	}
    }
    while (dirp != NULL);
    closedir(dp);
    return NULL;
}
