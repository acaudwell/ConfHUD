#include "conffile.h"

//section of config file
Regex ConfFile_section("^\\s*\\[([^\\]]+)\\]\\s*$");

// parse key value pair, seperated by an equals sign, removing white space on key and front of the value
Regex ConfFile_key_value("^\\s*([^=\\s]+)\\s*=\\s*([^\\s].+)?$");

ConfFile::ConfFile() {

}

ConfFile::~ConfFile() {
    clear();
}

void ConfFile::clear() {

    conf_error = "";

    //delete stuff from configmap
    for(std::map<std::string, std::map<std::string, std::string>*>::iterator it = confmap.begin(); it!= confmap.end(); it++) {
        std::map<std::string, std::string>* sectionmap = it->second;
        delete sectionmap;
    }

    confmap.clear();
}

std::string ConfFile::getError() {
    return conf_error;
}

bool ConfFile::load(std::string conffile) {
    this->conffile = conffile;
    load();
}

bool ConfFile::load() {
    debugLog("ConfFile::load(%s)\n", conffile.c_str());

    clear();

    char buff[1024];

    int lineno = 0;
    std::string section = "";

    std::ifstream in(conffile.c_str());

    if(!in.is_open()) {
        sprintf(buff, "failed to open config file %s", conffile.c_str());
        conf_error = std::string(buff);
        return false;
    }

    std::string whitespaces (" \t\f\v\n\r");

    while(in.getline(buff, 1024)) {

        lineno++;

        std::string line = std::string(buff);
        std::vector<std::string> matches;

        // blank line or commented out lines
        if(line.size() == 0 || line.size() > 0 && line[0] == '#') {

            continue;

        // sections
        } else if(ConfFile_section.match(line, &matches)) {

            section = matches[0];

        // key value pairs
        } else if(ConfFile_key_value.match(line, &matches)) {

            std::string key   = matches[0];
            std::string value = (matches.size()>1) ? matches[1] : "";

            //trim whitespace
            if(value.size()>0) {
                size_t string_end = value.find_last_not_of(whitespaces);

                if(string_end == std::string::npos) value = "";
                else if(string_end != value.size()-1) value = value.substr(0,string_end+1);
            }

            string_to_string_map* sectionmap = confmap[section];

            if(sectionmap == 0) {
                sectionmap = new std::map<std::string,std::string>();
                confmap[section] = sectionmap;
            }

            (*sectionmap)[key] = value;

            debugLog("%s: [%s] %s => %s\n", conffile.c_str(), section.c_str(), key.c_str(), value.c_str());

        } else {
            sprintf(buff, "failed to read line %d of config file %s", lineno, conffile.c_str());
            conf_error = std::string(buff);
            return false;
        }
    }

    in.close();

    return true;
}

bool ConfFile::hasValue(std::string section, std::string key) {
    std::string value = getString(section, key);

    if(value.size()>0) return true;

    return false;
}

bool ConfFile::hasSection(std::string section) {

    std::map<std::string, string_to_string_map*>::iterator section_finder = confmap.find(section);

    if(section_finder == confmap.end()) return false;

    return true;
}

std::string ConfFile::getString(std::string section, std::string key) {

    std::map<std::string, string_to_string_map*>::iterator section_finder = confmap.find(section);

    if(section_finder == confmap.end()) return std::string("");

    string_to_string_map* sectionmap = section_finder->second;

    std::map<std::string, std::string>::iterator key_finder = sectionmap->find(key);

    if(key_finder == sectionmap->end()) return std::string("");

    return key_finder->second;
}

int ConfFile::getInt(std::string section, std::string key) {
    std::string stringvalue = getString(section, key);

    return atoi(stringvalue.c_str());
}

float ConfFile::getFloat(std::string section, std::string key) {
    std::string stringvalue = getString(section, key);

    return atof(stringvalue.c_str());
}

bool ConfFile::getBool(std::string section, std::string key) {
    std::string stringvalue = getString(section, key);

    if(stringvalue == "1" || stringvalue == "yes" || stringvalue == "YES" || stringvalue == "Yes")
        return true;

    return false;
}
