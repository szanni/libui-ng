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

void buttonComboboxWindowOnClicked(uiButton *b, void *data)
{
	uiWindow *w;
	uiCombobox *child;
	w = uiNewWindow("Combobox", 300, 30, 0);
	uiWindowOnClosing(w, onClosingSub, NULL);
	child = uiNewCombobox();
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

	b = uiNewButton("Combobox");
	uiButtonOnClicked(b, buttonComboboxWindowOnClicked, NULL);
	uiWindowSetChild(w, uiControl(b));

	/* Interestingly enough this works AND prevents the crash from happening
	 * when triggering the creating through the callback later on. */
	//buttonComboboxWindowOnClicked(uiButton *b, void *data)

	uiControlShow(uiControl(w));
	uiMain();
	uiUninit();
	return 0;
}

