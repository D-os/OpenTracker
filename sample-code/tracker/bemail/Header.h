/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2001, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

BeMail(TM), Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/

//--------------------------------------------------------------------
//	
//	Header.h
//
//--------------------------------------------------------------------

#ifndef _HEADER_H
#define _HEADER_H

#include <fs_attr.h>
#include <NodeInfo.h>
#include <Point.h>
#include <Rect.h>
#include <TextControl.h>
#include <View.h>
#include <Window.h>

#include "ComboBox.h"

#define HEADER_HEIGHT		 82
#define MIN_HEADER_HEIGHT	(HEADER_HEIGHT - 8)//26)

#define TO_TEXT				"To:"
#define FROM_TEXT			"From:"
#define TO_FIELD_H			 39
#define FROM_FIELD_H		 31
#define TO_FIELD_V			  8
#define TO_FIELD_WIDTH		270
#define FROM_FIELD_WIDTH	280
#define TO_FIELD_HEIGHT		 16

#define SUBJECT_TEXT		"Subject:"
#define SUBJECT_FIELD_H		 18
#define SUBJECT_FIELD_V		 33
#define SUBJECT_FIELD_WIDTH	270
#define SUBJECT_FIELD_HEIGHT 16

#define CC_TEXT				"CC:"
#define CC_FIELD_H			 40
#define CC_FIELD_V			 58
#define CC_FIELD_WIDTH		192
#define CC_FIELD_HEIGHT		 16

#define BCC_TEXT			"BCC:"
#define BCC_FIELD_H			268
#define BCC_FIELD_V			 58
#define BCC_FIELD_WIDTH		197
#define BCC_FIELD_HEIGHT	 16

class	TTextControl;
class	BFile;
class	BMenuField;
class	BMenuItem;
class	BPopupMenu;
class	QPopupMenu;

//====================================================================

class THeaderView : public BBox {
public:
	THeaderView(BRect, BRect, bool, BFile*, bool); 
	virtual void MessageReceived(BMessage*);
	virtual void AttachedToWindow(void);
	void BuildMenus();
	void SetAddress(BMessage*);
	status_t LoadMessage(BFile*);

	TTextControl *fBcc;
	TTextControl *fCc;
	TTextControl *fSubject;
	TTextControl *fTo;

private:		
	void InitEmailCompletion();
	void InitGroupCompletion();
	
	bool fIncoming;
	bool fResending;
	QPopupMenu *fBccMenu;
	QPopupMenu *fCcMenu;
	QPopupMenu *fToMenu;
	BFile *fFile;
	BStringView *fDate;
	BDefaultChoiceList emailList;
};

//====================================================================

class TTextControl : public BComboBox {
public:
	TTextControl(BRect, char*, BMessage*, bool, bool, 
		int32 resizingMode = B_FOLLOW_NONE);
	virtual void AttachedToWindow();
	virtual void MessageReceived(BMessage*);

private:
	bool fIncoming;
	bool fResending;
	char fLabel[100];
	int32 fCommand;
};

#endif // #ifndef _HEADER_H
