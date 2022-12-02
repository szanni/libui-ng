#include "uipriv_unix.h"

struct uiSplit {
	uiUnixControl c;
	GtkWidget *widget;
	GtkContainer *container;
	GtkPaned *split;
	GArray *controls;
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
	uiControl *child;
	guint i;

	// free all controls
	for (i = 0; i < s->controls->len; i++) {
		child = g_array_index(s->controls, uiControl*, i);
		uiControlSetParent(child, NULL);
		uiUnixControlSetContainer(uiUnixControl(child), s->container, TRUE);
		uiControlDestroy(child);
	}

	g_array_free(s->controls, TRUE);
	// and then ourselves
	g_object_unref(s->widget);
	uiFreeControl(uiControl(s));
}

void uiSplitAdd1(uiSplit *s, uiControl *c, int expand, int shrink)
{
	GtkWidget *widget;

	widget = GTK_WIDGET(uiControlHandle(c));

	uiControlSetParent(c, uiControl(s));
	TODO_MASSIVE_HACK(uiUnixControl(c));
	gtk_paned_pack1(s->split, widget, expand, shrink);

	g_array_append_val(s->controls, c);
}

void uiSplitAdd2(uiSplit *s, uiControl *c, int expand, int shrink)
{
	GtkWidget *widget;

	widget = GTK_WIDGET(uiControlHandle(c));

	uiControlSetParent(c, uiControl(s));
	TODO_MASSIVE_HACK(uiUnixControl(c));
	gtk_paned_pack2(s->split, widget, expand, shrink);

	g_array_append_val(s->controls, c);
}

static uiSplit *uiNewSplit(GtkOrientation orientation)
{
	uiSplit *s;

	uiUnixNewControl(uiSplit, s);

	s->widget = gtk_paned_new(orientation);
	s->container = GTK_CONTAINER(s->widget);
	s->split = GTK_PANED(s->widget);


	if (orientation == GTK_ORIENTATION_VERTICAL) {
		gtk_widget_set_vexpand(s->widget, TRUE);
		gtk_widget_set_valign(s->widget, GTK_ALIGN_FILL);
	} else {
		gtk_widget_set_hexpand(s->widget, TRUE);
		gtk_widget_set_halign(s->widget, GTK_ALIGN_FILL);
	}

	s->controls = g_array_new(FALSE, TRUE, sizeof(uiControl*));

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

