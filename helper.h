#ifndef HELPER_HPP // This is used to avoid including a header more than once.
#define HELPER_HPP

#include <string>

std::string trim_comma(std::string);

// trim from start
static inline std::string &ltrim(std::string &); 

// trim from end
static inline std::string &rtrim(std::string &); 

// trim from both ends
std::string trim(std::string );

std::string GetBaseFilename(std::string);

std::string get_tomorrow(std::string);

#endif