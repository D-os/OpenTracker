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

#include <Application.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>
#include <Query.h>
#include <Roster.h>

#include <functional>
#include <algorithm>

#include "EntryIterator.h"
#include "FavoritesMenu.h"
#include "IconMenuItem.h"
#include "NavMenu.h"
#include "PoseView.h"
#include "QueryPoseView.h"
#include "Tracker.h"
#include "Utilities.h"


FavoritesMenu::FavoritesMenu(const char *title, BMessage *openFolderMessage,
	BMessage *openFileMessage, const BMessenger &target,
	bool isSavePanel)
	:	BSlowMenu(title),
		fOpenFolderMessage(openFolderMessage),
		fOpenFileMessage(openFileMessage),
		fTarget(target),
		fContainer(NULL),
		fInitialItemCount(0),
		fIsSavePanel(isSavePanel)
{
}

FavoritesMenu::~FavoritesMenu()
{
	delete fOpenFolderMessage;
	delete fOpenFileMessage;
	delete fContainer;
}

bool 
FavoritesMenu::StartBuildingItemList()
{
	// initialize the menu building state
	
	if (!fInitialItemCount)
		fInitialItemCount = CountItems();
	else {
		// strip the old items so we can add new fresh ones
		int32 count = CountItems() - fInitialItemCount;
		// keep the items that were added by the FavoritesMenu creator
		while (count--) 
			delete RemoveItem(fInitialItemCount);
	}

	fUniqueRefCheck.clear();
	fState = kStart;
	return true;
}

bool 
FavoritesMenu::AddNextItem()
{
	// run the next chunk of code for a given item adding state
	
	if (fState == kStart) {
		fState = kAddingFavorites;
		fSectionItemCount = 0;
		fAddedSeparatorForSection = false;
		// set up adding the GoTo menu items

		try {
			BPath path;
			ThrowOnError( find_directory (B_USER_SETTINGS_DIRECTORY, &path, true) );
			path.Append(kGoDirectory);
			mkdir(path.Path(), 0777);

			BEntry entry(path.Path());
			Model startModel(&entry, true);
			ThrowOnInitCheckError(&startModel);

			if (!startModel.IsContainer())
				throw B_ERROR;
		
			if (startModel.IsQuery()) 
				fContainer = new QueryEntryListCollection(&startModel);
			else
				fContainer = new DirectoryEntryList(*dynamic_cast<BDirectory *>
					(startModel.Node()));
				
			ThrowOnInitCheckError(fContainer);
			ThrowOnError( fContainer->Rewind() );

		} catch (...) {
			delete fContainer;
			fContainer = NULL;
		}
	}
	

	if (fState == kAddingFavorites) {	
		entry_ref ref;
		// limit nav menus to 20 items only
		if (fContainer
			&& fSectionItemCount < 20
			&& fContainer->GetNextRef(&ref) == B_OK) {
			Model model(&ref, true);
			if (model.InitCheck() != B_OK)
				return true;

			BMenuItem *item = BNavMenu::NewModelItem(&model,
				model.IsDirectory() ? fOpenFolderMessage : fOpenFileMessage,
				fTarget);

			item->SetLabel(ref.name);		// this is the name of the link in the Go dir

			if (!fAddedSeparatorForSection) {
				fAddedSeparatorForSection = true;
				AddItem(new TitledSeparatorItem("Favorite Folders"));
			}
			fUniqueRefCheck.push_back(*model.EntryRef());
			AddItem(item);
			fSectionItemCount++;
			return true;
		}
	
		// done with favorites, set up for adding recent files
		fState = kAddingFiles;
		
		fAddedSeparatorForSection = false;
		
		app_info info;
		be_app->GetAppInfo(&info);
		fItems.MakeEmpty();

		int32 apps, docs, folders;
		TTrackerState().RecentCounts(&apps, &docs, &folders);

		BRoster().GetRecentDocuments(&fItems, docs, NULL, info.signature);
		fIndex = 0;
		fSectionItemCount = 0;
	}
	
	if (fState == kAddingFiles) {
		//	if this is a Save panel, not an Open panel
		//	then don't add the recent documents
		if (!fIsSavePanel) {
			for (;;) {
				entry_ref ref;
				if (fItems.FindRef("refs", fIndex++, &ref) != B_OK)
					break;
				Model model(&ref, true);
				if (model.InitCheck() != B_OK)
					return true;
	
				BMenuItem *item = BNavMenu::NewModelItem(&model, fOpenFileMessage, fTarget);
				if (item) {
					if (!fAddedSeparatorForSection) {
						fAddedSeparatorForSection = true;
						AddItem(new TitledSeparatorItem("Recent Documents"));
					}
					AddItem(item);
					fSectionItemCount++;
					return true;
				}
			}
		}
		
		// done with recent files, set up for adding recent folders
		fState = kAddingFolders;
			
		fAddedSeparatorForSection = false;

		app_info info;
		be_app->GetAppInfo(&info);
		fItems.MakeEmpty();

		int32 apps, docs, folders;
		TTrackerState().RecentCounts(&apps, &docs, &folders);

		BRoster().GetRecentFolders(&fItems, folders, info.signature);
		fIndex = 0;
	}
	
	if (fState == kAddingFolders) {
		for (;;) {
			entry_ref ref;
			if (fItems.FindRef("refs", fIndex++, &ref) != B_OK)
				break;
			
			// don't add folders that are already in the GoTo section
			if (find_if(fUniqueRefCheck.begin(), fUniqueRefCheck.end(),
				bind2nd(std::equal_to<entry_ref>(), ref)) != fUniqueRefCheck.end()) 
				continue;

			Model model(&ref, true);
			if (model.InitCheck() != B_OK)
				return true;

			BMenuItem *item = BNavMenu::NewModelItem(&model, fOpenFolderMessage,
				fTarget, true);
			if (item) {
				if (!fAddedSeparatorForSection) {
					fAddedSeparatorForSection = true;
					AddItem(new TitledSeparatorItem("Recent Folders"));
				}
				AddItem(item);
				item->SetEnabled(true);
					// BNavMenu::NewModelItem returns a disabled item here -
					// need to fix this in BNavMenu::NewModelItem
				return true;
			}
		}
	}
	return false;
}

void 
FavoritesMenu::DoneBuildingItemList()
{
	SetTargetForItems(fTarget);
}

void 
FavoritesMenu::ClearMenuBuildingState()
{
	delete fContainer;
	fContainer = NULL;
	fState = kDone;

	// force the menu to get rebuilt each time
	fMenuBuilt = false;
}

