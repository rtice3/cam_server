#include <memory>
#include <string>
#include <vector>

#include <gphoto2/gphoto2-camera.h>
#include <jsoncpp/dist/json/json.h>

namespace orchid {

	class camera {
	public:
		camera(Camera*, GPContext*);
		~camera();
		Json::Value get_setting_tree();
	private:
		Camera* d_cam;
		GPContext* d_ctx;

		std::string get_camera_summary();
		std::string get_driver_summary();
		Json::Value get_list_data(CameraWidget*);
	};

	class camera_list {
	public:
		camera_list();
		~camera_list();
		int refresh(GPContext*);
		int size();
		std::vector<std::string> get_name_list();
		std::unique_ptr<orchid::camera> create_camera(int, GPContext*);
		std::unique_ptr<orchid::camera> create_camera(std::string, GPContext*);
	private:
		CameraList* d_list;
		CameraAbilitiesList* d_abilities;
		GPPortInfoList* d_pi_list;

		static std::unique_ptr<orchid::camera> handle_error(int, const char*, int);
	};

	using camera_ring = std::vector<std::unique_ptr<orchid::camera>>;

	class app {
	public:
		app();
		~app();
		bool init();
	private:
		GPContext* d_ctx;
		orchid::camera_list d_cam_list;
		camera_ring d_cam_ring;

		static void ctx_error(GPContext*, const char*, void*);
		static void ctx_status(GPContext*, const char*, void*);
	};
};
