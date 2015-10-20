#include <memory>
#include <string>
#include <vector>

#include <gphoto2/gphoto2-camera.h>

namespace orchid {

	class camera {
	public:
		camera(Camera*, GPContext*);
		~camera();
	private:
		Camera* d_cam;
		GPContext* d_ctx;
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
