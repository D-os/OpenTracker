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

#ifndef _BMAP_BUTTON_H
#define _BMAP_BUTTON_H

#include <Control.h>
#include <List.h>
#include <Locker.h>
#include <View.h>

class BBitmap;
class BResources;

class BmapButton : public BControl {
public:
	BmapButton(BRect frame, const char *name, const char *label,
		int32 enabledID, int32 disabledID, int32 rollID, int32 pressedID,
		bool showLabel, BMessage *message, uint32 resizeMask,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
	virtual ~BmapButton(void);
		
	// Hooks
	virtual void Draw(BRect updateRect);
	virtual void GetPreferredSize(float *width, float *height);
	virtual void MouseMoved(BPoint where, uint32 code, const BMessage *msg);
	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint where);
	virtual void WindowActivated(bool active);
		
	void InvokeOnButton(uint32 button);
	void ShowLabel(bool show);
	
protected:
	const BBitmap *RetrieveBitmap(int32 id);
	status_t ReleaseBitmap(const BBitmap *bm);
		
	const BBitmap *fEnabledBM;
	const BBitmap *fDisabledBM;
	const BBitmap *fRollBM;
	const BBitmap *fPressedBM;
	int32 fPressing;
	int32 fIsInBounds;
	uint32 fButtons;
	bool fShowLabel;
	bool fActive;
	BRect fBitmapRect;
	BPoint fWhere;
	uint32 fIButtons;
	
private:
	static BList fBitmapCache;
	static BLocker fBmCacheLock;
};

#endif // #ifndef _BMAP_BUTTON_H
