#pragma once
#include "ITreeBuilder.h"
#include "FileSystem.h"
#include <memory>

class StatisticsPrinter {
public:
    static void printStatistics(const ITreeBuilder& builder, bool useFilteredBuilder = false);
};