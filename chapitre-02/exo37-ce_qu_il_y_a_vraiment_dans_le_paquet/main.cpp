#include <iostream>
#include <sstream>
#include <string>

bool startsWith(const std::string &s, const std::string &prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const std::string &s, const std::string &suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int main() {
    std::string line;

    std::getline(std::cin, line);
    std::string arch = line;

    std::getline(std::cin, line);
    int f = std::stoi(line);

    long long totalSize = 0;
    bool signedPackage = false;
    bool abiOk = false;
    int inutileCount = 0;

    const std::string metaPrefix = "META-INF/";
    const std::string libPrefix = "lib/";
    const std::string abiPrefix = libPrefix + arch + "/";

    for (int i = 0; i < f; ++i) {
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string path;
        long long size;
        iss >> path >> size;

        totalSize += size;

        if (startsWith(path, metaPrefix) &&
            (endsWith(path, ".RSA") || endsWith(path, ".DSA") || endsWith(path, ".EC"))) {
            signedPackage = true;
        }

        if (startsWith(path, abiPrefix)) {
            abiOk = true;
        } else if (startsWith(path, libPrefix)) {
            ++inutileCount;
        }
    }

    std::cout << totalSize << "\n";
    std::cout << (signedPackage ? "SIGNE" : "NON SIGNE") << "\n";
    std::cout << "ABI " << (abiOk ? "OUI" : "NON") << "\n";
    std::cout << "INUTILE " << inutileCount << "\n";

    return 0;
}
