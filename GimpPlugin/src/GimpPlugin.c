#include "GimpPlugin.h"

#include "SuperSample.h"

#include <libgimp/gimpui.h>

// Mandatory parameter : linking to all the functions
static GimpPlugInInfo PLUG_IN_INFO = {
	NULL,
	NULL,
	query,
	run
};

/* Set up default values for options */
static FilterValues fvalues =
{
	1920, 1080
};


MAIN()

void
query(void)
{
	static GimpParamDef args[] = {
		{
			GIMP_PDB_INT32,
			"run-mode",
			"Run mode"
		},
		{
			GIMP_PDB_IMAGE,
			"image",
			"Input image"
		},
		{
			GIMP_PDB_DRAWABLE,
			"drawable",
			"Input drawable"
		}
	};

	gimp_install_procedure(
		"plug-in-supersample",
		"Advanced upsampling",
		"Upsamples an image from local self-examples",
		"Olivier Pinon",
		"Based on Fattal et al. [2010]",
		"2015",
		"Supersample",
		"RGB*, GRAY*",
		GIMP_PLUGIN,
		G_N_ELEMENTS(args), 0,
		args, NULL
		);

	gimp_plugin_menu_register(
		"plug-in-supersample",
		"<Image>/Image/Scale"
		);
}

void run(const gchar      *name,
	gint              nparams,
	const GimpParam  *param,
	gint             *nreturn_vals,
	GimpParam       **return_vals)
{
	static GimpParam values[1];
	GimpPDBStatusType status = GIMP_PDB_SUCCESS;
	GimpRunMode       run_mode;
	GimpDrawable     *drawable; // contains layers, maks, etc...

	/* Setting mandatory output values */
	*nreturn_vals = 1;
	*return_vals = values;

	values[0].type = GIMP_PDB_STATUS;
	values[0].data.d_status = status;

	/* Getting run_mode - we won't display a dialog if
	* we are in NONINTERACTIVE mode */
	run_mode = param[0].data.d_int32;

	/*  Get the specified drawable  */
	drawable = gimp_drawable_get(param[2].data.d_drawable);

	gimp_progress_init("Supersampling...");

	gint32 srcID = drawable->drawable_id;
	gint srcW = gimp_drawable_width(srcID);
	gint srcH = gimp_drawable_height(srcID);
	gint channels = gimp_drawable_bpp(srcID);

	GimpPixelRgn rgn_in, rgn_out;
	gimp_pixel_rgn_init(&rgn_in,
		drawable,
		0, 0,
		srcW, srcH,
		FALSE, FALSE);

	switch (run_mode)
	{
	case GIMP_RUN_INTERACTIVE:
		// Get options last values if needed
		gimp_get_data("plug-in-supersample", &fvalues);

		// Display the dialog
		if (!filter_dialog(drawable))
			return;
		gimp_set_data("plug-in-supersample", &fvalues, sizeof(FilterValues));
		break;

	case GIMP_RUN_NONINTERACTIVE:
		if (nparams != 5)
			status = GIMP_PDB_CALLING_ERROR;
		if (status == GIMP_PDB_SUCCESS) {
			fvalues.width = param[3].data.d_int32;
			fvalues.height = param[4].data.d_int32;
		}
		break;

	case GIMP_RUN_WITH_LAST_VALS:
		// Get options last values if needed
		gimp_get_data("plug-in-supersample", &fvalues);
		break;

	default:
		break;
	}

	guchar* srcPixels = g_new(guchar, srcW * srcH * channels);

	gimp_pixel_rgn_get_rect(&rgn_in, srcPixels, 0, 0, srcW, srcH);

	Image dst = supersample(srcPixels, srcW, srcH, channels, fvalues);
	guchar* dstPixels = dst.img;
	gint dstW = dst.width, dstH = dst.height;

	gint32 new_image_ID = gimp_image_new(dstW, dstH, gimp_image_base_type(param[1].data.d_int32));
	gint32 new_layer_ID = gimp_layer_new(new_image_ID, "Upscaled",
		dstW, dstH,
		gimp_drawable_type(drawable->drawable_id),
		100,
		GIMP_NORMAL_MODE);
	gimp_image_insert_layer(new_image_ID, new_layer_ID, -1, 0);

	GimpDrawable* new_drawable = gimp_drawable_get(new_layer_ID);
	gimp_pixel_rgn_init(&rgn_out,
		new_drawable,
		0, 0,
		dstW, dstH,
		TRUE, TRUE);

	//gimp_image_scale( param[1].data.d_int32, dstW, dstH );

	gimp_pixel_rgn_set_rect(&rgn_out, dstPixels, 0, 0, dstW, dstH);

	g_free(srcPixels);

	gimp_drawable_flush(new_drawable);
	gimp_drawable_merge_shadow(new_drawable->drawable_id, TRUE);
	gimp_drawable_update(new_drawable->drawable_id,
		0, 0,
		dstW, dstH);

	gimp_displays_flush();
	gimp_drawable_detach(new_drawable);
	gimp_drawable_detach(drawable);

	gimp_display_new(new_image_ID);

	g_free(dstPixels);
}

static gboolean
filter_dialog(GimpDrawable *drawable)
{
	GtkWidget *dialog;
	GtkWidget *main_vbox;
	GtkWidget *width_hbox;
	GtkWidget *height_hbox;
	GtkWidget *frame;
	GtkWidget *width_label;
	GtkWidget *height_label;
	GtkWidget *alignmentW;
	GtkWidget *alignmentH;
	GtkWidget *spinbuttonW;
	GtkWidget *spinbuttonH;
	GtkObject *spinbutton_adjW;
	GtkObject *spinbutton_adjH;
	GtkWidget *frame_label;
	gboolean   run;

	gimp_ui_init("Supersample", FALSE);

	dialog = gimp_dialog_new("Supersample", "Supersample",
		NULL, 0,
		gimp_standard_help_func, "plug-in-supersample",

		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_OK,

		NULL);

	main_vbox = gtk_vbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), main_vbox);
	gtk_widget_show(main_vbox);

	// Width

	alignmentW = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_widget_show(alignmentW);
	gtk_container_add(GTK_CONTAINER(main_vbox), alignmentW);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignmentW), 6, 6, 6, 6);

	width_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(width_hbox);
	gtk_container_add(GTK_CONTAINER(alignmentW), width_hbox);

	width_label = gtk_label_new_with_mnemonic("width:");
	gtk_widget_show(width_label);
	gtk_box_pack_start(GTK_BOX(width_hbox), width_label, FALSE, FALSE, 6);
	gtk_label_set_justify(GTK_LABEL(width_label), GTK_JUSTIFY_RIGHT);

	spinbutton_adjW = gtk_adjustment_new(drawable->width, drawable->width, 999999, 1, 1, 1); // HACK
	spinbuttonW = gtk_spin_button_new(GTK_ADJUSTMENT(spinbutton_adjW), 1, 0);
	gtk_widget_show(spinbuttonW);
	gtk_box_pack_start(GTK_BOX(width_hbox), spinbuttonW, FALSE, FALSE, 6);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinbuttonW), TRUE);

	// Height

	alignmentH = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_widget_show(alignmentH);
	gtk_container_add(GTK_CONTAINER(main_vbox), alignmentH);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignmentH), 6, 6, 6, 6);

	height_hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(height_hbox);
	gtk_container_add(GTK_CONTAINER(alignmentH), height_hbox);

	height_label = gtk_label_new_with_mnemonic("height");
	gtk_widget_show(height_label);
	gtk_box_pack_start(GTK_BOX(height_hbox), height_label, FALSE, FALSE, 6);
	gtk_label_set_justify(GTK_LABEL(height_label), GTK_JUSTIFY_RIGHT);

	spinbutton_adjH = gtk_adjustment_new(drawable->height, drawable->height, 999999, 1, 1, 1); // HACK
	spinbuttonH = gtk_spin_button_new(GTK_ADJUSTMENT(spinbutton_adjH), 1, 0);
	gtk_widget_show(spinbuttonH);
	gtk_box_pack_start(GTK_BOX(height_hbox), spinbuttonH, FALSE, FALSE, 6);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinbuttonH), TRUE);

	g_signal_connect(spinbutton_adjW, "value_changed",
		G_CALLBACK(gimp_int_adjustment_update),
		&fvalues.width);
	g_signal_connect(spinbutton_adjH, "value_changed",
		G_CALLBACK(gimp_int_adjustment_update),
		&fvalues.height);
	gtk_widget_show(dialog);

	run = (gimp_dialog_run(GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);

	gtk_widget_destroy(dialog);

	return run;
}