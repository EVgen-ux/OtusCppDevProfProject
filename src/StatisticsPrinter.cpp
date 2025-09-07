#include "StatisticsPrinter.h"
#include <iostream>

void StatisticsPrinter::printStatistics(const ITreeBuilder& builder, bool useFilteredBuilder) {
    std::cout << std::endl;
    std::cout << "Статистика:" << std::endl;
    
    auto stats = builder.getStatistics();
    auto displayStats = builder.getDisplayStatistics();
    
    if (displayStats.hiddenByDepth > 0) {
        std::cout << "  Директорий: " << displayStats.displayedDirectories << std::endl;
        std::cout << "  Файлов: " << displayStats.displayedFiles << std::endl;
        std::cout << "  Общий размер: " << FileSystem::formatSizeBothSystems(displayStats.displayedSize) << std::endl;
        std::cout << "  Скрыто по глубине: " << displayStats.hiddenByDepth << " директорий" << std::endl;
    } else {
        std::cout << "  Директорий: " << stats.totalDirectories << std::endl;
        std::cout << "  Файлов: " << stats.totalFiles << std::endl;
        std::cout << "  Общий размер: " << FileSystem::formatSizeBothSystems(stats.totalSize) << std::endl;
    }
    
    if (useFilteredBuilder) {
        std::cout << "  (Применены фильтры)" << std::endl;
    }
}