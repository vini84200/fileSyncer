//
// Created by vini84200 on 7/8/23.
//

#ifndef FILESYNCERCLIENT_UTILS_H
#define FILESYNCERCLIENT_UTILS_H

#include "cstring"
#include <string>

typedef unsigned char byte;
typedef std::basic_string<byte> byte_string;
typedef unsigned long FileDigest;

#define DIGEST_SIZE 8
#define CHUNK_SIZE (1024 * 1024 * 2) // 2MB

FileDigest getFileDigest(std::string path);
std::string digest_to_string(const FileDigest &md);

#endif // FILESYNCERCLIENT_UTILS_H
