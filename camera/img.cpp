#include <cstdio>
#include <errno.h>

#include "cam_exception.h"
#include "img.h"


orchid::img::img(std::string path) : d_img(path) {
	cv::Mat img, thumb;
	d_thumb = d_img;

	d_thumb.insert(d_img.size() - 4, "_thumb");
	std::cout << "d_thumb: " << d_thumb << std::endl;

    img = cv::imread(d_img, CV_LOAD_IMAGE_COLOR);

    if(!img.data)
    	throw cam_except("Could not open image: " + path);

    cv::resize(img, thumb, cv::Size(), 0.1, 0.1);
    cv::imwrite(d_thumb, thumb);
}

std::string orchid::img::get_thumb() { return d_thumb; }

std::string orchid::img::get_img() { return d_img; }

std::string orchid::img::delete_img() {
	std::string ret = "";

	if(::remove(d_thumb.c_str())) {
		ret = "Failed to delete " + d_thumb + ": " + std::to_string(errno);
		return ret;
	}
	if(::remove(d_img.c_str())) {
		ret = "Failed to delete " + d_img + ": " + std::to_string(errno);
		return ret;
	}
	return ret;
}

std::string orchid::img::to_web(std::string path) { return path.erase(0, 7); }

std::string orchid::img::from_web(std::string path) { return std::string("../web/" + path); }