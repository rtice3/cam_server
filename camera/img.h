#ifndef __IMG_H__
#define __IMG_H__

#include <string>
#include <uuid/uuid.h>

#include <opencv2/opencv.hpp>

static const std::string s_web_root = "../web/";
static const std::string s_tmp_fs = "tmp/";
static const std::string s_img_root = s_web_root + s_tmp_fs;
static const std::string s_img_ext = ".jpg";
static const std::string s_thumb_suffix = "_thumb";

namespace orchid {
	class uuid {
	public:
		uuid();
		std::string to_string();
	private:
		uuid_t d_uuid;
	};

	class img {
	public:
		img(std::string);
		void resize();
		std::string get_thumb_abs();
		std::string get_thumb_web();
		std::string get_img_abs();
		std::string get_img_web();
		std::string get_ftp_name();
		std::string delete_img();
	private:
		const std::string d_serial;
		orchid::uuid d_uuid;

		static std::string web_prefix();
		static std::string abs_prefix();
	};
};

#endif