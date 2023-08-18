//
// Created by vini84200 on 7/8/23.
//

#include <fstream>
#include <string>
#include <sstream>
#include "utils.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


std::string digest_to_string(const FileDigest &md) {
    int i;
    std::stringstream s;
    for (i = 0; i < DIGEST_SIZE; i++) {
        s << std::hex << (int) md[i];
    }
    return s.str();
}

void print_sha_digest(unsigned char *md) {
    std::string str = digest_to_string(md);
    printf("SHA digest: %s\n", str.c_str());
}

FileDigest getFileDigest(std::string path) {
    int file = open(path.c_str(), O_RDONLY);



}