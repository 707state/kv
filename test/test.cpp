
#include "boost/filesystem.hpp"
#include <iostream>

namespace fs = boost::filesystem;

int main() {
    // 定义一个文件路径
    fs::path filePath("example.txt");

    // 检查文件是否存在
    if (fs::exists(filePath)) {
        std::cout << filePath << " 文件存在。" << std::endl;
    } else {
        std::cout << filePath << " 文件不存在。" << std::endl;
    }

    // 创建一个目录
    fs::path dirPath("example_dir");
    if (fs::create_directory(dirPath)) {
        std::cout << dirPath << " 目录已创建。" << std::endl;
    }

    // 遍历当前目录
    fs::path currentPath = fs::current_path();
    std::cout << "当前目录：" << currentPath << std::endl;

    for (const auto& entry: fs::directory_iterator(currentPath)) {
        std::cout << entry.path() << std::endl;
    }

    // 删除文件和目录
    fs::remove(filePath);    // 删除文件
    fs::remove_all(dirPath); // 递归删除目录

    return 0;
}
