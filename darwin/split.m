#import "uipriv_darwin.h"

@interface splitChild : NSView {
	uiprivSingleChildConstraints constraints;
}
@property uiControl *c;
@property BOOL stretchy;
@property BOOL shrink;
@property NSLayoutPriority oldHorzHuggingPri;
@property NSLayoutPriority oldVertHuggingPri;
- (NSView *)view;
- (void)establishChildConstraints;
- (void)removeChildConstraints;
- (id)initWithControl:(uiControl *)c expand:(BOOL)e shrink:(BOOL)s;
@end

@interface splitView : NSSplitView {
	uiSplit *s;
@public
	NSMutableArray *children;
	BOOL vert;

        NSLayoutConstraint *first;
        NSLayoutConstraint *last;
        NSMutableArray *otherConstraints;

        NSLayoutAttribute primaryStart;
        NSLayoutAttribute primaryEnd;
        NSLayoutAttribute secondaryStart;
        NSLayoutAttribute secondaryEnd;
        NSLayoutAttribute primarySize;
        NSLayoutConstraintOrientation primaryOrientation;
        NSLayoutConstraintOrientation secondaryOrientation;
}
- (id)initWithControl:(uiSplit *)s vectival:(BOOL)v;
- (void)establishOurConstraints;
@end

struct uiSplit {
	uiDarwinControl c;
	splitView *splitview;
};

@implementation splitView

- (id)initWithControl:(uiSplit *)c vectival:(BOOL)v
{
	self = [super initWithFrame:NSZeroRect];
	if (self != nil) {
		self->s = c;
		self->vert = v;
		self->children = [NSMutableArray new];

		// NSSplitView defines vertical as the orientation of the spacers
		[self setVertical:!v];
		[self setDividerStyle:NSSplitViewDividerStyleThin];

		self->otherConstraints = [NSMutableArray new];

		if (self->vert) {
			self->primaryStart = NSLayoutAttributeTop;
			self->primaryEnd = NSLayoutAttributeBottom;
			self->secondaryStart = NSLayoutAttributeLeading;
			self->secondaryEnd = NSLayoutAttributeTrailing;
			self->primarySize = NSLayoutAttributeHeight;
			self->primaryOrientation = NSLayoutConstraintOrientationVertical;
			self->secondaryOrientation = NSLayoutConstraintOrientationHorizontal;
		} else {
			self->primaryStart = NSLayoutAttributeLeading;
			self->primaryEnd = NSLayoutAttributeTrailing;
			self->secondaryStart = NSLayoutAttributeTop;
			self->secondaryEnd = NSLayoutAttributeBottom;
			self->primarySize = NSLayoutAttributeWidth;
			self->primaryOrientation = NSLayoutConstraintOrientationHorizontal;
			self->secondaryOrientation = NSLayoutConstraintOrientationVertical;
		}
	}
	return self;
}

- (void)establishOurConstraints
{
	splitChild *bc;
	splitChild *sc;
	CGFloat padding;
	NSView *prev;
	NSLayoutConstraint *c;
	BOOL (*hugsSecondary)(uiDarwinControl *);
	int i = 0;

	[self removeOurConstraints];
	if ([self->children count] == 0)
		return;
	padding = 0;

	for (sc in self->children) {
		[sc establishChildConstraints];
	}

	for (sc in self->children) {
		if (i == 0) {
			c = uiprivMkConstraint(sc, NSLayoutAttributeLeading,
				NSLayoutRelationEqual,
				[sc view], NSLayoutAttributeLeading,
				1, 0,
				@"uiSplit child horizontal alignment start constraint");
			[sc addConstraint:c];
			c = uiprivMkConstraint(sc, NSLayoutAttributeTop,
				NSLayoutRelationEqual,
				[sc view], NSLayoutAttributeTop,
				1, 0,
				@"uiSplit child horizontal alignment top constraint");
			[sc addConstraint:c];

		}
		if (i == 1) {
			if (self->vert) {
				c = uiprivMkConstraint(sc, NSLayoutAttributeBottom,
					NSLayoutRelationEqual,
					[sc view], NSLayoutAttributeBottom,
					1, 0,
					@"uiSplit child horizontal alignment bottom constraint");
				[sc addConstraint:c];
			} else {
				c = uiprivMkConstraint(sc, NSLayoutAttributeTrailing,
					NSLayoutRelationEqual,
					[sc view], NSLayoutAttributeTrailing,
					1, 0,
					@"uiSplit child horizontal alignment trailing constraint");
				[sc addConstraint:c];
			}
		}
	}

	return;
	// shrink?
	for (sc in self->children) {
		if (self->vert) {
			CGFloat minHeight;
			if (sc.shrink)
				minHeight = 0;
			else
				minHeight = NSHeight([[sc view] frame]);;

			c = uiprivMkConstraint(sc, NSLayoutAttributeHeight,
				NSLayoutRelationGreaterThanOrEqual,
				nil, NSLayoutAttributeNotAnAttribute,
				1, minHeight,
				@"uiSplit child shrink/minimum height constraint");
			[sc addConstraint:c];

		} else {
			CGFloat minWidth;
			if (sc.shrink)
				minWidth = 0;
			else
				minWidth = NSWidth([[sc view] frame]);;

			c = uiprivMkConstraint(sc, NSLayoutAttributeWidth,
				NSLayoutRelationGreaterThanOrEqual,
				nil, NSLayoutAttributeNotAnAttribute,
				1, minWidth,
				@"uiSplit child shrink/minimum width constraint");
			[sc addConstraint:c];
		}
	}

	return;
	/*
	*/

	// first arrange in the primary direction
	prev = nil;
	for (bc in self->children) {
		if (!uiControlVisible(bc.c))
			continue;
		if (prev == nil) {			// first view
			self->first = uiprivMkConstraint(self, self->primaryStart,
				NSLayoutRelationEqual,
				[bc view], self->primaryStart,
				1, 0,
				@"uiBox first primary constraint");
			[self addConstraint:self->first];
			[self->first retain];
			prev = [bc view];
			continue;
		}
		// not the first; link it
		c = uiprivMkConstraint(prev, self->primaryEnd,
			NSLayoutRelationEqual,
			[bc view], self->primaryStart,
			1, 0,
			@"uiBox in-between primary constraint");
		[self addConstraint:c];
		prev = [bc view];
	}
	if (prev == nil)		// no control visible; act as if no controls
		return;
	self->last = uiprivMkConstraint(prev, self->primaryEnd,
		NSLayoutRelationEqual,
		self, self->primaryEnd,
		1, 0,
		@"uiBox last primary constraint");
	[self addConstraint:self->last];
	[self->last retain];

	return;
	// then arrange in the secondary direction
	if (self->vert)
		hugsSecondary = uiDarwinControlHugsTrailingEdge;
	else
		hugsSecondary = uiDarwinControlHugsBottom;

	for (bc in self->children) {
		if (!uiControlVisible(bc.c))
			continue;
		c = uiprivMkConstraint(self, self->secondaryStart,
			NSLayoutRelationEqual,
			[bc view], self->secondaryStart,
			1, 0,
			@"uiBox secondary start constraint");
		[self addConstraint:c];
		[self->otherConstraints addObject:c];
		c = uiprivMkConstraint([bc view], self->secondaryEnd,
			NSLayoutRelationLessThanOrEqual,
			self, self->secondaryEnd,
			1, 0,
			@"uiBox secondary end <= constraint");
		if ((*hugsSecondary)(uiDarwinControl(bc.c)))
			[c setPriority:NSLayoutPriorityDefaultLow];
		[self addConstraint:c];
		[self->otherConstraints addObject:c];
		c = uiprivMkConstraint([bc view], self->secondaryEnd,
			NSLayoutRelationEqual,
			self, self->secondaryEnd,
			1, 0,
			@"uiBox secondary end == constraint");
		if (!(*hugsSecondary)(uiDarwinControl(bc.c)))
			[c setPriority:NSLayoutPriorityDefaultLow];
		[self addConstraint:c];
		[self->otherConstraints addObject:c];
	}

	return;
	prev = nil;		// first stretchy view
	for (bc in self->children) {
		if (!uiControlVisible(bc.c))
			continue;
		if (!bc.stretchy)
			continue;
		if (prev == nil) {
			prev = [bc view];
			continue;
		}
		c = uiprivMkConstraint(prev, self->primarySize,
			NSLayoutRelationEqual,
			[bc view], self->primarySize,
			1, 0,
			@"uiBox stretchy size constraint");
		[self addConstraint:c];
		[self->otherConstraints addObject:c];
	}
}


- (void)removeOurConstraints
{
        if (self->first != nil) {
                [self removeConstraint:self->first];
                [self->first release];
                self->first = nil;
        }
        if (self->last != nil) {
                [self removeConstraint:self->last];
                [self->last release];
                self->last = nil;
        }
        if ([self->otherConstraints count] != 0) {
                [self removeConstraints:self->otherConstraints];
                [self->otherConstraints removeAllObjects];
        }
}

@end


@implementation splitChild

- (id)initWithControl:(uiControl *)c expand:(BOOL)e shrink:(BOOL)s
{
	self = [super initWithFrame:NSZeroRect];
	if (self != nil) {
		self.c = c;
		self.stretchy = e;
		self.shrink = s;

		self.oldHorzHuggingPri = uiDarwinControlHuggingPriority(uiDarwinControl(self.c), NSLayoutConstraintOrientationHorizontal);
		self.oldVertHuggingPri = uiDarwinControlHuggingPriority(uiDarwinControl(self.c), NSLayoutConstraintOrientationVertical);

		uiDarwinControlSetSuperview(uiDarwinControl(self.c), self);
	}
	return self;
}

- (void)dealloc
{
	[self removeChildConstraints];
	uiControlSetParent(self.c, NULL);
	uiDarwinControlSetSuperview(uiDarwinControl(self.c), nil);
	uiDarwinControlSetHuggingPriority(uiDarwinControl(self.c), self.oldHorzHuggingPri, NSLayoutConstraintOrientationHorizontal);
	uiDarwinControlSetHuggingPriority(uiDarwinControl(self.c), self.oldVertHuggingPri, NSLayoutConstraintOrientationVertical);
	[super dealloc];
}

- (NSView *)view
{
	return (NSView *) uiControlHandle(self.c);
}

- (void)establishChildConstraints
{
	[self removeChildConstraints];
	uiprivSingleChildConstraintsEstablish(&self->constraints,
		self, [self view], 1, 1, 0,
		@"splitChild constraints");
}

- (void)removeChildConstraints
{
	uiprivSingleChildConstraintsRemove(&self->constraints, self);
}

@end

static void uiSplitDestroy(uiControl *c)
{
	uiSplit *t = uiSplit(c);
	splitChild *page;

	// first remove all split pages so we can destroy all the children
	/* TODO IMP
	while ([t->splitview numberOfSplitViewItems] != 0)
		[t->splitview removeSplitViewItem:[t->splitview splitViewItemAtIndex:0]];
	*/
	// then destroy all the children
	for (page in t->splitview->children) {
		[page removeChildConstraints];
		uiControlSetParent(page.c, NULL);
		uiDarwinControlSetSuperview(uiDarwinControl(page.c), nil);
		uiControlDestroy(page.c);
	}
	// and finally destroy ourselves
	[t->splitview->children release];
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
	splitChild *sc;

	if (uiDarwinShouldStopSyncEnableState(uiDarwinControl(t), enabled))
		return;
	for (sc in t->splitview->children)
		uiDarwinControlSyncEnableState(uiDarwinControl(sc.c), enabled);
}

uiDarwinControlDefaultSetSuperview(uiSplit, splitview)

static void splitRelayout(uiSplit *t)
{
	[t->splitview establishOurConstraints];
	// and this gets rid of some weird issues with regards to box alignment
	uiprivJiggleViewLayout(t->splitview);
}

BOOL uiSplitHugsTrailingEdge(uiDarwinControl *c)
{
	uiSplit *s = uiSplit(c);

	if (s->splitview->vert)
		return NO;
	else
		return YES;

	return YES;
}

BOOL uiSplitHugsBottom(uiDarwinControl *c)
{
	uiSplit *s = uiSplit(c);

	if (s->splitview->vert)
		return YES;
	else
		return NO;
	return YES;
}

static void uiSplitChildEdgeHuggingChanged(uiDarwinControl *c)
{
	uiSplit *s = uiSplit(c);

	splitRelayout(s);
}

uiDarwinControlDefaultHuggingPriority(uiSplit, splitview)
uiDarwinControlDefaultSetHuggingPriority(uiSplit, splitview)

static void uiSplitChildVisibilityChanged(uiDarwinControl *c)
{
	uiSplit *t = uiSplit(c);

	splitRelayout(t);
}



void uiSplitInsertAt(uiSplit *t, int n, uiControl *c, int expand, int shrink)
{
	splitChild *sc;

	uiControlSetParent(c, uiControl(t));

	sc = [[splitChild alloc] initWithControl:c expand:expand shrink:shrink];
	//[sc setTranslatesAutoresizingMaskIntoConstraints:NO];
	//view = uiControlHandle(c);

	// note: if we turn off the autoresizing mask, nothing shows up
	uiDarwinControlSyncEnableState(uiDarwinControl(c), uiControlEnabledToUser(uiControl(t)));

	// don't hug, just in case we're a stretchy split
	/*
	page.oldHorzHuggingPri = uiDarwinControlHuggingPriority(uiDarwinControl(page.c), NSLayoutConstraintOrientationHorizontal);
	page.oldVertHuggingPri = uiDarwinControlHuggingPriority(uiDarwinControl(page.c), NSLayoutConstraintOrientationVertical);
	*/
	uiDarwinControlSetHuggingPriority(uiDarwinControl(sc.c), NSLayoutPriorityDefaultLow, t->splitview->primaryOrientation);
	uiDarwinControlSetHuggingPriority(uiDarwinControl(sc.c), NSLayoutPriorityDefaultLow, t->splitview->secondaryOrientation);

	[t->splitview->children insertObject:sc atIndex:n];
	[t->splitview insertArrangedSubview:sc atIndex:n];

	splitRelayout(t);
}

void uiSplitDelete(uiSplit *t, int n)
{
	splitChild *page;
	uiControl *child;

	page = (splitChild *) [t->splitview->children objectAtIndex:n];

	uiDarwinControlSetHuggingPriority(uiDarwinControl(page.c), page.oldHorzHuggingPri, NSLayoutConstraintOrientationHorizontal);
	uiDarwinControlSetHuggingPriority(uiDarwinControl(page.c), page.oldVertHuggingPri, NSLayoutConstraintOrientationVertical);

	child = page.c;
	[page removeChildConstraints];
	[t->splitview->children removeObjectAtIndex:n];

	uiControlSetParent(child, NULL);
	uiDarwinControlSetSuperview(uiDarwinControl(child), nil);

	/* TODO IMP
	i = [t->splitview splitViewItemAtIndex:n];
	[t->splitview removeSplitViewItem:i];
	*/

	splitRelayout(t);
}

static void uiSplitAppend(uiSplit *s, uiControl *child, int expand, int shrink)
{
	uiSplitInsertAt(s, [s->splitview->children count], child, expand, shrink);
}

void uiSplitAdd1(uiSplit *s, uiControl *c, int expand, int shrink)
{
	uiSplitAppend(s, c, expand, shrink);
}


void uiSplitAdd2(uiSplit *s, uiControl *c, int expand, int shrink)
{
	uiSplitAppend(s, c, expand, shrink);
}

static uiSplit *uiNewSplit(BOOL vertical)
{
	uiSplit *s;
	BOOL verticalDivider = !vertical;

	uiDarwinNewControl(uiSplit, s);

	s->splitview = [[splitView alloc] initWithControl:s vectival:vertical];

	return s;
}

uiSplit *uiNewVerticalSplit(void)
{
	return uiNewSplit(YES);
}


uiSplit *uiNewHorizontalSplit(void)
{
	return uiNewSplit(NO);
}
