#include "camera.h"

camera_list::camera_list() { gp_list_new(&d_list); }
camera_list::~camera_list() { gp_list_free(d_list); }
int camera_list::populate(GPContext* ctx) {
        gp_list_reset(d_list);
        return gp_camera_autodetect(d_list, ctx);
}
int camera_list::size() { gp_list_count(d_list); }
std::vector<std::string> get_name_list() {
        std::vector<std::string> ret;
        ret.reserve(this->size());

        for(int i = 0; i < this->size(); i++) {
                const char* name;
                gp_list_get_name(d_list, i, name);
                ret.push_back(std::string(name));
        }
        return ret;
}
Camera* get_camera(int index) {

}
Camera* get_camera(std::string name) {
        
}

camera::camera() {
        d_ctx = gp_context_new();
        gp_context_set_error_func(d_ctx, camera_context::ctx_error, NULL);
        gp_context_set_status_func(d_ctx, camera_context::ctx_status, NULL);

        gp_list_new(&d_cam_list);


        gp_camera_new (&d_cam);
}

camera::~camera() {
        gp_list_free(d_cam_list);
        gp_camera_exit(d_cam, d_ctx);
        gp_camera_free(d_cam);
        gp_context_unref(d_ctx);
}

int camera::init() {
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
}



static void camera::ctx_error(GPContext *context, const char *str, void *data) {
        fprintf(stderr, "\n*** Contexterror ***              \n%s\n",str);
        fflush(stderr);
}

static void camera::ctx_status(GPContext *context, const char *str, void *data) {
        fprintf(stderr, "%s\n", str);
        fflush(stderr);
}

