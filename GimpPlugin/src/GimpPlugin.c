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
	1920, 1080, LANCZOS, FALSE
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
	GimpPDBStatusType status = GIMP_PDB_SUCCESS;
	GimpRunMode       run_mode = param[0].data.d_int32;

	static GimpParam values[1];
	values[0].type = GIMP_PDB_STATUS;
	values[0].data.d_status = status;

	// Setting mandatory output values 
	*nreturn_vals = 1;
	*return_vals = values;

	// Get the specified drawable : layers, masks, etc...
	GimpDrawable* drawable = gimp_drawable_get(param[2].data.d_drawable);

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
	case GIMP_RUN_INTERACTIVE: // run from the GUI
		// Get options' last values if needed
		gimp_get_data("plug-in-supersample", &fvalues);

		// Display the dialog
		if (!filter_dialog(drawable)) {
			return;
		}
		gimp_set_data("plug-in-supersample", &fvalues, sizeof(FilterValues));
		break;

		/*case GIMP_RUN_NONINTERACTIVE: // run from a script (parameters are specified in *param)
			if (nparams != 7)
			status = GIMP_PDB_CALLING_ERROR;
			if (status == GIMP_PDB_SUCCESS) {
			fvalues.width = param[3].data.d_int32;
			fvalues.height = param[4].data.d_int32;
			fvalues.interpolation = param[5].data.d_int32;
			fvalues.adjust = param[6].data.d_int32;
			}
			break;*/

	case GIMP_RUN_WITH_LAST_VALS: // run from "Repeat Last"
		// Get options' last values if needed
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

	gint margin = 6;

	gimp_ui_init("SuperSample", FALSE);

	GtkWidget* dialog = gimp_dialog_new("SuperSample", "SuperSample", NULL, 0, gimp_standard_help_func,
		"plug-in-supersample", GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

	GtkWidget* main_vbox = gtk_vbox_new(FALSE, margin);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), main_vbox);
	gtk_widget_show(main_vbox);

	GtkWidget* frame = gtk_frame_new(NULL); // main frame
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(main_vbox), frame, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), margin);

	GtkWidget* frame_label = gtk_label_new("<b>Desired Size:</b>");
	gtk_widget_show(frame_label);
	gtk_frame_set_label_widget(GTK_FRAME(frame), frame_label);
	gtk_label_set_use_markup(GTK_LABEL(frame_label), TRUE);

	GtkWidget* alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_widget_show(alignment);
	gtk_container_add(GTK_CONTAINER(frame), alignment);
	gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), margin, margin, margin, margin);

	GtkWidget* vbox = gtk_vbox_new(FALSE, 0); // list of options
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(alignment), vbox);

	// width
	{
		GtkWidget* width_hbox = gtk_hbox_new(FALSE, 0);
		gtk_widget_show(width_hbox);
		gtk_container_add(GTK_CONTAINER(vbox), width_hbox);

		GtkObject* width_adj = gtk_adjustment_new(drawable->width, 0, G_MAXDOUBLE, 1, 32, 32);
		g_signal_connect(width_adj, "value_changed", G_CALLBACK(gimp_int_adjustment_update), &fvalues.width);

		GtkWidget* width_button = gtk_spin_button_new(GTK_ADJUSTMENT(width_adj), 1, 0);
		gtk_widget_show(width_button);
		gtk_box_pack_start(GTK_BOX(width_hbox), width_button, FALSE, FALSE, margin);
		gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(width_button), TRUE);

		GtkWidget* width_label = gtk_label_new_with_mnemonic("(px) Width");
		gtk_widget_show(width_label);
		gtk_box_pack_start(GTK_BOX(width_hbox), width_label, FALSE, FALSE, margin);
		gtk_label_set_justify(GTK_LABEL(width_label), GTK_JUSTIFY_RIGHT);
	}

	// height
	{
		GtkWidget* height_hbox = gtk_hbox_new(FALSE, 0);
		gtk_widget_show(height_hbox);
		gtk_container_add(GTK_CONTAINER(vbox), height_hbox);

		GtkObject* height_adj = gtk_adjustment_new(drawable->width, 0, G_MAXDOUBLE, 1, 32, 32);
		g_signal_connect(height_adj, "value_changed", G_CALLBACK(gimp_int_adjustment_update), &fvalues.height);

		GtkWidget* height_button = gtk_spin_button_new(GTK_ADJUSTMENT(height_adj), 1, 0);
		gtk_widget_show(height_button);
		gtk_box_pack_start(GTK_BOX(height_hbox), height_button, FALSE, FALSE, margin);
		gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(height_button), TRUE);

		GtkWidget* height_label = gtk_label_new_with_mnemonic("(px) Height");
		gtk_widget_show(height_label);
		gtk_box_pack_start(GTK_BOX(height_hbox), height_label, FALSE, FALSE, margin);
		gtk_label_set_justify(GTK_LABEL(height_label), GTK_JUSTIFY_RIGHT);
	}

	// adjust checkBox
	GtkWidget* adjust_box = gtk_check_button_new_with_label("Adjust");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adjust_box), fvalues.adjust);
	gtk_widget_show(adjust_box);
	gtk_container_add(GTK_CONTAINER(vbox), adjust_box);

	// interpolation selector
	GtkWidget* interp_combo = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(interp_combo), "Cubic (Cleaner)");
	gtk_combo_box_append_text(GTK_COMBO_BOX(interp_combo), "Lanczos (Details)");
	gtk_combo_box_set_active(GTK_COMBO_BOX(interp_combo), fvalues.interpolation);
	gtk_widget_show(interp_combo);
	gtk_container_add(GTK_CONTAINER(vbox), interp_combo);

	// finally displaying the dialog and exiting
	gtk_widget_show(dialog);
	gboolean run = (gimp_dialog_run(GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK);
	fvalues.adjust = GTK_TOGGLE_BUTTON(adjust_box)->active;
	fvalues.interpolation = gtk_combo_box_get_active(GTK_COMBO_BOX(interp_combo));
	gtk_widget_destroy(dialog);

	return run;
}