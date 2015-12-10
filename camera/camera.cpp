#include <cstdio>
#include <iostream>

#include "camera.h"


orchid::camera_list::camera_list() { gp_list_new(&d_list); }

orchid::camera_list::~camera_list() { gp_list_free(d_list); }

bool orchid::camera_list::refresh_list(GPContext* ctx) {
    d_cam_ring.erase(d_cam_ring.begin(), d_cam_ring.end());

    if(gp_list_reset(d_list) < GP_OK)
        return false;
    if(gp_camera_autodetect(d_list, ctx) < GP_OK)
        return false;

    d_cam_ring.reserve(this->size());
    if(create_cameras(ctx))
        return true;
    return false;
}

Json::Value orchid::camera_list::get_camera_tree(int index) {
    return d_cam_ring[index]->get_camera_config();
}

Json::Value orchid::camera_list::get_full_tree() {
    Json::Value root, array;
    for(auto i = 0u; i < d_cam_ring.size(); i++)
        array[i] = this->get_camera_tree(i);
    root["cameras"] = array;
    return root;
}

bool orchid::camera_list::set_camera_attribute(Json::Value& val) {
    Json::Value root;
    root["key"] = val["key"];
    root["value"] = val["value"];
    return d_cam_ring[val["index"].asInt()]->set_camera_config(val);
}

int orchid::camera_list::size() { return gp_list_count(d_list); }

bool orchid::camera_list::create_cameras(GPContext* ctx) {
    for(int i = 0; i < this->size(); i++) {
        std::unique_ptr<orchid::camera> temp = std::move(create_camera(i, ctx));
        if(!temp)
            return false;
        else
            d_cam_ring.push_back(std::move(temp));
    }
    return true;
}

std::vector<std::string> orchid::camera_list::get_name_list() {
    std::vector<std::string> ret;
    ret.reserve(this->size());

    for(auto i = 0; i < this->size(); i++) {
        const char* name;
        gp_list_get_name(d_list, i, &name);
        ret.push_back(std::string(name));
    }
    return ret;
}

orchid::unique_cam orchid::camera_list::create_camera(int index, GPContext* ctx) {
    const char* model;
    const char* port;
    int ret, m, p;
    CameraAbilities a;
    GPPortInfo pi;
    Camera* cam;
    CameraAbilitiesList* abilities_list;
    GPPortInfoList* pi_list;

    gp_list_get_name(d_list, index, &model);
    gp_list_get_value(d_list, index, &port);
//    std::cout << "name: " << model << " port: " << port << std::endl;

    if((ret = gp_camera_new(&cam)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);

    if((ret = gp_abilities_list_new(&abilities_list)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    if((ret = gp_abilities_list_load(abilities_list, ctx)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    

    if((m = gp_abilities_list_lookup_model(abilities_list, model)) < GP_OK)
        return orchid::camera_list::handle_error(m, __FILE__, __LINE__);
    if((ret = gp_abilities_list_get_abilities(abilities_list, m, &a)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    if((ret = gp_camera_set_abilities(cam, a)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);

    if((ret = gp_port_info_list_new(&pi_list)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    if((ret = gp_port_info_list_load(pi_list)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    if((gp_port_info_list_count(pi_list)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);

    if((p = gp_port_info_list_lookup_path(pi_list, port)) == GP_ERROR_UNKNOWN_PORT) {
        fprintf(stderr, "The port you specified ('%s') cannot be found.\n", port);
        return orchid::camera_list::handle_error(p, __FILE__, __LINE__);
    }
    else if(p < GP_OK)
        return orchid::camera_list::handle_error(p, __FILE__, __LINE__);

    if((ret = gp_port_info_list_get_info(pi_list, p, &pi)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);
    if((ret = gp_camera_set_port_info(cam, pi)) < GP_OK)
        return orchid::camera_list::handle_error(ret, __FILE__, __LINE__);

    return std::make_unique<orchid::camera>(cam, ctx);
}

orchid::unique_cam orchid::camera_list::create_camera(std::string name, GPContext* ctx) {
    int ret, index;
    auto cname = name.c_str();
    if((ret = gp_list_find_by_name(d_list, &index, cname)) == GP_OK)
        return this->create_camera(index, ctx);
    return nullptr;
}

orchid::unique_cam orchid::camera_list::handle_error(int error, const char* file, int line) {
    printf("%s:%d ERROR# %d: %s", file, line, error, gp_result_as_string(error));
    return nullptr;
}

orchid::camera::camera(Camera* cam, GPContext* ctx) : d_cam(cam), d_ctx(ctx) { 
    gp_camera_new(&d_cam);
}

orchid::camera::~camera() {
    gp_camera_exit(d_cam, d_ctx);
    gp_camera_free(d_cam);
}

std::string orchid::camera::get_camera_summary() {
    CameraText text;

    if(gp_camera_get_summary(d_cam, &text, d_ctx) < GP_OK)
        return "error";
    return std::string(text.text);
}

std::string orchid::camera::get_driver_summary() {
    CameraText text;

    if(gp_camera_get_about(d_cam, &text, d_ctx) < GP_OK)
        return "error";
    return std::string(text.text);
}

bool orchid::camera::set_camera_config(Json::Value& root) {
    CameraWidget* child = nullptr;
    CameraWidgetType* type = nullptr;

    if(gp_widget_get_child_by_name(d_widget, root["key"].asCString(), &child) < GP_OK)
        return false;

    if(gp_widget_get_type(child, type) < GP_OK)
        return false;

    switch(*type) {
        case GP_WIDGET_MENU:
        case GP_WIDGET_RADIO:
        case GP_WIDGET_TEXT: {
            auto temp = root["value"].asCString();
            if(gp_widget_set_value(child, &temp) < GP_OK)
                return false;
            return true;
        }
        case GP_WIDGET_RANGE: {
            auto temp = root["value"].asFloat();
            if(gp_widget_set_value(child, &temp) < GP_OK)
                return false;
            return true;
        }
        case GP_WIDGET_DATE:
        case GP_WIDGET_TOGGLE: {
            auto temp = root["value"].asInt();
            if(gp_widget_set_value(child, &temp) < GP_OK)
                return false;
            return true;
        }
        default: {
            return false;
        }
    }
    return false;
}

Json::Value orchid::camera::get_child(CameraWidget* parent, int num_children) {
    Json::Value root;
    for(auto i = 0; i < num_children; i++) {
        CameraWidget* child = nullptr;
        CameraWidgetType child_type;
        const char* child_name;
        const char* child_label;
        const char* child_info;

        if(gp_widget_get_child(parent, i, &child) < GP_OK)
            continue;

        if(gp_widget_get_name(child, &child_name) < GP_OK)
            root[i]["name"] = "error";
        else
            root[i]["name"] = std::string(child_name);

        if(gp_widget_get_label(child, &child_label) < GP_OK)
            root[i]["label"] = "error";
        else
            root[i]["label"] = std::string(child_label);

        if(gp_widget_get_info(child, &child_info) < GP_OK)
            root[i]["info"] = "error";
        else
            root[i]["info"] = std::string(child_info);

        gp_widget_get_type(child, &child_type);

        switch(child_type) {
            case GP_WIDGET_WINDOW: {
                root[i]["type"] = "window";
                break;
            }
            case GP_WIDGET_SECTION: {
                root[i]["type"] = "section";
                break;
            }
            case GP_WIDGET_TEXT: {
                const char* value;
                root[i]["type"] = "text";
                gp_widget_get_value(child, &value);
                root[i]["value"] = Json::Value(value);
                break;
            }
            case GP_WIDGET_RANGE: {
                float min, max, inc, value;
                gp_widget_get_value(child, &value);
                root[i]["type"] = "range";
                if(gp_widget_get_range(child, &min, &max, &inc) < GP_OK)
                    break;
                root[i]["min"] = Json::Value(min);
                root[i]["max"] = Json::Value(max);
                root[i]["inc"] = Json::Value(inc);
                root[i]["value"] = Json::Value(value);
                break;
            }
            case GP_WIDGET_TOGGLE: {
                int value;
                gp_widget_get_value(child, &value);
                root[i]["type"] = "toggle";
                root[i]["value"] = Json::Value(value);
                break;
            }
            case GP_WIDGET_RADIO: {
                const char* value;
                gp_widget_get_value(child, &value);
                root[i]["type"] = "radio";
                root[i]["choices"] = this->get_list_data(child);
                root[i]["value"] = Json::Value(value);
                break;
            }
            case GP_WIDGET_MENU: {
                const char* value;
                gp_widget_get_value(child, &value);
                root[i]["type"] = "menu";
                root[i]["choices"] = this->get_list_data(child);
                root[i]["value"] = Json::Value(value);
                break;
            }
            case GP_WIDGET_BUTTON: {
                root[i]["type"] = "button";
                break;
            }
            case GP_WIDGET_DATE: {
                int value;
                gp_widget_get_value(child, &value);
                root[i]["type"] = "date";
                root[i]["value"] = Json::Value(value);
                break;
            }
            default: {
                root[i]["type"] = "unknown";
                break;
            }
        }
        int sub_children = gp_widget_count_children(child);
        if(sub_children > 0)
            root[i]["zchildren"] = this->get_child(child, sub_children);
    }
    return root;
}

Json::Value orchid::camera::get_camera_config() {
    int num_children;
    const char* parent_name;
    const char* parent_label;

    Json::Value root;

    if(gp_camera_get_config(d_cam, &d_widget, d_ctx) < GP_OK)
        return root;

    if(gp_widget_get_name(d_widget, &parent_name) < GP_OK)
        root["name"] = "error";
    else
        root["name"] = std::string(parent_name);

    if(gp_widget_get_label(d_widget, &parent_label) < GP_OK) 
        root["label"] = "error";
    else
        root["label"] = std::string(parent_label);

    root["summary"] = this->get_camera_summary();
    root["driver"] = this->get_driver_summary();

    num_children = gp_widget_count_children(d_widget);
    root["zchildren"] = this->get_child(d_widget, num_children);

    return root;
}

Json::Value orchid::camera::get_list_data(CameraWidget* child) {
    Json::Value vec;
    int numc = gp_widget_count_choices(child);
    if(numc < GP_OK)
        return vec;
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

    gp_context_set_error_func(d_ctx, orchid::app::ctx_error, NULL);
    gp_context_set_status_func(d_ctx, orchid::app::ctx_status, NULL);
}
orchid::app::~app() {
    gp_context_unref(d_ctx);
}

bool orchid::app::init() {
    return d_cam_list.refresh_list(d_ctx);
}

std::string orchid::app::get_tree() {
#ifdef DEBUG
    Json::StyledWriter writer;
#else
    Json::FastWriter writer;
#endif
    return writer.write(d_cam_list.get_full_tree());
}

bool orchid::app::set_value(Json::Value& val) {
    return d_cam_list.set_camera_attribute(val);
}

void orchid::app::ctx_error(GPContext *context, const char *str, void *data) {
        fprintf  (stderr, "\n*** Contexterror ***              \n%s\n",str);
        fflush   (stderr);
}

void orchid::app::ctx_status(GPContext *context, const char *str, void *data) {
        fprintf  (stderr, "%s\n", str);
        fflush   (stderr);
}