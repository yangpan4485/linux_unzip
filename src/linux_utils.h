#pragma once

#include <string>

/**
 * @brief 解压缩文件
 *
 * @param zipfile 压缩文件
 * @param unzip_dir 解压到哪个目录
 *
 * @return 成功返回0，失败返回对应错误码
 */
int UnzipFile(const std::string& zipfile, const std::string& unzip_dir);

/**
 * @brief 解压缩文件
 *
 * @param zipfile 压缩文件
 * @param unzip_dir 解压到哪个目录
 * @param password 解压密码
 *
 * @return 成功返回0，失败返回对应错误码
 */
int UnzipFile(const std::string& zipfile, const std::string& unzip_dir,
              const std::string& password);
