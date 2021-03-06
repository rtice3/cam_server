#include <cstdio>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>

#include "server.h"

#define FTP_SERVER   "IGBT.local:2121"

static const char* s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

static void ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
	((orchid::server*)nc->user_data)->handler(nc, ev, (struct http_message*)ev_data);
}

orchid::server::server() {
	mg_mgr_init(&d_mgr, NULL);
	d_nc = mg_bind(&d_mgr, s_http_port, ev_handler);

	mg_set_protocol_http_websocket(d_nc);
	s_http_server_opts.document_root = s_web_root;

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
							if(!d_lastimg)
								d_lastimg = d_app.capture(root);
							ret = d_lastimg->get_thumb_web();
						}
						catch(cam_exception& e) {
							ret = std::string(e.what());
						}
						return ret;
					});
					post(nc);
				}
				else if(mg_vcmp(&hm->uri, "/save_img") == 0) {
					std::unique_ptr<ftplib> ftp = std::make_unique<ftplib>();
					std::string ret = "";
					auto thumb = std::string(hm->body.p, hm->body.len);
					if(!d_lastimg) {
						ret = "No img on deck.";
						orchid::server::xmit_txt(nc, ret);
						break;
					}
					if(thumb != d_lastimg->get_thumb_web()) {
						ret = "Img ID doesn't match img on deck.";
						orchid::server::xmit_txt(nc, ret);
						break;
					}
					if(ftp->Connect(FTP_SERVER) == 0) {
						ret = "FTP connect failed.";
						orchid::server::xmit_txt(nc, ret);
						break;
					}
					if(ftp->Login("pi", "raspberry") == 0) {
						ret = "FTP login failed.";
						orchid::server::xmit_txt(nc, ret);
						break;
					}
					std::cout << "FTPing: " << d_lastimg->get_img_abs() << " as " << d_lastimg->get_ftp_name() << std::endl;
					if(ftp->Put(d_lastimg->get_img_abs().c_str(), d_lastimg->get_ftp_name().c_str(), ftplib::image) == 0) {
						ret = "FTP error";
						orchid::server::xmit_txt(nc, ret);
						ftp->Quit();
						break;
					}
					d_lastimg->delete_img();
					d_lastimg.reset(nullptr);

					orchid::server::xmit_txt(nc, ret);
					ftp->Quit();
				}
				else if(mg_vcmp(&hm->uri, "/reject_img") == 0) {
					std::string ret = "";
					auto thumb = std::string(hm->body.p, hm->body.len);
					if(d_lastimg) {
						if(thumb != d_lastimg->get_thumb_web())) 
							ret = "Img ID doesn't match img on deck.";
						else {
							std::cout << "Deleting: " << d_lastimg->get_img_abs() << std::endl;
							ret = d_lastimg->delete_img();
							d_lastimg.reset(nullptr);
						}
					}

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
					mg_serve_http(nc, hm, s_http_server_opts);
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
