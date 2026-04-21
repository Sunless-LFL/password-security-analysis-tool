#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string>

std::string dictionaryAttack(const std::string& target_hash,
                             const std::string& wordlist_path);

#endif