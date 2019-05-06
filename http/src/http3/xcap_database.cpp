#include "xcap_database.hpp"
#include "xml_builder.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>


const static char *opt_socket_name = NULL; /* SOCKET NAME, DO NOT CHANGE */
const static unsigned int opt_flags = 0; /* CONNECTION FLAGS, DO NOT CHANGE */


Xcap_database::Xcap_database (const std::string &config_root) : 
    config_root_ (config_root), section1_connection_(NULL) 
{
    // section1
    section1_connection_ = mysql_init(NULL);
    if (section1_connection_ == NULL)
    {
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << "\n Error initializing databases connection!"
            << std::endl;
        return;
    }

    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(config_root_ + "/config.ini", pt);
    std::string section1_host_name = pt.get<std::string>("section1.section1_host_name");
    std::string section1_user_name = pt.get<std::string>("section1.section1_user_name");
    std::string section1_password = pt.get<std::string>("section1.section1_password");
    std::string section1_database_name = pt.get<std::string>("section1.section1_database_name");
    int section1_port_num = pt.get<int>("section1.section1_port_num");

    MYSQL *mysql = mysql_real_connect (
            section1_connection_, 
            section1_host_name.c_str(),
            section1_user_name.c_str(), 
            section1_password.c_str(),
            section1_database_name.c_str(),
            section1_port_num,
            opt_socket_name, 
            opt_flags
            );

    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
        << " configiguration file: " << config_root_ + "/config.ini"
        << ", section1_host_name: " << section1_host_name
        << ", section1_user_name: " << section1_user_name
        << ", section1_password:" << section1_password
        << ", section1_database_name:" << section1_database_name
        << std::endl;

    if (mysql == 0)
    {
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << " Not successfully initializing section1_databases connection!"
            << ", check your configruation file!"
            << std::endl;
        exit (1);
    }

    return; 
}


/**
 *
 * if returns 0, then find a result
 * otherwise, returns non-zero value
 */
int Xcap_database::get_client_user_addr (const std::string &imsi, std::string &output) 
{
    std::string query = "SELECT user_addr FROM USER_CACHE WHERE imsi = '" + imsi + "';";
    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
        << "\n query: " << query
        << std::endl;

    mysql_query(section1_connection_, query.c_str());
    MYSQL_RES *database_result = mysql_store_result(section1_connection_);
    if (mysql_num_rows (database_result) == 0 ) 
    {
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << " No result " << query
            << std::endl;
        return 1;
    }

    // get the number of the columns
    //  int num_fields = mysql_num_fields(database_result);

    // Fetch one rows from the result
    MYSQL_ROW row;
    output = "mdn=";
    if ((row = mysql_fetch_row(database_result)))
    {
        // Make sure row[i] is valid!
        if(row[0] == NULL) 
        {
            std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                << "\n NULL " 
                << std::endl;
        }
        else
        {
            output += row[0];
        }
    }

    if(database_result != NULL) 
    {
        mysql_free_result(database_result);
    }

    return 1;
}


/**
 *
 * if returns 0, then find a result
 * otherwise, returns non-zero value
 */
int Xcap_database::get_client_xml (const std::string &imsi, std::string &xml_output) 
{

    std::string query = "SELECT imsi, user_addr, calltype_provisioning FROM USER_CACHE WHERE imsi = '" + imsi + "';";
    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
        << "\n query: " << query
        << std::endl;

    mysql_query(section1_connection_, query.c_str());
    MYSQL_RES *database_result = mysql_store_result(section1_connection_);
    if (mysql_num_rows (database_result) == 0 ) 
    {
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << " No result " << query
            << std::endl;
        return 1;
    }

    // get the number of the columns
    int num_fields = mysql_num_fields(database_result);

    // Fetch one rows from the result
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(database_result)))
    {
        // Print all columns
        for(int i = 0; i < num_fields; i++)
        {
            // Make sure row[i] is valid!
            if(row[i] == NULL) 
            {

                std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                    << "\n NULL " 
                    << std::endl;
                break;
            }

            if (i == 1) {

                Xml_builder xml_builder;
                xml_builder.get_client_xml (row[i], xml_output);
                std::cout << row[i] << std::endl;
                break;
            }

            // TODO
            // We need to fetch calltype_provisioning
        }
    }

    if(database_result != NULL) {
        mysql_free_result(database_result);
    }

    return 1;
}


/**
 *
 * if returns 0, then find a result
 * otherwise, returns non-zero value
 */
int Xcap_database::get_group_xml (const std::string &imsi, std::string &xml_output) 
{
    std::string query = "SELECT g.groupList FROM GROUP_MAPPING_CACHE g, USER_CACHE u WHERE g.user_addr = u.user_addr AND u.imsi = '" + imsi + "';";
    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
        << "\n query: " << query
        << std::endl;

    mysql_query(section1_connection_, query.c_str());
    MYSQL_RES *database_result = mysql_store_result(section1_connection_);
    std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
        << " query: " << query
        << std::endl;

    if (mysql_num_rows (database_result) == 0 ) 
    {
        std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
            << " No result " << query
            << std::endl;

        return 1;
    }

    // We assume only have one record, one column
    // so no need to check fields
    // get the number of the columns
    // int num_fields = mysql_num_fields(database_result);

    MYSQL_ROW row;
    if ((row = mysql_fetch_row(database_result)))
    {
        if(row[0] == NULL) 
        {
            std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << ":" << pthread_self()
                << " groups is NULL " 
                << std::endl;

            // don't forget to clean result after you don't need it anymore
            if(database_result != NULL) 
            {
                mysql_free_result(database_result);
            }

            return 1;
        }

        // append even the group is empty
        Xml_builder xml_builder(config_root_);
        std::string groupList0 = row[0];
        xml_builder.get_group_xml (row[0], xml_output);
    }

    // don't forget to clean result after you don't need it anymore
    if(database_result != NULL) 
    {
        mysql_free_result(database_result);
    }

    return 0;
}


Xcap_database::~Xcap_database() 
{
    // disconnect from server 
    mysql_close (section1_connection_);
}
