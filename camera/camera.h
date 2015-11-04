#include <memory>
#include <string>
#include <vector>

#include <gphoto2/gphoto2-camera.h>
#include <json/json.h>

namespace orchid {

	class camera {
	public:
		camera(Camera*, GPContext*);
		~camera();

		Json::Value get_camera_config();
		bool set_camera_config(Json::Value);
	private:
		Camera* d_cam;
		CameraWidget* d_widget;
		GPContext* d_ctx;

		Json::Value get_child(CameraWidget*, int);
		std::string get_camera_summary();
		std::string get_driver_summary();
		Json::Value get_list_data(CameraWidget*);
	};

	class camera_list {
	public:
		camera_list();
		~camera_list();

		bool refresh_list(GPContext*);
		Json::Value get_camera_tree(int);
		Json::Value get_full_tree();
		bool set_camera_attribute(int, Json::Value);
	private:
		CameraList* d_list;
		std::vector<std::unique_ptr<orchid::camera>> d_cam_ring;

		int size();
		std::vector<std::string> get_name_list();
		bool create_cameras(GPContext*);
		std::unique_ptr<orchid::camera> create_camera(int, GPContext*);
		std::unique_ptr<orchid::camera> create_camera(std::string, GPContext*);

		static std::unique_ptr<orchid::camera> handle_error(int, const char*, int);
	};

	using camera_ring = std::vector<std::unique_ptr<orchid::camera>>;

	class app {
	public:
		app();
		~app();

		bool init();
		std::string get_tree();
		bool set_value(int, Json::Value);
	private:
		GPContext* d_ctx;
		orchid::camera_list d_cam_list;

		static void ctx_error(GPContext*, const char*, void*);
		static void ctx_status(GPContext*, const char*, void*);
	};
};
