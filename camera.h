#include <string>

#include <gphoto2/gphoto2-camera.h>

class camera_list {
public:
	camera_list();
	~camera_list();
	int populate();
	int size();
	std::vector<std::string> get_name_list();
	Camera* get_camera(int);
	Camera* get_camera(std::string);
private:
	CameraList* d_list;
};

class camera {
public:
	camera();
	~camera();
	int init();
private:
	GPContext* d_ctx;
	CameraList* d_cam_list;
	Camera* d_cam;

	static void ctx_error(GPContext*, const char*, void*);
	static void ctx_status(GPContext*, const char*, void*);
};
