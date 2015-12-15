#ifndef __SERVER_H__
#define __SERVER_H__

#include <functional>
#include <string>


#include "camera.h"
#include "cam_exception.h"
#include "mongoose.h"

namespace orchid {
	class server {
	public:
		server(std::string, std::string)
		~server();
		void poll(int);
		void handler(struct mg_connection*, int, struct http_message*);
		static void xmit_txt(struct mg_connection*, std::string);
	private:
		orchid::app d_app;
		struct mg_mgr d_mgr;
		struct mg_connection* d_nc;
		struct mg_serve_http_opts d_http_opts;
	};

	class http {
	public:
		http(std::string, std::function<std::string(Json::Value&)> = nullptr);
		void operator()(struct mg_connection*);
	private:
		Json::Value d_root;
		std::function<std::string(Json::Value&)> d_func;
	};

	class ftp {
	public:
		ftp(std::string, std::string);
		std::string operator()(void);
	private:
		std::string d_server;
		std::string d_fn;
	};
};

#endif