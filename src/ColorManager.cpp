
#include "ColorManager.h"

bool ColorManager::colorsEnabled = true;

void ColorManager::disableColors() {
    colorsEnabled = false;
}

void ColorManager::enableColors() {
    colorsEnabled = true;
}

std::string ColorManager::getDirNameColor() {
    return colorsEnabled ? constants::DIR_NAME_COLOR : "";
}

std::string ColorManager::getDirLabelColor() {
    return colorsEnabled ? constants::DIR_LABEL_COLOR : "";
}

std::string ColorManager::getSizeColor() {
    return colorsEnabled ? constants::SIZE_COLOR : "";
}

std::string ColorManager::getDateColor() {
    return colorsEnabled ? constants::DATE_COLOR : "";
}

std::string ColorManager::getPermissionsColor() {
    return colorsEnabled ? constants::PERMISSIONS_COLOR : "";
}

std::string ColorManager::getHiddenContentColor() {
    return colorsEnabled ? constants::HIDDEN_CONTENT_COLOR : "";
}

std::string ColorManager::getReset() {
    return colorsEnabled ? constants::RESET : "";
}