#include <cstdio>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include "server.h"

#define FTP_SERVER   "ftp://green.local"


static void ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
	((orchid::server*)nc->user_data)->handler(nc, ev, (struct http_message*)ev_data);
}

orchid::server::server(std::string root, std::string port) {
	mg_mgr_init(&d_mgr, NULL);
	d_nc = mg_bind(&d_mgr, port.c_str(), ev_handler);

	mg_set_protocol_http_websocket(d_nc);
	s_http_server_opts.document_root = root.c_str();

	d_nc->user_data = (void*)this;
}

orchid::server::~server() {
	mg_mgr_free(&d_mgr);
}

void orchid::server::poll(int rate) {
	for(;;) {
		mg_mgr_poll(&d_mgr, rate);
	}
}

void orchid::server::handler(struct mg_connection* nc, int ev, struct http_message* hm) {
	switch(ev) {
		case MG_EV_HTTP_REQUEST: {
			if(mg_vcmp(&hm->method, "POST") == 0) {
				if(mg_vcmp(&hm->uri, "/val") == 0) {
					orchid::http post(std::string(hm->body.p, hm->body.len), [&](Json::Value& root) {
						std::string ret = "";
						try {
							d_app.set_value(root);
						}
						catch(cam_exception& e) {
							ret = std::string(e.what());
						}
						return ret;
					});
					post(nc);
				}
				else if(mg_vcmp(&hm->uri, "/capture") == 0) {
					orchid::http post(std::string(hm->body.p, hm->body.len), [&](Json::Value& root) {
						std::string ret;
						try {
							ret = d_app.capture(root);
						}
						catch(cam_exception& e) {
							ret = std::string(e.what());
						}
						return ret;
					});
					post(nc);
				}
				else if(mg_vcmp(&hm->uri, "/save_img") == 0) {
					orchid::ftp ftp(FTP_SERVER, std::string(hm->body.p, hm->body.len));
					std::string ret;
					try {
						ret = ftp();
					}
					catch (curlpp::LogicError & e) {
						ret = e.what();
					}
					catch (curlpp::RuntimeError & e) {
						ret = e.what();
					}
					orchid::server::xmit_txt(nc, ret);	
				}
				else if(mg_vcmp(&hm->uri, "/reject_img") == 0) {
					std::string ret = "";
					auto img = std::string(hm->body.p, hm->body.len);
					std::cout << "Deleting: " << img << std::endl;
					if(::remove(img.c_str()))
						ret = "Failed: " + std::to_string(errno);
					orchid::server::xmit_txt(nc, ret);
				}
				else
					break;
			}
			else {
				if(mg_vcmp(&hm->uri, "/refresh.live") == 0) {
					orchid::http get("", [&](Json::Value& root) {
						std::string ret;
						try {
							d_app.init();
							ret = d_app.get_tree();
						}
						catch(cam_exception& e) {
							ret = std::string(e.what());
						}
						return ret;
					});
					get(nc);
				}
				else {
					mg_serve_http(d_nc, hm, d_http_opts);
				}
			}
			break;
		}
		default:
		break;
	}
}

void orchid::server::xmit_txt(struct mg_connection* nc, std::string msg) {
	mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
		"Content-Type: text/plain\r\n\r\n%s",
		msg.size(), msg.c_str());
}

orchid::http::http(std::string st, std::function<std::string(Json::Value&)> func) : d_func(func) {
	Json::Reader reader;
	reader.parse(st, d_root);
}

void orchid::http::operator()(struct mg_connection* nc) {
	orchid::server::xmit_txt(nc, d_func(d_root));
}

orchid::ftp::ftp(std::string server, std::string fn) : d_server(server), d_fn(fn) { }


std::string orchid::ftp::operator()(void) {
	std::cout << "FTPing: " << d_fn << std::endl;

	curlpp::Cleanup cleaner;
	curlpp::Easy req;
	struct stat finfo;

	if(::stat(d_fn.c_str(), &finfo))
		throw std::runtime_error("error stat'ing file: " + d_fn);

	FILE* src = ::fopen(d_fn.c_str(), "rb");

	req.setOpt(new curlpp::Options::Url(FTP_SERVER));
	req.setOpt(new curlpp::Options::Upload(true));
#ifdef DEBUG
	req.setOpt(new curlpp::Options::Verbose(true));
#endif
	req.setOpt(new curlpp::Options::InfileSize(finfo.st_size));
	req.setOpt(new curlpp::Options::ReadData(src));

	req.perform();

	return "";
}
