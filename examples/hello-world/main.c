#include <stdio.h>
#include <ui.h>

int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

int onClosingSub(uiWindow *w, void *data)
{
	return 1;
}

void buttonSliderWindowOnClicked(uiButton *b, void *data)
{
	uiWindow *w;
	uiSlider *child;
	w = uiNewWindow("Slider", 300, 30, 0);
	uiWindowOnClosing(w, onClosingSub, NULL);
	child = uiNewSlider(0, 10);
	uiWindowSetChild(w, uiControl(child));
	uiControlShow(uiControl(w));
}

int main(void)
{
	uiInitOptions o = {0};
	const char *err;
	uiWindow *w;
	uiButton *b;

	err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "Error initializing libui-ng: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	// Create a new window
	w = uiNewWindow("Hello World!", 300, 30, 0);
	uiWindowOnClosing(w, onClosing, NULL);

	b = uiNewButton("Slider");
	uiButtonOnClicked(b, buttonSliderWindowOnClicked, NULL);
	uiWindowSetChild(w, uiControl(b));

	/* Interestingly enough this works AND prevents the crash from happening
	 * when triggering the creating through the callback later on. */
	//buttonComboboxWindowOnClicked(uiButton *b, void *data)

	uiControlShow(uiControl(w));
	uiMain();
	uiUninit();
	return 0;
}

