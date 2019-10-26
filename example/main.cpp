#include <iostream>
#include <string>

#include "linux_utils.h"

int main(void) {
    // zip文件路径
    std::string zipfile = "../test.zip";
    // 解压目录
    std::string zipdir = "../test/";
    // 解压密码
    std::string password = "hello";

    int ret = UnzipFile(zipfile, zipdir, password);
    if (ret == 0) {
        std::cout << "success" << std::endl;
    } else {
        std::cout << "failed" << std::endl;
        std::cout << "ret:" << ret << std::endl;
    }
    return 0;
}
