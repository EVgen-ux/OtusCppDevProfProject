#pragma once
#include "Constants.h"
#include <string>

class ColorManager {
public:
    static void disableColors();
    static void enableColors();
    static bool areColorsEnabled();
    
    static std::string getDirNameColor();
    static std::string getDirLabelColor();
    static std::string getSizeColor();
    static std::string getDateColor();
    static std::string getPermissionsColor();
    static std::string getHiddenContentColor();
    static std::string getReset();
    
private:
    static bool colorsEnabled;
};