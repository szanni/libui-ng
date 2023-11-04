#include "uipriv_unix.h"

struct uiSplit {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *container;
	GtkPaned *split;
	uiControl *first;
	uiControl *second;
};

uiUnixControlAllDefaultsExceptDestroy(uiSplit)

#define TODO_MASSIVE_HACK(c) \
        if (!uiUnixControl(c)->addedBefore) { \
                g_object_ref_sink(GTK_WIDGET(uiControlHandle(uiControl(c)))); \
                gtk_widget_show(GTK_WIDGET(uiControlHandle(uiControl(c)))); \
                uiUnixControl(c)->addedBefore = TRUE; \
        }

static void uiSplitDestroyChild(uiSplit *s, uiControl *child)
{
	uiControlSetParent(child, NULL);
	uiUnixControlSetContainer(uiUnixControl(child), s->container, TRUE);
	uiControlDestroy(child);
}

static void uiSplitDestroy(uiControl *c)
{
	uiSplit *s = uiSplit(c);

	if (s->first != NULL)
		uiSplitDestroyChild(s, s->first);
	if (s->second != NULL)
		uiSplitDestroyChild(s, s->second);

	g_object_unref(s->widget);
	uiFreeControl(uiControl(s));
}

void uiSplitSetFirst(uiSplit *s, uiControl *c)
{
	GtkWidget *widget;

	if (s->first != NULL)
		uiSplitDestroyChild(s, s->first);

	widget = GTK_WIDGET(uiControlHandle(c));

	uiControlSetParent(c, uiControl(s));
	TODO_MASSIVE_HACK(uiUnixControl(c));
	gtk_paned_pack1(s->split, widget, 1, 0);

	s->first = c;
}

void uiSplitSetSecond(uiSplit *s, uiControl *c)
{
	GtkWidget *widget;

	if (s->second != NULL)
		uiSplitDestroyChild(s, s->second);

	widget = GTK_WIDGET(uiControlHandle(c));

	uiControlSetParent(c, uiControl(s));
	TODO_MASSIVE_HACK(uiUnixControl(c));
	gtk_paned_pack2(s->split, widget, 1, 0);

	s->second = c;
}

static uiSplit *uiNewSplit(GtkOrientation orientation)
{
	uiSplit *s;

	uiUnixNewControl(uiSplit, s);

	s->widget = gtk_paned_new(orientation);
	s->container = GTK_CONTAINER(s->widget);
	s->split = GTK_PANED(s->widget);
	s->first = NULL;
	s->second = NULL;

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

