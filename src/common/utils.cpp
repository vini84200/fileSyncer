//
// Created by vini84200 on 7/8/23.
//

#include <string>
#include <sstream>
#include "utils.h"
#include <fcntl.h>

#include <csignal>
#include <xxhash.hpp>

std::string digest_to_string(const FileDigest &md) {
    // Get the bytes from the digest
    std::array<byte, DIGEST_SIZE> bytes{};
    bytes = reinterpret_cast<const std::array<byte, DIGEST_SIZE> &>(md);
    int i;
    std::stringstream s;
    for (i = 0; i < DIGEST_SIZE; i++) {
        s << std::hex << (int) bytes[i];
    }
    return s.str();
}

void print_digest(const FileDigest &md) {
    std::string str = digest_to_string(md);
    printf("SHA digest: %s\n", str.c_str());
}

FileDigest getFileDigest(std::string path) {
    int file = open(path.c_str(), O_RDONLY);

    if (file < 0) {
        printf("WARNING: Error opening file %s to calculate digest\n", path.c_str());
        return {};
    }

    std::array<byte, CHUNK_SIZE> buffer{0};
    xxh::hash_state_t<64> hashStream;
    while (true) {
        ssize_t readBytes = read(file, buffer.data(), CHUNK_SIZE);
        if (readBytes <= 0) {
            break;
        }
        hashStream.update(buffer);
    }
    xxh::hash_t<64> hash = hashStream.digest();
    return hash;
}