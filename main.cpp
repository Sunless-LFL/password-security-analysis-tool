/**
 * ================================================================
 *  Password Security Analysis Tool
 *  Phase 1: Exhaustive Search Engine (Plaintext Baseline)
 * ================================================================
 *
 *  PURPOSE:
 *  --------
 *  This tool analyzes how resistant a password is to exhaustive
 *  search attacks. By measuring how quickly a simple engine can
 *  recover a plaintext password, we build the foundation for
 *  understanding WHY password hashing, salting, and complexity
 *  policies exist in real security systems.
 *
 *  This is Phase 1 intentionally minimal. No hashing, no
 *  threads, no dictionary. Just the raw search engine, fully
 *  transparent, so every line is understandable before we build
 *  on top of it in later phases.
 *
 *  COMPILE:
 *  --------
 *      g++ -std=c++17 -O2 -o psat main.cpp
 *
 *  RUN:
 *  ----
 *      ./psat
 *  Enter a short test password (e.g. "abc", "zz", "hello").
 *  Keep it ≤ 5 chars while testing or it will take a very long time.
 *
 * ================================================================
 */

#include <iostream>
#include <string>
#include <chrono>   // measuring elapsed time
#include <cstdint>  // uint64_t for large candidate counts
#include <iomanip>  // for formatting output
#include "hash.h"



const std::string CHARSET = "abcdefghijklmnopqrstuvwxyz";


const int MAX_LENGTH = 5;


std::string generate(uint64_t index, const std::string& charset) {
    size_t base = charset.size();
    std::string result = "";

    index += 1; // shift: index 0 maps to first char, not empty string

    while (index > 0) {
        index--;
        result = charset[index % base] + result; // prepend current "digit"
        index /= base;                            // move to next "digit"
    }

    return result;
}


uint64_t computeSearchSpace(size_t base, int maxLen) {
    uint64_t total = 0;
    uint64_t power = 1;
    for (int i = 1; i <= maxLen; i++) {
        power *= base;
        total += power;
    }
    return total;
}

std::string analyze(const std::string& target_hash) {
    size_t base = CHARSET.size();
    uint64_t total = computeSearchSpace(base, MAX_LENGTH);

    std::cout << "  Charset size : " << base << " characters\n";
    std::cout << "  Max length   : " << MAX_LENGTH << "\n";
    std::cout << "  Search space : " << total << " candidates\n";
    std::cout << "\n  Running exhaustive search...\n\n";

    for (uint64_t i = 0; i < total; i++) {

        std::string candidate = generate(i, CHARSET);

        // Progress report every 500,000 attempts
        if (i > 0 && i % 500000 == 0) {
            std::cout << "  [~] " << i << " candidates tested | current: \""
                      << candidate << "\"\n";
        }

       
       if (sha256(candidate) == target_hash) {
            return candidate;
        }
    }

    return ""; // not found within MAX_LENGTH
}



int main() {
    std::cout << "\n";
    std::cout << "================================================\n";
    std::cout << "  Password Security Analysis Tool Phase 1  \n";
    std::cout << "  Exhaustive Search (Plaintext Baseline)        \n";
    std::cout << "================================================\n\n";

    std::string target;
    std::cout << "  Enter SHA-256 hash to analyze: ";
    std::cin >> target;
    std::cout << "\n";

    // ── Timer start ───────────────────────────────────────────
    auto start = std::chrono::high_resolution_clock::now();

    std::string result = analyze(target);

    auto end   = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    // ─────────────────────────────────────────────────────────

    std::cout << "\n------------------------------------------------\n";
    std::cout << "  ANALYSIS REPORT\n";
    std::cout << "------------------------------------------------\n";

    if (!result.empty()) {
        std::cout << "  Status    : RECOVERED\n";
        std::cout << "  Password  : \"" << result << "\"\n";
    } else {
        std::cout << "  Status    : NOT FOUND\n";
        std::cout << "  Reason    : Exceeds MAX_LENGTH or not in charset\n";
    }

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  Time      : " << elapsed << " seconds\n";
    std::cout << "------------------------------------------------\n\n";

    return 0;
}
