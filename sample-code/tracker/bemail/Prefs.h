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
//	Prefs.h
//
//--------------------------------------------------------------------

#ifndef _PREFS_H
#define _PREFS_H

#include <Font.h>
#include <Window.h>

#define	PREF_WIDTH			300
#define PREF_HEIGHT			250

#define SIG_NONE			"None"
#define SIG_RANDOM			"Random"

class Button;

//====================================================================

class TPrefsWindow : public BWindow {
public:
	TPrefsWindow(BRect, BFont*, int32*, bool*, char**, 
		uint32 *encoding, bool *buttonBar);
	~TPrefsWindow();
	virtual void MessageReceived(BMessage*);
	BPopUpMenu* BuildFontMenu(BFont*);
	BPopUpMenu* BuildLevelMenu(int32);
	BPopUpMenu* BuildSignatureMenu(char*);
	BPopUpMenu* BuildSizeMenu(BFont*);
	BPopUpMenu* BuildWrapMenu(bool);
	BPopUpMenu* BuildEncodingMenu(uint32 encoding);
	BPopUpMenu* BuildButtonBarMenu(bool show);

private:
	bool *fNewWrap;
	bool *fNewButtonBar;
	char *fSignature;
	char **fNewSignature;
	int32 fLevel;
	int32 *fNewLevel;
	BButton *fOK;
	BButton *fCancel;
	BButton *fRevert;
	BFont fFont;
	BFont *fNewFont;
	uint32 fEncoding;
	uint32 *fNewEncoding;
	BPopUpMenu *fFontMenu;
	BPopUpMenu *fSizeMenu;
	BPopUpMenu *fLevelMenu;
	BPopUpMenu *fWrapMenu;
	BPopUpMenu *fSignatureMenu;
	BPopUpMenu *fEncodingMenu;
	BPopUpMenu *fButtonBarMenu;
	bool fWrap;
	bool fButtonBar;
};

#endif // #ifndef _PREFS_H
