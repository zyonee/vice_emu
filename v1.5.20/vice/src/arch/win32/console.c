/*
 * console.c - Console access interface.
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
 *
 * Totally rewritten for using an own window by
 *  Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "console.h"
#include "fullscreen.h"
#include "res.h"
#include "ui.h"
#include "uimon.h"
#include "utils.h"
#include "winmain.h"

/*
 MAX_WIDTH is not allowed to be bigger than (MIN_XSIZE * MIN_YSIZE) !
*/

/*
 @SRT TODO: currently, MAX_WIDTH plus a prompt which is on the screen
 is not even allowed to be bigger than MIN_XSIZE; that's because the
 routines are not multi-line-input safe yet!
*/

#define MAX_OUTPUT_LENGTH 2000 /* max. length of an output string after expansion with vsprintf() */
#define MAX_WIDTH           60 /* max. char count for an input line  */
#define MAX_HISTORY         20 /* maximum history entrys per console */

#define MIN_XSIZE           30
#define MIN_YSIZE           10

/*
 When resizing the window, the new window buffer is not reallocated for every
 one increment, but always in size of RESIZE_INCREMENT_...
 So, we don't need to reallocate very frequently and so we don't convert the 
 memory into little parts which cannot be rejoined.
*/
#define RESIZE_INCREMENT_X  40
#define RESIZE_INCREMENT_Y  20


/*
 The following structure contains the file-private informations of a console window
*/
typedef struct console_private_s 
{
	struct console_private_s
				*pcpNext;	/* pointer to the next console_private_t structure */

	console_t	*pConsole;

	char		*pchWindowBuffer;	/* a buffer to the window contents */

	char		*pchName;			/*  the name of the window; 
									this is used to recognize the window when re-opening */

	char		*pchOnClose;		/* the string to be returned when the window is closed */

	char		*history[MAX_HISTORY];	/* ring buffer of pointers to the history entries */
	unsigned	 nHistory;		/* index to next entry to be written in history buffer */
	unsigned	 nCurrentSelectHistory;	/* index relative to nHistory to the entry of the 
									   history which will be shown next */

	/* the position of the cursor in the window */
	unsigned	 xPos;
	unsigned	 yPos;

	unsigned	 xMax;
	unsigned	 yMax;
	unsigned	 xMin;
	unsigned	 yMin;

	/* the dimensions of one character in the window */
	unsigned	 xCharDimension;
	unsigned	 yCharDimension;

	HWND		 hwndConsole;	/* the HWND of the window */
	HWND		 hwndPreviousActive;
	HWND		 hwndParent;
	HDC	         hdc;			/* a DC for writing inside the window */

	BOOLEAN      bIsMdiChild;
    HWND         hwndMdiClient;

	int          xWindow;		/* the position of the window for re-opening */
	int          yWindow;		/* the position of the window for re-opening */
	BOOLEAN		 bInputReady;
	BOOLEAN		 bBlinkOn;

	char		 achInputBuffer[MAX_WIDTH+1];
	unsigned	 cntInputBuffer;
	unsigned	 posInputBuffer;
	BOOLEAN		 bInsertMode;

    FILE        *fileOutput;    /* file handle for outputting */

} console_private_t;


static void FileOpen( console_private_t *pcp )
{
    pcp->fileOutput = ui_console_save_dialog( pcp->hwndConsole );
}

static void FileClose( console_private_t *pcp )
{
    fclose( pcp->fileOutput );
    pcp->fileOutput = NULL;
}

static void FileOut( console_private_t *pcp, const char * const pstr )
{
    if (pcp->fileOutput)
        fprintf( pcp->fileOutput, "%s", pstr );
}


/*
 The only non-dynamic variable: Pointer to the first console_private_t
 structure.
*/
static console_private_t *first_window;


static void process_break( void )
{
}

static void add_to_history( console_private_t *pcp, const char *entry )
{
	if (entry[0])
	{
		if (pcp->history[pcp->nHistory] != NULL)
		{
			/* delete old history entry */
			free( pcp->history[pcp->nHistory] );
		};

		pcp->history[pcp->nHistory] = stralloc( entry );

		pcp->nHistory = (pcp->nHistory+1) % MAX_HISTORY;
	}
}

static const char *get_history_entry( console_private_t *pcp )
{
	return pcp->history[(pcp->nHistory - pcp->nCurrentSelectHistory + MAX_HISTORY) % MAX_HISTORY];
	/*
	remark: "+ MAX_HISTORY" to avoid the following portability problem:
	what means -3 % 7? A compiler might output -3 or 4 at its own choice,
	which are both mathematically correct!
	*/
}


/*
 calculate a pointer into the pchWindowBuffer
*/
#define CALC_POS(xxx, yyy) (pcp->xMax * (yyy) + (xxx))

static void redraw_window(console_private_t *pcp, LPPAINTSTRUCT pps)
{
	unsigned row;

	unsigned yMin = 0;
	unsigned yMax = pcp->pConsole->console_yres;

	unsigned xMin = 0;
	unsigned xMax = pcp->pConsole->console_xres;

	if (pps)
	{
		/* we have an update region, so only update necessary parts */
		xMin = pps->rcPaint.left   / pcp->xCharDimension;
		yMin = pps->rcPaint.top    / pcp->yCharDimension;

		/*
		 the "+ ..." force a rounding up.
		*/
		xMax = (pps->rcPaint.right  + pcp->xCharDimension-1) / pcp->xCharDimension;
		yMax = (pps->rcPaint.bottom + pcp->yCharDimension-1) / pcp->yCharDimension;
	}

	for (row = yMin; row < yMax; row++)
	{
		/* draw a single line */
		TextOut( pcp->hdc, 
			xMin * pcp->xCharDimension,
			row  * pcp->yCharDimension,
			&(pcp->pchWindowBuffer[CALC_POS(xMin,row)]), 
			xMax
			);
	}
}



static void move_upwards( console_private_t *pcp )
{
	--pcp->yPos;
}

static void scroll_up( console_private_t *pcp )
{
	/* move all lines one line up */
	memmove( pcp->pchWindowBuffer,
		&pcp->pchWindowBuffer[pcp->xMax],
		CALC_POS(0,pcp->yMax-1)
		);

	/* clear the last line */
	memset(&pcp->pchWindowBuffer[CALC_POS(0,pcp->yMax-1)],
		' ',
		pcp->xMax
		);

	move_upwards(pcp);

	/* force repainting of the whole window */
#if 0
	/*
	 @SRT this variant takes less processor time because
	 hopefully, the window needs not to be updated with
	 every single scroll.
	*/
	InvalidateRect( pcp->hwndConsole, NULL, FALSE );
#else
	/*
	 @SRT this variant looks more realistic since every
	 single scroll can be seen by the user
	*/
	redraw_window( pcp, NULL );
#endif
}

static void move_downwards( console_private_t *pcp )
{
	if (++pcp->yPos == pcp->pConsole->console_yres)
	{
		/* we must scroll the window */
		scroll_up(pcp);
	}
}

static void move_backwards( console_private_t *pcp )
{
	if (--pcp->xPos < 0)
	{
		pcp->xPos = pcp->pConsole->console_xres-1;
		move_upwards(pcp);
	}
}

static void move_forwards( console_private_t *pcp )
{
	if (++pcp->xPos >= pcp->pConsole->console_xres)
	{
		pcp->xPos = 0;
		move_downwards(pcp);
	}
}

/*
 calculate the dimensions of one character 
 and set console_private_t structure accordingly
*/
static void get_char_dimensions( console_private_t *pcp )
{
	SIZE size;

	GetTextExtentPoint32( pcp->hdc, " ", 1, &size );

	pcp->xCharDimension  = size.cx;
	pcp->yCharDimension = size.cy; 
}


static void size_window( console_private_t *pcp )
{ 
	RECT rect;

    GetClientRect( pcp->hwndConsole, &rect );

	ClientToScreen( pcp->hwndConsole,  (LPPOINT) &rect);
	ClientToScreen( pcp->hwndConsole, ((LPPOINT) &rect) + 1);

	if (pcp->bIsMdiChild)
	{
		ScreenToClient( pcp->hwndMdiClient,  (LPPOINT) &rect);
		ScreenToClient( pcp->hwndMdiClient, ((LPPOINT) &rect) + 1);
	}

    rect.right  = rect.left + pcp->pConsole->console_xres * pcp->xCharDimension;
	rect.bottom = rect.top  + pcp->pConsole->console_yres * pcp->yCharDimension;

	AdjustWindowRect( &rect, GetWindowLong( pcp->hwndConsole, GWL_STYLE ), FALSE );

	MoveWindow( pcp->hwndConsole, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top, TRUE );

	pcp->xWindow = rect.left;
	pcp->yWindow = rect.top;
}


/*
 allocate memory for the window buffer 
 and initializie console_private_t structure
*/
static console_private_t *allocate_window_memory( console_private_t* pcp )
{
	if (!pcp->pchWindowBuffer)
	{
		unsigned n;

		n = pcp->xMax * pcp->yMax;

		/* allocate buffer for window contents */
		pcp->pchWindowBuffer = xmalloc(sizeof(char) * n );

		/* clear the buffer with spaces */
		memset( pcp->pchWindowBuffer, ' ', sizeof(char) * n );
	}

	return pcp;
}

static console_private_t *reallocate_window_memory( console_private_t* pcp, unsigned xDim, unsigned yDim )
{
	unsigned xOldDim = pcp->xMax;
	unsigned yOldDim = pcp->yMax;
	char *pOldBuffer = pcp->pchWindowBuffer;

	unsigned y;

	/* get new memory buffer */
	pcp->pchWindowBuffer = NULL;
	pcp->xMax            = max(xDim, pcp->xMax+RESIZE_INCREMENT_X );
	pcp->yMax            = max(yDim, pcp->yMax+RESIZE_INCREMENT_Y );

	allocate_window_memory( pcp );

	/* now, copy the contents of the old buffer into the new one */
	for (y=0; y<yOldDim; y++)
	{
		memmove(&pcp->pchWindowBuffer[CALC_POS(0,y)],
			&pOldBuffer[xOldDim * y],
			xOldDim);
	}

	/* we're done, release the old buffer */
	free( pOldBuffer );

	return pcp;
}


static void free_window_memory( console_private_t *pcp )
{
	if (pcp->pchWindowBuffer)
	{
		free( pcp->pchWindowBuffer);
		pcp->pchWindowBuffer = NULL;
	}

    if (pcp->fileOutput)
        FileClose( pcp );

	if (pcp->pchName)
		free( pcp->pchName );

	if (pcp->pchOnClose)
		free( pcp->pchOnClose );

	if (pcp->pConsole)
		free( pcp->pConsole );

	if (pcp->history[0])
	{
		int i;

		for (i=0; i<MAX_HISTORY; i++)
		{
			if (pcp->history[i] == NULL)
				break;

			free( pcp->history[i] );
		}
	}

	free( pcp );
}


static void draw_character( console_private_t *pcp, char ch )
{
	TextOut( pcp->hdc,
		pcp->xPos * pcp->xCharDimension,
		pcp->yPos * pcp->yCharDimension,
		&ch,
		1
		);
}


static void draw_current_character( console_private_t *pcp )
{
	TextOut( pcp->hdc,
		pcp->xPos * pcp->xCharDimension,
		pcp->yPos * pcp->yCharDimension,
		&pcp->pchWindowBuffer[CALC_POS(pcp->xPos, pcp->yPos)],
		1
		);
}


static void restore_current_character( console_private_t *pcp )
{
	/* remark: in general, we don't need to bother 
	   synchronizing with the blink timer, because the 
	   timer cannot effectively expire unless the next
       message is dispatched due to the cooperative
       multitasking in Windows.
	*/

	pcp->bBlinkOn = FALSE;
	draw_current_character( pcp );
}

static void paint_cursor( console_private_t *pcp )
{
	int yFirstCursorLine =  pcp->yPos    * pcp->yCharDimension + pcp->yCharDimension;
	int yLastCursorLine  =  pcp->yPos    * pcp->yCharDimension + pcp->yCharDimension;
	int xLeft            =  pcp->xPos    * pcp->xCharDimension;
	int xRight           = (pcp->xPos+1) * pcp->xCharDimension - 1;
	int rop2Old;

	POINT point;

	pcp->bBlinkOn = TRUE;

	yFirstCursorLine -= pcp->yCharDimension / (pcp->bInsertMode ? 2 : 4);


	/*
	 paint the cursor
	*/

	rop2Old = SetROP2( pcp->hdc, R2_NOT );

	while (yFirstCursorLine < yLastCursorLine)
	{
		MoveToEx( pcp->hdc, xLeft,  yFirstCursorLine,  &point );
		LineTo  ( pcp->hdc, xRight, yFirstCursorLine++        );
	}

	SetROP2( pcp->hdc, rop2Old );
}


static void console_out_character(console_private_t *pcp, const unsigned char ch)
{
	if (ch>=32)
	{
		pcp->pchWindowBuffer[CALC_POS(pcp->xPos, pcp->yPos)] = ch;
		draw_current_character(pcp);
		move_forwards(pcp);
	}
	else
	{
		/* do we have a backspace? */
		if (ch==8)
		{
			move_backwards(pcp);
			pcp->pchWindowBuffer[CALC_POS(pcp->xPos,pcp->yPos)] = ' ';
			draw_current_character(pcp);
		}

		/* do we have a return? */
		if ( (ch==13) || (ch=='\n'))
		{
			pcp->xPos = 0;
			move_downwards(pcp);
		}
	}
}


static void draw_current_input( console_private_t *pcp )
{
	int xPos = pcp->xPos;
	int yPos = pcp->yPos;

	/* go to where the input line begins */
	/* @SRT TODO: does not work with multi-line inputs */
	pcp->xPos -= pcp->posInputBuffer;

	/* set zero at end of input buffer */

	pcp->achInputBuffer[pcp->cntInputBuffer] = 0;

	console_out( pcp->pConsole, "%s", pcp->achInputBuffer );

	/* output a blank to delete a possibly character after the
	   input (needed for outputting after backspace or delete
	*/
	console_out_character( pcp, ' ' );

	/* restore cursor position */
	pcp->xPos = xPos;
	pcp->yPos = yPos;
}


static void start_timer( console_private_t *pcp )
{
	SetTimer( pcp->hwndConsole, 1, 500, NULL );
}

int console_out(console_t *log, const char *format, ...)
{
	console_private_t *pcp = log->private;

	va_list ap;

	char ch;
	char buffer[MAX_OUTPUT_LENGTH];
	char *pBuffer      = buffer;

    va_start(ap, format);
    vsprintf(buffer, format, ap);

    FileOut( pcp, pBuffer );

	/* restore character under cursor */
	restore_current_character( pcp );

	while ( (ch = *pBuffer++) != 0)
	{
		console_out_character( pcp, ch );
	}

    /* dispatch events if there is one, so the keyboard will be handled */
    /* @SRT: should we move this into scroll_up(), so the performance
       will be better when using many console_out() w/o doing a CR? */
    ui_dispatch_events();

	return 0;
}




char *console_in(console_t *log)
{
	console_private_t *pcp = log->private ;

	char *p;

	pcp->posInputBuffer =
	pcp->cntInputBuffer = 0;
	pcp->bInsertMode    =
	pcp->bInputReady    = FALSE;

	pcp->nCurrentSelectHistory = 0;

	/* set a timer for the cursor */
	start_timer( pcp );

	do 
	{
		ui_dispatch_next_event();
	}
	while (!pcp->bInputReady);

	p = stralloc( pcp->achInputBuffer );


    /* Remove trailing newlines.  */
    {
        int len;

        for (len = strlen(p);
             len > 0 && (p[len - 1] == '\r'
                         || p[len - 1] == '\n');
             len--)
            p[len - 1] = '\0';
    }

    /* output input as output into file */
    FileOut( pcp, p );
    FileOut( pcp, "\n" );

	/* stop the timer for the cursor */
	KillTimer(pcp->hwndConsole, 1); 

	/* restore character under cursor */
	restore_current_character( pcp );

	add_to_history( pcp, p );

	return p;
}



static void replace_current_input( console_private_t *pcp, const char *p )
{
	/* @SRT TODO: not multi-line safe! */

	unsigned nOldBufferLength = pcp->cntInputBuffer;

	/* restore character under cursor */
	restore_current_character( pcp );

	strcpy( pcp->achInputBuffer, p );
	pcp->cntInputBuffer = strlen( pcp->achInputBuffer );

	draw_current_input( pcp );
	pcp->xPos += pcp->cntInputBuffer - pcp->posInputBuffer;
	pcp->posInputBuffer = pcp->cntInputBuffer;

	/* test: is the old line longer than the new one? */
	if (pcp->cntInputBuffer < nOldBufferLength )
	{
		/* yes, delete old lines with blanks */
		int xPos = pcp->xPos;

		nOldBufferLength -= pcp->cntInputBuffer;

		while (nOldBufferLength-- > 0)
		{
			console_out_character( pcp, ' ' );
		}

		pcp->xPos = xPos;
	}
}


static void external_resize_window( console_private_t *pcp, int nWidth, int nHeight )
{
	unsigned xDim = nWidth  / pcp->xCharDimension;
	unsigned yDim = nHeight / pcp->yCharDimension;

	/* @SRT TODO: if a multi-line-input is given, make sure that the
	   x dimension is not changed OR that the input is correctly redrawn!
	*/

	/* 
	test if window is bigger than ever before; if so,
	get new memory for new window buffer
	*/
	if ((xDim > pcp->xMax) || (yDim > pcp->yMax))
		reallocate_window_memory( pcp, xDim, yDim );

	pcp->pConsole->console_xres = xDim;
	pcp->pConsole->console_yres = yDim;

	/* make sure the cursor is inside the visible area */
	while (pcp->yPos >= yDim)
	{
		scroll_up( pcp );
	}

    {
        HWND hwndFrame = (HWND)GetWindowLong((HWND)GetWindowLong(pcp->hwndConsole,GWL_HWNDPARENT),GWL_HWNDPARENT);
        SendMessage(hwndFrame,WM_CONSOLE_RESIZED,0,0);
    }
}


static bIsMdiChild = FALSE;

/* window procedure */
static long CALLBACK console_window_proc(HWND hwnd, 
	UINT msg, WPARAM wParam, LPARAM lParam)

{
	console_private_t *pcp = (console_private_t*) GetWindowLong( hwnd, GWL_USERDATA );

    if (pcp)
    {
        bIsMdiChild = pcp->bIsMdiChild;
    }

	switch (msg)
	{
    case WM_GETMINMAXINFO:
        if (bIsMdiChild)
            DefMDIChildProc(hwnd, msg, wParam, lParam );
        else
            DefWindowProc(hwnd, msg, wParam, lParam);

        /* adjust: minimum size */
        if (pcp)
        {
            LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam; // address of structure 

            lpmmi->ptMinTrackSize.x += max(MIN_XSIZE,pcp->xPos + 1) * pcp->xCharDimension;
            lpmmi->ptMinTrackSize.y += MIN_YSIZE * pcp->yCharDimension;
        }
        return 0;

	case WM_SIZE:
        if (pcp)
    		external_resize_window( pcp, LOWORD(lParam), HIWORD(lParam) );
        break;
		// return 0;

	case WM_CLOSE:
		/* if the window is closed, i.e. by pressing the close
		   button, we simulate the typing of a specific line
		*/
        /* inform parent that window is closed */
        if (bIsMdiChild)
        {
            HWND hwndFrame = (HWND)GetWindowLong((HWND)GetWindowLong(hwnd,GWL_HWNDPARENT),GWL_HWNDPARENT);
            SendMessage(hwndFrame,WM_CONSOLE_CLOSED,0,0);
            pcp->bInputReady       = TRUE;
            replace_current_input( pcp, "" );
        }
        else
        {
            if (pcp->achInputBuffer)
            {
                pcp->bInputReady       = TRUE;
                replace_current_input( pcp, pcp->pchOnClose );
                console_out_character( pcp, '\n' );
            }
        }
        break;

    case WM_DESTROY:
        /* no PostQuitMessage(), because else, the whole application
           (VICE) would be closed - as it occurred in the old version
           with the standard console
        */
        UnregisterHotKey( hwnd, IDHOT_SNAPWINDOW );
        break;
//        return 0;

    case WM_TIMER:
        if (wParam == 1)
        {
            if (pcp->bBlinkOn)
            {
                /* restore previous character */
                restore_current_character( pcp );
            }
            else
            {
                /* paint cursor */
                paint_cursor( pcp );
            }

            start_timer( pcp );
            return 0;
        }
        break;


	case WM_KEYDOWN:
		{
			int nVirtKey = (int) wParam;

			switch (nVirtKey)
			{
            case VK_ESCAPE:
                /* treat ESCAPE key as CTRL+BREAK */
                process_break();
                return 0;

			case VK_UP:
				if (pcp->nCurrentSelectHistory < MAX_HISTORY)
				{
					const char *p;
					++pcp->nCurrentSelectHistory;

					p = get_history_entry(pcp);

					if (p)
						replace_current_input( pcp, p );
					else
					{
						/* undo the increment above */
						/*
						 remark: get_history_entry() above depends on the
						 increment! 
						*/
						--pcp->nCurrentSelectHistory;
					}
				}
				return 0;

			case VK_DOWN:
				if (pcp->nCurrentSelectHistory > 1)
				{
					--pcp->nCurrentSelectHistory;
					replace_current_input( pcp, get_history_entry( pcp ) );
				}
				else
				{
					pcp->nCurrentSelectHistory = 0;
					replace_current_input( pcp, "" );
				}
				return 0;

			case VK_LEFT:
				/* restore character under cursor */
				restore_current_character( pcp );

				if (pcp->posInputBuffer>0)
				{
					--pcp->posInputBuffer;
					move_backwards(pcp);
				}
				return 0;

			case VK_RIGHT:
				/* restore character under cursor */
				restore_current_character( pcp );

				if (pcp->posInputBuffer < pcp->cntInputBuffer)
				{
					++pcp->posInputBuffer;
					move_forwards(pcp);
				}
				return 0;

			case VK_HOME:
				/* restore character under cursor */
				restore_current_character( pcp );

				/* @SRT TODO: not multi-line safe! */
				pcp->xPos -= pcp->posInputBuffer;
				pcp->posInputBuffer = 0;
				return 0;

			case VK_END:
				/* restore character under cursor */
				restore_current_character( pcp );

				pcp->xPos += (pcp->cntInputBuffer - pcp->posInputBuffer );
				pcp->posInputBuffer = pcp->cntInputBuffer;
				return 0;


			case VK_INSERT:
				pcp->bInsertMode = pcp->bInsertMode ? FALSE : TRUE;

				/* repaint the cursor: */
				restore_current_character( pcp );
				paint_cursor( pcp );
				start_timer( pcp );
				return 0;

			case VK_DELETE:
				/* check not to clear more characters than there were */
				if (pcp->posInputBuffer < pcp->cntInputBuffer)
				{
					/* only process del if we're not at the end of the buffer */
					--pcp->cntInputBuffer;

					memmove(
						&pcp->achInputBuffer[pcp->posInputBuffer], 
						&pcp->achInputBuffer[pcp->posInputBuffer+1],
						pcp->cntInputBuffer - pcp->posInputBuffer
						);
				}

				draw_current_input( pcp );

				return 0;

			}
		}
		break;

    case WM_CONSOLE_INSERTLINE:
        // pcp->achInputBuffer[pcp->cntInputBuffer] = 0;
        pcp->bInputReady = TRUE;
        break;

	case WM_CHAR:
		{
			/* a key is pressed, process it! */
			char chCharCode = (char) wParam;

			/* restore character under cursor */
			restore_current_character( pcp );

			if (chCharCode >= 32)
			{
				/* it's a printable character, process it */

				if (pcp->bInsertMode)
				{
					/* insert mode */

					/* only insert if there's room in the buffer */
					if (pcp->cntInputBuffer < MAX_WIDTH)
					{
						++pcp->cntInputBuffer;

						memmove(
							&pcp->achInputBuffer[pcp->posInputBuffer+1], 
							&pcp->achInputBuffer[pcp->posInputBuffer],
							pcp->cntInputBuffer - pcp->posInputBuffer
							);

						draw_current_input( pcp );

						pcp->achInputBuffer[pcp->posInputBuffer++] = chCharCode;

						/* output the character */
						console_out_character( pcp, chCharCode );
					}
				}
				else
				{
					/* overwrite mode */

					/* processing only if the buffer is not full! */
					if (pcp->cntInputBuffer < MAX_WIDTH)
					{
						pcp->achInputBuffer[pcp->posInputBuffer++] = chCharCode;

						/* output the character */
						console_out_character( pcp, chCharCode );

						/* if we're at the end of the buffer, it's a kind of insert mode */
						if (pcp->cntInputBuffer < pcp->posInputBuffer)
							++pcp->cntInputBuffer;
					}
				}

				return 0;
			}


			if (chCharCode == 8)
			{
				/* it's a backspace, process it if possible */

				/* check not to clear more characters than there were */
				if (pcp->posInputBuffer > 0)
				{
					/* move the characters forward */

					if (pcp->posInputBuffer < pcp->cntInputBuffer)
					{
						memmove(
							&pcp->achInputBuffer[pcp->posInputBuffer-1], 
							&pcp->achInputBuffer[pcp->posInputBuffer],
							pcp->cntInputBuffer - pcp->posInputBuffer
							);

						--pcp->cntInputBuffer;
						draw_current_input( pcp );

						move_backwards(pcp);
					}
					else
					{
						/* only last character deleted, use faster method */
						console_out_character( pcp, chCharCode );
						--pcp->cntInputBuffer;
					}

					--pcp->posInputBuffer;
				}

				return 0;
			}

			if (chCharCode == 13)
			{
				/* it's a CR, so the input is ready */
				pcp->achInputBuffer[pcp->cntInputBuffer] = 0;
				pcp->bInputReady                        = TRUE;

				console_out_character( pcp, chCharCode );
				return 0;
			}

            if (chCharCode == 3) /* 3 is ASCII for CTRL+C */
            {
                /* it's a CTRL+C or CTRL+BREAK */
                process_break();
                return 0;
            }

			/* any other key will be ignored */
		}
		break;


	case WM_MDIACTIVATE:
		if ((HWND)wParam==hwnd)
		{
			// we are deactivated
			UnregisterHotKey( hwnd, IDHOT_SNAPWINDOW );
		}
		if ((HWND)lParam==hwnd)
		{
			// we are activated

			RegisterHotKey( hwnd, IDHOT_SNAPWINDOW, MOD_ALT, VK_SNAPSHOT );
		}
		break;

	case WM_ACTIVATE:
		// @@@SRT
		if (LOWORD(wParam)!=WA_INACTIVE)
			RegisterHotKey( hwnd, IDHOT_SNAPWINDOW, MOD_ALT, VK_SNAPSHOT );
		else
			UnregisterHotKey( hwnd, IDHOT_SNAPWINDOW );
		break;


	case WM_HOTKEY:
		if ((int)wParam == IDHOT_SNAPWINDOW)
		{   
			if (pcp->fileOutput)
				FileClose( pcp );
			else
				FileOpen ( pcp );
		}
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint(hwnd,&ps);

			redraw_window( pcp, &ps );

			EndPaint(hwnd,&ps);

			return 0;
		}
	}

    if (bIsMdiChild)
    {
        return DefMDIChildProc(hwnd, msg, wParam, lParam );
    }
    else
    {
	    return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}


static console_private_t *find_console_entry(const char *id)
{
	console_private_t *pcp = first_window;

	while (pcp)
	{
		if (strcmp(pcp->pchName, id)==0)
			break;

		pcp = pcp->pcpNext;
	}

	if (!pcp)
	{
		console_t *pConsole;
		
		pConsole = xmalloc( sizeof(console_t) );
		pcp = xmalloc( sizeof(console_private_t) );

		/* clear the whole structures */
		memset( pConsole, 0, sizeof(console_t) );
		memset( pcp, 0, sizeof(console_private_t) );

		/* link the structures to each other */
		pcp->pConsole     = pConsole;
		pConsole->private = pcp;

		/* copy the console name into the structure */
		pcp->pchName = stralloc(id);

		/* set the input to be returned when window is closed */
		/* @SRT TODO: this should be set by a function! */
		pcp->pchOnClose = stralloc( "x" );

		/* do first inits */
		pcp->xMax                   =
		pcp->pConsole->console_xres = 80;
		pcp->yMax                   =
		pcp->pConsole->console_yres = 25;

		pcp->xMin                   = MIN_XSIZE;
		pcp->yMin                   = MIN_YSIZE;

		pcp->xWindow                = CW_USEDEFAULT;
		pcp->yWindow                = CW_USEDEFAULT;

        pcp->fileOutput             = NULL;

		/* now, link the console_private_t structure into the list */
		pcp->pcpNext = first_window;
		first_window = pcp;
	}

	return pcp;
}

static console_t *console_open_internal(const char *id, HWND hwndParent, HWND hwndMdiClient)
{
	console_private_t *pcp;
	
	pcp = find_console_entry( id );

	allocate_window_memory( pcp );

	pcp->hwndParent    = hwndParent; 
    bIsMdiChild        =
    pcp->bIsMdiChild   = hwndMdiClient ? TRUE : FALSE;
    pcp->hwndMdiClient = hwndMdiClient;

    if (pcp->bIsMdiChild)
    {
        pcp->hwndConsole = CreateMDIWindow(CONSOLE_CLASS,
    		(LPTSTR) id,
	    	WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SIZEBOX,
            pcp->xWindow,
            pcp->yWindow,
            CW_USEDEFAULT,
            0,
            hwndMdiClient,
            winmain_instance,
            0);

		/* no previous active window */
		pcp->hwndPreviousActive = NULL;
    }
    else
    {
		SuspendFullscreenMode( pcp->hwndParent );

        pcp->hwndConsole = CreateWindow(CONSOLE_CLASS,
    		id,
	    	WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SIZEBOX,
            pcp->xWindow,
            pcp->yWindow,
            1,
            1,
            pcp->hwndParent,
            NULL,
            winmain_instance,
            NULL);

		/* get the previous active window, and set myself active */
		pcp->hwndPreviousActive = SetActiveWindow( pcp->hwndConsole );
    }



	/* get a DC and select proper font */
	pcp->hdc = GetDC( pcp->hwndConsole );
	SelectObject( pcp->hdc, GetStockObject( ANSI_FIXED_FONT ) );

	/* set colors for output */
	SetTextColor( pcp->hdc, GetSysColor( COLOR_WINDOWTEXT ) );
	SetBkColor( pcp->hdc, GetSysColor( COLOR_WINDOW ) );

	/* store pointer to structure with window */
	SetWindowLong( pcp->hwndConsole, GWL_USERDATA, (long) pcp );

	/* get the dimensions of one char */
	get_char_dimensions( pcp );

	/* set the window to the correct size */
	size_window( pcp );

	/* now show the window */
	ShowWindow( pcp->hwndConsole, SW_SHOW );

	pcp->pConsole->console_can_stay_open = 1;

    return pcp->pConsole;
}

console_t *console_open(const char *id)
{
    return console_open_internal(id,GetActiveWindow(),NULL);
}

console_t *arch_console_open_mdi(const char *id, void *hw, void *hwndParent,
                                 void *hwMdiClient)
{
    console_t *console_log;

    console_log = console_open_internal(id, *(HWND*)hwndParent, *(HWND*)hwMdiClient);

    if (hw)
    {
        *(HWND *)hw = console_log->private->hwndConsole;
    }

    return console_log;
}

int console_close(console_t *log)
{
	console_private_t *pcp = log->private;

	ReleaseDC( pcp->hwndConsole, pcp->hdc );

	DestroyWindow(pcp->hwndConsole);

	pcp->hwndConsole = NULL;

	/* set the previous active window as new active one */
	if (pcp->hwndPreviousActive)
		SetActiveWindow( pcp->hwndPreviousActive );

	if (pcp->bIsMdiChild)
	{
	}
	else
	{
		ResumeFullscreenMode( pcp->hwndParent );
	}

    return 0;
}


int console_init( void )
{
	WNDCLASSEX wc;

	/* mark: we don't have any console_private_t yet */
	first_window = NULL;

	/* Register 2nd window class for the monitor window */
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_CLASSDC;
	wc.lpfnWndProc   = console_window_proc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = winmain_instance;
	wc.hIcon         = LoadIcon(winmain_instance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0) + 1);
	wc.lpszMenuName  = 0; /* @SRT: no menu yet MAKEINTRESOURCE(menu); */
	wc.lpszClassName = CONSOLE_CLASS;
	wc.hIconSm       = NULL;

	RegisterClassEx(&wc);

	return 0;
}


int console_close_all( void )
{
	console_private_t *pcp;

	/* step through the windows, close all and free the used memory locations */
	pcp = first_window;

	while (pcp)
	{
		console_private_t *pcpNext = pcp->pcpNext;

		console_close( pcp->pConsole );
		free_window_memory( pcp );

		pcp = pcpNext;
	}

	first_window = NULL;

	UnregisterClass(CONSOLE_CLASS,winmain_instance);

	return 0;
}
