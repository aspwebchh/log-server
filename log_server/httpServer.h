#pragma once

#include <iostream>
#include <string>
#include <sstream>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <Winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "commons.h"
#include <functional>
#include <vector>
#include <map>
#include <string>

using namespace std;

namespace httpServer {

    map<string, function<string(map<string, string>)>> httpServerGetHandler;


    class HttpServer {
    private:
        static void queryString2Map(const string& queryString, map<string, string>& map) {
            if (queryString == "") {
                return;
            }
            vector<string> paramItems;
            split(queryString, paramItems, "&");
            if (paramItems.size() == 0) {
                return;
            }
            for (auto paramItem : paramItems) {
                vector<string> keyAndValue;
                split(paramItem, keyAndValue, "=");
                if (keyAndValue.size() < 2) {
                    continue;
                }
                map.insert(pair<string, string>(keyAndValue[0], keyAndValue[1]));
            }
        }

        static void generic_handler(struct evhttp_request* req, void* arg)
        {
            struct evbuffer* buf = evbuffer_new();
            if (!buf)
            {
                cout << "failed to create response buffer" << endl;
                return;
            }

            string url = evhttp_request_get_uri(req);
           // string decodeUrl = UTF8ToGBEx(evhttp_decode_uri(url.c_str()));
            string decodeUrl = evhttp_decode_uri(url.c_str());

            vector<string> items;
            split(decodeUrl, items, "?");

            string path;
            string query;
            if (items.size() >= 1) {
                path = items[0];
            }
            if (items.size() >= 2) {
                query = items[1];
            }

            string responseText;
            if (httpServerGetHandler.count(path) >= 1) {
                map<string, string> params;
                auto handler = httpServerGetHandler[path];
                queryString2Map(query, params);
                responseText = handler(params);
            }
            
            evbuffer_add_printf(buf, responseText.c_str());
            evhttp_send_reply(req, HTTP_OK, "OK", buf);
            evbuffer_free(buf);
        }


        int init_win_socket()
        {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            {
                return -1;
            }
            return 0;
        }



    public:
        int server( int httpPort) {
#ifdef WIN32
            init_win_socket();
#endif

            short  http_port = httpPort;
            string http_addr = "127.0.0.1";

            struct event_base* base = event_base_new();

            struct evhttp* http_server = evhttp_new(base);
            if (!http_server)
            {
                return -1;
            }

            int ret = evhttp_bind_socket(http_server, http_addr.c_str(), http_port);
            if (ret != 0)
            {
                return -1;
            }

            evhttp_set_gencb(http_server, HttpServer::generic_handler, NULL);

            cout << "http server start OK! port=" + to_string( http_port ) << endl;;

            event_base_dispatch(base);

            evhttp_free(http_server);

#ifdef WIN32
            WSACleanup();
#endif

            return 0;
        };


        void get(const string& path, function<string(map<string, string>)> handler) {
            httpServerGetHandler.insert(make_pair(path, handler));
        };
    };

}
