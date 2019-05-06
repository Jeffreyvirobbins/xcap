//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"

namespace http {
    namespace server3 {

        request_handler::request_handler (const std::string& doc_root) :
            xcap_database_ (doc_root)
        {
        }

        void request_handler::handle_request(const request& req, reply& rep)
        {
            std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                << " uri: " << req.uri 
                << std::endl;

            // Decode url to path.
            std::string request_path;
            if (!url_decode(req.uri, request_path))
            {
                rep = reply::stock_reply(reply::bad_request);
                return;
            }

#if 0
            // Request path must be absolute and not contain "..".
            if (request_path.empty() || request_path[0] != '/'
                    || request_path.find("..") != std::string::npos)
            {
                std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                    << "\nuri: " << req.uri 
                    << std::endl;
                rep = reply::stock_reply(reply::bad_request);
                return;
            }

            // If path ends in slash (i.e. is a directory) then add "index.html".
            if (request_path[request_path.size() - 1] == '/')
            {
                request_path += "index.html";
            }

#endif
            // Determine the file extension.
            std::size_t last_slash_pos = request_path.find_last_of("/");
            std::size_t last_dot_pos = request_path.find_last_of(".");
            std::string extension;
            if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
            {
                extension = request_path.substr(last_dot_pos + 1);
            }

            // Open the file to send back.
#if 0
            std::string full_path = doc_root_ + request_path;
            std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
            if (!is)
            {
                rep = reply::stock_reply(reply::not_found);
                return;
            }
#endif

            // Fill out the reply to be sent to the client.
            rep.status = reply::ok;
#if 0
            char buf[512];
            while (is.read(buf, sizeof(buf)).gcount() > 0)
                rep.content.append(buf, is.gcount());
#endif

            std::string reply_database;
            std::size_t position1 = req.uri.find ("user-config");
            std::size_t position2 = req.uri.find ("group-association");
            std::size_t position3 = req.uri.find ("imsi");
            std::string etag;
            if (position1 != std::string::npos || position2 != std::string::npos)
            {
                // find ClientID
                std::string client_id;
                for (unsigned int i = 0; i < req.headers.size(); i++ ) {
                    if (req.headers[i].name == "ClientID") {
                        client_id = req.headers[i].value;
                        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                            << " client_id: " << client_id
                            << std::endl;
                        break;
                    }
                }

                if (client_id.size() == 0) {
                    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                        << " No ClientID in headers" 
                        << std::endl;
                    rep = reply::stock_reply(reply::not_found);
                    return;
                }

                // find Etag
                // default value is 000000
                // We just copy client's etag and send it back
                // We assume it is a number of string format now
                for (unsigned int i = 0; i < req.headers.size(); i++ ) {
                    if (req.headers[i].name == "Etag") {
                        etag = req.headers[i].value;

                        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                            << " etag: " << etag
                            << std::endl;
                        break;
                    }
                }

                if (etag.size() == 0) {
                    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                        << " No Etag in headers, using default 000000" 
                        << std::endl;
                    etag = "000000";
                }

                // determine client or group
                std::size_t at_position = client_id.find_first_of("@");
                if (at_position == std::string::npos)
                {
                    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                        << " Not a valid ClientID, no imsi found"
                        << std::endl;
                    rep = reply::stock_reply(reply::not_found);
                    return;
                }

                std::string imsi = client_id.substr (0, at_position);
                std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                    << " imsi: " << imsi
                    << std::endl;
                if (position1 != std::string::npos)
                {
                    xcap_database_.get_client_xml (imsi, reply_database);
                }
                else 
                {
                    xcap_database_.get_group_xml (imsi, reply_database);
                }
            } 
            else if (position3 != std::string::npos) // 4g
            {
                std::size_t last_equal_pos = req.uri.find_last_of("=");
                std::string imsi = req.uri.substr (last_equal_pos +1);
                std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                    << " 4G: " << req.uri
                    << std::endl;
                    xcap_database_.get_client_user_addr (imsi, reply_database);
            }
            else
            {
                std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                    << " No user-config or group-association: " << req.uri
                    << std::endl;
                rep = reply::stock_reply(reply::not_found);
                return;
            }

            std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                << " reply: " << reply_database
                << std::endl;
            rep.content.append (reply_database);

            rep.headers.resize(3);
            rep.headers[0].name = "Content-Length";
            rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
            rep.headers[1].name = "Content-Type";
            rep.headers[1].value = mime_types::extension_to_type(extension);
            rep.headers[2].name = "Etag";
            rep.headers[2].value = etag;
        }

        bool request_handler::url_decode(const std::string& in, std::string& out)
        {
            out.clear();
            out.reserve(in.size());
            for (std::size_t i = 0; i < in.size(); ++i)
            {
                if (in[i] == '%')
                {
                    if (i + 3 <= in.size())
                    {
                        int value = 0;
                        std::istringstream is(in.substr(i + 1, 2));
                        if (is >> std::hex >> value)
                        {
                            out += static_cast<char>(value);
                            i += 2;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                else if (in[i] == '+')
                {
                    out += ' ';
                }
                else
                {
                    out += in[i];
                }
            }
            return true;
        }

    } // namespace server3
} // namespace http
