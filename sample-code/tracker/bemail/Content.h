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
//	Content.h
//	
//--------------------------------------------------------------------

#ifndef _CONTENT_H
#define _CONTENT_H

#include <FilePanel.h>
#include <FindDirectory.h>
#include <Font.h>
#include <fs_attr.h>
#include <Point.h>
#include <Rect.h>

#define MESSAGE_TEXT		"Message:"
#define MESSAGE_TEXT_H		 16
#define MESSAGE_TEXT_V		 5
#define MESSAGE_FIELD_H		 59
#define MESSAGE_FIELD_V		 11

#define CONTENT_TYPE		"content-type: "
#define CONTENT_ENCODING	"content-transfer-encoding: "
#define CONTENT_DISPOSITION	"Content-Disposition: "
#define MIME_TEXT			"text/"
#define MIME_MULTIPART		"multipart/"

class TMailWindow;
class TScrollView;
class TTextView;
class BFile;
class BList;
class BPopupMenu;

struct text_run_array;

typedef struct {
	bool header;
	bool raw;
	bool quote;
	bool incoming;
	bool close;
	bool mime;
	TTextView *view;
	BFile *file;
	BList *enclosures;
	sem_id *stop_sem;
} reader;

enum ENCLOSURE_TYPE {
	TYPE_ENCLOSURE = 100,
	TYPE_BE_ENCLOSURE,
	TYPE_URL,
	TYPE_MAILTO
};

typedef struct {
	int32 type;
	char *name;
	char *content_type;
	char *encoding;
	int32 text_start;
	int32 text_end;
	off_t file_offset;
	off_t file_length;
	bool saved;
	bool have_ref;
	entry_ref ref;
	node_ref node;
} hyper_text;

bool get_semaphore(BWindow*, sem_id*);
bool insert(reader*, char*, int32, bool);
bool parse_header(char*, char*, off_t, char*, reader*, off_t*);
bool strip_it(char*, int32, reader*);

class TSavePanel;


//====================================================================

class TContentView : public BView {
public:
	TContentView(BRect, bool, BFile*, BFont*); 
	virtual void MessageReceived(BMessage*);
	void FindString(const char*);
	void Focus(bool);
	void FrameResized(float, float);
	
	TTextView *fTextView;

private:
	bool fFocus;
	bool fIncoming;
	float fOffset;
};

//====================================================================

enum {
	S_CLEAR_ERRORS = 1,
	S_SHOW_ERRORS = 2
};

class TTextView : public BTextView {
public:
	TTextView(BRect, BRect, bool, BFile*, TContentView*,BFont*);
	~TTextView();
	virtual	void AttachedToWindow();
	virtual void KeyDown(const char*, int32);
	virtual void MakeFocus(bool);
	virtual void MessageReceived(BMessage*);
	virtual void MouseDown(BPoint);
	virtual void MouseMoved(BPoint, uint32, const BMessage*);
	virtual void InsertText( const char *text, int32 length, int32 offset,
		const text_run_array *runs );
	virtual void  DeleteText(int32 start, int32 finish);
            
	void ClearList();
	void LoadMessage(BFile*, bool, bool, const char*);
	void Open(hyper_text*);
	static status_t	Reader(reader*);
	status_t Save(BMessage*, bool makeNewFile = true);
	void SaveBeFile(BFile*, char*, ssize_t);
	void StopLoad();
	void AddAsContent(BMailMessage*, bool);
	void CheckSpelling(int32 start, int32 end,
		int32 flags = S_CLEAR_ERRORS | S_SHOW_ERRORS);
	void FindSpellBoundry(int32 length, int32 offset, int32 *start,
		int32 *end);
	void EnableSpellCheck(bool enable);

	bool fHeader;
	bool fReady;

private:
	void ContentChanged( void );
	
	char *fYankBuffer;
	int32 fLastPosition;
	BFile *fFile;
	BFont fFont;
	TContentView *fParent;
	sem_id fStopSem;
	thread_id fThread;
	BList *fEnclosures;
	BPopUpMenu *fEnclosureMenu;
	BPopUpMenu *fLinkMenu;
	TSavePanel *fPanel;
	bool fIncoming;
	bool fSpellCheck;
	bool fRaw;
	bool fCursor;
};


//====================================================================

class TSavePanel : public BFilePanel {
public:
	TSavePanel(hyper_text*, TTextView*);
	virtual void SendMessage(const BMessenger*, BMessage*);
	void SetEnclosure(hyper_text*);

private:
	hyper_text *fEnclosure;
	TTextView *fView;
};

#endif // #ifndef _CONTENT_H
