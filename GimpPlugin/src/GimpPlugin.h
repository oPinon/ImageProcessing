#pragma once

#include <libgimp/gimp.h>

// executed by Gimp when loading the plugin
void query(void);

// when running the plugin
void run(const gchar      *name,
	gint              nparams,
	const GimpParam  *param,
	gint             *nreturn_vals,
	GimpParam       **return_vals);
static gboolean filter_dialog(GimpDrawable *drawable);

typedef struct
{
	gint width, height;
} FilterValues;

