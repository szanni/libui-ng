#import "uipriv_darwin.h"

@interface splitPage : NSObject {
	uiprivSingleChildConstraints constraints;
	int margined;
	NSView *view;		// the NSSplitViewItem view itself
	NSObject *pageID;
}
@property uiControl *c;
@property NSLayoutPriority oldHorzHuggingPri;
@property NSLayoutPriority oldVertHuggingPri;
- (id)initWithView:(NSView *)v pageID:(NSObject *)o;
- (NSView *)childView;
- (void)establishChildConstraints;
- (void)removeChildConstraints;
- (int)isMargined;
- (void)setMargined:(int)m;
@end

struct uiSplit {
	uiDarwinControl c;
	NSSplitView *splitview;
	NSMutableArray *pages;
	NSLayoutPriority horzHuggingPri;
	NSLayoutPriority vertHuggingPri;
};

@implementation splitPage

- (id)initWithView:(NSView *)v pageID:(NSObject *)o
{
	self = [super init];
	if (self != nil) {
		self->view = [v retain];
		self->pageID = [o retain];
	}
	return self;
}

- (void)dealloc
{
	[self removeChildConstraints];
	[self->view release];
	[self->pageID release];
	[super dealloc];
}

- (NSView *)childView
{
	return (NSView *) uiControlHandle(self.c);
}

- (void)establishChildConstraints
{
	[self removeChildConstraints];
	if (self.c == NULL)
		return;
	uiprivSingleChildConstraintsEstablish(&(self->constraints),
		self->view, [self childView],
		uiDarwinControlHugsTrailingEdge(uiDarwinControl(self.c)),
		uiDarwinControlHugsBottom(uiDarwinControl(self.c)),
		self->margined,
		@"uiSplit page");
}

- (void)removeChildConstraints
{
	uiprivSingleChildConstraintsRemove(&(self->constraints), self->view);
}

- (int)isMargined
{
	return self->margined;
}

- (void)setMargined:(int)m
{
	self->margined = m;
	uiprivSingleChildConstraintsSetMargined(&(self->constraints), self->margined);
}

@end

static void uiSplitDestroy(uiControl *c)
{
	uiSplit *t = uiSplit(c);
	splitPage *page;

	// first remove all split pages so we can destroy all the children
	/* TODO IMP
	while ([t->splitview numberOfSplitViewItems] != 0)
		[t->splitview removeSplitViewItem:[t->splitview splitViewItemAtIndex:0]];
	*/
	// then destroy all the children
	for (page in t->pages) {
		[page removeChildConstraints];
		uiControlSetParent(page.c, NULL);
		uiDarwinControlSetSuperview(uiDarwinControl(page.c), nil);
		uiControlDestroy(page.c);
	}
	// and finally destroy ourselves
	[t->pages release];
	[t->splitview release];
	uiFreeControl(uiControl(t));
}

uiDarwinControlDefaultHandle(uiSplit, splitview)
uiDarwinControlDefaultParent(uiSplit, splitview)
uiDarwinControlDefaultSetParent(uiSplit, splitview)
uiDarwinControlDefaultToplevel(uiSplit, splitview)
uiDarwinControlDefaultVisible(uiSplit, splitview)
uiDarwinControlDefaultShow(uiSplit, splitview)
uiDarwinControlDefaultHide(uiSplit, splitview)
uiDarwinControlDefaultEnabled(uiSplit, splitview)
uiDarwinControlDefaultEnable(uiSplit, splitview)
uiDarwinControlDefaultDisable(uiSplit, splitview)

static void uiSplitSyncEnableState(uiDarwinControl *c, int enabled)
{
	uiSplit *t = uiSplit(c);
	splitPage *page;

	if (uiDarwinShouldStopSyncEnableState(uiDarwinControl(t), enabled))
		return;
	for (page in t->pages)
		uiDarwinControlSyncEnableState(uiDarwinControl(page.c), enabled);
}

uiDarwinControlDefaultSetSuperview(uiSplit, splitview)

static void splitRelayout(uiSplit *t)
{
	splitPage *page;

	for (page in t->pages)
		[page establishChildConstraints];
	// and this gets rid of some weird issues with regards to box alignment
	uiprivJiggleViewLayout(t->splitview);
}

BOOL uiSplitHugsTrailingEdge(uiDarwinControl *c)
{
	uiSplit *t = uiSplit(c);

	return t->horzHuggingPri < NSLayoutPriorityWindowSizeStayPut;
}

BOOL uiSplitHugsBottom(uiDarwinControl *c)
{
	uiSplit *t = uiSplit(c);

	return t->vertHuggingPri < NSLayoutPriorityWindowSizeStayPut;
}

static void uiSplitChildEdgeHuggingChanged(uiDarwinControl *c)
{
	uiSplit *t = uiSplit(c);

	splitRelayout(t);
}

static NSLayoutPriority uiSplitHuggingPriority(uiDarwinControl *c, NSLayoutConstraintOrientation orientation)
{
	uiSplit *t = uiSplit(c);

	if (orientation == NSLayoutConstraintOrientationHorizontal)
		return t->horzHuggingPri;
	return t->vertHuggingPri;
}

static void uiSplitSetHuggingPriority(uiDarwinControl *c, NSLayoutPriority priority, NSLayoutConstraintOrientation orientation)
{
	uiSplit *t = uiSplit(c);

	if (orientation == NSLayoutConstraintOrientationHorizontal)
		t->horzHuggingPri = priority;
	else
		t->vertHuggingPri = priority;
	uiDarwinNotifyEdgeHuggingChanged(uiDarwinControl(t));
}

static void uiSplitChildVisibilityChanged(uiDarwinControl *c)
{
	uiSplit *t = uiSplit(c);

	splitRelayout(t);
}


void uiSplitInsertAt(uiSplit *t, const char *name, int n, uiControl *child);

void uiSplitAppend(uiSplit *t, const char *name, uiControl *child)
{
	uiSplitInsertAt(t, name, [t->pages count], child);
}

void uiSplitAdd1(uiSplit *b, uiControl *child, int expand, int shrink)
{
	uiSplitAppend(b, "none", child);
}


void uiSplitAdd2(uiSplit *b, uiControl *child, int expand, int shrink)
{
	uiSplitAppend(b, "none", child);
}

void uiSplitInsertAt(uiSplit *t, const char *name, int n, uiControl *child)
{
	splitPage *page;
	NSView *view;
	//NSSplitViewItem *i;
	NSObject *pageID;

	uiControlSetParent(child, uiControl(t));

	view = [[[NSView alloc] initWithFrame:NSZeroRect] autorelease];
	// note: if we turn off the autoresizing mask, nothing shows up
	uiDarwinControlSetSuperview(uiDarwinControl(child), view);
	uiDarwinControlSyncEnableState(uiDarwinControl(child), uiControlEnabledToUser(uiControl(t)));

	// the documentation says these can be nil but the headers say these must not be; let's be safe and make them non-nil anyway
	pageID = [NSObject new];
	page = [[[splitPage alloc] initWithView:view pageID:pageID] autorelease];
	page.c = child;

	// don't hug, just in case we're a stretchy split
	page.oldHorzHuggingPri = uiDarwinControlHuggingPriority(uiDarwinControl(page.c), NSLayoutConstraintOrientationHorizontal);
	page.oldVertHuggingPri = uiDarwinControlHuggingPriority(uiDarwinControl(page.c), NSLayoutConstraintOrientationVertical);
	uiDarwinControlSetHuggingPriority(uiDarwinControl(page.c), NSLayoutPriorityDefaultLow, NSLayoutConstraintOrientationHorizontal);
	uiDarwinControlSetHuggingPriority(uiDarwinControl(page.c), NSLayoutPriorityDefaultLow, NSLayoutConstraintOrientationVertical);

	[t->pages insertObject:page atIndex:n];

	/*
	i = [[[NSSplitViewItem alloc] initWithIdentifier:pageID] autorelease];
	[i setLabel:uiprivToNSString(name)];
	[i setView:view];
	*/
	[t->splitview insertArrangedSubview:view atIndex:n];

	splitRelayout(t);
}

void uiSplitDelete(uiSplit *t, int n)
{
	splitPage *page;
	uiControl *child;
	NSSplitViewItem *i;

	page = (splitPage *) [t->pages objectAtIndex:n];

	uiDarwinControlSetHuggingPriority(uiDarwinControl(page.c), page.oldHorzHuggingPri, NSLayoutConstraintOrientationHorizontal);
	uiDarwinControlSetHuggingPriority(uiDarwinControl(page.c), page.oldVertHuggingPri, NSLayoutConstraintOrientationVertical);

	child = page.c;
	[page removeChildConstraints];
	[t->pages removeObjectAtIndex:n];

	uiControlSetParent(child, NULL);
	uiDarwinControlSetSuperview(uiDarwinControl(child), nil);

	/* TODO IMP
	i = [t->splitview splitViewItemAtIndex:n];
	[t->splitview removeSplitViewItem:i];
	*/

	splitRelayout(t);
}

int uiSplitNumPages(uiSplit *t)
{
	return [t->pages count];
}

int uiSplitMargined(uiSplit *t, int n)
{
	splitPage *page;

	page = (splitPage *) [t->pages objectAtIndex:n];
	return [page isMargined];
}

void uiSplitSetMargined(uiSplit *t, int n, int margined)
{
	splitPage *page;

	page = (splitPage *) [t->pages objectAtIndex:n];
	[page setMargined:margined];
}

static uiSplit *uiNewSplit(BOOL verticalDivider)
{
	uiSplit *t;

	uiDarwinNewControl(uiSplit, t);

	t->splitview = [[NSSplitView alloc] initWithFrame:NSZeroRect];
	// also good for NSSplitView (same selector and everything)
	//uiDarwinSetControlFont((NSControl *) (t->splitview), NSRegularControlSize);

	[t->splitview setVertical:verticalDivider];

	t->pages = [NSMutableArray new];

	// default to low hugging to not hug edges
	t->horzHuggingPri = NSLayoutPriorityDefaultLow;
	t->vertHuggingPri = NSLayoutPriorityDefaultLow;

	return t;
}

uiSplit *uiNewVerticalSplit(void)
{
	return uiNewSplit(NO);
}


uiSplit *uiNewHorizontalSplit(void)
{
	return uiNewSplit(YES);
}
