#pragma once

#include "MouseFx/Utils/StringUtils.h"

#include <string>
#include <vector>

namespace mousefx::automation_chain {

inline std::vector<std::string> SplitChainTokens(const std::string& raw) {
    std::vector<std::string> tokens;
    std::string current;
    current.reserve(raw.size());

    const auto pushCurrent = [&]() {
        std::string token = TrimAscii(current);
        while (!token.empty() && token.back() == '-') {
            token.pop_back();
        }
        token = TrimAscii(token);
        if (!token.empty()) {
            tokens.push_back(token);
        }
        current.clear();
    };

    for (char ch : raw) {
        if (ch == '>') {
            pushCurrent();
            continue;
        }
        current.push_back(ch);
    }
    pushCurrent();

    return tokens;
}

template <typename NormalizeFn>
inline std::vector<std::string> NormalizeChainTokens(const std::string& raw, NormalizeFn normalizeToken) {
    std::vector<std::string> tokens;
    const std::vector<std::string> split = SplitChainTokens(raw);
    for (const std::string& token : split) {
        std::string normalized = normalizeToken(token);
        normalized = TrimAscii(normalized);
        if (normalized.empty()) {
            continue;
        }
        tokens.push_back(std::move(normalized));
    }
    return tokens;
}

inline std::string JoinChainTokens(const std::vector<std::string>& tokens) {
    std::string out;
    bool first = true;
    for (const std::string& token : tokens) {
        if (token.empty()) {
            continue;
        }
        if (!first) {
            out.push_back('>');
        }
        out += token;
        first = false;
    }
    return out;
}

template <typename NormalizeFn>
inline std::string NormalizeChainText(const std::string& raw, NormalizeFn normalizeToken) {
    return JoinChainTokens(NormalizeChainTokens(raw, normalizeToken));
}

template <typename NormalizeFn>
inline size_t NormalizedChainLength(const std::string& raw, NormalizeFn normalizeToken) {
    return NormalizeChainTokens(raw, normalizeToken).size();
}

} // namespace mousefx::automation_chain
