#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "server.h"


int main(int argc, char** argv) {
   orchid::server server("../web", "8000");
   server.poll(1000);

	return 0;
}
