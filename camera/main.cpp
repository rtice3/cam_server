#include <iostream>

#include <cgicc/Cgicc.h>

#include "camera.h"


int main(int argc, char** argv) {
	orchid::app app;

	try {
		cgicc::Cgicc cgicc;

		app.init();

		auto env = cgicc.getEnvironment();
		if(env.getRequestMethod() == "GET") {
			// TODO: uhhh?
		}
		else if(env.getRequestMethod == "POST") {
			std::string post_data = env.getPostData();

			if(post_data == "refresh") {
				std::cout << "Content-type:application/json\r\n\r\n";
				std::cout << app.get_tree();
			}
			else {
				// TODO: pass JSON data to set value
			}
		}
		else {
			// TODO: uhhhhh??
		}
	}
	catch(exception& e) {

	}

	return 0;
}
