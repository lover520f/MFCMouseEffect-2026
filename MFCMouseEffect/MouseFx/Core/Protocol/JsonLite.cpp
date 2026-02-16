#include "pch.h"

#include "JsonLite.h"

namespace mousefx {

std::string ExtractJsonStringValue(const std::string& json, const std::string& key) {
    const std::string search = "\"" + key + "\"";
    const size_t keyPos = json.find(search);
    if (keyPos == std::string::npos) return "";

    size_t startQuote = json.find('"', keyPos + search.length());
    if (startQuote == std::string::npos) {
        startQuote = json.find('"', keyPos + search.length() + 1);
    }
    if (startQuote == std::string::npos) return "";

    const size_t endQuote = json.find('"', startQuote + 1);
    if (endQuote == std::string::npos) return "";

    return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

} // namespace mousefx

