// 19 may 2017
#import "uipriv_darwin.h"

// functions and constants FROM THE FUTURE!
// note: for constants, dlsym() returns the address of the constant itself, as if we had done &constantName

// note that we treat any error as "the symbols aren't there" (and don't care if dlclose() failed)
void uiprivLoadFutures(void)
{
	void *handle;

	// dlsym() walks the dependency chain, so opening the current process should be sufficient
	handle = dlopen(NULL, RTLD_LAZY);
	if (handle == NULL)
		return;
#define GET(var, fn) *((void **) (&var)) = dlsym(handle, #fn)
	dlclose(handle);
}

// wrappers for methods that exist in the future that we can check for with respondsToSelector:
// keep them in one place for convenience

// added in 10.11; we need 10.8
// return whether this was done because we recreate its effects if not (see winmoveresize.m)
BOOL uiprivFUTURE_NSWindow_performWindowDragWithEvent(NSWindow *w, NSEvent *initialEvent)
{
	id cw = (id) w;

	if ([w respondsToSelector:@selector(performWindowDragWithEvent:)]) {
		[cw performWindowDragWithEvent:initialEvent];
		return YES;
	}
	return NO;
}
