#pragma once

#include <cstdint>
#include <string>

namespace mousefx::wasm {

struct BudgetCheckInput final {
    uint32_t outputBudgetBytes = 0;
    uint32_t commandBudgetCount = 0;
    double maxExecutionMs = 0.0;
    uint32_t returnedBytes = 0;
    uint32_t parsedCommandCount = 0;
    double executionMs = 0.0;
    bool commandLimitTruncated = false;
};

struct BudgetCheckResult final {
    bool accepted = true;
    bool outputTruncated = false;
    bool commandTruncated = false;
    std::string reason{};
};

class WasmExecutionBudgetGuard final {
public:
    static BudgetCheckResult Evaluate(const BudgetCheckInput& input);
};

} // namespace mousefx::wasm

