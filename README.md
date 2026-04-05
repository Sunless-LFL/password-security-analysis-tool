# 🔐 Password Security Analysis Tool (PSAT)

A C++ tool for understanding **how passwords fail** — and therefore how to make them stronger.

Built phase by phase as a learning project. Each phase isolates one attack technique, explains why it works, and connects it to a real-world defensive lesson. The goal isn't to crack passwords — it's to understand them deeply enough to defend against attacks.

> ⚠️ **For educational and authorized testing only.**
> Only run this against passwords you own or have explicit written permission to test.

---

## 🧭 Project Philosophy

> *"To defend a system, you must understand how it breaks."*

Every phase in this project answers two questions:
1. **Attack**: how does this technique recover a password?
2. **Defense**: what does this teach us about building better systems?

The code is the same code any security researcher writes. The difference is understanding *why* each piece exists.

---

## 📌 Roadmap

| Phase | Topic | What you learn | Status |
|-------|-------|---------------|--------|
| 1 | Exhaustive search (plaintext baseline) | Search space, base-N mapping, timing | ✅ Done |
| 2 | Hash analysis (SHA-256) | Why hashing matters, what "cracking a hash" means | ✅ Done |
| 3 | Dictionary attack | Why common passwords fail even with hashing | 🔜 Planned |
| 4 | Rule-based mutations | How attackers extend dictionaries, why "P@ssw0rd" is weak | 🔜 Planned |
| 5 | Strength estimator | Predict crack time without running the attack | 🔜 Planned |
| 6 | Multi-threading + bcrypt analysis | Why slow hashes (bcrypt, Argon2) exist | 🔜 Planned |

---

## ⚙️ Phase 1 — Exhaustive Search (Plaintext Baseline)

### What it does
Tries every possible combination of characters from a defined **charset**,
up to a configurable **max length**, until it recovers the target password.
Times the operation and reports a structured result.

### What you learn
- How exhaustive search actually works under the hood
- The concept of **search space**: charset size × password length → total candidates
- Why password length matters **more** than complexity at short lengths
- Direct observation of the time-to-crack relationship (the seed of Phase 5)

### The core insight: index → candidate
The engine doesn't use nested loops or recursion. It maps a plain integer
to a candidate string using **base-N arithmetic** — exactly like converting
a decimal number to another base. This is the cleanest way to enumerate
all combinations and the same engine powers every future phase.

```
CHARSET = "abc"  →  base 3

index 0  →  "a"       index 3  →  "aa"      index 9   →  "aaa"
index 1  →  "b"       index 4  →  "ab"      index 10  →  "aab"
index 2  →  "c"       index 5  →  "ac"      ...
```

### How to compile & run
```bash
g++ -std=c++17 -O2 -o psat main.cpp
./psat
```

### Configuration (edit top of `main.cpp`)

| Constant | Default | What it controls |
|----------|---------|-----------------|
| `CHARSET` | lowercase a–z | Characters included in the search |
| `MAX_LENGTH` | `5` | Give up after this many characters |

### Search space reference table

| Charset | Length 4 | Length 5 | Length 6 |
|---------|----------|----------|----------|
| lowercase (26) | 456K | 11.8M | 308M |
| + uppercase (52) | 7.3M | 380M | 19.7B |
| + digits (62) | 14.7M | 916M | 56.8B |
| + symbols (92) | 71.6M | 6.5B | 606B |

**Defensive takeaway**: a 6-character lowercase password has 308 million candidates.
A modern GPU can test ~10 billion MD5 hashes per second.
That means 308M candidates falls in **under 1 second** against a GPU.
Length and charset diversity are not optional — they are the entire defense.

---

## 🔜 Phase 2 — Hash Analysis (MD5 / SHA-256)

### What it does
Replaces the plaintext target with a **hash**. The engine generates candidates,
hashes each one, and compares hashes. Supports MD5 and SHA-256 via OpenSSL.

### What you learn
- What a hash function actually does (one-way, deterministic, fixed-length output)
- Why storing plaintext passwords is catastrophic
- Why MD5 and SHA-1 are broken for password storage
- The difference between a **fast hash** (MD5, SHA-256) and a **slow hash** (bcrypt, Argon2)

### The one-line change from Phase 1
```cpp
// Phase 1:
if (candidate == target) { ... }

// Phase 2:
if (sha256(candidate) == target_hash) { ... }
```
The entire engine stays the same. This is intentional — it illustrates that
hashing is a thin layer, and a fast hash provides almost no real protection
against an exhaustive search.

### Defensive takeaway
SHA-256 is a **fast** hash. A GPU can compute ~4 billion SHA-256 hashes/sec.
bcrypt at cost factor 12 can compute ~200 hashes/sec on the same hardware.
That's a 20,000,000× slowdown — by design. This is why password storage
must use bcrypt, scrypt, or Argon2 and nothing else.

---

## 🔜 Phase 3 — Dictionary Attack

### What it does
Instead of generating every possible combination, reads candidates from a
**wordlist file** (e.g. `rockyou.txt` — 14 million real leaked passwords).
Tests each word (and its hash in Phase 3+2 combined mode) against the target.

### What you learn
- Why the most common attack vector isn't brute force — it's dictionaries
- How real leaked password databases are used in audits (e.g. `rockyou.txt`)
- The concept of **credential stuffing** and why password reuse is dangerous
- Why even a "complex-looking" password like `Fluffy2019!` is in every wordlist

### Defensive takeaway
Password managers exist because humans are predictable. The average person
picks passwords based on names, years, and common substitutions — and
all of those patterns are in every wordlist. A randomly generated 16-character
password cannot be in any dictionary.

---

## 🔜 Phase 4 — Rule-Based Mutations

### What it does
Takes a wordlist and applies **transformation rules** to each word:
- Capitalize first letter: `password` → `Password`
- Leet substitutions: `password` → `p@ssw0rd`
- Append years: `password` → `password2023`, `password2024`
- Append common suffixes: `password` → `password!`, `password123`
- Reverse: `password` → `drowssap`

### What you learn
- How tools like Hashcat use rule engines to extend dictionaries by orders of magnitude
- Why "clever" transformations that feel unique are actually in every ruleset
- How security policies that require "one uppercase, one number, one symbol"
  are almost completely defeated by predictable human mutation patterns

### Defensive takeaway
Every rule you think of — hackers already have. The only real defense is
**length + true randomness** (i.e. a password manager). No transformation
of a dictionary word is safe.

---

## 🔜 Phase 5 — Strength Estimator

### What it does
Given a password, **predicts** how long it would take to recover it
without actually running the full search. Reports estimated crack time
across different attack scenarios:
- Laptop CPU (single-threaded)
- Server CPU (multi-threaded)
- Consumer GPU
- Cloud GPU cluster

### What you learn
- How to quantify password strength with math, not feelings
- Why `Tr0ub4dor&3` (from XKCD) is actually weaker than `correct horse battery staple`
- How entropy is calculated and what it actually means

### Sample output (planned)
```
  Password     : "Hunter2!"
  Entropy      : ~30 bits
  Search space : 735,091,890,625 candidates

  Estimated crack time:
    Laptop CPU  (10M  hash/s)  →  20.4 hours
    Server CPU  (500M hash/s)  →  24.5 minutes
    GTX 4090    (10B  hash/s)  →  73.5 seconds    ← assuming MD5
    GPU cluster (1T   hash/s)  →  0.73 seconds

  Against bcrypt (cost 12, ~200 hash/s per GPU):
    GTX 4090                   →  116 years        ← real protection
```

---

## 🔜 Phase 6 — Multi-Threading + bcrypt Analysis

### What it does
- Distributes the search space across multiple CPU threads (`std::thread`)
- Demonstrates the actual speed difference between MD5 and bcrypt
- Benchmarks: how many hashes/sec does this machine do for each algorithm?

### What you learn
- How parallelism affects crack time (near-linear scaling)
- Why bcrypt's intentional slowness is a security feature, not a bug
- The concept of **work factor / cost factor** and how it scales with hardware

---

## 📁 Project Structure

```
psat/
├── main.cpp          ← Phase 1 (current)
├── README.md         ← This file
│
│   (added in future phases)
├── hash.cpp/.h       ← Phase 2: hashing utilities
├── dictionary.cpp/.h ← Phase 3: wordlist reader
├── rules.cpp/.h      ← Phase 4: mutation engine
├── estimator.cpp/.h  ← Phase 5: strength calculator
└── threads.cpp/.h    ← Phase 6: parallel search
```

---

## 🛠 Requirements

- C++17 or later
- g++ or clang++
- OpenSSL (required from Phase 2 onward): `sudo apt install libssl-dev`

---
## ✅ Phase 2 Complete — SHA-256 Hash Analysis

Phase 2 is done. The tool now takes a SHA-256 hash as input instead 
of a plaintext password. It hashes every candidate it generates and 
compares it against the target hash — which means it can now analyze 
real hashed passwords without ever knowing the plaintext upfront.

The hashing is handled through OpenSSL's EVP interface, which is the 
modern API used in production systems. Each candidate gets fed through 
SHA-256 and compared byte by byte until a match is found.

To test it yourself, generate a hash first:

    echo -n "yourpassword" | openssl dgst -sha256

Then run the tool and paste the hash when prompted. It will recover 
the original password if it falls within the search space.

### What this phase taught me
Fast hashing algorithms like SHA-256 provide almost no real protection 
against exhaustive search. A modern GPU can compute billions of SHA-256 
hashes per second. This is exactly why password storage systems use 
slow hashing algorithms like bcrypt instead — which is what Phase 6 
will demonstrate.

### Coming next — Phase 3: Dictionary Attack
Instead of generating every possible combination, the tool will load 
a real wordlist and test each entry. This will recover passwords like 
"iloveyou" or "monkey" in milliseconds that would take hours by brute 
force alone.

## ✍️ Author

Built as a cybersecurity learning project to understand how password attacks
work at the implementation level — and what that means for designing systems
that store and protect credentials properly.
