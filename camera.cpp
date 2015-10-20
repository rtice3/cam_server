#include "camera.h"

orchid::camera_list::camera_list() { gp_list_new(&d_list); }

orchid::camera_list::~camera_list() { gp_list_free(d_list); }

int orchid::camera_list::refresh(GPContext* ctx) {
    gp_list_reset(d_list);
    return gp_camera_autodetect(d_list, ctx);
}

int orchid::camera_list::size() { gp_list_count(d_list); }

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

std::unique_ptr<orchid::camera> orchid::camera_list::create_camera(int index, GPContext* ctx) {
    const char* model;
    const char* port;
    int ret, m, p;
    CameraAbilities a;
    GPPortInfo pi;
    Camera* cam;

    gp_list_get_name(d_list, index, &model);
    gp_list_get_value(d_list, index, &port);

    // TODO: fixx all return ints below. 

    if((ret = gp_camera_new(&cam)) < GP_OK)
        return ret;

    if(!d_abilities) {
        if((ret = gp_abilities_list_new(&d_abilities)) < GP_OK)
            return ret;
        if((ret = gp_abilities_list_load(d_abilities, ctx)) < GP_OK)
            return ret;
    }

    if((m = gp_abilities_list_lookup_model(d_abilities, model)) < GP_OK)
        return m;
    if((ret = gp_abilities_list_get_abilities(d_abilities, m, &a)) < GP_OK)
        return ret;
    if((ret = gp_camera_set_abilities(cam, a)) < GP_OK)
        return ret;

    if(!d_pi_list) {
        if((ret = gp_port_info_list_new(&d_pi_list)) < GP_OK)
            return ret;
        if((ret = gp_port_info_list_load(d_pi_list)) < GP_OK)
            return ret;
        if((gp_port_info_list_count(d_pi_list)) < GP_OK)
            return ret;
    }

    if((p = gp_port_info_list_lookup_path(d_pi_list, port)) == GP_ERROR_UNKNOWN_PORT)
        fprintf(stderr, "The port you specified ('%s') cannot be found.\n", port);
    else if(p < GP_OK)
        return p;

    if((ret = gp_port_info_list_get_info(d_pi_list, p, &pi)) < GP_OK)
        return ret;
    if((ret = gp_camera_set_port_info(cam, pi)) < GP_OK)
        return ret;

    return std::move(std::unique_ptr<orchid::camera>(new orchid::camera(cam, ctx)));
}

std::unique_ptr<orchid::camera> orchid::camera_list::create_camera(std::string name, GPContext* ctx) {
    int ret, index;
    if((ret = gp_list_find_by_name(d_list, index, name.c_str())) == GP_OK)
        return this->get_camera(index, ctx);
    return nullptr;
}

orchid::camera::camera(Camera* cam, GPContext* ctx) : d_cam(cam), d_ctx(ctx) { }

orchid::camera::~camera() {
    gp_camera_exit(d_cam, d_ctx);
    gp_camera_free(d_cam);
}

orchid::app::app() {
    d_ctx = gp_context_new();

    gp_context_set_error_func(d_ctx, camera_context::ctx_error, NULL);
    gp_context_set_status_func(d_ctx, camera_context::ctx_status, NULL);
}
orchid::app::~app() {
    gp_context_unref(d_ctx);
}

bool orchid::app::init() {
    d_cam_list.refresh(d_ctx);
}

