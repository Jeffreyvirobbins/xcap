#include "xml_builder.hpp"
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/beast/core/detail/base64.hpp>

const static char *opt_socket_name = NULL; /* SOCKET NAME, DO NOT CHANGE */
const static unsigned int opt_flags = 0; /* CONNECTION FLAGS, DO NOT CHANGE */

// for base64_encode and base64_decode
using namespace boost::beast::detail;


Xml_builder::Xml_builder() :
    section2_connection_(NULL) 
{
}

 
Xml_builder::Xml_builder(const std::string &config_root) :
    config_root_ (config_root), section2_connection_(NULL) 
{
    // section2
    section2_connection_ = mysql_init(NULL);
    if (section2_connection_ == NULL)
    {
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << "\n Error initializing databases connection!"
            << std::endl;
        return;
    }

    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(config_root_ + "/config.ini", pt);
    std::string section2_host_name = pt.get<std::string>("section2.section2_host_name");
    std::string section2_user_name = pt.get<std::string>("section2.section2_user_name");
    std::string section2_password = pt.get<std::string>("section2.section2_password");
    std::string section2_database_name = pt.get<std::string>("section2.section2_database_name");
    int section2_port_num = pt.get<int>("section2.section2_port_num");

    MYSQL *mysql = mysql_real_connect (
            section2_connection_, 
            section2_host_name.c_str(),
            section2_user_name.c_str(), 
            section2_password.c_str(),
            section2_database_name.c_str(),
            section2_port_num,
            opt_socket_name, 
            opt_flags
            );

    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
        << " configiguration file: " << config_root_ + "/config.ini"
        << ", section2_host_name: " << section2_host_name
        << ", section2_user_name: " << section2_user_name
        << ", section2_password:" << section2_password
        << ", section2_database_name:" << section2_database_name
        << std::endl;

    if (mysql == 0)
    {
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << " Not successfully initializing section2_databases connection!"
            << ", check your configruation file!"
            << std::endl;
        exit (1);
    }

}


/**
 *
 */
int Xml_builder::get_client_xml (const std::string &input, std::string &xml_output) 
{

    xml_output = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
                  <user-config xmlns=\"urn:alcatel-lucent:xml:xdm:user-config\"> \
                  <user-address> <address>";
    xml_output += input;
    xml_output += "@ptt.189.cn</address> </user-address> \
                   <user-capability> <call-type-capability>03ff</call-type-capability> \
                   <add-mem-disable-privilege>false</add-mem-disable-privilege> \
                   <talk-group-permission>false</talk-group-permission> \
                   </user-capability> \
                   </user-config>";
    return 0;
}


/**
 * Input:
 * null
 * empty
 * {"groups":[]} 
 * {"groups":["f5d5df71@group.ptt.189.cn"]} 
 * {"groups":["920ade39@group.ptt.189.cn","361370ba@group.ptt.189.cn"]} 
 *
 */
int Xml_builder::get_group_xml (const std::string &input, std::string &xml_output) 
{

    xml_output = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><group-association>";
    std::size_t position_left_square_bracket = input.find_first_of ("[");
    std::size_t position_right_square_bracket = input.find_first_of ("]");
    if ((position_right_square_bracket - position_left_square_bracket) == 1)
    {
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << " No valid group found: " << input
            << std::endl;
        xml_output += "</group-association>";
        return 1;
    }

    // remove both []
    std::string groups_string = input.substr (position_left_square_bracket + 1,
            position_right_square_bracket - position_left_square_bracket - 1);
    std::vector<std::string> groups;

    // now we have at least one group
    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
        << "\n group string: " << groups_string
        << std::endl;
    std::string one_group;
    for (unsigned int i = 0; i < groups_string.size(); i++) 
    {
        if (groups_string[i] == ',') 
        {
            groups.push_back (one_group);
            one_group.clear();
        }
        else 
        {
            one_group.push_back (groups_string[i]);
        }
    }
    groups.push_back (one_group);

    for (unsigned int i = 0; i < groups.size(); i++) 
    {
        xml_output += "<group uri=";
        xml_output += groups[i];
        xml_output += " premium-flag=\"false\"> <display-name>200";

#if 0
        // For testing
        std::string chinese_characters ("e4 ba ba e7 be a4 e7 bb 84 e6 b5 8b e8 af 95 e6 b5 8b e8 af 95");
        std::istringstream hex_chars_stream (chinese_characters);

        // Note that c must be an int (or long, or some other integer type), not a char; 
        // if it is a char (or unsigned char), the wrong >> overload will be called 
        // and individual characters will be extracted from the string, not hexadecimal integer strings.
        // Additional error checking to ensure that the extracted value fits within a char would be a good idea.
        unsigned int c;
        std::string dispaly_name_binary_section2;
        while (hex_chars_stream >> std::hex >> c)
        {
            // xml_output.push_back (c);
            dispaly_name_binary_section2.push_back (c);
        }
        std::string dispaly_name_base64_section2 = base64_encode (dispaly_name_binary_section2);
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << " dispaly_name_binary_section2: " << dispaly_name_binary_section2
            << " dispaly_name_base64_section2: " << dispaly_name_base64_section2
            << std::endl;
        // End of for testing
#endif

        std::string query = "SELECT group_display_name FROM GROUP_CACHE WHERE group_addr = '" + groups[i] + "';";
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << " query: " << query
            << std::endl;

        mysql_query(section2_connection_, query.c_str());
        MYSQL_RES *database_result = mysql_store_result(section2_connection_);
        if (mysql_num_rows (database_result) == 0 ) 
        {
            std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                << " No result " << query
                << std::endl;
            return 1;
        }

        // get the number of the columns
        // int num_fields = mysql_num_fields(database_result);

        // We assume only one row at most
        MYSQL_ROW row;
        std::string dispaly_name_base64;
        if ((row = mysql_fetch_row(database_result)))
        {
            if(row[0] == NULL) 
            {
                std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                    << " groups_dispaly_name is NULL " 
                    << std::endl;
            }
            else
            {
                dispaly_name_base64 = row[0];
                std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                    << " groups_dispaly_name_base64: " << dispaly_name_base64
                    << std::endl;
            }
        }

        if(database_result != NULL) {
            mysql_free_result(database_result);
        }

        // change from base64 to binary
        // One solution is at https://github.com/kenba/via-httplib/blob/master/src/via/http/authentication/base64.cpp
        // We use solution at https://www.boost.org/doc/libs/1_70_0/boost/beast/core/detail/base64.hpp
        std::string dispaly_name_binary = base64_decode (dispaly_name_base64);

        xml_output += dispaly_name_binary;

        xml_output += "</display-name><group-type>close-group</group-type><conference-id></conference-id></group>";
    }

    xml_output += "</group-association>";
    return 0;
}

Xml_builder::~Xml_builder() 
{
    // disconnect from server 
    mysql_close (section2_connection_);
}
