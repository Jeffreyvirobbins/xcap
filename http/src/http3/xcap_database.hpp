#ifndef XCAP_DATABASE_HPP
#define XCAP_DATABASE_HPP

#include <iostream>
#include <mysql/mysql.h>

class Xcap_database
{
    private:
        std::string config_root_;
        MYSQL *section1_connection_;

    public:

        Xcap_database (const std::string &config_root);

        int get_client_user_addr (const std::string &json_input, std::string &output);
        int get_client_xml (const std::string &json_input, std::string &output);
        int get_group_xml (const std::string &json_input, std::string &output);

        ~Xcap_database();
};


#endif 
