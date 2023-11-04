#include "uipriv_unix.h"

struct uiSplit {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *container;
	GtkPaned *split;
	uiControl* controls[2];
};

uiUnixControlAllDefaultsExceptDestroy(uiSplit)

#define TODO_MASSIVE_HACK(c) \
        if (!uiUnixControl(c)->addedBefore) { \
                g_object_ref_sink(GTK_WIDGET(uiControlHandle(uiControl(c)))); \
                gtk_widget_show(GTK_WIDGET(uiControlHandle(uiControl(c)))); \
                uiUnixControl(c)->addedBefore = TRUE; \
        }

static void uiSplitDestroy(uiControl *c)
{
	uiSplit *s = uiSplit(c);
	int i;

	for (i = 0; i < 2; ++i) {
		uiControl *child = s->controls[i];
		if (child == NULL)
			continue;
		uiControlSetParent(child, NULL);
		uiUnixControlSetContainer(uiUnixControl(child), s->container, TRUE);
		uiControlDestroy(child);
	}

	g_object_unref(s->widget);
	uiFreeControl(uiControl(s));
}

void uiSplitSetFirst(uiSplit *s, uiControl *c)
{
	GtkWidget *widget;

	widget = GTK_WIDGET(uiControlHandle(c));

	uiControlSetParent(c, uiControl(s));
	TODO_MASSIVE_HACK(uiUnixControl(c));
	gtk_paned_pack1(s->split, widget, 1, 0);

	s->controls[0] = c;
}

void uiSplitSetSecond(uiSplit *s, uiControl *c)
{
	GtkWidget *widget;

	widget = GTK_WIDGET(uiControlHandle(c));

	uiControlSetParent(c, uiControl(s));
	TODO_MASSIVE_HACK(uiUnixControl(c));
	gtk_paned_pack2(s->split, widget, 1, 0);

	s->controls[1] = c;
}

static uiSplit *uiNewSplit(GtkOrientation orientation)
{
	uiSplit *s;
	int i;

	uiUnixNewControl(uiSplit, s);

	s->widget = gtk_paned_new(orientation);
	s->container = GTK_CONTAINER(s->widget);
	s->split = GTK_PANED(s->widget);
	for (i = 0; i < 2; ++i)
		s->controls[i] = NULL;

	if (orientation == GTK_ORIENTATION_VERTICAL) {
		gtk_widget_set_vexpand(s->widget, TRUE);
		gtk_widget_set_valign(s->widget, GTK_ALIGN_FILL);
	} else {
		gtk_widget_set_hexpand(s->widget, TRUE);
		gtk_widget_set_halign(s->widget, GTK_ALIGN_FILL);
	}

	return s;
}

uiSplit *uiNewHorizontalSplit(void)
{
	return uiNewSplit(GTK_ORIENTATION_HORIZONTAL);
}

uiSplit *uiNewVerticalSplit(void)
{
	return uiNewSplit(GTK_ORIENTATION_VERTICAL);
}

