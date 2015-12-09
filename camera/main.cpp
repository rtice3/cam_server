#include "camera.h"
#include "mongoose.h"

static orchid::app app;
static const char* s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

static void ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
   struct http_message* hm = (struct http_message*)ev_data;

   switch(ev) {
      case MG_EV_HTTP_REQUEST: {
         if(mg_vcmp(&hm->method, "POST") == 0) {
            if(mg_vcmp(&hm->uri, "/val") == 0) {
               std::string ret;
               if(app.set_value(std::string(hm->body.p, hm->body.len)))
                  ret = "true";
               else
                  ret = "false";
               mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                   "Content-Type: text/plain\r\n\r\n%s",
                   ret.size(), ret.c_str());
            }
            else
               break;
         }
         else {
            if(mg_vcmp(&hm->uri, "refresh.live") == 0) {
               auto ret = app.get_tree();
               mg_printf(nc, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n"
                   "Content-Type: application/json\r\n\r\n%s",
                   ret.size(), ret.c_str());
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


int main(int argc, char** argv) {
	struct mg_mgr mgr;
   struct mg_connection* nc;

	try {
		app.init();

      mg_mgr_init(&mgr, NULL);
      nc = mg_bind(&mgr, s_http_port, ev_handler);

      mg_set_protocol_http_websocket(nc);
      s_http_server_opts.document_root = "../web";

      for(;;) {
         mg_mgr_poll(&mgr, 1000);
      }

      mg_mgr_free(&mgr);

	}
	catch(cam_exception& e) {
      std::cout << e.what() << std::endl;
      exit(1);
	}

	return 0;
}
