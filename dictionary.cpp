#include "dictionary.h"
#include "hash.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
std::string dictionaryAttack(const std::string& target_hash,
                             const std::string& wordlist_path) {

    std::ifstream file(wordlist_path);

    if (!file.is_open()) {
        std::cout << "  [!] Could not open wordlist: " << wordlist_path << "\n";
        return "";
    }

    std::cout << "  [*] Wordlist loaded: " << wordlist_path << "\n";
    std::cout << "  [*] Running dictionary attack...\n\n";

    std::string word;
    uint64_t count = 0;

    while (std::getline(file, word)) {
        count++;

        if (count % 1000000 == 0) {
            std::cout << "  [~] " << count << " words tested...\n";
        }

        if (sha256(word) == target_hash) {
            file.close();
            return word;
        }
    }

    file.close();
    return "";
}