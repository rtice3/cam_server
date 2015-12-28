#ifndef __IMG_H__
#define __IMG_H__

#include <string>

#include <opencv2/opencv.hpp>

namespace orchid {
	class img {
	public:
		img(std::string);
		std::string get_thumb();
		std::string get_img();
		std::string delete_img();
		static std::string to_web(std::string);
		static std::string from_web(std::string);
	private:
		const std::string d_img;
		std::string d_thumb;
	};
};

#endif