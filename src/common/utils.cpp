//
// Created by vini84200 on 7/8/23.
//

#include <openssl/sha.h>
#include <fstream>
#include <string>
#include <sstream>
#include "utils.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

void digest_to_string(const unsigned char *md, std::string &str) {
    int i;
    std::stringstream s;
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        s << std::hex << (int) md[i];
    }
    str = s.str();
}

void print_sha_digest(unsigned char *md) {
    std::string str;
    digest_to_string(md, str);
    printf("SHA digest: %s\n", str.c_str());
}

std::string sha256_file(std::string path) {
    int file = open(path.c_str(), O_RDONLY);
    if (file >= 0) {
        char *memblock{nullptr};

        unsigned long size = 0;
        struct stat statbuf;
        if  (fstat(file, &statbuf) < 0) {
            perror("fstat");
            return "";
        }
        size = statbuf.st_size;
        memblock = (char*) mmap(nullptr, size, PROT_READ, MAP_SHARED, file, 0);
        char rs[SHA256_DIGEST_LENGTH ];
        unsigned char *hash = SHA256((unsigned char *) memblock, size, nullptr);
        munmap(memblock, size);
        std::string hash_str;
        digest_to_string(hash, hash_str);
        return hash_str;
    } else {
        return "";
    }
}