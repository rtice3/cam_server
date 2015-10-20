#include <gphoto2/gphoto2-camera.h>

static GPPortInfoList* portinfolist = NULL;
static CameraAbilitiesList* abilities = NULL;

int autodetect(CameraList* list, GPContext* context) {
	gp_list_reset(list);
	return gp_camera_autodetect(list, context);
}

int open_camera(Camera** camera, const char* model, const char* port, GPContext* context) {
	int ret, m, p;
	CameraAbilities a;
	GPPortInfo pi;

	ret = gp_camera_new(camera);
	if (ret < GP_OK) 
		return ret;

	if (!abilities) {
		/* Load all the camera drivers we have... */
		ret = gp_abilities_list_new (&abilities);
		if (ret < GP_OK) 
			return ret;
		ret = gp_abilities_list_load (abilities, context);
		if (ret < GP_OK) 
			return ret;
	}

	/* First lookup the model / driver */
   	m = gp_abilities_list_lookup_model (abilities, model);
	if (m < GP_OK) 
		return ret;
  	ret = gp_abilities_list_get_abilities (abilities, m, &a);
	if (ret < GP_OK) 
		return ret;
 	ret = gp_camera_set_abilities (*camera, a);
	if (ret < GP_OK) 
		return ret;

	if (!portinfolist) {
		/* Load all the port drivers we have... */
		ret = gp_port_info_list_new (&portinfolist);
		if (ret < GP_OK) 
			return ret;
		ret = gp_port_info_list_load (portinfolist);
		if (ret < 0) 
			return ret;
		ret = gp_port_info_list_count (portinfolist);
		if (ret < 0) 
			return ret;
	}

	/* Then associate the camera with the specified port */
    p = gp_port_info_list_lookup_path (portinfolist, port);
    switch (p) {
    case GP_ERROR_UNKNOWN_PORT:
            fprintf (stderr, "The port you specified "
                    "('%s') can not be found. Please "
                    "specify one of the ports found by "
                    "'gphoto2 --list-ports' and make "
                    "sure the spelling is correct "
                    "(i.e. with prefix 'serial:' or 'usb:').",
                            port);
            break;
    default:
            break;
    }
    if (p < GP_OK) 
    	return p;

    ret = gp_port_info_list_get_info (portinfolist, p, &pi);
    if (ret < GP_OK) 
    	return ret;
    ret = gp_camera_set_port_info (*camera, pi);
    if (ret < GP_OK) 
    	return ret;
	return GP_OK;
}


int main(int argc char** argc) {
	Camera		*camera;
	int		ret;
	char		*owner;
	GPContext	*context;
	CameraText	text;

	context = sample_create_context(); /* see context.c */
	gp_camera_new (&camera);

	/* This call will autodetect cameras, take the
	 * first one from the list and use it. It will ignore
	 * any others... See the *multi* examples on how to
	 * detect and use more than the first one.
	 */
	ret = gp_camera_init (camera, context);
	if (ret < GP_OK) {
		printf("No camera auto detected.\n");
		gp_camera_free (camera);
		return 0;
	}

	/* Simple query the camera summary text */
	ret = gp_camera_get_summary (camera, &text, context);
	if (ret < GP_OK) {
		printf("Camera failed retrieving summary.\n");
		gp_camera_free (camera);
		return 0;
	}
	printf("Summary:\n%s\n", text.text);

	/* Simple query of a string configuration variable. */
	ret = get_config_value_string (camera, "owner", &owner, context);
	if (ret >= GP_OK) {
		printf("Owner: %s\n", owner);
		free (owner);
	}
	gp_camera_exit (camera, context);
	gp_camera_free (camera);
	gp_context_unref (context);
	return 0;
}
