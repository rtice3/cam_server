#include <iostream>

#include "camera.h"

#define DEBUG	1


int main(int argc char** argc) {
	orchid::app app();

	app.init();

	std::cout << app.get_tree();

	return 0;
}
