#include "helper.h"

std::string trim_comma(std::string str) {
  return str[str.size()-1] == ',' ? str.substr(0, str.size()-1) : str;
}
// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
std::string trim(std::string s) {
        return ltrim(rtrim(s));
}

std::string GetBaseFilename(std::string fName)
{
    int pos = fName.find_last_of(".");
    if(pos == std::string::npos)  //No extension.
        return fName;
    if(pos == 0)    //. is at the front. Not an extension.
        return fName;
    return fName.substr(0, pos);
}


std::string get_tomorrow(std::string today) {
  int pos, year, month, day;
  
  pos = today.find('-');
  year = atoi(today.substr(0, pos).c_str());
  today = today.substr(pos+1);
  pos = today.find('-');
  month = atoi(today.substr(0, pos).c_str());
  pos = today.find('-');
  day = atoi(today.substr(pos+1).c_str());

  std::tm t = {};
  t.tm_year = year-1900;
  t.tm_mon  = month-1;
  t.tm_mday = day;
  // modify
  t.tm_mday += 1;
  std::mktime(&t);
  char yearchar[80];
  strftime(yearchar, sizeof(yearchar), "%Y-%m-%d", &t);
  return std::string(yearchar);
}