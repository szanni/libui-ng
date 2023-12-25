// 29 june 2016
#include "uipriv_unix.h"

// functions FROM THE FUTURE!
// in some cases, because being held back by LTS releases sucks :/
// in others, because parts of GTK+ being unstable until recently also sucks :/

// added in pango 1.38; we need 1.36
static PangoAttribute *(*newFeaturesAttr)(const gchar *features) = NULL;
static PangoAttribute *(*newFGAlphaAttr)(guint16 alpha) = NULL;
static PangoAttribute *(*newBGAlphaAttr)(guint16 alpha) = NULL;

// note that we treat any error as "the symbols aren't there" (and don't care if dlclose() failed)
void uiprivLoadFutures(void)
{
	void *handle;

	// dlsym() walks the dependency chain, so opening the current process should be sufficient
	handle = dlopen(NULL, RTLD_LAZY);
	if (handle == NULL)
		return;
#define GET(var, fn) *((void **) (&var)) = dlsym(handle, #fn)
	GET(newFeaturesAttr, pango_attr_font_features_new);
	GET(newFGAlphaAttr, pango_attr_foreground_alpha_new);
	GET(newBGAlphaAttr, pango_attr_background_alpha_new);
	dlclose(handle);
}

PangoAttribute *uiprivFUTURE_pango_attr_font_features_new(const gchar *features)
{
	if (newFeaturesAttr == NULL)
		return NULL;
	return (*newFeaturesAttr)(features);
}

PangoAttribute *uiprivFUTURE_pango_attr_foreground_alpha_new(guint16 alpha)
{
	if (newFGAlphaAttr == NULL)
		return NULL;
	return (*newFGAlphaAttr)(alpha);
}

PangoAttribute *uiprivFUTURE_pango_attr_background_alpha_new(guint16 alpha)
{
	if (newBGAlphaAttr == NULL)
		return NULL;
	return (*newBGAlphaAttr)(alpha);
}

