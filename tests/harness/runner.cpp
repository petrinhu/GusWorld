// SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
// SPDX-License-Identifier: AGPL-3.0-or-later
#include "runner.hpp"

#include "check.hpp"
#include "test_registry.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <string_view>

namespace gusworld::test {

namespace {

constexpr int exit_pass = 0;
constexpr int exit_failure = 1;
constexpr int exit_usage = 2;

// A case fails when its body throws anything, or when a check was recorded
// even though the body did not throw (a caught case_check_failed, for example).
bool run_case(const test_case& c) {
    reset_failure_count();
    bool completed = true;
    try {
        c.fn();
    } catch (const case_check_failed&) {
        // GUSWORLD_CHECK already printed where and what; only the unwind lands here.
        completed = false;
    } catch (const std::exception& e) {
        std::cerr << "unexpected std::exception: " << e.what() << '\n';
        completed = false;
    } catch (...) {
        std::cerr << "unexpected exception of a non-std type\n";
        completed = false;
    }
    return completed && failure_count() == 0;
}

bool run_and_report(const test_case& c) {
    const bool passed = run_case(c);
    std::cout << (passed ? "[ PASS ] " : "[ FAIL ] ") << c.name << '\n';
    return passed;
}

int summarize(std::size_t run, int failed) {
    std::cout << "gusworld tests: " << run << " run, " << failed << " failed\n";
    return failed == 0 ? exit_pass : exit_failure;
}

int run_every_case() {
    int failed = 0;
    for (const test_case& c : all_cases()) {
        if (!run_and_report(c)) {
            ++failed;
        }
    }
    return summarize(all_cases().size(), failed);
}

// An unknown name is an error, never a silent zero: a filter that matches
// nothing must not exit with success.
int run_named_case(std::string_view name) {
    for (const test_case& c : all_cases()) {
        if (c.name == name) {
            return summarize(1, run_and_report(c) ? 0 : 1);
        }
    }
    std::cerr << "[ ERROR ] no such case: " << name << '\n';
    return exit_usage;
}

void list_cases() {
    for (const test_case& c : all_cases()) {
        std::cout << c.name << '\n';
    }
}

} // namespace

int run_main(int argc, char** argv) {
    // A binary that registered nothing is broken, not green: refuse to run.
    if (all_cases().empty()) {
        std::cerr << "[ ERROR ] no test case registered\n";
        return exit_usage;
    }
    if (argc == 1) {
        return run_every_case();
    }
    const std::string_view flag{argv[1]};
    if (argc == 2 && flag == "--list") {
        list_cases();
        return exit_pass;
    }
    if (argc == 3 && flag == "--case") {
        return run_named_case(argv[2]);
    }
    std::cerr << "usage: " << argv[0] << " [--list | --case NAME]\n";
    return exit_usage;
}

} // namespace gusworld::test
