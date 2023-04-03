// 26 june 2015
#import "uipriv_darwin.h"

// LONGTERM restructure this whole file
// LONGTERM explicitly document this works as we want
// LONGTERM note that font and color buttons also do this

#define windowWindow(w) ((NSWindow *) uiControlHandle(uiControl(w)))

// source of code modal logic: http://stackoverflow.com/questions/604768/wait-for-nsalert-beginsheetmodalforwindow

// note: whether extensions are actually shown depends on a user setting in Finder; we can't control it here
static void setupSavePanel(NSSavePanel *s)
{
	[s setCanCreateDirectories:YES];
	[s setShowsHiddenFiles:YES];
	[s setExtensionHidden:NO];
	[s setCanSelectHiddenExtension:NO];
	[s setTreatsFilePackagesAsDirectories:YES];
}

static char *runSavePanel(NSWindow *parent, NSSavePanel *s)
{
	char *filename;

	[s beginSheetModalForWindow:parent completionHandler:^(NSInteger result) {
		[uiprivNSApp() stopModalWithCode:result];
	}];
	if ([uiprivNSApp() runModalForWindow:s] != NSModalResponseOK)
		return NULL;
	filename = uiDarwinNSStringToText([[s URL] path]);
	return filename;
}

char *uiOpenFile(uiWindow *parent)
{
	NSOpenPanel *o;

	o = [NSOpenPanel openPanel];
	[o setCanChooseFiles:YES];
	[o setCanChooseDirectories:NO];
	[o setResolvesAliases:NO];
	[o setAllowsMultipleSelection:NO];
	setupSavePanel(o);
	// panel is autoreleased
	return runSavePanel(windowWindow(parent), o);
}

char *uiOpenFolder(uiWindow *parent)
{
	NSOpenPanel *o;

	o = [NSOpenPanel openPanel];
	[o setCanChooseFiles:NO];
	[o setCanChooseDirectories:YES];
	[o setResolvesAliases:NO];
	[o setAllowsMultipleSelection:NO];
	setupSavePanel(o);
	// panel is autoreleased
	return runSavePanel(windowWindow(parent), o);
}

char *uiSaveFile(uiWindow *parent)
{
	NSSavePanel *s;

	s = [NSSavePanel savePanel];
	setupSavePanel(s);
	// panel is autoreleased
	return runSavePanel(windowWindow(parent), s);
}

static void msgbox(NSWindow *parent, const char *title, const char *description, NSAlertStyle style)
{
	NSAlert *a;

	a = [NSAlert new];
	[a setAlertStyle:style];
	[a setShowsHelp:NO];
	[a setShowsSuppressionButton:NO];
	[a setMessageText:uiprivToNSString(title)];
	[a setInformativeText:uiprivToNSString(description)];
	[a addButtonWithTitle:@"OK"];
	[a beginSheetModalForWindow:parent completionHandler:^(NSInteger result) {
		[uiprivNSApp() stopModalWithCode:result];
	}];
	[uiprivNSApp() runModalForWindow:[a window]];
	[a release];
}

void uiMsgBox(uiWindow *parent, const char *title, const char *description)
{
	msgbox(windowWindow(parent), title, description, NSAlertStyleInformational);
}

void uiMsgBoxError(uiWindow *parent, const char *title, const char *description)
{
	msgbox(windowWindow(parent), title, description, NSAlertStyleCritical);
}
