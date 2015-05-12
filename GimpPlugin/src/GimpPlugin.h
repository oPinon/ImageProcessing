#pragma once

#include <libgimp/gimp.h>

// executed by Gimp when loading the plugin
static void query(void);

// when running the plugin
void run(const gchar      *name,
	gint              nparams,
	const GimpParam  *param,
	gint             *nreturn_vals,
	GimpParam       **return_vals);

// dialog used for the plugin's parameters
static gboolean filter_dialog(GimpDrawable *drawable);

typedef enum _Kernel // Kernel used for analytical interpolation
{
	CUBIC, // Cubic( 1/3, 1/3 )
	LANCZOS // Lanczos2
} Kernel;

typedef struct
{
	gint width, height;
	Kernel interpolation;
	gboolean adjust; // finally resize to the exact size with analytical interpolation ?

} FilterValues;

