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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <Alert.h>
#include <Autolock.h>
#include <Debug.h>
#include <FindDirectory.h>
#include <fs_attr.h>
#include <fs_info.h>
#include <image.h>
#include <MenuItem.h>
#include <NodeInfo.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <Roster.h>
#include <StopWatch.h>
#include <Volume.h>
#include <VolumeRoster.h>

#include "Attributes.h"
#include "AutoLock.h"
#include "AutoMounter.h"
#include "AutoMounterSettings.h"
#include "BackgroundImage.h"
#include "Bitmaps.h"
#include "Commands.h"
#include "ContainerWindow.h"
#include "DeskWindow.h"
#include "FindPanel.h"
#include "FSUtils.h"
#include "InfoWindow.h"
#include "MimeTypes.h"
#include "MimeTypeList.h"
#include "NodePreloader.h"
#include "OpenWithWindow.h"
#include "PoseView.h"
#include "QueryContainerWindow.h"
#include "StatusWindow.h"
#include "Tracker.h"
#include "TrashWatcher.h"
#include "FunctionObject.h"
#include "TrackerSettings.h"
#include "TrackerSettingsWindow.h"
#include "TaskLoop.h"
#include "Thread.h"
#include "Utilities.h"
#include "VolumeWindow.h"

// PPC binary compatibility.
#include "AboutBox.cpp"

// prototypes for some private kernel calls that will some day be public
#if B_BEOS_VERSION_DANO
#define _IMPEXP_ROOT
#endif
extern "C" _IMPEXP_ROOT int _kset_fd_limit_(int num);
extern "C" _IMPEXP_ROOT int _kset_mon_limit_(int num);
#if B_BEOS_VERSION_DANO
#undef _IMPEXP_ROOT
#endif
	// from priv_syscalls.h

const int32 DEFAULT_MON_NUM = 4096;
	// copied from fsil.c

const int8 kOpenWindowNoFlags = 0;
const int8 kOpenWindowMinimized = 1;
const int8 kOpenWindowHasState = 2;

TTrackerState gTrackerState;


TTracker::TTracker()
	:	BApplication(kTrackerSignature),
		fSettingsWindow(NULL)
{
	// set the cwd to /boot/home, anything that's launched 
	// from Tracker will automatically inherit this 
	BPath homePath;
	
	if (find_directory(B_USER_DIRECTORY, &homePath) == B_OK)
		chdir(homePath.Path());
	
	_kset_fd_limit_(512);
		// ask for a bunch more file descriptors so that nested copying
		// works well
	
	fNodeMonitorCount = DEFAULT_MON_NUM;

	TTrackerState::InitIfNeeded();	
	gTrackerState.LoadSettingsIfNeeded();

	TrackerInitIconPreloader();

#ifdef LEAK_CHECKING
	SetNewLeakChecking(true);
	SetMallocLeakChecking(true);
#endif

	//This is how often it should update the free space bar on the volume icons
	SetPulseRate(1000000);
}

TTracker::~TTracker()
{
}

uint32
GetVolumeFlags(Model *model)
{
	fs_info info;
	if (model->IsVolume()) {
		// search for the correct volume
		int32 cookie = 0;
		dev_t device;
		while ((device = next_dev(&cookie)) >= B_OK) {
			if (fs_stat_dev(device,&info))
				continue;

			if (!strcmp(info.volume_name,model->Name()))
				return info.flags;
		}
		return B_FS_HAS_ATTR;
	}
	if (!fs_stat_dev(model->NodeRef()->device,&info))
		return info.flags;
	
	return B_FS_HAS_ATTR;
}

bool
TTracker::QuitRequested()
{
	// don't allow user quitting
	if (CurrentMessage() && CurrentMessage()->FindBool("shortcut"))
		return false;

	gStatusWindow->AttemptToQuit();
		// try quitting the copy/move/empty trash threads
		
	BVolume bootVolume;
	DEBUG_ONLY(status_t err =) BVolumeRoster().GetBootVolume(&bootVolume);
	ASSERT(err == B_OK);
	BMessage message;
	AutoLock<WindowList> lock(&fWindowList);
	// save open windows in a message inside an attribute of the desktop
	int32 count = fWindowList.CountItems();
	for (int32 i = 0; i < count; i++) {
		BContainerWindow *window = dynamic_cast<BContainerWindow *>
			(fWindowList.ItemAt(i));

		if (window && window->TargetModel() && !window->PoseView()->IsDesktopWindow()) {
			if (window->TargetModel()->IsRoot())
				message.AddBool("open_disks_window", true);
			else {
				BEntry entry;
				BPath path;
				const entry_ref *ref = window->TargetModel()->EntryRef();
				if (entry.SetTo(ref) == B_OK && entry.GetPath(&path) == B_OK) {
					int8 flags = window->IsMinimized() ? kOpenWindowMinimized : kOpenWindowNoFlags;
					uint32 deviceFlags = GetVolumeFlags(window->TargetModel());

					// save state for every window which is
					//	a) already open on another workspace
					//	b) on a volume not capable of writing attributes
					if (window != FindContainerWindow(ref)
						|| (deviceFlags & (B_FS_HAS_ATTR | B_FS_IS_READONLY)) != B_FS_HAS_ATTR) {
						BMessage stateMessage;
						window->SaveState(stateMessage);
						window->SetSaveStateEnabled(false);
							// This is to prevent its state to be saved to the node when closed.
						message.AddMessage("window state", &stateMessage);
						flags |= kOpenWindowHasState;
					}
					const char *target;
					bool pathAlreadyExists = false;
					for (int32 index = 0;message.FindString("paths", index, &target) == B_OK;index++) {
						if (!strcmp(target,path.Path())) {
							pathAlreadyExists = true;
							break;
						}
					}
					if (!pathAlreadyExists)
						message.AddString("paths", path.Path());
					message.AddInt8(path.Path(), flags);
				}
			}	
		}
	}
	lock.Unlock();

	// write windows to open on disk
	BDirectory deskDir;
	if (!BootedInSafeMode() && FSGetDeskDir(&deskDir, bootVolume.Device()) == B_OK) {
		// if message is empty, delete the corresponding attribute
		if (message.CountNames(B_ANY_TYPE)) {
			size_t size = (size_t)message.FlattenedSize();
			char *buffer = new char[size];
			message.Flatten(buffer, (ssize_t)size);
			deskDir.WriteAttr(kAttrOpenWindows, B_MESSAGE_TYPE, 0, buffer, size);
			delete [] buffer;
		} else
			deskDir.RemoveAttr(kAttrOpenWindows);
	}

	for (int32 count = 0; count == 50; count++) {
		// wait 5 seconds for the copiing/moving to quit
		if (gStatusWindow->AttemptToQuit())
			break;

		snooze(100000);
	}

	return _inherited::QuitRequested();
}

NodePreloader *gPreloader = NULL;

void
TTracker::Quit()
{
	gTrackerState.SaveSettings(false);
	
	fAutoMounter->Lock();
	fAutoMounter->QuitRequested();	// automounter does some stuff in QuitRequested
	fAutoMounter->Quit();			// but we really don't care if it is cooperating or not

	fTrashWatcher->Lock();
	fTrashWatcher->Quit();

	WellKnowEntryList::Quit();
	
	delete gPreloader;
	delete fTaskLoop;
	delete IconCache::iconCache;

	_inherited::Quit();
}

void
TTracker::MessageReceived(BMessage *message)
{
	if (HandleScriptingMessage(message))
		return;

	switch (message->what) {
		case kGetInfo:
			OpenInfoWindows(message);
			break;

		case kMoveToTrash:
			MoveRefsToTrash(message);
			break;

		case kCloseWindowAndChildren:
			{
				const node_ref *itemNode;
				int32 bytes;
				message->FindData("node_ref", B_RAW_TYPE,
					(const void **)&itemNode, &bytes);
				CloseWindowAndChildren(itemNode);
				break;
			}
			
		case kCloseAllWindows:
			CloseAllWindows();
			break;

		case kFindButton:
			(new FindWindow())->Show();
			break;

		case kEditQuery:
			EditQueries(message);
			break;

		case kUnmountVolume:
			//	When the user attempts to unmount a volume from the mount
			//	context menu, this is where the message gets received.  Save
			//	pose locations and forward this to the automounter
			SaveAllPoseLocations();
			fAutoMounter->PostMessage(message);
			break;

		case kRunAutomounterSettings:
			AutomountSettingsDialog::RunAutomountSettings(fAutoMounter);
			break;

		case kShowSplash:
			{
				// The AboutWindow was moved out of the Tracker in preparation
				// for when we open source it. The AboutBox contains important
				// credit and license issues that shouldn't be modified, and
				// therefore shouldn't be open sourced. However, there is a public
				// API for 3rd party apps to tell the Tracker to open the AboutBox.
				run_be_about();
				break;
			}

		case kAddPrinter:
			// show the addprinter window
			run_add_printer_panel();
			break;
			
		case kMakeActivePrinter:
			// get the current selection
			SetDefaultPrinter(message);
			break;

#ifdef MOUNT_MENU_IN_DESKBAR

		case 'gmtv':
			{
				// Someone (probably the deskbar) has requested a list of
				// mountable volumes.
				BMessage reply;
				AutoMounterLoop()->EachMountableItemAndFloppy(&AddMountableItemToMessage,
				  &reply);
				message->SendReply(&reply);
				break;
			}

#endif

		case kMountVolume:
		case kMountAllNow:
			AutoMounterLoop()->PostMessage(message);
			break;


		case kRestoreBackgroundImage:
			{
				BDeskWindow *desktop = GetDeskWindow();
				AutoLock<BWindow> lock(desktop);
				desktop->UpdateDesktopBackgroundImages();
			}
			break;

 		case kShowSettingsWindow:
 			ShowSettingsWindow();
 			break;

		case kFavoriteCountChangedExternally:
			SendNotices(kFavoriteCountChangedExternally, message);
			break;

		default:
			_inherited::MessageReceived(message);
			break;
	}
}

void
TTracker::Pulse()
{
	if (!ShowVolumeSpaceBar())
		return;

	// update the volume icon's free space bars
	BVolumeRoster roster;

 	BVolume volume;
	while(roster.GetNextVolume(&volume) == B_NO_ERROR)
	{
		BDirectory dir;
		volume.GetRootDirectory(&dir);
		node_ref nodeRef;
		dir.GetNodeRef(&nodeRef);

		BMessage notificationMessage;
		notificationMessage.AddInt32("device", *(int32 *)&nodeRef.device);
		SendNotices(kUpdateVolumeSpaceBar, &notificationMessage);
	}
}

const uint32 PSV_MAKE_PRINTER_ACTIVE_QUIETLY = 'pmaq';
	// from pr_server.h

void
TTracker::SetDefaultPrinter(const BMessage *message)
{
	//	get the first item selected
	int32 count = 0;
	uint32 type = 0;
	message->GetInfo("refs", &type, &count);

	if (count <= 0)
		return;

	//	will make the first item the default printer, disregards any other files
	entry_ref ref;
	ASSERT(message->FindRef("refs", 0, &ref) == B_OK);
	if (message->FindRef("refs", 0, &ref) != B_OK)
		return;

#if B_BEOS_VERSION_DANO
	set_default_printer(ref.name);
#else
	// 	create a message for the print server
	BMessenger messenger("application/x-vnd.Be-PSRV", -1);
	if (!messenger.IsValid())
		return;

	//	send the selection to the print server
	BMessage makeActiveMessage(PSV_MAKE_PRINTER_ACTIVE_QUIETLY);
	makeActiveMessage.AddString("printer", ref.name);

	BMessage reply;
	messenger.SendMessage(&makeActiveMessage, &reply);
#endif
}

void
TTracker::MoveRefsToTrash(const BMessage *message)
{
	int32 count;
	uint32 type;
	message->GetInfo("refs", &type, &count);

	if (count <= 0)
		return;

	BObjectList<entry_ref> *srcList = new BObjectList<entry_ref>(count, true);

	for (int32 index = 0; index < count; index++) {

		entry_ref ref;
		ASSERT(message->FindRef("refs", index, &ref) == B_OK);
		if (message->FindRef("refs", index, &ref) != B_OK)
			continue;

		AutoLock<WindowList> lock(&fWindowList);
		BContainerWindow *window = FindParentContainerWindow(&ref);
		if (window)
			// if we have a window open for this entry, ask the pose to
			// delete it, this will select the next entry
			window->PoseView()->MoveEntryToTrash(&ref);
		else
			// add all others to a list that gets deleted separately
			srcList->AddItem(new entry_ref(ref));
	}

	if (srcList->CountItems())
		// async move to trash
		FSMoveToTrash(srcList);
}

template <class T, class FT>
class EntryAndNodeDoSoonWithMessageFunctor : public FunctionObjectWithResult<bool> {
public:
	EntryAndNodeDoSoonWithMessageFunctor(FT func, T *target, const entry_ref *child,
		const node_ref *parent, const BMessage *message)
		:	fFunc(func),
			fTarget(target),
			fNode(*parent),
			fEntry(*child)
		{
			fSendMessage = (message != NULL);
			if (message)
				fMessage = *message;
		}

	virtual ~EntryAndNodeDoSoonWithMessageFunctor() {}
	virtual void operator()()
		{ result = (fTarget->*fFunc)(&fEntry, &fNode, fSendMessage ? &fMessage : NULL); }

protected:
	FT fFunc;
	T *fTarget;
	node_ref fNode;
	entry_ref fEntry;
	BMessage fMessage;
	bool fSendMessage;
};

bool 
TTracker::LaunchAndCloseParentIfOK(const entry_ref *launchThis,
	const node_ref *closeThis, const BMessage *messageToBundle)
{
	BMessage refsReceived(B_REFS_RECEIVED);
	if (messageToBundle) {
		refsReceived = *messageToBundle;
		refsReceived.what = B_REFS_RECEIVED;
	}
	refsReceived.AddRef("refs", launchThis);	
	// synchronous launch, we are already in our own thread
	if (TrackerLaunch(&refsReceived, false) == B_OK) {
		// if launched fine, close parent window in a bit
		fTaskLoop->RunLater(NewMemberFunctionObject(&TTracker::CloseParent, this, *closeThis),
			1000000);
	}
	return false;
}

status_t
TTracker::OpenRef(const entry_ref *ref, const node_ref *nodeToClose,
	const node_ref *nodeToSelect, OpenSelector selector,
	const BMessage *messageToBundle)
{
	Model *model = NULL;
	BEntry entry(ref, true);
	status_t result = entry.InitCheck();

	bool brokenLinkWithSpecificHandler = false;
	BString brokenLikPreferredApp;

	if (result != B_OK) {
		model = new Model(ref, false);
		if (model->IsSymLink() && !model->LinkTo()) {
			model->GetPreferredAppForBrokenSymLink(brokenLikPreferredApp);
			if (brokenLikPreferredApp.Length() && brokenLikPreferredApp != kTrackerSignature)
				brokenLinkWithSpecificHandler = true;
		}
		
		if (!brokenLinkWithSpecificHandler) {
			delete model;
			(new BAlert("", "There was an error resolving the link.",
				"Cancel", 0, 0,
				B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
			return result;
		}
	} else 
		model = new Model(&entry);

	result = model->InitCheck();
	if (result != B_OK) {
		delete model;
		return result;
	}

	bool openAsContainer = model->IsContainer();
	
	if (openAsContainer && selector != kOpenWith) {
		// if folder or query has a preferred handler and it's not the
		// Tracker, open it by sending refs to the handling app
		
		// if we are responding to the final open of OpenWith, just
		// skip this and proceed to opening the container with Tracker
		model->OpenNode();
		BNodeInfo nodeInfo(model->Node());
		char preferredApp[B_MIME_TYPE_LENGTH];
		if (nodeInfo.GetPreferredApp(preferredApp) == B_OK
			&& strcasecmp(preferredApp, kTrackerSignature) != 0)
			openAsContainer = false;
		model->CloseNode();
	}

	if (openAsContainer || selector == kRunOpenWithWindow) {
		// special case opening plain folders, queries or using open with
		OpenContainerWindow(model, 0, selector);	// window adopts model
		if (nodeToClose)
			CloseParentWaitingForChildSoon(ref, nodeToClose);
	} else if (model->IsQueryTemplate()) {
		// query template - open new find window
		(new FindWindow(model->EntryRef()))->Show();
		if (nodeToClose)
			CloseParentWaitingForChildSoon(ref, nodeToClose);
	} else {
		delete model;
		// run Launch in a separate thread
		// and close parent if successfull
		if (nodeToClose)
			Thread::Launch(new EntryAndNodeDoSoonWithMessageFunctor<TTracker,
				bool (TTracker::*)(const entry_ref *, const node_ref *,
				const BMessage *)>(&TTracker::LaunchAndCloseParentIfOK, this,
				ref, nodeToClose, messageToBundle));
		else {
			BMessage refsReceived(B_REFS_RECEIVED);
			if (messageToBundle) {
				refsReceived = *messageToBundle;
				refsReceived.what = B_REFS_RECEIVED;
			}
			refsReceived.AddRef("refs", ref);
			if (brokenLinkWithSpecificHandler)
				// This cruft is to support a hacky workaround for double-clicking
				// broken refs for cifs; should get fixed in R5
				LaunchBrokenLink(brokenLikPreferredApp.String(), &refsReceived);
			else
				TrackerLaunch(&refsReceived, true);
		}
	}
	if (nodeToSelect)
		SelectChildInParentSoon(ref, nodeToSelect);

	return B_OK;
}

void
TTracker::RefsReceived(BMessage *message)
{
	OpenSelector selector = kOpen;
	if (message->HasInt32("launchUsingSelector"))
		selector = kRunOpenWithWindow;
	
	entry_ref handlingApp;
	if (message->FindRef("handler", &handlingApp) == B_OK)
		selector = kOpenWith;

	int32 count;
	uint32 type;
	message->GetInfo("refs", &type, &count);
	
	switch (selector) {
		case kRunOpenWithWindow:
			OpenContainerWindow(0, message, selector);
				// window adopts model
			break;

		case kOpenWith:
			{
				// Open With resulted in passing refs and a handler, open the files
				// with the handling app
				message->RemoveName("handler");
				
				// have to find out if handling app is the Tracker
				// if it is, just pass it to the active Tracker, no matter which Tracker
				// was chosen to handle the refs
				char signature[B_MIME_TYPE_LENGTH];
				signature[0] = '\0';
				{
					BFile handlingNode(&handlingApp, O_RDONLY);
					BAppFileInfo appInfo(&handlingNode);
					appInfo.GetSignature(signature);
				}
	
				if (strcasecmp(signature, kTrackerSignature) != 0) {
					// handling app not Tracker, pass entries to the apps RefsReceived
					TrackerLaunch(&handlingApp, message, true);
					break;
				}
				// fall thru, opening refs by the Tracker, as if they were double clicked
			}

		case kOpen:
			{
				// copy over "Poses" messenger so that refs received recipients know
				// where the open came from
				BMessage *bundleThis = NULL;
				BMessenger messenger;
				if (message->FindMessenger("TrackerViewToken", &messenger) == B_OK) {
					bundleThis = new BMessage();
					bundleThis->AddMessenger("TrackerViewToken", messenger);
				}
	
				for (int32 index = 0; index < count; index++) {
					entry_ref ref;
					message->FindRef("refs", index, &ref);
			
					const node_ref *nodeToClose = NULL;
					const node_ref *nodeToSelect = NULL;
					ssize_t numBytes;
		
					message->FindData("nodeRefsToClose", B_RAW_TYPE, index,
						(const void **)&nodeToClose, &numBytes);
					message->FindData("nodeRefToSelect", B_RAW_TYPE, index,
						(const void **)&nodeToSelect, &numBytes);
			
					OpenRef(&ref, nodeToClose, nodeToSelect, selector, bundleThis);
				}
	
				delete bundleThis;
				break;
			}
	}
}

void
TTracker::ArgvReceived(int32 argc, char **argv)
{
	BMessage *message = CurrentMessage();
	const char *currentWorkingDirectoryPath = NULL;
	entry_ref ref;

	if (message->FindString("cwd", &currentWorkingDirectoryPath) == B_OK) {
		BDirectory workingDirectory(currentWorkingDirectoryPath);
		for (int32 index = 1; index < argc; index++) {
			BEntry entry;
			if (entry.SetTo(&workingDirectory, argv[index]) == B_OK
				&& entry.GetRef(&ref) == B_OK) 
				OpenRef(&ref);
			else if (get_ref_for_path(argv[index], &ref) == B_OK)
				OpenRef(&ref);
		}
	}
}

void
TTracker::OpenContainerWindow(Model *model, BMessage *originalRefsList,
	OpenSelector openSelector, uint32 openFlags, bool checkAlreadyOpen,
	const BMessage *stateMessage)
{
	AutoLock<WindowList> lock(&fWindowList);
	BContainerWindow *window = NULL;
	if (checkAlreadyOpen && openSelector != kRunOpenWithWindow)
		// find out if window already open
		window = FindContainerWindow(model->NodeRef());

	bool someWindowActivated = false;
	
	uint32 workspace = (uint32)(1 << current_workspace());		
	int32 windowCount = 0;
	
	while (window) {
		// At least one window open, just pull to front
		// make sure we don't jerk workspaces around		
		uint32 windowWorkspaces = window->Workspaces();
		if (windowWorkspaces & workspace) {
			window->Activate();
			someWindowActivated = true;
		}
		window = FindContainerWindow(model->NodeRef(), ++windowCount);
	}
	
	if (someWindowActivated) {
		delete model;
		return;	
	} // If no window was actiated, (none in the current workspace
	  // we open a new one.
	
	if (openSelector == kRunOpenWithWindow) {
		BMessage *refList = NULL;
		if (!originalRefsList) {
			// when passing just a single model, stuff it's entry in a single
			// element list anyway
			ASSERT(model);
			refList = new BMessage;
			refList->AddRef("refs", model->EntryRef());
			delete model;
			model = NULL;
		} else
			// clone the message, window adopts it for it's own use
			refList = new BMessage(*originalRefsList);
		window = new OpenWithContainerWindow(refList, &fWindowList);
	} else if (model->IsRoot()) {
		// window will adopt the model
		window = new BVolumeWindow(&fWindowList, openFlags);
	} else if (model->IsQuery()) {
		// window will adopt the model
		window = new BQueryContainerWindow(&fWindowList, openFlags);
	} else
		// window will adopt the model
		window = new BContainerWindow(&fWindowList, openFlags);
	
	if (model)
		window->CreatePoseView(model);

	BMessage restoreStateMessage(kRestoreState);
	
	if (stateMessage)
		restoreStateMessage.AddMessage("state", stateMessage);

	window->PostMessage(&restoreStateMessage);
}

void
TTracker::EditQueries(const BMessage *message)
{
	bool editOnlyIfTemplate;
	if (message->FindBool("editQueryOnPose", &editOnlyIfTemplate) != B_OK)
		editOnlyIfTemplate = false;

	type_code type;
	int32 count;
	message->GetInfo("refs", &type, &count);
	for (int32 index = 0; index < count; index++) {
		entry_ref ref;
		message->FindRef("refs", index, &ref);
		BEntry entry(&ref, true);
		if (entry.InitCheck() == B_OK && entry.Exists()) 
			(new FindWindow(&ref, editOnlyIfTemplate))->Show();

	}
}

void
TTracker::OpenInfoWindows(BMessage *message)
{
	type_code type;
	int32 count;
	message->GetInfo("refs", &type, &count);

	for (int32 index = 0; index < count; index++) {
		entry_ref ref;
		message->FindRef("refs", index, &ref);
		BEntry entry;
		if (entry.SetTo(&ref) == B_OK) {
			Model *model = new Model(&entry);
			if (model->InitCheck() != B_OK) {
				delete model;
				continue;
			}

			AutoLock<WindowList> lock(&fWindowList);
			BInfoWindow *wind = FindInfoWindow(model->NodeRef());

			if (wind) {
				wind->Activate();
				delete model;
			} else {
				wind = new BInfoWindow(model, index, &fWindowList);
				wind->PostMessage(kRestoreState);
			}

		}
	}
}

BDeskWindow *
TTracker::GetDeskWindow() const
{
	int32 count = fWindowList.CountItems();
	for (int32 index = 0; index < count; index++) {
		BDeskWindow *window = dynamic_cast<BDeskWindow *>
			(fWindowList.ItemAt(index));

		if (window)
			return window;
	}
	TRESPASS();
	return NULL;
}

BContainerWindow *
TTracker::FindContainerWindow(const node_ref *node, int32 number) const
{
	ASSERT(fWindowList.IsLocked());
	
	int32 count = fWindowList.CountItems();

	int32 windowsFound = 0;

	for (int32 index = 0; index < count; index++) {
		BContainerWindow *window = dynamic_cast<BContainerWindow *>
			(fWindowList.ItemAt(index));
			
		if (window && window->IsShowing(node) && number == windowsFound++)
			return window;
	}
	return NULL;
}

BContainerWindow *
TTracker::FindContainerWindow(const entry_ref *entry, int32 number) const
{
	ASSERT(fWindowList.IsLocked());
	
	int32 count = fWindowList.CountItems();

	int32 windowsFound = 0;

	for (int32 index = 0; index < count; index++) {
		BContainerWindow *window = dynamic_cast<BContainerWindow *>
			(fWindowList.ItemAt(index));

		if (window && window->IsShowing(entry) && number == windowsFound++)
			return window;
	}
	return NULL;
}

bool 
TTracker::EntryHasWindowOpen(const entry_ref *entry)
{
	AutoLock<WindowList> lock(&fWindowList);
	return FindContainerWindow(entry) != NULL;
}


BContainerWindow *
TTracker::FindParentContainerWindow(const entry_ref *ref) const
{
	BEntry entry(ref);
	BEntry parent;
	
	if (entry.GetParent(&parent) != B_OK)
		return NULL;
	
	entry_ref parentRef;
	parent.GetRef(&parentRef);

	ASSERT(fWindowList.IsLocked());
	
	int32 count = fWindowList.CountItems();
	for (int32 index = 0; index < count; index++) {
		BContainerWindow *window = dynamic_cast<BContainerWindow *>
			(fWindowList.ItemAt(index));
		if (window && window->IsShowing(&parentRef))
			return window;
	}
	return NULL;
}

BInfoWindow *
TTracker::FindInfoWindow(const node_ref* node) const
{
	ASSERT(fWindowList.IsLocked());
	
	int32 count = fWindowList.CountItems();
	for (int32 index = 0; index < count; index++) {
		BInfoWindow *window = dynamic_cast<BInfoWindow *>
			(fWindowList.ItemAt(index));
		if (window && window->IsShowing(node))
			return window;
	}
	return NULL;
}

bool 
TTracker::QueryActiveForDevice(dev_t device)
{
	AutoLock<WindowList> lock(&fWindowList);
	int32 count = fWindowList.CountItems();
	for (int32 index = 0; index < count; index++) {
		BQueryContainerWindow *window = dynamic_cast<BQueryContainerWindow *>
			(fWindowList.ItemAt(index));
		if (window) {
			AutoLock<BWindow> lock(window);
			if (window->ActiveOnDevice(device))
				return true;
		}
	}
	return false;
}

void 
TTracker::CloseActiveQueryWindows(dev_t device)
{
	// used when trying to unmount a volume - an active query would prevent that from
	// happening
	bool closed = false;
	AutoLock<WindowList> lock(fWindowList);
	for (int32 index = fWindowList.CountItems(); index >= 0; index--) {
		BQueryContainerWindow *window = dynamic_cast<BQueryContainerWindow *>
			(fWindowList.ItemAt(index));
		if (window) {
			AutoLock<BWindow> lock(window);
			if (window->ActiveOnDevice(device)) {
				window->PostMessage(B_CLOSE_REQUESTED);
				closed = true;
			}
		}
	}
	lock.Unlock();
	if (closed)
		for (int32 timeout = 30; timeout; timeout--) {
			// wait a bit for windows to fully close
			if (!QueryActiveForDevice(device))
				return;
			snooze(100000);
		}
}

void
TTracker::SaveAllPoseLocations()
{
	int32 numWindows = fWindowList.CountItems();
	for (int32 windowIndex = 0; windowIndex < numWindows; windowIndex++) {
		BContainerWindow *window = dynamic_cast<BContainerWindow *>
			(fWindowList.ItemAt(windowIndex));

		if (window) {
			AutoLock<BWindow> lock(window);
			BDeskWindow *deskWindow = dynamic_cast<BDeskWindow *>(window);
		
			if (deskWindow) 
				deskWindow->SaveDesktopPoseLocations();
			else
				window->PoseView()->SavePoseLocations();
		}
	}
}


void
TTracker::CloseWindowAndChildren(const node_ref *node)
{
	BDirectory dir(node);
	if (dir.InitCheck() != B_OK)
		return;

	AutoLock<WindowList> lock(&fWindowList);
	BObjectList<BContainerWindow> closeList;

	// make a list of all windows to be closed
	// count from end to beginning so we can remove items safely
	for (int32 index = fWindowList.CountItems() - 1; index >= 0; index--) {
		BContainerWindow *window = dynamic_cast<BContainerWindow *>
			(fWindowList.ItemAt(index));
		if (window && window->TargetModel()) {
			BEntry wind_entry;
			wind_entry.SetTo(window->TargetModel()->EntryRef());

			if ((*window->TargetModel()->NodeRef() == *node)
				|| dir.Contains(&wind_entry)) {
				
				// ToDo:
				// get rid of the Remove here, BContainerWindow::Quit does it
				fWindowList.RemoveItemAt(index);
				closeList.AddItem(window);
			}
		}
	}

	// now really close the windows
	int32 numItems = closeList.CountItems();
	for (int32 index = 0; index < numItems; index++) {
		BContainerWindow *window = closeList.ItemAt(index);
		window->PostMessage(B_CLOSE_REQUESTED);
	}
}

void
TTracker::CloseAllWindows()
{
	// this is a response to the DeskBar sending us a B_QUIT, when it really
	// means to say close all your windows. It might be better to have it
	// send a kCloseAllWindows message and have windowless apps stay running,
	// which is what we will do for the Tracker
	AutoLock<WindowList> lock(&fWindowList);

	int32 count = CountWindows();
	for (int32 index = 0; index < count; index++) {
		BWindow *window = WindowAt(index);
		// avoid the desktop
		if (!dynamic_cast<BDeskWindow *>(window)
			&& !dynamic_cast<BStatusWindow *>(window))
			window->PostMessage(B_CLOSE_REQUESTED);
	}
	// count from end to beginning so we can remove items safely
	for (int32 index = fWindowList.CountItems() - 1; index >= 0; index--) {
		BWindow *window = fWindowList.ItemAt(index);
		if (!dynamic_cast<BDeskWindow *>(window)
			&& !dynamic_cast<BStatusWindow *>(window))
				// ToDo:
				// get rid of the Remove here, BContainerWindow::Quit does it
			fWindowList.RemoveItemAt(index);
	}	
}


static void
HideVarDir()
{
	BPath path;
	status_t err = find_directory(B_COMMON_VAR_DIRECTORY, &path);
	
	if (err != B_OK){
		PRINT(("var err = %s\n", strerror(err)));
		return;
	}

	BDirectory varDirectory(path.Path());
	if (varDirectory.InitCheck() == B_OK) {
		PoseInfo info;
		// make var dir invisible
		info.fInvisible = true;
		info.fInitedDirectory = -1;
		
		if (varDirectory.WriteAttr(kAttrPoseInfo, B_RAW_TYPE, 0, &info, sizeof(info))
			== sizeof(info))
			varDirectory.RemoveAttr(kAttrPoseInfoForeign);
	}
}

void
TTracker::ReadyToRun()
{
	gStatusWindow = new BStatusWindow();
	InitMimeTypes();
	InstallDefaultTemplates();
	InstallIndices();
	
	HideVarDir();

	fTrashWatcher = new BTrashWatcher();
	fTrashWatcher->Run();

	fAutoMounter = new AutoMounter();
	fAutoMounter->Run();
	
	fTaskLoop = new StandAloneTaskLoop(true);

	bool openDisksWindow = false;

	// open desktop window 
	BContainerWindow *deskWindow = NULL;
	BVolume	bootVol;
	BVolumeRoster().GetBootVolume(&bootVol);
	BDirectory deskDir;
	if (FSGetDeskDir(&deskDir, bootVol.Device()) == B_OK) {
		// create desktop
		BEntry entry;
		deskDir.GetEntry(&entry);
		Model *model = new Model(&entry);
		if (model->InitCheck() == B_OK) {
			AutoLock<WindowList> lock(&fWindowList);
			deskWindow = new BDeskWindow(&fWindowList);
			AutoLock<BWindow> windowLock(deskWindow);
			deskWindow->CreatePoseView(model);
			deskWindow->Init();
		} else
			delete model;

		// open previously open windows
		attr_info attrInfo;
		if (!BootedInSafeMode()
			&& deskDir.GetAttrInfo(kAttrOpenWindows, &attrInfo) == B_OK) {
			char *buffer = (char *)malloc((size_t)attrInfo.size);
			BMessage message;
			if (deskDir.ReadAttr(kAttrOpenWindows, B_MESSAGE_TYPE, 0, buffer, (size_t)attrInfo.size)
				== attrInfo.size
				&& message.Unflatten(buffer) == B_OK) {

				node_ref nodeRef;
				deskDir.GetNodeRef(&nodeRef);
	
				int32 stateMessageCounter = 0;
				const char *path;
				for (int32 outer = 0;message.FindString("paths", outer, &path) == B_OK;outer++) {
					int8 flags = 0;
					for (int32 inner = 0;message.FindInt8(path, inner, &flags) == B_OK;inner++) {
						BEntry entry(path, true);
						if (entry.InitCheck() == B_OK) {
							Model *model = new Model(&entry);
							if (model->InitCheck() == B_OK && model->IsContainer()) {
								BMessage state;
								bool restoreStateFromMessage = false;
								if ((flags & kOpenWindowHasState) != 0
									&& message.FindMessage("window state", stateMessageCounter++, &state) == B_OK)
									restoreStateFromMessage = true;

								if (restoreStateFromMessage)
									OpenContainerWindow(model, 0, kOpen, 
										kRestoreWorkspace | (flags & kOpenWindowMinimized ? kIsHidden : 0U),
										false, &state);
								else
									OpenContainerWindow(model, 0, kOpen, 
										kRestoreWorkspace | (flags & kOpenWindowMinimized ? kIsHidden : 0U));
							} else
								delete model;
						}
					}
				}
	
				if (message.HasBool("open_disks_window"))
					openDisksWindow = true;
			}
			free(buffer);
		}
	}

	// create model for root of everything
	if (deskWindow) {
		BEntry entry("/");
		Model model(&entry);
		if (model.InitCheck() == B_OK) {

			if (ShowDisksIcon()) {
				// add the root icon to desktop window
				BMessage message;
				message.what = B_NODE_MONITOR;
				message.AddInt32("opcode", B_ENTRY_CREATED);
				message.AddInt32("device", model.NodeRef()->device);
				message.AddInt64("node", model.NodeRef()->node);
				message.AddInt64("directory", model.EntryRef()->directory);
				message.AddString("name", model.EntryRef()->name);
				deskWindow->PostMessage(&message, deskWindow->PoseView());
			}
			
			if (openDisksWindow)
				OpenContainerWindow(new Model(model), 0, kOpen, kRestoreWorkspace);
		}
	}

	// kick off building the mime type list for find panels, etc.
	fMimeTypeList = new MimeTypeList();

	if (!BootedInSafeMode())
		// kick of transient query killer
		DeleteTransientQueriesTask::StartUpTransientQueryCleaner();
}

MimeTypeList *
TTracker::MimeTypes() const
{
	return fMimeTypeList;
}	

void 
TTracker::SelectChildInParentSoon(const entry_ref *parent,
	const node_ref *child)
{
	fTaskLoop->RunLater(NewMemberFunctionObjectWithResult
		(&TTracker::SelectChildInParent, this, parent, child),
		100000, 200000, 5000000);
}

void 
TTracker::CloseParentWaitingForChildSoon(const entry_ref *child,
	const node_ref *parent)
{
	fTaskLoop->RunLater(NewMemberFunctionObjectWithResult
		(&TTracker::CloseParentWaitingForChild, this, child, parent),
		200000, 100000, 5000000);
}

void 
TTracker::SelectPoseAtLocationSoon(node_ref parent, BPoint pointInPose)
{
	fTaskLoop->RunLater(NewMemberFunctionObject
		(&TTracker::SelectPoseAtLocationInParent, this, parent, pointInPose),
		100000);
}

void 
TTracker::SelectPoseAtLocationInParent(node_ref parent, BPoint pointInPose)
{
	AutoLock<WindowList> lock(&fWindowList);
	BContainerWindow *parentWindow = FindContainerWindow(&parent);
	if (parentWindow) {
		AutoLock<BWindow> lock(parentWindow);
		parentWindow->PoseView()->SelectPoseAtLocation(pointInPose);
	}
}

bool 
TTracker::CloseParentWaitingForChild(const entry_ref *child,
	const node_ref *parent)
{
	AutoLock<WindowList> lock(&fWindowList);
	
	BContainerWindow *parentWindow = FindContainerWindow(parent);
	if (!parentWindow)
		// parent window already closed, give up
		return true;

	// If child is a symbolic link, dereference it, so that
	// FindContainerWindow will succeed.
	BEntry entry(child, true);
	entry_ref resolvedChild;
	if (entry.GetRef(&resolvedChild) != B_OK)
		resolvedChild = *child;

	BContainerWindow *window = FindContainerWindow(&resolvedChild);
	if (window) {
		AutoLock<BWindow> lock(window);
		if (!window->IsHidden())
			return CloseParentWindowCommon(parentWindow);
	}
	return false;	
}

void 
TTracker::CloseParent(node_ref parent)
{
	AutoLock<WindowList> lock(&fWindowList);
	if (!lock)
		return;

	CloseParentWindowCommon(FindContainerWindow(&parent));
}

void
TTracker::ShowSettingsWindow()
{
	if (!fSettingsWindow) {
		fSettingsWindow = new TrackerSettingsWindow();
		fSettingsWindow->Show();
	} else {
		if (fSettingsWindow->Lock()) {
			if (fSettingsWindow->IsHidden())
				fSettingsWindow->Show();
			else
				fSettingsWindow->Activate();
			fSettingsWindow->Unlock();
		}
	}
}

bool 
TTracker::CloseParentWindowCommon(BContainerWindow *window)
{
	ASSERT(fWindowList.IsLocked());
	
	if (dynamic_cast<BDeskWindow *>(window))
		// don't close the destop
		return false;

	window->PostMessage(B_CLOSE_REQUESTED);
	return true;
}

bool 
TTracker::SelectChildInParent(const entry_ref *parent, const node_ref *child)
{
	AutoLock<WindowList> lock(&fWindowList);
	
	BContainerWindow *window = FindContainerWindow(parent);
	if (!window) 
		// parent window already closed, give up
		return false;

	AutoLock<BWindow> windowLock(window);
	
	if (windowLock.IsLocked()) {
		BPoseView *view = window->PoseView();
		int32 index;
		BPose *pose = view->FindPose(child, &index);
		if (pose) {
			view->SelectPose(pose, index);
			return true;
		}
	}
	return false;	
}

const int32 kNodeMonitorBumpValue = 512;

status_t 
TTracker::NeedMoreNodeMonitors()
{
	fNodeMonitorCount += kNodeMonitorBumpValue;
	PRINT(("bumping nodeMonitorCount to %d\n", fNodeMonitorCount));

	return _kset_mon_limit_(fNodeMonitorCount);
}

status_t
TTracker::WatchNode(const node_ref *node, uint32 flags,
	BMessenger target)
{
	status_t result = watch_node(node, flags, target);
	
	if (result == B_OK || result != ENOMEM)
		// need to make sure this uses the same error value as
		// the node monitor code
		return result;
	
	PRINT(("failed to start monitoring, trying to allocate more "
		"node monitors\n"));

	TTracker *tracker = dynamic_cast<TTracker *>(be_app);
	if (!tracker)
		// we are the file panel only, just fail
		return result;

	result = tracker->NeedMoreNodeMonitors();
	
	if (result != B_OK) {
		PRINT(("failed to allocate more node monitors, %s\n",
			strerror(result)));
		return result;
	}

	// try again, this time with more node monitors
	return watch_node(node, flags, target);
}

bool 
TTracker::ShowDisksIcon()
{
	return gTrackerState.ShowDisksIcon();
}

void
TTracker::SetShowDisksIcon(bool enabled)
{
	gTrackerState.SetShowDisksIcon(enabled);
}


bool 
TTracker::MountVolumesOntoDesktop()
{
	return gTrackerState.MountVolumesOntoDesktop();
}

void
TTracker::SetMountVolumesOntoDesktop(bool enabled)
{
	gTrackerState.SetMountVolumesOntoDesktop(enabled);
}

bool 
TTracker::MountSharedVolumesOntoDesktop()
{
	return gTrackerState.MountSharedVolumesOntoDesktop();
}

void
TTracker::SetMountSharedVolumesOntoDesktop(bool enabled)
{
	gTrackerState.SetMountSharedVolumesOntoDesktop(enabled);
}

bool 
TTracker::IntegrateNonBootBeOSDesktops()
{
	return gTrackerState.IntegrateNonBootBeOSDesktops();
}

void
TTracker::SetIntegrateNonBootBeOSDesktops(bool enabled)
{
	gTrackerState.SetIntegrateNonBootBeOSDesktops(enabled);
}

bool 
TTracker::IntegrateAllNonBootDesktops()
{
	return gTrackerState.IntegrateAllNonBootDesktops();
}

bool 
TTracker::DesktopFilePanelRoot()
{
	return gTrackerState.DesktopFilePanelRoot();
}

void
TTracker::SetDesktopFilePanelRoot(bool enabled)
{
	gTrackerState.SetDesktopFilePanelRoot(enabled);
}

bool 
TTracker::ShowVolumeSpaceBar()
{
	return gTrackerState.ShowVolumeSpaceBar();
}

void
TTracker::SetShowVolumeSpaceBar(bool enabled)
{
	gTrackerState.SetShowVolumeSpaceBar(enabled);
}

rgb_color 
TTracker::UsedSpaceColor()
{
	return gTrackerState.UsedSpaceColor();
}

void
TTracker::SetUsedSpaceColor(rgb_color color)
{
	gTrackerState.SetUsedSpaceColor(color);
}

rgb_color 
TTracker::FreeSpaceColor()
{
	return gTrackerState.FreeSpaceColor();
}

void
TTracker::SetFreeSpaceColor(rgb_color color)
{
	gTrackerState.SetFreeSpaceColor(color);
}

rgb_color 
TTracker::WarningSpaceColor()
{
	return gTrackerState.WarningSpaceColor();
}

void
TTracker::SetWarningSpaceColor(rgb_color color)
{
	gTrackerState.SetWarningSpaceColor(color);
}

bool
TTracker::ShowFullPathInTitleBar()
{
	return gTrackerState.ShowFullPathInTitleBar();
}

void
TTracker::SetShowFullPathInTitleBar(bool enabled)
{
	gTrackerState.SetShowFullPathInTitleBar(enabled);
}

bool
TTracker::SortFolderNamesFirst()
{
	return gTrackerState.SortFolderNamesFirst();
}

void
TTracker::SetSortFolderNamesFirst(bool enabled)
{
	gTrackerState.SetSortFolderNamesFirst(enabled);
}

bool
TTracker::ShowSelectionWhenInactive()
{
	return gTrackerState.ShowSelectionWhenInactive();
}

void
TTracker::SetShowSelectionWhenInactive(bool enabled)
{
	gTrackerState.SetShowSelectionWhenInactive(enabled);

}


bool
TTracker::SingleWindowBrowse()
{
	return gTrackerState.SingleWindowBrowse();
}

void
TTracker::SetSingleWindowBrowse(bool enabled)
{
	gTrackerState.SetSingleWindowBrowse(enabled);
}

bool
TTracker::ShowNavigator()
{
	return gTrackerState.ShowNavigator();
}

void
TTracker::SetShowNavigator(bool enabled)
{
	gTrackerState.SetShowNavigator(enabled);
}


void
TTracker::RecentCounts(int32 *applications, int32 *documents, int32 *folders)
{
	gTrackerState.RecentCounts(applications, documents, folders);
}

void
TTracker::SetRecentApplicationsCount(int32 count)
{
	gTrackerState.SetRecentApplicationsCount(count);
}

void
TTracker::SetRecentDocumentsCount(int32 count)
{
	gTrackerState.SetRecentDocumentsCount(count);
}

void
TTracker::SetRecentFoldersCount(int32 count)
{
	gTrackerState.SetRecentFoldersCount(count);
}

FormatSeparator
TTracker::TimeFormatSeparator()
{
	return gTrackerState.TimeFormatSeparator();
}

void
TTracker::SetTimeFormatSeparator(FormatSeparator separator)
{
	gTrackerState.SetTimeFormatSeparator(separator);
}

DateOrder
TTracker::DateOrderFormat()
{
	return gTrackerState.DateOrderFormat();
}

void
TTracker::SetDateOrderFormat(DateOrder order)
{
	gTrackerState.SetDateOrderFormat(order);

	TTracker *tracker = dynamic_cast<TTracker*>(be_app);
	if (!tracker)
		return;
}

bool
TTracker::ClockIs24Hr()
{
	return gTrackerState.ClockIs24Hr();
}

void
TTracker::SetClockTo24Hr(bool enabled)
{
	gTrackerState.SetClockTo24Hr(enabled);

	TTracker *tracker = dynamic_cast<TTracker*>(be_app);
	if (!tracker)
		return;
}

bool
TTracker::DontMoveFilesToTrash()
{
	return gTrackerState.DontMoveFilesToTrash();
}

void
TTracker::SetDontMoveFilesToTrash(bool enabled)
{
	gTrackerState.SetDontMoveFilesToTrash(enabled);
}

bool
TTracker::AskBeforeDeleteFile()
{
	return gTrackerState.AskBeforeDeleteFile();
}

void
TTracker::SetAskBeforeDeleteFile(bool enabled)
{
	gTrackerState.SetAskBeforeDeleteFile(enabled);
}

void
TTracker::SaveSettings(bool onlyIfNonDefault)
{
	gTrackerState.SaveSettings(onlyIfNonDefault);
}


AutoMounter *
TTracker::AutoMounterLoop()
{
	return fAutoMounter;
}

bool 
TTracker::InTrashNode(const entry_ref *node) const
{
	return FSInTrashDir(node);
}

bool
TTracker::TrashFull() const
{
	return fTrashWatcher->CheckTrashDirs();
}
	
bool
TTracker::IsTrashNode(const node_ref *node) const
{
	return fTrashWatcher->IsTrashNode(node);
}	


// #pragma mark -

TTrackerState::TTrackerState()
	:	Settings("TrackerSettings", "Tracker"),
		fInited(false),
		fSettingsLoaded(false)
{
}

TTrackerState::TTrackerState(const TTrackerState&)
	:	Settings("", "")
{
	// Placeholder copy constructor to prevent others from accidentally using the
	// default copy constructor.  Note, the DEBUGGER call is for the off chance that
	// a TTrackerState method (or friend) tries to make a copy.
	DEBUGGER("Don't make a copy of this!");
}


void
TTrackerState::InitIfNeeded()
{
	AutoLock<Benaphore> lock(gTrackerState.fInitLock);
	
	if (gTrackerState.fInited)
		return;

#ifdef CHECK_OPEN_MODEL_LEAKS
	InitOpenModelDumping();
#endif

	IconCache::iconCache = new IconCache();
	gTrackerState.fInited = true;
}

namespace BPrivate {

void 
TrackerInitIconPreloader()
{
	TTrackerState::InitIfNeeded();
	
	if (gPreloader)
		return;

	gPreloader = NodePreloader::InstallNodePreloader("NodePreloader", be_app);
}

}

TTrackerState::~TTrackerState()
{
	if (fInited) 
		fInited = false;
}

void 
TTrackerState::SaveSettings(bool onlyIfNonDefault)
{
	if (fSettingsLoaded)
		_inherited::SaveSettings(onlyIfNonDefault);
}

bool 
TTrackerState::ShowDisksIcon()
{
	LoadSettingsIfNeeded();
	return fShowDisksIcon->Value();
}

void
TTrackerState::SetShowDisksIcon(bool enabled)
{
	fShowDisksIcon->SetValue(enabled);
}

bool 
TTrackerState::DesktopFilePanelRoot()
{
	LoadSettingsIfNeeded();
	return fDesktopFilePanelRoot->Value();
}

void 
TTrackerState::SetDesktopFilePanelRoot(bool enabled)
{
	fDesktopFilePanelRoot->SetValue(enabled);
}

bool 
TTrackerState::MountVolumesOntoDesktop()
{
	LoadSettingsIfNeeded();
	return fMountVolumesOntoDesktop->Value();
}

void 
TTrackerState::SetMountVolumesOntoDesktop(bool enabled)
{
	fMountVolumesOntoDesktop->SetValue(enabled);
}

bool
TTrackerState::MountSharedVolumesOntoDesktop()
{
	LoadSettingsIfNeeded();
	return fMountSharedVolumesOntoDesktop->Value();
}

void 
TTrackerState::SetMountSharedVolumesOntoDesktop(bool enabled)
{
	fMountSharedVolumesOntoDesktop->SetValue(enabled);
}

bool 
TTrackerState::IntegrateNonBootBeOSDesktops()
{
	LoadSettingsIfNeeded();
	return fIntegrateNonBootBeOSDesktops->Value();
}

void 
TTrackerState::SetIntegrateNonBootBeOSDesktops(bool enabled)
{
	fIntegrateNonBootBeOSDesktops->SetValue(enabled);
}

bool 
TTrackerState::IntegrateAllNonBootDesktops()
{
	LoadSettingsIfNeeded();
	return fIntegrateAllNonBootDesktops->Value();
}

bool 
TTrackerState::ShowVolumeSpaceBar()
{
	LoadSettingsIfNeeded();
	return fShowVolumeSpaceBar->Value();
}

void
TTrackerState::SetShowVolumeSpaceBar(bool enabled)
{
	fShowVolumeSpaceBar->SetValue(enabled);
}

rgb_color ValueToColor(int32 value)
{
	rgb_color color;
	color.alpha = static_cast<uchar>((value >> 24L) & 0xff);
	color.red = static_cast<uchar>((value >> 16L) & 0xff);
	color.green = static_cast<uchar>((value >> 8L) & 0xff);
	color.blue = static_cast<uchar>(value & 0xff);

	// zero alpha is invalid
	if (color.alpha == 0)
		color.alpha = 192;

	return color;	
}

int32 ColorToValue(rgb_color color)
{
	// zero alpha is invalid
	if (color.alpha == 0)
		color.alpha = 192;

	return	color.alpha << 24L
			| color.red << 16L
			| color.green << 8L
			| color.blue;
}

rgb_color
TTrackerState::UsedSpaceColor()
{
	LoadSettingsIfNeeded();
	return ValueToColor(fUsedSpaceColor->Value());
}

void
TTrackerState::SetUsedSpaceColor(rgb_color color)
{
	if (color.alpha == 0)
		color.alpha = 192;
	fUsedSpaceColor->ValueChanged(ColorToValue(color));
}

rgb_color
TTrackerState::FreeSpaceColor()
{
	LoadSettingsIfNeeded();
	return ValueToColor(fFreeSpaceColor->Value());
}

void
TTrackerState::SetFreeSpaceColor(rgb_color color)
{
	if (color.alpha == 0)
		color.alpha = 192;
	fFreeSpaceColor->ValueChanged(ColorToValue(color));
}

rgb_color
TTrackerState::WarningSpaceColor()
{
	LoadSettingsIfNeeded();
	return ValueToColor(fWarningSpaceColor->Value());
}

void
TTrackerState::SetWarningSpaceColor(rgb_color color)
{
	if (color.alpha == 0)
		color.alpha = 192;
	fWarningSpaceColor->ValueChanged(ColorToValue(color));
}

bool 
TTrackerState::ShowFullPathInTitleBar()
{
	LoadSettingsIfNeeded();
	return fShowFullPathInTitleBar->Value();
}

void
TTrackerState::SetShowFullPathInTitleBar(bool enabled)
{
	fShowFullPathInTitleBar->SetValue(enabled);
}

bool 
TTrackerState::ShowSelectionWhenInactive()
{
	LoadSettingsIfNeeded();
	return fShowSelectionWhenInactive->Value();
}

void
TTrackerState::SetShowSelectionWhenInactive(bool enabled)
{
	fShowSelectionWhenInactive->SetValue(enabled);
}

bool
TTrackerState::SortFolderNamesFirst()
{
	LoadSettingsIfNeeded();
	return fSortFolderNamesFirst->Value();
}

void
TTrackerState::SetSortFolderNamesFirst(bool enabled)
{
	fSortFolderNamesFirst->SetValue(enabled);
	NameAttributeText::SetSortFolderNamesFirst(enabled);
}

bool
TTrackerState::SingleWindowBrowse()
{
	LoadSettingsIfNeeded();
	return fSingleWindowBrowse->Value();
}

void
TTrackerState::SetSingleWindowBrowse(bool enabled)
{
	fSingleWindowBrowse->SetValue(enabled);
}

bool
TTrackerState::ShowNavigator()
{
	LoadSettingsIfNeeded();
	return fShowNavigator->Value();
}

void
TTrackerState::SetShowNavigator(bool enabled)
{
	fShowNavigator->SetValue(enabled);
}

void 
TTrackerState::RecentCounts(int32 *applications, int32 *documents, int32 *folders)
{
	LoadSettingsIfNeeded();
	*applications = fRecentApplicationsCount->Value();
	*documents = fRecentDocumentsCount->Value();
	*folders = fRecentFoldersCount->Value();
}

void  
TTrackerState::SetRecentApplicationsCount(int32 count)
{
	LoadSettingsIfNeeded();
	fRecentApplicationsCount->ValueChanged(count);
}

void  
TTrackerState::SetRecentDocumentsCount(int32 count)
{
	LoadSettingsIfNeeded();
	fRecentDocumentsCount->ValueChanged(count);
}

void  
TTrackerState::SetRecentFoldersCount(int32 count)
{
	LoadSettingsIfNeeded();
	fRecentFoldersCount->ValueChanged(count);
}

FormatSeparator
TTrackerState::TimeFormatSeparator()
{
	LoadSettingsIfNeeded();
	return (FormatSeparator)fTimeFormatSeparator->Value();
}

void
TTrackerState::SetTimeFormatSeparator(FormatSeparator separator)
{
	fTimeFormatSeparator->ValueChanged((int32)separator);
}

DateOrder
TTrackerState::DateOrderFormat()
{
	LoadSettingsIfNeeded();
	return (DateOrder)fDateOrderFormat->Value();
}

void
TTrackerState::SetDateOrderFormat(DateOrder order)
{
	fDateOrderFormat->ValueChanged((int32)order);
}

bool
TTrackerState::ClockIs24Hr()
{
	LoadSettingsIfNeeded();
	return f24HrClock->Value();
}

void
TTrackerState::SetClockTo24Hr(bool enabled)
{
	f24HrClock->SetValue(enabled);
}

bool 
TTrackerState::DontMoveFilesToTrash()
{
	LoadSettingsIfNeeded();
	return fDontMoveFilesToTrash->Value();
}

void
TTrackerState::SetDontMoveFilesToTrash(bool enabled)
{
	fDontMoveFilesToTrash->SetValue(enabled);
}

bool 
TTrackerState::AskBeforeDeleteFile()
{
	LoadSettingsIfNeeded();
	return fAskBeforeDeleteFile->Value();
}

void
TTrackerState::SetAskBeforeDeleteFile(bool enabled)
{
	fAskBeforeDeleteFile->SetValue(enabled);
}


void 
TTrackerState::LoadSettingsIfNeeded()
{
	if (fSettingsLoaded)
		return;

	Add(fShowDisksIcon = new BooleanValueSetting("ShowDisksIcon", false));
	Add(fMountVolumesOntoDesktop = new BooleanValueSetting("MountVolumesOntoDesktop", true));
	Add(fMountSharedVolumesOntoDesktop =
		new BooleanValueSetting("MountSharedVolumesOntoDesktop", false));
	Add(fIntegrateNonBootBeOSDesktops = new BooleanValueSetting
		("IntegrateNonBootBeOSDesktops", true));
	Add(fIntegrateAllNonBootDesktops = new BooleanValueSetting
		("IntegrateAllNonBootDesktops", false));
	Add(fDesktopFilePanelRoot = new BooleanValueSetting("DesktopFilePanelRoot", true));
	Add(fShowFullPathInTitleBar = new BooleanValueSetting("ShowFullPathInTitleBar", false));
	Add(fShowSelectionWhenInactive = new BooleanValueSetting("ShowSelectionWhenInactive", true));
	Add(fSortFolderNamesFirst = new BooleanValueSetting("SortFolderNamesFirst", false));
 	Add(fSingleWindowBrowse = new BooleanValueSetting("SingleWindowBrowse", false));
	Add(fShowNavigator = new BooleanValueSetting("ShowNavigator", false));
	
	Add(fRecentApplicationsCount = new ScalarValueSetting("RecentApplications", 10, "", ""));
	Add(fRecentDocumentsCount = new ScalarValueSetting("RecentDocuments", 10, "", ""));
	Add(fRecentFoldersCount = new ScalarValueSetting("RecentFolders", 10, "", ""));

	Add(fTimeFormatSeparator = new ScalarValueSetting("TimeFormatSeparator", 3, "", ""));
	Add(fDateOrderFormat = new ScalarValueSetting("DateOrderFormat", 2, "", ""));
	Add(f24HrClock = new BooleanValueSetting("24HrClock", false));

	Add(fShowVolumeSpaceBar = new BooleanValueSetting("ShowVolumeSpaceBar", false));

	Add(fUsedSpaceColor = new HexScalarValueSetting("UsedSpaceColor", 0xc000cb00, "", ""));
	Add(fFreeSpaceColor = new HexScalarValueSetting("FreeSpaceColor", 0xc0ffffff, "", ""));
	Add(fWarningSpaceColor = new HexScalarValueSetting("WarningSpaceColor", 0xc0cb0000, "", ""));

	Add(fDontMoveFilesToTrash = new BooleanValueSetting("DontMoveFilesToTrash", false));
	Add(fAskBeforeDeleteFile = new BooleanValueSetting("AskBeforeDeleteFile", true));

	TryReadingSettings();

	NameAttributeText::SetSortFolderNamesFirst(fSortFolderNamesFirst->Value());

	fSettingsLoaded = true;
}

namespace BPrivate {

BStatusWindow *gStatusWindow = NULL;

}
