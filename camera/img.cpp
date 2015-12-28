#include <cstdio>
#include <errno.h>

#include "cam_exception.h"
#include "img.h"

orchid::uuid::uuid() { ::uuid_generate(d_uuid); }

std::string orchid::uuid::to_string() {
	char str[37];
	::uuid_unparse(d_uuid, str);
	return std::string(str);
}

orchid::img::img(std::string serial) : d_serial(serial) { }

void orchid::img::resize() {
	cv::Mat img, thumb;
    img = cv::imread(d_img_path, CV_LOAD_IMAGE_COLOR);

    if(!img.data)
    	throw cam_except("Could not open image: " + d_img_path);

    cv::resize(img, thumb, cv::Size(), 0.1, 0.1);
    cv::imwrite(d_thumb_path, thumb);
}

std::string orchid::img::get_thumb_abs() { return orchid::img::abs_prefix + d_uuid.to_string() + s_thumb_suffix + s_img_ext; }

std::string orchid::img::get_thumb_web() { return orchid::img::web_prefix + d_uuid.to_string() + s_thumb_suffix + s_img_ext; }

std::string orchid::img::get_img_abs() { return orchid::img::abs_prefix + d_uuid.to_string() + s_img_ext; }

std::string orchid::img::get_img_web() { return orchid::img::web_prefix + d_uuid.to_string() + s_img_ext; }

std::string orchid::img::get_ftp_name() { return d_serial + s_img_ext; }

std::string orchid::img::delete_img() {
	std::string ret = "";

	if(::remove(get_thumb_abs().c_str())) {
		ret = "Failed to delete " + get_thumb_abs() + ": " + std::to_string(errno);
		return ret;
	}
	if(::remove(get_img_abs().c_str())) {
		ret = "Failed to delete " + get_img_abs() + ": " + std::to_string(errno);
		return ret;
	}
	return ret;
}

std::string orchid::img::web_prefix() { return s_tmp_fs; }

std::string orchid::img::abs_prefix() { return s_img_root; }