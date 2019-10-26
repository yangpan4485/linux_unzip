#include "linux_utils.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include "unzip.h"
}

const uint32_t kMaxFileNameLen = 256;

bool DeleteDir(const std::string& dirname) {
    char cur_dir[] = ".";
    char up_dir[] = "..";
    struct dirent* dp;
    struct stat dir_stat;

    if (0 != access(dirname.data(), F_OK)) {
        return true;
    }
    if (0 > stat(dirname.data(), &dir_stat)) {
        return false;
    }

    if (S_ISREG(dir_stat.st_mode)) {
        remove(dirname.data());
    } else if (S_ISDIR(dir_stat.st_mode)) {
        DIR* dirp = opendir(dirname.data());
        while ((dp = readdir(dirp)) != NULL) {
            if ((0 == strcmp(cur_dir, dp->d_name)) ||
                (0 == strcmp(up_dir, dp->d_name))) {
                continue;
            }
            std::string dir_name = dirname + "/" + dp->d_name;
            DeleteDir(dir_name);
        }
        closedir(dirp);
        rmdir(dirname.data());
    }
    return true;
}

bool CreateDir(const std::string& dirname) {
    if (mkdir(dirname.c_str(), 0777) != 0) {
        return false;
    }
    return true;
}

int UnzipFile(const std::string& zipfile, const std::string& unzip_dir) {
    return UnzipFile(zipfile, unzip_dir, "");
}

int UnzipFile(const std::string& zipfile, const std::string& unzip_dir,
              const std::string& password) {
    // 首先删除原来的目录
    if (!DeleteDir(unzip_dir)) {
        std::cout << "delete unzip dir failed" << std::endl;
        return -1;
    }

    // 然后创建一个新目录
    if (!CreateDir(unzip_dir)) {
        std::cout << "create dir failed" << std::endl;
        return -2;
    }

    // 打开压缩文件
    unzFile unzfile = unzOpen(zipfile.c_str());
    if (unzfile == nullptr) {
        std::cout << "open zipfile failed,please insure zipfile exist"
                  << std::endl;
        return -3;
    }

    // RAII return的时候执行unzClose
    std::shared_ptr<void> raii_close(nullptr,
                                     [&](void*) { unzClose(unzfile); });

    // 获取压缩文件信息
    unz_global_info global_info;
    int ret = unzGetGlobalInfo(unzfile, &global_info);
    if (ret != UNZ_OK) {
        std::cout << "get global info failed" << std::endl;
        return -4;
    }

    // 一项一项的解压缩
    for (int i = 0; i < (int)global_info.number_entry; ++i) {
        char filename[kMaxFileNameLen] = "";
        unz_file_info file_info;
        ret = unzGetCurrentFileInfo(unzfile, &file_info, filename,
                                    kMaxFileNameLen, NULL, 0, NULL, 0);
        if (ret != UNZ_OK) {
            std::cout << "get current file info failed" << std::endl;
            return -5;
        }

        // 判断文件是不是一个目录
        if (filename[strlen(filename) - 1] == '/') {
            std::string dir = unzip_dir + filename;
            if (!CreateDir(dir)) {
                std::cout << "create sub dir failed" << std::endl;
                return -6;
            }
        } else {
            std::string content_file = unzip_dir + filename;
            if (password.empty()) {
                ret = unzOpenCurrentFile(unzfile);
            } else {
                ret = unzOpenCurrentFilePassword(unzfile, password.c_str());
            }
            if (ret != UNZ_OK) {
                std::cout << "open sub file failed" << std::endl;
                return -7;
            }

            auto filesize = file_info.uncompressed_size;
            std::vector<char> data;
            data.resize(filesize);

            std::ofstream fout(content_file.c_str(),
                               std::ios::out | std::ios::binary);
            if (!fout) {
                std::cout << "open file failed" << std::endl;
                return -8;
            }

            while (true) {
                int read_size =
                    unzReadCurrentFile(unzfile, data.data(), data.size());
                if (read_size < 0) {
                    unzCloseCurrentFile(unzfile);
                    fout.close();
                    std::cout << "read file failed" << std::endl;
                    return -9;
                } else if (read_size == 0) {
                    unzCloseCurrentFile(unzfile);
                    break;
                } else {
                    fout.write(data.data(), data.size());
                }
            }
            fout.close();
        }
        unzGoToNextFile(unzfile);
    }

    return 0;
}
