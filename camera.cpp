#include <cstdio>

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

    if((ret = gp_camera_new(&cam)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);

    if(!d_abilities) {
        if((ret = gp_abilities_list_new(&d_abilities)) < GP_OK)
            return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
        if((ret = gp_abilities_list_load(d_abilities, ctx)) < GP_OK)
            return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    }

    if((m = gp_abilities_list_lookup_model(d_abilities, model)) < GP_OK)
        return orchid::camera_list::handle_error(m, __FILE__, __LINE__);
    if((ret = gp_abilities_list_get_abilities(d_abilities, m, &a)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    if((ret = gp_camera_set_abilities(cam, a)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);

    if(!d_pi_list) {
        if((ret = gp_port_info_list_new(&d_pi_list)) < GP_OK)
            return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
        if((ret = gp_port_info_list_load(d_pi_list)) < GP_OK)
            return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
        if((gp_port_info_list_count(d_pi_list)) < GP_OK)
            return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    }

    if((p = gp_port_info_list_lookup_path(d_pi_list, port)) == GP_ERROR_UNKNOWN_PORT) {
        fprintf(stderr, "The port you specified ('%s') cannot be found.\n", port);
        return orchid::camera_list::handle_error(p, __FILE__, __LINE__);
    }
    else if(p < GP_OK)
        return orchid::camera_list::handle_error(p, __FILE__, __LINE__);

    if((ret = gp_port_info_list_get_info(d_pi_list, p, &pi)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    if((ret = gp_camera_set_port_info(cam, pi)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);

    return std::make_unique<orchid::camera>(cam, ctx);
}

std::unique_ptr<orchid::camera> orchid::camera_list::create_camera(std::string name, GPContext* ctx) {
    int ret, index;
    if((ret = gp_list_find_by_name(d_list, index, name.c_str())) == GP_OK)
        return this->get_camera(index, ctx);
    return nullptr;
}

std::unique_ptr<orchid::camera> orchid::camera_list::handle_error(int error, const char* file, int line) {
    printf("%s:%d ERROR# %d: %s", file, line, error, gp_port_result_as_string(error));
    return nullptr;
}

orchid::camera::camera(Camera* cam, GPContext* ctx) : d_cam(cam), d_ctx(ctx) { 
    gp_camera_new(d_cam);
}

orchid::camera::~camera() {
    gp_camera_exit(d_cam, d_ctx);
    gp_camera_free(d_cam);
}

std::string orchid::camera::get_camera_summary() {
    CameraText text;

    if(gp_camera_get_summary(d_cam, &text, d_ctx) < GP_OK)
        return "No Camera Summary available.";
    return std::string(text.text);
}

std::string orchid::camera::get_driver_summary() {
    CameraText text;

    if(gp_camera_get_about(d_cam, &text, d_ctx) < GP_OK)
        return "No Driver Summary available.";
    return std::string(text.text);
}

Json::Value orchid::camera::get_setting_tree() {
    CameraWidget* widget;
    int num_children;
    const char* parent_name;
    const char* parent_label;

    Json::Value root;

    if(gp_camera_get_config(d_cam, &widget, d_ctx) < GP_OK)
        return root;

    if(gp_widget_get_name(widget, &parent_name) < GP_OK)
        root["name"] = "error";
    else
        root["name"] = std::string(parent_name);

    if(gp_widget_get_label(widget, &parent_label) < GP_OK) 
        root["label"] = "error";
    else
        root["label"] = std::string(parent_label);

    root["summary"] = this->get_camera_summary();
    root["driver"] = this->get_driver_summary();

    num_children = gp_widget_count_children(widget);
    root["number_children"] = Json::Value(num_children);

    for(int i = 0; i < num_children; i++) {
        CameraWidget* child;
        CameraWidgetType* child_type;
        const char* child_name;
        const char* child_label;
        const char* child_info;
        void* child_value;

        if(gp_widget_get_child(widget, i, &child) < GP_OK)
            continue;

        if(gp_widget_get_name(child, &child_name) < GP_OK)
            root[std::to_string(i)]["name"] = "error";
        else
            root[std::to_string(i)]["name"] = std::string(child_name);

        if(gp_widget_get_label(child, &child_label) < GP_OK)
            root[std::to_string(i)]["label"] = "error";
        else
            root[std::to_string(i)]["label"] = std::string(child_label);

        if(gp_widget_get_info(child, &child_info) < GP_OK)
            root[std::to_string(i)]["info"] = "error";
        else
            root[std::to_string(i)]["info"] = std::string(child_info);

        if(gp_wdiget_get_type(child, child_type) < GP_OK)
            continue;

        gp_widget_get_value(child, child_value);

        switch(*child_type) {
            case GP_WIDGET_WINDOW: {
                root[std::to_string(i)]["type"] = "window";
                break;
            }
            case GP_WIDGET_SECTION: {
                root[std::to_string(i)]["type"] = "section";
                break;
            }
            case GP_WIDGET_TEXT: {
                root[std::to_string(i)]["type"] = "text";
                root[std::to_string(i)]["value"] = std::string((const char*)child_value);
                break;
            }
            case GP_WIDGET_RANGE: {
                float min, max, inc;
                root[std::to_string(i)]["type"] = "range";
                if(gp_widget_get_range(child, &min, &max, &inc) < GP_OK)
                    return ret_str;
                root[std::to_string(i)]["min"] = Json::Value(min);
                root[std::to_string(i)]["max"] = Json::Value(max);
                root[std::to_string(i)]["inc"] = Json::Value(inc);
                root[std::to_string(i)]["value"] = Json::Value(float(*child_value));
                break;
            }
            case GP_WIDGET_TOGGLE: {
                root[std::to_string(i)]["type"] = "toggle";
                root[std::to_string(i)]["value"] = Json::Value(int(*child_value));
                break;
            }
            case GP_WIDGET_RADIO: {
                root[std::to_string(i)]["type"] = "radio";
                root[std::to_string(i)]["choices"] = this->get_list_data(child);
                root[std::to_string(i)]["value"] = std::to_string((const char*)child_value);
                break;
            }
            case GP_WIDGET_MENU: {
                root[std::to_string(i)]["type"] = "menu";
                root[std::to_string(i)]["choices"] = this->get_list_data(child);
                root[std::to_string(i)]["value"] = std::to_string((const char*)child_value);
                break;
            }
            case GP_WIDGET_BUTTON: {
                root[std::to_string(i)]["type"] = "button";
                break;
            }
            case GP_WIDGET_DATE: {
                root[std::to_string(i)]["type"] = "date";
                root[std::to_string(i)]["value"] = Json::Value((int)*child_value);
                break;
            }
            default: {
                root[std::to_string(i)]["type"] = "unknown";
                break;
            }
        }
    }
    return root;
}

Json::Value orchid::camera::get_list_data(CameraWidget* child) {
    Json::Value vec;
    int numc = gp_widget_count_choices(child);
    if(numc < GP_OK)
        return root;
    for(int i = 0; i < numc; i++) {
        const char* namec;
        if(gp_widget_get_choice(child, i, &namec) < GP_OK)
            continue;
        vec.append(std::string(namec));
    }
    return vec;
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

