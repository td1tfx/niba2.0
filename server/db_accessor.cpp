#include "db_accessor.h"
#include <algorithm>
#include <openssl/rand.h>
#include <openssl/sha.h>

using namespace nibaserver;

std::unordered_map<std::string, db_accessor::user> db_accessor::db_;

constexpr std::size_t HASH_SIZE = 32;

bool nibaserver::db_accessor::login(const std::string &id, const std::string &password) {
    auto iter = db_.find(id);
    if (iter == db_.end())
        return false;

    unsigned char buffer[HASH_SIZE] = {0};
    memcpy(buffer, password.data(), (std::min)(HASH_SIZE, password.size()));
    for (std::size_t i = 0; i < HASH_SIZE; i++) {
        buffer[i] ^= iter->second.salt[i];
    }

    unsigned char digest[HASH_SIZE] = {0};
    SHA256_CTX context;
    if (!SHA256_Init(&context))
        return false;

    if (!SHA256_Update(&context, (unsigned char *)buffer, HASH_SIZE))
        return false;

    if (!SHA256_Final(digest, &context))
        return false;

    if (memcmp(digest, iter->second.hashed_password, HASH_SIZE) == 0) {
        if (iter->second.logged_in)
            return false;
        iter->second.logged_in = true;
        return true;
    }

    return false;
}

bool nibaserver::db_accessor::logout(const std::string &id) {
    auto iter = db_.find(id);
    if (iter == db_.end())
        return false;
    iter->second.logged_in = false;
    return true;
}

bool nibaserver::db_accessor::create_user(const std::string &id, const std::string &password) {
    auto iter = db_.find(id);
    if (iter != db_.end())
        return false;

    db_accessor::user u;
    if (RAND_bytes(u.salt, HASH_SIZE) != 1)
        return false;

    unsigned char buffer[HASH_SIZE] = {0};
    memcpy(buffer, password.data(), (std::min)(HASH_SIZE, password.size()));
    for (std::size_t i = 0; i < HASH_SIZE; i++) {
        buffer[i] ^= u.salt[i];
    }

    SHA256_CTX context;
    if (!SHA256_Init(&context))
        return false;

    if (!SHA256_Update(&context, (unsigned char *)buffer, HASH_SIZE))
        return false;

    if (!SHA256_Final(u.hashed_password, &context))
        return false;

    u.logged_in = false;

    db_[id] = u;

    return true;
}