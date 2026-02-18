#include "pch.h"

#include "WasmExecutionBudgetGuard.h"

#include <sstream>

namespace mousefx::wasm {

BudgetCheckResult WasmExecutionBudgetGuard::Evaluate(const BudgetCheckInput& input) {
    BudgetCheckResult result{};
    result.outputTruncated = input.returnedBytes > input.outputBudgetBytes;
    result.commandTruncated =
        input.commandLimitTruncated || (input.parsedCommandCount > input.commandBudgetCount);

    if (input.maxExecutionMs > 0.0 && input.executionMs > input.maxExecutionMs) {
        result.accepted = false;
        std::ostringstream ss;
        ss << "execution time exceeded budget: actual=" << input.executionMs
           << "ms budget=" << input.maxExecutionMs << "ms";
        result.reason = ss.str();
        return result;
    }

    if (result.outputTruncated || result.commandTruncated) {
        std::ostringstream ss;
        ss << "budget truncation: output_truncated=" << (result.outputTruncated ? "true" : "false")
           << " command_truncated=" << (result.commandTruncated ? "true" : "false");
        result.reason = ss.str();
    }
    return result;
}

} // namespace mousefx::wasm

