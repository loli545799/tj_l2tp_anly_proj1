#include <iostream>
#include <string>
#include <unordered_map>
#include <regex>
#include <system_error>
#include <fstream>
#include <cstdlib>

// ִ��shell�����ȡ���
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
            // ȥ��β���Ļ��з�
            line.pop_back();
            // ȥ���м����Ŀո�
            size_t start = line.find_first_not_of(" \t");
            if (start == std::string::npos)
                continue; // ���Կ���
            size_t end = line.find_last_not_of(" \t");
            line = line.substr(start, end - start + 1);
            known_versions.push_back(line);
        }
        file.close();
    }
    else {
        throw std::runtime_error("�޷����ļ�!");
    }

    return known_versions;
}

int main() {
    int count = 0;
    try {
        // ��ȡ��ǰOpenSSL�汾
        std::string openssl_version = exec_command("openssl version -a | grep 'OpenSSL' | awk '{print $2}'");
        std::cout << "��ǰOpenSSL�汾: " << openssl_version << std::endl;
        // ���ļ���ȡ��֪©���汾���б�        
        std::vector<std::string> known_versions = read_versions_from_file("versions.txt");
        // ���©��          
        for (const auto& version : known_versions) {
            std::regex version_regex(version);
            if (std::regex_search(openssl_version, version_regex)) {
                count++;
            }
        }
        if (count != 0) {
            std::cout << "���棺������֪©����" << std::endl;
            std::cout << "�ð汾����©��" << count << "��" << std::endl;
            return 1;
        }
        // ���û�з���©���������ȫ��Ϣ       
        std::cout << "��ǰOpenSSL�汾�ǰ�ȫ�ġ�" << std::endl;
        return 0; // û��©���������˳�   
    }
    catch (const std::exception& e) {
        std::cerr << "����" << e.what() << std::endl;
        return 0; // �����쳣���˳�����        
    }
}
