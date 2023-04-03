// 14 august 2015
#import "uipriv_darwin.h"

// NSProgressIndicator has no intrinsic width by default; use the default width in Interface Builder
#define progressIndicatorWidth 100

@interface intrinsicWidthNSProgressIndicator : NSProgressIndicator
@end

@implementation intrinsicWidthNSProgressIndicator

- (NSSize)intrinsicContentSize
{
	NSSize s;

	s = [super intrinsicContentSize];
	s.width = progressIndicatorWidth;
	return s;
}

@end

struct uiProgressBar {
	uiDarwinControl c;
	NSProgressIndicator *pi;
};

uiDarwinControlAllDefaults(uiProgressBar, pi)

int uiProgressBarValue(uiProgressBar *p)
{
	if ([p->pi isIndeterminate])
		return -1;
	return [p->pi doubleValue];
}

void uiProgressBarSetValue(uiProgressBar *p, int value)
{
	if (value == -1) {
		[p->pi setIndeterminate:YES];
		[p->pi startAnimation:p->pi];
		return;
	}

	if ([p->pi isIndeterminate]) {
		[p->pi setIndeterminate:NO];
		[p->pi stopAnimation:p->pi];
	}

	if (value < 0 || value > 100)
		uiprivUserBug("Value %d out of range for a uiProgressBar.", value);

	[p->pi setDoubleValue:((double) value)];
}

uiProgressBar *uiNewProgressBar(void)
{
	uiProgressBar *p;

	uiDarwinNewControl(uiProgressBar, p);

	p->pi = [[intrinsicWidthNSProgressIndicator alloc] initWithFrame:NSZeroRect];
	[p->pi setControlSize:NSControlSizeRegular];
	[p->pi setBezeled:YES];
	[p->pi setStyle:NSProgressIndicatorStyleBar];
	[p->pi setIndeterminate:NO];

	return p;
}
