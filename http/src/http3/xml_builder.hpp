#ifndef XML_BUILDER_HPP
#define XML_BUILDER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <mysql/mysql.h>

class Xml_builder
{
    private:
        std::string config_root_;
        MYSQL *section2_connection_;

    public:

        Xml_builder();
        Xml_builder(const std::string &config_root);

        int get_client_xml (const std::string &input, std::string &xml_input);
        int get_group_xml (const std::string &input, std::string &xml_input);

        ~Xml_builder();
};

#endif 
