#include <stdlib.h>
#include "../ui.h"


int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

void app(void)
{
	uiInitOptions o = {0};
	uiWindow *w;

	if (uiInit(&o) != NULL) exit(1);
	w = uiNewWindow("App", 100, 100, 0);
	uiWindowOnClosing(w, onClosing, NULL);
	uiControlShow(uiControl(w));
	uiMain();
	uiUninit();
}

int main(void)
{
	app();
	app();
	return 0;
}
