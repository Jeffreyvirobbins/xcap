#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <fstream>
#include <map>
#include <sstream>
#include <string>

class configuration final
{
    public:

        using storage_type = std::map<std::string,std::string>;

        // instantiate from set of values (that can be empty)
        explicit configuration(storage_type values = {}) noexcept;

        // note default destructor is enough to have - so don't declare it here

        // return a const reference instead of an object
        // we are not modifying the class so we can be const and noexcept
        const std::string& get_value(const std::string& key) const noexcept
        {
            using namespace std::string_literals;
            try
            {
                return values.at(key);
            }
            catch(std::out_of_bounds const&)
            {
                static const auto empty = ""_s;
                return empty;
            }
        }

    private:
        storage_type data_;
};


#endif // CONFIGURATION_HPP
