#include <iostream>
#include <string>
#include <unordered_map>
#include <regex>
#include <system_error>
#include <fstream>
#include <cstdlib>

// 执行shell命令并获取输出
std::string exec_command(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::system_error(errno, std::system_category(), "popen failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::vector<std::string> read_versions_from_file(const std::string& file_path) {
    std::vector<std::string> known_versions;
    std::ifstream file(file_path);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            // 去除尾部的换行符
            line.pop_back();
            // 去除中间多余的空格
            size_t start = line.find_first_not_of(" \t");
            if (start == std::string::npos)
                continue; // 忽略空行
            size_t end = line.find_last_not_of(" \t");
            line = line.substr(start, end - start + 1);
            known_versions.push_back(line);
        }
        file.close();
    }
    else {
        throw std::runtime_error("无法打开文件!");
    }

    return known_versions;
}

int main() {
    int count = 0;
    try {
        // 获取当前OpenSSL版本
        std::string openssl_version = exec_command("openssl version -a | grep 'OpenSSL' | awk '{print $2}'");
        std::cout << "当前OpenSSL版本: " << openssl_version << std::endl;
        // 从文件读取已知漏洞版本号列表        
        std::vector<std::string> known_versions = read_versions_from_file("versions.txt");
        // 检测漏洞          
        for (const auto& version : known_versions) {
            std::regex version_regex(version);
            if (std::regex_search(openssl_version, version_regex)) {
                count++;
            }
        }
        if (count != 0) {
            std::cout << "警告：发现已知漏洞！" << std::endl;
            std::cout << "该版本存在漏洞" << count << "个" << std::endl;
            return 1;
        }
        // 如果没有发现漏洞，输出安全信息       
        std::cout << "当前OpenSSL版本是安全的。" << std::endl;
        return 0; // 没有漏洞，正常退出   
    }
    catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 0; // 捕获到异常，退出程序        
    }
}
