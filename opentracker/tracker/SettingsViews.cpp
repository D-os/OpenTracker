/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

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

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/

#include <Alert.h>

#include "Commands.h"
#include "DeskWindow.h"
#include "Model.h"
#include "SettingsViews.h"
#include "Tracker.h"
#include "WidgetAttributeText.h"

#include <Box.h>
#include <Button.h>
#include <MenuField.h>
#include <ColorControl.h>
#include <NodeMonitor.h>
#include <StringView.h>

const uint32 kSpaceBarSwitchColor = 'SBsc';


SettingsView::SettingsView(BRect rect, const char *name)
	:	BView(rect, name, B_FOLLOW_ALL_SIDES, 0)
{
}

SettingsView::~SettingsView()
{
}

// The inherited functions should set the default values
// and update the UI gadgets. The latter can by done by
// calling ShowCurrentSettings().
void SettingsView::SetDefaults() {}
	
// The inherited functions should set the values that was
// active when the settings window opened. It should also
// update the UI widgets accordingly, preferrable by calling
// ShowCurrentSettings().
void SettingsView::Revert() {}

// This function is called when the window is shown to let
// the settings views record the state to revert to.
void SettingsView::RecordRevertSettings() {}

// This function is used by the window to tell the view
// to display the current settings in the tracker.
void SettingsView::ShowCurrentSettings(bool) {}

// This function is used by the window to tell whether
// it can ghost the revert button or not. It it shows the
// reverted settings, this function should return true.
bool SettingsView::ShowsRevertSettings() const { return true; }

namespace BPrivate {
	const float kBorderSpacing = 5.0f;
	const float kItemHeight = 18.0f;
	const float kItemExtraSpacing = 2.0f;
	const float kIndentSpacing = 12.0f;
}

//------------------------------------------------------------------------
// #pragma mark -

DesktopSettingsView::DesktopSettingsView(BRect rect)
	:	SettingsView(rect, "DesktopSettingsView")
{
	BRect frame = BRect(kBorderSpacing, kBorderSpacing, rect.Width()
		- 2 * kBorderSpacing, kBorderSpacing + kItemHeight);

	fShowDisksIconRadioButton = new BRadioButton(frame, "", "Show Disks Icon",
		new BMessage(kShowDisksIconChanged));
	AddChild(fShowDisksIconRadioButton);
	fShowDisksIconRadioButton->ResizeToPreferred();
	
	const float itemSpacing = fShowDisksIconRadioButton->Bounds().Height() + kItemExtraSpacing;

	frame.OffsetBy(0, itemSpacing);
	
	fMountVolumesOntoDesktopRadioButton =
		new BRadioButton(frame, "", "Show Volumes On Desktop",
			new BMessage(kVolumesOnDesktopChanged));
	AddChild(fMountVolumesOntoDesktopRadioButton);
	fMountVolumesOntoDesktopRadioButton->ResizeToPreferred();

	frame.OffsetBy(20, itemSpacing);

	fMountSharedVolumesOntoDesktopCheckBox =
		new BCheckBox(frame, "", "Show Shared Volumes On Desktop",
			new BMessage(kVolumesOnDesktopChanged));
	AddChild(fMountSharedVolumesOntoDesktopCheckBox);
	fMountSharedVolumesOntoDesktopCheckBox->ResizeToPreferred();

	frame.OffsetBy(-20, 2 * itemSpacing);

	fIntegrateNonBootBeOSDesktopsCheckBox =
		new BCheckBox(frame, "", "Integrate Non-Boot BeOS Desktops",
			new BMessage(kDesktopIntegrationChanged));
	AddChild(fIntegrateNonBootBeOSDesktopsCheckBox);
	fIntegrateNonBootBeOSDesktopsCheckBox->ResizeToPreferred();

	frame.OffsetBy(0, itemSpacing);

	BButton *button =
		new BButton(BRect(kBorderSpacing, rect.Height() - kBorderSpacing - 20,
			kBorderSpacing + 100, rect.Height() - kBorderSpacing),
			"", "Mount Settings"B_UTF8_ELLIPSIS, new BMessage(kRunAutomounterSettings));
	AddChild(button);		

	button->ResizeToPreferred();
	button->MoveBy(0, rect.Height() - kBorderSpacing - button->Frame().bottom);
	button->SetTarget(be_app);

}

void
DesktopSettingsView::AttachedToWindow()
{
	fShowDisksIconRadioButton->SetTarget(this);
	fMountVolumesOntoDesktopRadioButton->SetTarget(this);
	fMountSharedVolumesOntoDesktopCheckBox->SetTarget(this);
	fIntegrateNonBootBeOSDesktopsCheckBox->SetTarget(this);
}

void
DesktopSettingsView::MessageReceived(BMessage *message)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	switch (message->what) {
		
		case kShowDisksIconChanged:
			{
				// Turn on and off related settings:
				fMountVolumesOntoDesktopRadioButton->SetValue(
					!fShowDisksIconRadioButton->Value() == 1);
				fMountSharedVolumesOntoDesktopCheckBox->SetEnabled(
					fMountVolumesOntoDesktopRadioButton->Value() == 1);
				
				// Set the new settings in the tracker:
				tracker->SetShowDisksIcon(fShowDisksIconRadioButton->Value() == 1);
				tracker->SetMountVolumesOntoDesktop(
					fMountVolumesOntoDesktopRadioButton->Value() == 1);
				tracker->SetMountSharedVolumesOntoDesktop(
					fMountSharedVolumesOntoDesktopCheckBox->Value() == 1);
				
				// Construct the notification message:				
				BMessage notificationMessage;
				notificationMessage.AddBool("ShowDisksIcon",
					fShowDisksIconRadioButton->Value() == 1);
				notificationMessage.AddBool("MountVolumesOntoDesktop",
					fMountVolumesOntoDesktopRadioButton->Value() == 1);
				notificationMessage.AddBool("MountSharedVolumesOntoDesktop",
					fMountSharedVolumesOntoDesktopCheckBox->Value() == 1);

				// Send the notification message:
				tracker->SendNotices(kVolumesOnDesktopChanged, &notificationMessage);
				
				// Tell the settings window the contents have changed:
				Window()->PostMessage(kSettingsContentsModified);
				break;
			}
			
		case kVolumesOnDesktopChanged:
			{
				// Turn on and off related settings:
				fShowDisksIconRadioButton->SetValue(
					!fMountVolumesOntoDesktopRadioButton->Value() == 1);
				fMountSharedVolumesOntoDesktopCheckBox->SetEnabled(
					fMountVolumesOntoDesktopRadioButton->Value() == 1);
				
				// Set the new settings in the tracker:
				tracker->SetShowDisksIcon(fShowDisksIconRadioButton->Value() == 1);
				tracker->SetMountVolumesOntoDesktop(
					fMountVolumesOntoDesktopRadioButton->Value() == 1);
				tracker->SetMountSharedVolumesOntoDesktop(
					fMountSharedVolumesOntoDesktopCheckBox->Value() == 1);
				
				// Construct the notification message:				
				BMessage notificationMessage;
				notificationMessage.AddBool("ShowDisksIcon",
					fShowDisksIconRadioButton->Value() == 1);
				notificationMessage.AddBool("MountVolumesOntoDesktop",
					fMountVolumesOntoDesktopRadioButton->Value() == 1);
				notificationMessage.AddBool("MountSharedVolumesOntoDesktop",
					fMountSharedVolumesOntoDesktopCheckBox->Value() == 1);

				// Send the notification message:
				tracker->SendNotices(kVolumesOnDesktopChanged, &notificationMessage);
				
				// Tell the settings window the contents have changed:
				Window()->PostMessage(kSettingsContentsModified);
				break;
			}
		
		case kDesktopIntegrationChanged:
			{
				// Set the new settings in the tracker:
				tracker->SetIntegrateNonBootBeOSDesktops(
					fIntegrateNonBootBeOSDesktopsCheckBox->Value() == 1);
				
				// Construct the notification message:				
				BMessage notificationMessage;
				notificationMessage.AddBool("MountVolumesOntoDesktop",
					fMountVolumesOntoDesktopRadioButton->Value() == 1);
				notificationMessage.AddBool("MountSharedVolumesOntoDesktop",
					fMountSharedVolumesOntoDesktopCheckBox->Value() == 1);
				notificationMessage.AddBool("IntegrateNonBootBeOSDesktops",
					fIntegrateNonBootBeOSDesktopsCheckBox->Value() == 1);

				// Send the notification message:
				tracker->SendNotices(kDesktopIntegrationChanged, &notificationMessage);
				
				// Tell the settings window the contents have changed:
				Window()->PostMessage(kSettingsContentsModified);
				break;
			}

		default:
			_inherited::MessageReceived(message);
	}
}

	
void
DesktopSettingsView::SetDefaults()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;
	
	// ToDo: Avoid the duplication of the default values.
	tracker->SetShowDisksIcon(false);
	tracker->SetMountVolumesOntoDesktop(true);
	tracker->SetMountSharedVolumesOntoDesktop(false);
	tracker->SetIntegrateNonBootBeOSDesktops(true);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
DesktopSettingsView::Revert()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	tracker->SetShowDisksIcon(fShowDisksIcon);
	tracker->SetMountVolumesOntoDesktop(fMountVolumesOntoDesktop);
	tracker->SetMountSharedVolumesOntoDesktop(fMountSharedVolumesOntoDesktop);
	tracker->SetIntegrateNonBootBeOSDesktops(fIntegrateNonBootBeOSDesktops);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
DesktopSettingsView::ShowCurrentSettings(bool sendNotices)
{
	fShowDisksIconRadioButton->SetValue(TTracker::ShowDisksIcon());
	fMountVolumesOntoDesktopRadioButton->SetValue(TTracker::MountVolumesOntoDesktop());

	fMountSharedVolumesOntoDesktopCheckBox->SetValue(TTracker::MountSharedVolumesOntoDesktop());
	fMountSharedVolumesOntoDesktopCheckBox->SetEnabled(TTracker::MountVolumesOntoDesktop());

	fIntegrateNonBootBeOSDesktopsCheckBox->SetValue(TTracker::IntegrateNonBootBeOSDesktops());

	if (sendNotices) {
		TTracker *tracker = dynamic_cast<TTracker *>(be_app);
		if (!tracker)
			return;
	
		// Construct the notification message:				
		BMessage notificationMessage;
		notificationMessage.AddBool("ShowDisksIcon",
			fShowDisksIconRadioButton->Value() == 1);
		notificationMessage.AddBool("MountVolumesOntoDesktop",
			fMountVolumesOntoDesktopRadioButton->Value() == 1);
		notificationMessage.AddBool("MountSharedVolumesOntoDesktop",
			fMountSharedVolumesOntoDesktopCheckBox->Value() == 1);
		notificationMessage.AddBool("IntegrateNonBootBeOSDesktops",
			fIntegrateNonBootBeOSDesktopsCheckBox->Value() == 1);

		// Send notices to the tracker about the change:
		tracker->SendNotices(kVolumesOnDesktopChanged, &notificationMessage);
		tracker->SendNotices(kDesktopIntegrationChanged, &notificationMessage);
	}
}


void
DesktopSettingsView::RecordRevertSettings()
{
	fShowDisksIcon = TTracker::ShowDisksIcon();
	fMountVolumesOntoDesktop = TTracker::MountVolumesOntoDesktop();
	fMountSharedVolumesOntoDesktop = TTracker::MountSharedVolumesOntoDesktop();
	fIntegrateNonBootBeOSDesktops = TTracker::IntegrateNonBootBeOSDesktops();
}

bool
DesktopSettingsView::ShowsRevertSettings() const
{
	return
		(fShowDisksIcon ==
			(fShowDisksIconRadioButton->Value() > 0))
		&& (fMountVolumesOntoDesktop ==
			(fMountVolumesOntoDesktopRadioButton->Value() > 0))
		&& (fMountSharedVolumesOntoDesktop ==
			(fMountSharedVolumesOntoDesktopCheckBox->Value() > 0))
		&& (fIntegrateNonBootBeOSDesktops == 
			(fIntegrateNonBootBeOSDesktopsCheckBox->Value() > 0));
}

//------------------------------------------------------------------------
// #pragma mark -

WindowsSettingsView::WindowsSettingsView(BRect rect)
	:	SettingsView(rect, "WindowsSettingsView")
{
	BRect frame = BRect(kBorderSpacing, kBorderSpacing, rect.Width()
		- 2 * kBorderSpacing, kBorderSpacing + kItemHeight);

	fShowFullPathInTitleBarCheckBox = new BCheckBox(frame, "", "Show Full Path In Title Bar",
		new BMessage(kWindowsShowFullPathChanged));
	AddChild(fShowFullPathInTitleBarCheckBox);
	fShowFullPathInTitleBarCheckBox->ResizeToPreferred();

	const float itemSpacing = fShowFullPathInTitleBarCheckBox->Bounds().Height() + kItemExtraSpacing;

	frame.OffsetBy(0, itemSpacing);

	fSingleWindowBrowseCheckBox = new BCheckBox(frame, "", "Single Window Browse",
		new BMessage(kSingleWindowBrowseChanged));
	AddChild(fSingleWindowBrowseCheckBox);
	fSingleWindowBrowseCheckBox->ResizeToPreferred();

	frame.OffsetBy(20, itemSpacing);

	fShowNavigatorCheckBox = new BCheckBox(frame, "", "Show Navigator",
		new BMessage(kShowNavigatorChanged));
	AddChild(fShowNavigatorCheckBox);
	fShowNavigatorCheckBox->ResizeToPreferred();

	frame.OffsetBy(-20, itemSpacing);


	fShowSelectionWhenInactiveCheckBox = new BCheckBox(frame, "", "Show Selection When Inactive",
		new BMessage(kShowSelectionWhenInactiveChanged));
	AddChild(fShowSelectionWhenInactiveCheckBox);
	fShowSelectionWhenInactiveCheckBox->ResizeToPreferred();

	frame.OffsetBy(0, itemSpacing);

	fSortFolderNamesFirstCheckBox = new BCheckBox(frame, "", "Sort Folder Names First",
		new BMessage(kSortFolderNamesFirstChanged));
	AddChild(fSortFolderNamesFirstCheckBox);
	fSortFolderNamesFirstCheckBox->ResizeToPreferred();
}

void
WindowsSettingsView::AttachedToWindow()
{
	fSingleWindowBrowseCheckBox->SetTarget(this);
	fShowNavigatorCheckBox->SetTarget(this);
	fShowFullPathInTitleBarCheckBox->SetTarget(this);
	fShowSelectionWhenInactiveCheckBox->SetTarget(this);
	fSortFolderNamesFirstCheckBox->SetTarget(this);
}

void
WindowsSettingsView::MessageReceived(BMessage *message)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;
		
	switch (message->what) {
		case kWindowsShowFullPathChanged:
			tracker->SetShowFullPathInTitleBar(fShowFullPathInTitleBarCheckBox->Value() == 1);
			tracker->SendNotices(kWindowsShowFullPathChanged);
			Window()->PostMessage(kSettingsContentsModified);
			break;
		
		case kSingleWindowBrowseChanged:
			tracker->SetSingleWindowBrowse(fSingleWindowBrowseCheckBox->Value() == 1);
			if (fSingleWindowBrowseCheckBox->Value() == 0) {
				fShowNavigatorCheckBox->SetEnabled(false);
				tracker->SetShowNavigator(0);
			} else {
				fShowNavigatorCheckBox->SetEnabled(true);
				tracker->SetShowNavigator(fShowNavigatorCheckBox->Value() != 0);
			}				
			tracker->SendNotices(kShowNavigatorChanged);
			tracker->SendNotices(kSingleWindowBrowseChanged);
			Window()->PostMessage(kSettingsContentsModified);
			break;

		case kShowNavigatorChanged:
			tracker->SetShowNavigator(fShowNavigatorCheckBox->Value() == 1);
			tracker->SendNotices(kShowNavigatorChanged);
			Window()->PostMessage(kSettingsContentsModified);
			break;

		case kShowSelectionWhenInactiveChanged:
			{
				tracker->SetShowSelectionWhenInactive(fShowSelectionWhenInactiveCheckBox->Value() == 1);

				// Make the notification message and send it to the tracker:
				BMessage notificationMessage;
				notificationMessage.AddBool("ShowSelectionWhenInactive",
					fShowSelectionWhenInactiveCheckBox->Value() == 1);
				tracker->SendNotices(kShowSelectionWhenInactiveChanged, &notificationMessage);

				Window()->PostMessage(kSettingsContentsModified);
			}
			break;

		case kSortFolderNamesFirstChanged:
			{
				tracker->SetSortFolderNamesFirst(fSortFolderNamesFirstCheckBox->Value() == 1);

				// Make the notification message and send it to the tracker:
				BMessage notificationMessage;
				notificationMessage.AddBool("SortFolderNamesFirst",
					fSortFolderNamesFirstCheckBox->Value() == 1);
				tracker->SendNotices(kSortFolderNamesFirstChanged, &notificationMessage);

				Window()->PostMessage(kSettingsContentsModified);
			}
			break;

		default:
			_inherited::MessageReceived(message);
			break;
	}
}

	
void
WindowsSettingsView::SetDefaults()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	tracker->SetShowFullPathInTitleBar(false);
	tracker->SetSingleWindowBrowse(false);
	tracker->SetShowNavigator(false);
	tracker->SetShowSelectionWhenInactive(true);
	tracker->SetSortFolderNamesFirst(false);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
WindowsSettingsView::Revert()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	tracker->SetShowFullPathInTitleBar(fShowFullPathInTitleBar);
	tracker->SetSingleWindowBrowse(fSingleWindowBrowse);
	tracker->SetShowNavigator(fShowNavigator);
	tracker->SetShowSelectionWhenInactive(fShowSelectionWhenInactive);
	tracker->SetSortFolderNamesFirst(fSortFolderNamesFirst);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
WindowsSettingsView::ShowCurrentSettings(bool sendNotices)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	fShowFullPathInTitleBarCheckBox->SetValue(tracker->ShowFullPathInTitleBar());
	fSingleWindowBrowseCheckBox->SetValue(tracker->SingleWindowBrowse());
	fShowNavigatorCheckBox->SetEnabled(tracker->SingleWindowBrowse());
	fShowNavigatorCheckBox->SetValue(tracker->ShowNavigator());
	fShowSelectionWhenInactiveCheckBox->SetValue(tracker->ShowSelectionWhenInactive());
	fSortFolderNamesFirstCheckBox->SetValue(tracker->SortFolderNamesFirst());
	
	if (sendNotices) {
		tracker->SendNotices(kSingleWindowBrowseChanged);
		tracker->SendNotices(kShowNavigatorChanged);
		tracker->SendNotices(kWindowsShowFullPathChanged);
		tracker->SendNotices(kShowSelectionWhenInactiveChanged);
		tracker->SendNotices(kSortFolderNamesFirstChanged);
	}
}


void
WindowsSettingsView::RecordRevertSettings()
{
	fShowFullPathInTitleBar = TTracker::ShowFullPathInTitleBar();
	fSingleWindowBrowse = TTracker::SingleWindowBrowse();
	fShowNavigator = TTracker::ShowNavigator();
	fShowSelectionWhenInactive = TTracker::ShowSelectionWhenInactive();
	fSortFolderNamesFirst = TTracker::SortFolderNamesFirst();
}

bool
WindowsSettingsView::ShowsRevertSettings() const
{
	return
		(fShowFullPathInTitleBar ==
			(fShowFullPathInTitleBarCheckBox->Value() > 0))
		&& (fSingleWindowBrowse ==
			(fSingleWindowBrowseCheckBox->Value() > 0))
		&& (fShowNavigator ==
			(fShowNavigatorCheckBox->Value() > 0))
		&& (fShowSelectionWhenInactive ==
			(fShowSelectionWhenInactiveCheckBox->Value() > 0))
		&& (fSortFolderNamesFirst ==
			(fSortFolderNamesFirstCheckBox->Value() > 0));
}

//------------------------------------------------------------------------
// #pragma mark -

FilePanelSettingsView::FilePanelSettingsView(BRect rect)
	:	SettingsView(rect, "FilePanelSettingsView")
{
	BRect frame = BRect(kBorderSpacing, kBorderSpacing, rect.Width()
		- 2 * kBorderSpacing, kBorderSpacing + kItemHeight);

	fDesktopFilePanelRootCheckBox = new BCheckBox(frame, "", "File Panel Root is Desktop",
		new BMessage(kDesktopFilePanelRootChanged));
	AddChild(fDesktopFilePanelRootCheckBox);
	fDesktopFilePanelRootCheckBox->ResizeToPreferred();

	const float itemSpacing = fDesktopFilePanelRootCheckBox->Bounds().Height() + kItemExtraSpacing;

	frame.OffsetBy(0, itemSpacing);
	
	BRect recentBoxFrame(kBorderSpacing, frame.bottom, rect.Width() - kBorderSpacing, frame.top);

	BBox *recentBox = new BBox(recentBoxFrame, "recentBox");
	recentBox->SetLabel("Recent" B_UTF8_ELLIPSIS);

	AddChild(recentBox);
	
	frame = recentBoxFrame.OffsetToCopy(0,0);
	frame.OffsetTo(kBorderSpacing, 3 * kBorderSpacing);
	
	frame.right = StringWidth("##Applications###10##");
	float divider = StringWidth("Applications") + 10;
		
	fRecentDocumentsTextControl = new BTextControl(frame, "", "Documents", "10",
		new BMessage(kFavoriteCountChanged));

	fRecentDocumentsTextControl->SetDivider(divider);

	frame.OffsetBy(0, itemSpacing);

	fRecentFoldersTextControl = new BTextControl(frame, "", "Folders", "10",
		new BMessage(kFavoriteCountChanged));

	fRecentFoldersTextControl->SetDivider(divider);

	recentBox->AddChild(fRecentDocumentsTextControl);
	recentBox->AddChild(fRecentFoldersTextControl);
	
	recentBox->ResizeTo(recentBox->Frame().Width(), fRecentFoldersTextControl->Frame().bottom + kBorderSpacing);
	
	StartWatching(be_app, kFavoriteCountChangedExternally);
}

FilePanelSettingsView::~FilePanelSettingsView()
{
	StopWatching(be_app, kFavoriteCountChangedExternally);
}


void
FilePanelSettingsView::AttachedToWindow()
{
	fDesktopFilePanelRootCheckBox->SetTarget(this);
	fRecentDocumentsTextControl->SetTarget(this);
	fRecentFoldersTextControl->SetTarget(this);
}

void
FilePanelSettingsView::MessageReceived(BMessage *message)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;
		
	switch (message->what) {
		case kDesktopFilePanelRootChanged:
			{
				tracker->SetDesktopFilePanelRoot(fDesktopFilePanelRootCheckBox->Value() == 1);

				// Make the notification message and send it to the tracker:
				BMessage message;
				message.AddBool("DesktopFilePanelRoot", fDesktopFilePanelRootCheckBox->Value() == 1);
				tracker->SendNotices(kDesktopFilePanelRootChanged, &message);

				Window()->PostMessage(kSettingsContentsModified);
			}
			break;
		
		case kFavoriteCountChanged:
			{
				GetAndRefreshDisplayedFigures();
				tracker->SetRecentDocumentsCount(fDisplayedDocCount);
				tracker->SetRecentFoldersCount(fDisplayedFolderCount);

				// Make the notification message and send it to the tracker:
				BMessage message;
				message.AddInt32("RecentDocuments", fDisplayedDocCount);
				message.AddInt32("RecentFolders", fDisplayedFolderCount);
				tracker->SendNotices(kFavoriteCountChanged, &message);

				Window()->PostMessage(kSettingsContentsModified);
			}
		break;

		case B_OBSERVER_NOTICE_CHANGE:
			{
				int32 observerWhat;
				if (message->FindInt32("be:observe_change_what", &observerWhat) == B_OK) {
					switch (observerWhat) {
						case kFavoriteCountChangedExternally:
							{
								int32 count;
								if (message->FindInt32("RecentApplications", &count) == B_OK) {
									tracker->SetRecentApplicationsCount(count);
									ShowCurrentSettings();
								}

								if (message->FindInt32("RecentDocuments", &count) == B_OK) {
									tracker->SetRecentDocumentsCount(count);
									ShowCurrentSettings();
								}

								if (message->FindInt32("RecentFolders", &count) == B_OK) {
									tracker->SetRecentFoldersCount(count);
									ShowCurrentSettings();
								}
							}
							break;
					}
				}
			}
			break;

		default:
			_inherited::MessageReceived(message);
			break;
	}	
}


void
FilePanelSettingsView::SetDefaults()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;
	
	tracker->SetDesktopFilePanelRoot(true);
	tracker->SetRecentDocumentsCount(10);
	tracker->SetRecentFoldersCount(10);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
FilePanelSettingsView::Revert()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	tracker->SetDesktopFilePanelRoot(fDesktopFilePanelRoot);
	tracker->SetRecentDocumentsCount(fRecentDocuments);
	tracker->SetRecentFoldersCount(fRecentFolders);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
FilePanelSettingsView::ShowCurrentSettings(bool sendNotices)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	fDesktopFilePanelRootCheckBox->SetValue(tracker->DesktopFilePanelRoot());

	int32 recentApplications, recentDocuments, recentFolders;
	
	tracker->RecentCounts(&recentApplications, &recentDocuments, &recentFolders);

	BString docCountText;
	docCountText << recentDocuments;
	fRecentDocumentsTextControl->SetText(docCountText.String());

	BString folderCountText;
	folderCountText << recentFolders;
	fRecentFoldersTextControl->SetText(folderCountText.String());

	if (sendNotices) {
		// Make the notification message and send it to the tracker:

		BMessage message;
		message.AddBool("DesktopFilePanelRoot", fDesktopFilePanelRootCheckBox->Value() == 1);
		tracker->SendNotices(kDesktopFilePanelRootChanged, &message);
	
		message.AddInt32("RecentDocuments", recentDocuments);
		message.AddInt32("RecentFolders", recentFolders);
		tracker->SendNotices(kFavoriteCountChanged, &message);
	}
}

void
FilePanelSettingsView::RecordRevertSettings()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;
	
	fDesktopFilePanelRoot = tracker->DesktopFilePanelRoot();
	tracker->RecentCounts(&fRecentApplications, &fRecentDocuments, &fRecentFolders);
}

bool
FilePanelSettingsView::ShowsRevertSettings() const
{
	GetAndRefreshDisplayedFigures();

	return
		(fDesktopFilePanelRoot == (fDesktopFilePanelRootCheckBox->Value() > 0))
		&& (fDisplayedDocCount == fRecentDocuments)
		&& (fDisplayedFolderCount == fRecentFolders);
}

void
FilePanelSettingsView::GetAndRefreshDisplayedFigures() const
{
	sscanf(fRecentDocumentsTextControl->Text(), "%ld", &fDisplayedDocCount);
	sscanf(fRecentFoldersTextControl->Text(), "%ld", &fDisplayedFolderCount);
	
	BString docCountText;
	docCountText << fDisplayedDocCount;
	fRecentDocumentsTextControl->SetText(docCountText.String());

	BString folderCountText;
	folderCountText << fDisplayedFolderCount;
	fRecentFoldersTextControl->SetText(folderCountText.String());
}

//------------------------------------------------------------------------
// #pragma mark -

TimeFormatSettingsView::TimeFormatSettingsView(BRect rect)
	:	SettingsView(rect, "WindowsSettingsView")
{
	BRect clockBoxFrame = BRect(kBorderSpacing, kBorderSpacing,
		rect.Width() / 2 - 4 * kBorderSpacing, kBorderSpacing + 5 * kItemHeight);

	BBox *clockBox = new BBox(clockBoxFrame, "Clock");
	clockBox->SetLabel("Clock");
	
	AddChild(clockBox);
	
	BRect frame = BRect(kBorderSpacing, 2.5f*kBorderSpacing,
		clockBoxFrame.Width() - 2 * kBorderSpacing, kBorderSpacing + kItemHeight);

	f24HrRadioButton = new BRadioButton(frame, "", "24 Hour",
		new BMessage(kSettingsContentsModified));
	clockBox->AddChild(f24HrRadioButton);
	f24HrRadioButton->ResizeToPreferred();

	const float itemSpacing = f24HrRadioButton->Bounds().Height() + kItemExtraSpacing;

	frame.OffsetBy(0, itemSpacing);

	f12HrRadioButton = new BRadioButton(frame, "", "12 Hour",
		new BMessage(kSettingsContentsModified));
	clockBox->AddChild(f12HrRadioButton);
	f12HrRadioButton->ResizeToPreferred();

	clockBox->ResizeTo(clockBox->Bounds().Width(), f12HrRadioButton->Frame().bottom + kBorderSpacing);


	BRect dateFormatBoxFrame = BRect(clockBoxFrame.right + kBorderSpacing, kBorderSpacing,
		rect.right - kBorderSpacing, kBorderSpacing + 5 * itemSpacing);

	BBox *dateFormatBox = new BBox(dateFormatBoxFrame, "Date Order");
	dateFormatBox->SetLabel("Date Order");

	AddChild(dateFormatBox);
	
	frame = BRect(kBorderSpacing, 2.5f*kBorderSpacing,
		dateFormatBoxFrame.Width() - 2 * kBorderSpacing, kBorderSpacing + kItemHeight);

	fYMDRadioButton = new BRadioButton(frame, "", "Year-Month-Day",
		new BMessage(kSettingsContentsModified));
	dateFormatBox->AddChild(fYMDRadioButton);
	fYMDRadioButton->ResizeToPreferred();

	frame.OffsetBy(0, itemSpacing);

	fDMYRadioButton = new BRadioButton(frame, "", "Day-Month-Year",
		new BMessage(kSettingsContentsModified));
	dateFormatBox->AddChild(fDMYRadioButton);
	fDMYRadioButton->ResizeToPreferred();

	frame.OffsetBy(0, itemSpacing);

	fMDYRadioButton = new BRadioButton(frame, "", "Month-Day-Year",
		new BMessage(kSettingsContentsModified));
	dateFormatBox->AddChild(fMDYRadioButton);
	fMDYRadioButton->ResizeToPreferred();

	dateFormatBox->ResizeTo(dateFormatBox->Bounds().Width(), fMDYRadioButton->Frame().bottom + kBorderSpacing);

	BPopUpMenu *menu = new BPopUpMenu("Separator");
	
	menu->AddItem(new BMenuItem("None", new BMessage(kSettingsContentsModified)));
	menu->AddItem(new BMenuItem("Space", new BMessage(kSettingsContentsModified)));
	menu->AddItem(new BMenuItem("-", new BMessage(kSettingsContentsModified)));
	menu->AddItem(new BMenuItem("/", new BMessage(kSettingsContentsModified)));
	menu->AddItem(new BMenuItem("\\", new BMessage(kSettingsContentsModified)));
	menu->AddItem(new BMenuItem(".", new BMessage(kSettingsContentsModified)));

	frame = BRect(clockBox->Frame().left, dateFormatBox->Frame().bottom + kBorderSpacing,
		rect.right - kBorderSpacing, dateFormatBox->Frame().bottom + kBorderSpacing + itemSpacing);

	fSeparatorMenuField = new BMenuField(frame, "Separator", "Separator", menu);
	fSeparatorMenuField->ResizeToPreferred();
	AddChild(fSeparatorMenuField);

	frame.OffsetBy(0, 30.0f);
	
	BStringView *exampleView = new BStringView(frame, "", "Examples:");
	AddChild(exampleView);
	exampleView->ResizeToPreferred();
	
	frame.OffsetBy(0, itemSpacing);

	fLongDateExampleView = new BStringView(frame, "", "");
	AddChild(fLongDateExampleView);
	fLongDateExampleView->ResizeToPreferred();

	frame.OffsetBy(0, itemSpacing);

	fShortDateExampleView = new BStringView(frame, "", "");
	AddChild(fShortDateExampleView);
	fShortDateExampleView->ResizeToPreferred();

	UpdateExamples();
}

void
TimeFormatSettingsView::AttachedToWindow()
{
	f24HrRadioButton->SetTarget(this);
	f12HrRadioButton->SetTarget(this);
	fYMDRadioButton->SetTarget(this);
	fDMYRadioButton->SetTarget(this);
	fMDYRadioButton->SetTarget(this);
	
	fSeparatorMenuField->Menu()->SetTargetForItems(this);
}

void
TimeFormatSettingsView::MessageReceived(BMessage *message)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	switch (message->what) {
		case kSettingsContentsModified:
			{
				int32 separator = 0;
				BMenuItem *item = fSeparatorMenuField->Menu()->FindMarked();
				if (item) {
					separator = fSeparatorMenuField->Menu()->IndexOf(item);
					if (separator >= 0)
						tracker->SetTimeFormatSeparator((FormatSeparator)separator);
				}
			
				DateOrder format =
					fYMDRadioButton->Value() ? kYMDFormat :
					fDMYRadioButton->Value() ? kDMYFormat : kMDYFormat;
			
				tracker->SetDateOrderFormat(format);
				tracker->SetClockTo24Hr(f24HrRadioButton->Value() == 1);

				// Make the notification message and send it to the tracker:
				BMessage notificationMessage;
				notificationMessage.AddInt32("TimeFormatSeparator", separator);
				notificationMessage.AddInt32("DateOrderFormat", format);
				notificationMessage.AddBool("24HrClock", f24HrRadioButton->Value() == 1);
				tracker->SendNotices(kDateFormatChanged, &notificationMessage);

				UpdateExamples();

				Window()->PostMessage(kSettingsContentsModified);
				break;
			}
		
		default:
			_inherited::MessageReceived(message);
	}
}

void
TimeFormatSettingsView::SetDefaults()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;
	
	tracker->SetTimeFormatSeparator(kSlashSeparator);
	tracker->SetDateOrderFormat(kMDYFormat);
	tracker->SetClockTo24Hr(false);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
TimeFormatSettingsView::Revert()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	tracker->SetTimeFormatSeparator(fSeparator);
	tracker->SetDateOrderFormat(fFormat);
	tracker->SetClockTo24Hr(f24HrClock);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
TimeFormatSettingsView::ShowCurrentSettings(bool sendNotices)
{
	f24HrRadioButton->SetValue(TTracker::ClockIs24Hr());
	f12HrRadioButton->SetValue(!TTracker::ClockIs24Hr());
	
	switch (TTracker::DateOrderFormat()) {
		
		case kYMDFormat:
			fYMDRadioButton->SetValue(1);
			break;
		default:
	
		case kDMYFormat:
			fDMYRadioButton->SetValue(1);
			break;
	
		case kMDYFormat:
			fMDYRadioButton->SetValue(1);
			break;
	}
	
	FormatSeparator separator = TTracker::TimeFormatSeparator();
	
	if (separator >= kNoSeparator && separator < kSeparatorsEnd)
		fSeparatorMenuField->Menu()->ItemAt((int32)separator)->SetMarked(true);

	UpdateExamples();
	
	if (sendNotices) {
		TTracker *tracker = dynamic_cast<TTracker *>(be_app);
		if (!tracker)
			return;
	
		// Make the notification message and send it to the tracker:
		BMessage notificationMessage;
		notificationMessage.AddInt32("TimeFormatSeparator", (int32)tracker->TimeFormatSeparator());
		notificationMessage.AddInt32("DateOrderFormat", (int32)tracker->DateOrderFormat());
		notificationMessage.AddBool("24HrClock", tracker->ClockIs24Hr());
		tracker->SendNotices(kDateFormatChanged, &notificationMessage);
	}
}

void
TimeFormatSettingsView::RecordRevertSettings()
{
	f24HrClock = TTracker::ClockIs24Hr();
	fSeparator = TTracker::TimeFormatSeparator();
	fFormat = TTracker::DateOrderFormat();
}

bool
TimeFormatSettingsView::ShowsRevertSettings() const
{
	FormatSeparator separator;

	BMenuItem *item = fSeparatorMenuField->Menu()->FindMarked();
	if (item) {
		int32 index = fSeparatorMenuField->Menu()->IndexOf(item);
		if (index >= 0)
			separator = (FormatSeparator)index;
		else
			return false;
	} else
		return false;
		
	DateOrder format =
		fYMDRadioButton->Value() ? kYMDFormat :
		(fDMYRadioButton->Value() ? kDMYFormat : kMDYFormat);

	return
		f24HrClock == (f24HrRadioButton->Value() > 0)
		&& separator == fSeparator
		&& format == fFormat;
}

void
TimeFormatSettingsView::UpdateExamples()
{
	time_t timeValue = (time_t)time(NULL);
	tm timeData;
	localtime_r(&timeValue, &timeData);
	BString timeFormat = "Internal Error!";
	char buffer[256];

	FormatSeparator separator;

	BMenuItem *item = fSeparatorMenuField->Menu()->FindMarked();
	if (item) {
		int32 index = fSeparatorMenuField->Menu()->IndexOf(item);
		if (index >= 0)
			separator = (FormatSeparator)index;
		else
			separator = kSlashSeparator;
	} else
		separator = kSlashSeparator;
		
	DateOrder order =
		fYMDRadioButton->Value() ? kYMDFormat :
		(fDMYRadioButton->Value() ? kDMYFormat : kMDYFormat);

	bool clockIs24hr = (f24HrRadioButton->Value() > 0);

	TimeFormat(timeFormat, 0, separator, order, clockIs24hr);
	strftime(buffer, 256, timeFormat.String(), &timeData);

	fLongDateExampleView->SetText(buffer);
	fLongDateExampleView->ResizeToPreferred();

	TimeFormat(timeFormat, 4, separator, order, clockIs24hr);
	strftime(buffer, 256, timeFormat.String(), &timeData);
	
	fShortDateExampleView->SetText(buffer);
	fShortDateExampleView->ResizeToPreferred();
}

//------------------------------------------------------------------------
// #pragma mark -

SpaceBarSettingsView::SpaceBarSettingsView(BRect rect)
	:	SettingsView(rect, "SpaceBarSettingsView")
{
	BRect frame = BRect(kBorderSpacing, kBorderSpacing, rect.Width()
		- 2 * kBorderSpacing, kBorderSpacing + kItemHeight);

	fSpaceBarShowCheckBox = new BCheckBox(frame, "", "Show Space Bars On Volumes",
		new BMessage(kUpdateVolumeSpaceBar));
	AddChild(fSpaceBarShowCheckBox);
	fSpaceBarShowCheckBox->ResizeToPreferred();
	float itemSpacing = fSpaceBarShowCheckBox->Bounds().Height() + kItemExtraSpacing;
	frame.OffsetBy(0, itemSpacing);
	
	BPopUpMenu *menu = new BPopUpMenu(B_EMPTY_STRING);
	menu->SetFont(be_plain_font);

	BMenuItem *item;
	menu->AddItem(item = new BMenuItem("Used Space Color", new BMessage(kSpaceBarSwitchColor)));
	item->SetMarked(true);
	fCurrentColor = 0;
	menu->AddItem(new BMenuItem("Free Space Color", new BMessage(kSpaceBarSwitchColor)));
	menu->AddItem(new BMenuItem("Warning Space Color", new BMessage(kSpaceBarSwitchColor)));

	BBox *box = new BBox(frame);
	box->SetLabel(fColorPicker = new BMenuField(frame,NULL,NULL,menu));
	AddChild(box);

	fColorControl = new BColorControl(
			BPoint(8,fColorPicker->Bounds().Height() + 8 + kItemExtraSpacing),
			B_CELLS_16x16,1,"SpaceColorControl",new BMessage(kSpaceBarColorChanged));
	fColorControl->SetValue(TTracker::UsedSpaceColor());
	fColorControl->ResizeToPreferred();
	box->AddChild(fColorControl);
	box->ResizeTo(fColorControl->Bounds().Width() + 16,fColorControl->Frame().bottom + 8);
}

SpaceBarSettingsView::~SpaceBarSettingsView()
{
}

void
SpaceBarSettingsView::AttachedToWindow()
{
	fSpaceBarShowCheckBox->SetTarget(this);
	fColorControl->SetTarget(this);
	fColorPicker->Menu()->SetTargetForItems(this);
}

void
SpaceBarSettingsView::MessageReceived(BMessage *message)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;
		
	switch (message->what) {
		case kUpdateVolumeSpaceBar:
		{
			tracker->SetShowVolumeSpaceBar(fSpaceBarShowCheckBox->Value() == 1);
			Window()->PostMessage(kSettingsContentsModified);
			BMessage notificationMessage;
			notificationMessage.AddBool("ShowVolumeSpaceBar", tracker->ShowVolumeSpaceBar());
			tracker->SendNotices(kShowVolumeSpaceBar, &notificationMessage);
			break;
		}

		case kSpaceBarSwitchColor:
		{
			fCurrentColor = message->FindInt32("index");
			switch (fCurrentColor) {
				case 0:
					fColorControl->SetValue(TTracker::UsedSpaceColor());
					break;
				case 1:
					fColorControl->SetValue(TTracker::FreeSpaceColor());
					break;
				case 2:
					fColorControl->SetValue(TTracker::WarningSpaceColor());
					break;
			}
			break;
		}
		case kSpaceBarColorChanged:
		{
			switch (fCurrentColor) {
				case 0:
					tracker->SetUsedSpaceColor(fColorControl->ValueAsColor());
					break;
				case 1:
					tracker->SetFreeSpaceColor(fColorControl->ValueAsColor());
					break;
				case 2:
					tracker->SetWarningSpaceColor(fColorControl->ValueAsColor());
					break;
			}

			Window()->PostMessage(kSettingsContentsModified);
			BMessage notificationMessage;
			tracker->SendNotices(kSpaceBarColorChanged, &notificationMessage);
			break;
		}
	
		default:
			_inherited::MessageReceived(message);
			break;
	}	
}

void
SpaceBarSettingsView::SetDefaults()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	tracker->SetShowVolumeSpaceBar(false);

	tracker->SetUsedSpaceColor(Color(0,0xcb,0,192));
	tracker->SetFreeSpaceColor(Color(0xff,0xff,0xff,192));
	tracker->SetWarningSpaceColor(Color(0xcb,0,0,192));

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
SpaceBarSettingsView::Revert()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	tracker->SetShowVolumeSpaceBar(fSpaceBarShow);
	tracker->SetUsedSpaceColor(fUsedSpaceColor);
	tracker->SetFreeSpaceColor(fFreeSpaceColor);
	tracker->SetWarningSpaceColor(fWarningSpaceColor);
	
	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
SpaceBarSettingsView::ShowCurrentSettings(bool sendNotices)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	fSpaceBarShowCheckBox->SetValue(tracker->ShowVolumeSpaceBar());
	
	switch (fCurrentColor) {
		case 0:
			fColorControl->SetValue(tracker->UsedSpaceColor());
			break;
		case 1:
			fColorControl->SetValue(tracker->FreeSpaceColor());
			break;
		case 2:
			fColorControl->SetValue(tracker->WarningSpaceColor());
			break;
	}

	if (sendNotices) {
		BMessage notificationMessage;
		notificationMessage.AddBool("ShowVolumeSpaceBar", tracker->ShowVolumeSpaceBar());
		tracker->SendNotices(kShowVolumeSpaceBar, &notificationMessage);

		Window()->PostMessage(kSettingsContentsModified);
		BMessage notificationMessage2;
		tracker->SendNotices(kSpaceBarColorChanged, &notificationMessage2);
	}
}

void
SpaceBarSettingsView::RecordRevertSettings()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;
	
	fSpaceBarShow = tracker->ShowVolumeSpaceBar();
	fUsedSpaceColor = tracker->UsedSpaceColor();
	fFreeSpaceColor = tracker->FreeSpaceColor();
	fWarningSpaceColor = tracker->WarningSpaceColor();
}

bool
SpaceBarSettingsView::ShowsRevertSettings() const
{
	return (fSpaceBarShow == (fSpaceBarShowCheckBox->Value() == 1));
}


//------------------------------------------------------------------------
// #pragma mark -


TrashSettingsView::TrashSettingsView(BRect rect)
	:	SettingsView(rect, "TrashSettingsView")
{
	BRect frame = BRect(kBorderSpacing, kBorderSpacing, rect.Width()
			- 2 * kBorderSpacing, kBorderSpacing + kItemHeight);

	fDontMoveFilesToTrashCheckBox = new BCheckBox(frame, "", "Don't Move Files To Trash",
			new BMessage(kDontMoveFilesToTrashChanged));
	AddChild(fDontMoveFilesToTrashCheckBox);
	fDontMoveFilesToTrashCheckBox->ResizeToPreferred();

	frame.OffsetBy(0, fDontMoveFilesToTrashCheckBox->Bounds().Height() + kItemExtraSpacing);

	fAskBeforeDeleteFileCheckBox = new BCheckBox(frame, "", "Ask Before Delete",
			new BMessage(kAskBeforeDeleteFileChanged));
	AddChild(fAskBeforeDeleteFileCheckBox);
	fAskBeforeDeleteFileCheckBox->ResizeToPreferred();
}

void
TrashSettingsView::AttachedToWindow()
{
	fDontMoveFilesToTrashCheckBox->SetTarget(this);
	fAskBeforeDeleteFileCheckBox->SetTarget(this);
}

void
TrashSettingsView::MessageReceived(BMessage *message)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;
		
	switch (message->what) {
		case kDontMoveFilesToTrashChanged:
			tracker->SetDontMoveFilesToTrash(fDontMoveFilesToTrashCheckBox->Value() == 1);

			tracker->SendNotices(kDontMoveFilesToTrashChanged);
			Window()->PostMessage(kSettingsContentsModified);
			break;

		case kAskBeforeDeleteFileChanged:
			tracker->SetAskBeforeDeleteFile(fAskBeforeDeleteFileCheckBox->Value() == 1);

			tracker->SendNotices(kAskBeforeDeleteFileChanged);
			Window()->PostMessage(kSettingsContentsModified);
			break;

		default:
			_inherited::MessageReceived(message);
			break;
	}
}

	
void
TrashSettingsView::SetDefaults()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	tracker->SetDontMoveFilesToTrash(false);
	tracker->SetAskBeforeDeleteFile(true);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
TrashSettingsView::Revert()
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	tracker->SetDontMoveFilesToTrash(fDontMoveFilesToTrash);
	tracker->SetAskBeforeDeleteFile(fAskBeforeDeleteFile);

	ShowCurrentSettings(true);
		// true -> send notices about the change
}

void
TrashSettingsView::ShowCurrentSettings(bool sendNotices)
{
	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		return;

	fDontMoveFilesToTrashCheckBox->SetValue(tracker->DontMoveFilesToTrash());
	fAskBeforeDeleteFileCheckBox->SetValue(tracker->AskBeforeDeleteFile());

	if (sendNotices) {
		Window()->PostMessage(kSettingsContentsModified);
	}
}

void
TrashSettingsView::RecordRevertSettings()
{
	fDontMoveFilesToTrash = TTracker::DontMoveFilesToTrash();
	fAskBeforeDeleteFile = TTracker::AskBeforeDeleteFile();
}

bool
TrashSettingsView::ShowsRevertSettings() const
{
	return (fDontMoveFilesToTrash == (fDontMoveFilesToTrashCheckBox->Value() > 0))
			&& (fAskBeforeDeleteFile == (fAskBeforeDeleteFileCheckBox->Value() > 0));
}
