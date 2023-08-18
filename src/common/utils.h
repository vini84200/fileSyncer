//
// Created by vini84200 on 7/8/23.
//

#ifndef FILESYNCERCLIENT_UTILS_H
#define FILESYNCERCLIENT_UTILS_H

typedef unsigned char byte;
typedef std::basic_string<byte> byte_string;
typedef byte_string FileDigest;

#define DIGEST_SIZE 32

FileDigest getFileDigest(std::string path);
std::string digest_to_string(const FileDigest &md);

#endif // FILESYNCERCLIENT_UTILS_H
