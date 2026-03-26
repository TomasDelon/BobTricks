#pragma once

#include <cmath>
#include <iostream>
#include <string>

class TestSuite
{
public:
    explicit TestSuite(std::string name) : m_name(std::move(name)) {}

    void expect(bool cond, const char* expr, const char* file, int line,
                const std::string& detail = {})
    {
        if (cond)
            return;
        ++m_failures;
        std::cerr << "FAIL  " << m_name << "  " << file << ':' << line
                  << "  " << expr;
        if (!detail.empty())
            std::cerr << "  (" << detail << ')';
        std::cerr << '\n';
    }

    int finish() const
    {
        if (m_failures == 0) {
            std::cout << "PASS  " << m_name << '\n';
            return 0;
        }
        std::cerr << "FAIL  " << m_name << "  failures=" << m_failures << '\n';
        return 1;
    }

private:
    std::string m_name;
    int         m_failures = 0;
};

#define TEST_EXPECT(suite, cond) \
    (suite).expect((cond), #cond, __FILE__, __LINE__)

#define TEST_EXPECT_MSG(suite, cond, detail) \
    (suite).expect((cond), #cond, __FILE__, __LINE__, (detail))

#define TEST_EXPECT_NEAR(suite, lhs, rhs, eps)                                       \
    do {                                                                              \
        const double test_lhs = static_cast<double>(lhs);                             \
        const double test_rhs = static_cast<double>(rhs);                             \
        const double test_eps = static_cast<double>(eps);                             \
        (suite).expect(std::fabs(test_lhs - test_rhs) <= test_eps, #lhs " ~= " #rhs, \
                       __FILE__, __LINE__);                                           \
    } while (false)
