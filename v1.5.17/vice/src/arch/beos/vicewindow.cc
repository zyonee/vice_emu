/*
 * vicewindow.cc - Implementation of the BeVICE's window
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@arcormail.de>
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


#include <Alert.h>
#include <Application.h>
#include <FilePanel.h>
#include <MenuItem.h>

#include "vicemenu.h"

extern "C" {
#include "attach.h"
#include "constants.h"
#include "datasette.h"
#include "drive.h"
#include "info.h"
#include "interrupt.h"
#include "kbd.h"
#include "log.h"
#include "main.h"
#include "mouse.h"
#include "keyboard.h"
#include "machine.h"
#include "maincpu.h"
#include "main_exit.h"
#include "resources.h"
#include "sound.h"
#include "statusbar.h"
#include "tape.h"
#include "ui.h"
#include "ui_file.h"
#include "utils.h"
#include "vicewindow.h"
#include "vsync.h"
}

/* FIXME: some stuff we need from the ui module */
extern ViceWindow 			*windowlist[];
extern int 					window_count;
extern ui_res_value_list   	*machine_specific_values;
extern ui_menu_toggle      	*machine_specific_toggles;


void ViceWindow::Update_Menus(
		ui_menu_toggle *toggle_list,
		ui_res_value_list *value_list)
{
    int i,j;
    int value;
    int result;
	BMenuItem *item;
	
	/* the general toggle items */
	for (i = 0; toggle_list[i].name != NULL; i++) {
        resources_get_value(toggle_list[i].name, (resource_value_t *) &value);
        if (item = menubar->FindItem(toggle_list[i].item_id))
        	item->SetMarked(value ? true : false);
    }
    /* the machine specific toggle items */
    if (machine_specific_toggles) {
        for (i = 0; machine_specific_toggles[i].name != NULL; i++) {
            resources_get_value(machine_specific_toggles[i].name, (resource_value_t *) &value);
    	    if (item = menubar->FindItem(machine_specific_toggles[i].item_id))
	        	item->SetMarked(value ? true : false);
        }
    }

	/* the general multiple-value-items */
    for (i = 0; value_list[i].name != NULL; i++) {
        result=resources_get_value(value_list[i].name,
                                   (resource_value_t *) &value);
        if (result==0) {
            for (j = 0; value_list[i].vals[j].item_id != 0; j++) {
                if (value == value_list[i].vals[j].value) {
                	/* the corresponding menu is supposed to be in RadioMode */
                    if (item = menubar->FindItem(value_list[i].vals[j].item_id))
                    	item->SetMarked(true);
                }
            }
        }
    }
	/* the machine specific multiple-value-items */
    if (machine_specific_values){
        for (i = 0; machine_specific_values[i].name != NULL; i++) {
            result=resources_get_value(machine_specific_values[i].name,
                                       (resource_value_t *) &value);
            if (result==0) {
                for (j = 0; machine_specific_values[i].vals[j].item_id != 0; j++) {
                    if (value == machine_specific_values[i].vals[j].value) {
                        if (item = menubar->FindItem(machine_specific_values[i].vals[j].item_id))
                        	item->SetMarked(true);
                    }
                }
            }
        }
    }
}


/* the view for the emulators bitmap */
class ViceView : public BView {
	public:
		ViceView(BRect rect);
		virtual void Draw(BRect rect);
		virtual void MouseDown(BPoint point);
		virtual void MouseUp(BPoint point);
};

ViceView::ViceView(BRect rect) 
	: BView(rect,"view",B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW)
{
}

void ViceView::Draw(BRect rect) {
	BRect source_rect = rect;
	ViceWindow *wnd = (ViceWindow *)Window();

	if (wnd->bitmap) {
		source_rect.OffsetBy(wnd->bitmap_offset);
		DrawBitmap(wnd->bitmap, source_rect, rect);
	}
}

/* some hooks for the 1351 mouse emulation */
void ViceView::MouseDown(BPoint point) {
	BMessage *msg;
	int32 buttons;
	
	if (!_mouse_enabled)
		return;
	
	msg = Window()->CurrentMessage();
	msg->FindInt32("buttons", &buttons);
	if (buttons & B_PRIMARY_MOUSE_BUTTON)
		joystick_set_value_or(1,16);
}

void ViceView::MouseUp(BPoint point) {
	
	if (!_mouse_enabled)
		return;
	
	joystick_set_value_and(1,239);
}

ViceWindow::ViceWindow(BRect frame, char const *title) 
		: BWindow(frame, title,
		B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS) {

	BMenu *menu, *submenu;
	BMenuItem *item;
	BRect r;

	/* create the menubar; key events reserved for the emu */
	menubar = menu_create(machine_class);
	AddChild(menubar);
	SetKeyMenuBar(NULL);

	/* create the File Panel */
	filepanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL,
				B_FILE_NODE, false);

	/* create the Save Panel */
	savepanel = new BFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL,
				B_FILE_NODE, false);
		
	/* the view for the canvas */
	r = menubar->Frame();
	view = new ViceView(BRect(frame.left,frame.top+r.Height()+1,
						frame.right,frame.bottom+r.Height()+2));
	AddChild(view);
	
	/* bitmap is NULL; will be registered by canvas_refresh */
	bitmap = NULL;
	
	/* the statusbar is created in Resize() */
	statusbar = NULL;

	/* register the window */
	windowlist[window_count++] = this;

	/* finally display the window */
	Resize(frame.Width(), frame.Height());
	Show();
}

ViceWindow::~ViceWindow() {
	RemoveChild(menubar);
	delete menubar;
	RemoveChild(view);
	delete view;
	RemoveChild(statusbar);
	delete statusbar;
	delete filepanel;
	delete savepanel;
}


bool ViceWindow::QuitRequested() {
	/* send an exit request to ui's event loop but dont't close the window here */
	BMessage msg;
	msg.what = MENU_EXIT_REQUESTED;
	ui_add_event(&msg);
	return false;
}


void ViceWindow::MessageReceived(BMessage *message) {
	/* FIXME: sometimes the menubar holds the focus so we have to delete it */ 
	if (CurrentFocus()) {
		CurrentFocus()->MakeFocus(false);
	}
		
	ui_add_event(message);
	switch(message->what) {
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

void ViceWindow::Resize(unsigned int width, unsigned int height) {
	BRect statusbar_frame;
	float new_windowheight;
	
	/* bitmap has to be reregistered with new framebuffer */ 
	bitmap = NULL;
	if (BWindow::Lock()) {
		view->ResizeTo(width, height);
		if (statusbar) {
			RemoveChild(statusbar);
			delete statusbar;
			statusbar = NULL;
		}
		statusbar_frame.top = view->Frame().bottom+1;
		statusbar_frame.bottom = view->Frame().bottom+41;
		statusbar_frame.left = 0;
		statusbar_frame.right = view->Frame().right;
		statusbar = new ViceStatusbar(statusbar_frame);
		AddChild(statusbar);
		ui_statusbar_update();
		new_windowheight = 	menubar->Frame().Height()+
			view->Frame().Height()+
			statusbar->Frame().Height();
		BWindow::ResizeTo(width-1, new_windowheight);
		/* who knows why the window width has to be width-1 */
		BWindow::Unlock();
	}
}

void ViceWindow::DrawBitmap(BBitmap *framebitmap, 
	int xs, int ys, int xi, int yi, int w, int h) {
	if	(BWindow::Lock()) {
		view->DrawBitmap(framebitmap, 
			BRect(xs,ys,xs+w,ys+h),
			BRect(xi,yi,xi+w,yi+h) );
		view->Sync();
		BWindow::Unlock();
	} 
}
	
