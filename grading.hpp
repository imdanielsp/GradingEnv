/* *
 * MIT License
 *
 * Copyright (c) 2018 University of Massachusetts Lowell
 * Written by Daniel Santos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * */

#include <functional>
#include <initializer_list>
#include <any>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <cmath>

// Represents a record for a test
using GERecord = struct Record
{
  bool pass;
  std::string feedback;
  std::string name;

  friend std::ostream &operator<<(std::ostream &os, const struct Record &rec);
};

using GERecords = std::vector<Record>;

std::ostream &operator<<(std::ostream &os, const struct Record &rec)
{
  std::stringstream ss;
  if (!rec.pass)
  {
    ss << "[" << rec.name << " FAILED]" << std::endl
      << rec.feedback << std::endl;
  }
  else
  {
    ss << "[" << rec.name << " PASSED]" << std::endl;
  }
  os << ss.str();
  return os;
}

// Class Forwarding
class GradingEnv;

// 
using GEFunction = std::function<void(GradingEnv &env)>;

// Represents a test
using GETest = struct GETest
{
  std::string name;
  GEFunction f;
};

class GradingEnv
{
public:
  GradingEnv()
      : _currentTest(0),
        _currentTestName("")
  {
  }

  ~GradingEnv() = default;

  // Non-copyable and non-movable
  GradingEnv(GradingEnv &) = delete;
  GradingEnv(GradingEnv &&) = delete;
  GradingEnv &operator=(GradingEnv &) = delete;
  GradingEnv &&operator=(GradingEnv &&) = delete;

  void add_test(std::pair<std::string, GEFunction> t)
  {
    _tests.push_back(GETest{t.first, t.second});
  }

  void add_test(
      const std::initializer_list<
          std::pair<const std::string &, GEFunction>> &tests)
  {
    for (const auto &test : tests)
    {
      _tests.push_back(GETest{test.first, test.second});
    }
  }

  void run_all(bool report = true)
  {
    std::for_each(
        _tests.cbegin(),
        _tests.cend(),
        [this](const GETest &it) {
          _currentTestName = it.name;
          // Each function needs the environment, thus *this
          it.f(*this);
          _nextTest();
        });

    if (report)
      _report();
  }

  // Assert equality within the environment
  template <typename T>
  void expect_eq(const T &l, const T &r, const std::string &msg)
  {
    Record rec;
    rec.name = _currentTestName;
    if (l == r)
    {
      rec.pass = true;
      rec.feedback = "Passed!";
    }
    else
    {
      rec.pass = false;
      std::stringstream ss;
      ss << "  Expected: " << r << std::endl
         << "  Got: " << l << std::endl
         << "  Message: " << msg;
      rec.feedback = ss.str();
    }
    _insertRecord(rec);
  }

private:
  // Inserts a list of records in the record map if it doesn't exist, otherwise
  // appends a the given record to the record lists associated with the current
  // test.
  void _insertRecord(const Record &rec)
  {
    auto record = _records.find(_currentTest);

    if (record != _records.end())
    {
      record->second.push_back(rec);
    }
    else
    {
      _records.emplace(_currentTest, GERecords{rec});
    }
  }

  void _report()
  {
    float passed = 0;
    float tests = 0;
    std::for_each(
        _records.begin(),
        _records.end(),
        [&passed, &tests](const auto &pair) {
          std::for_each(
              std::begin(pair.second),
              std::end(pair.second),
              [&passed, &tests](const Record &rec) {
                tests++;
                if (rec.pass == true)
                {
                  passed++;
                }
                std::cout << rec << std::endl;
              });
        });

    int percentage = std::ceil((passed / tests) * 100);
    std::cout << percentage << "% of test passed" << std::endl;
  }

  // Prepares the environment for the next test. Might seen trivial now, but if
  // it the environment gets more complex, this can serve as a "next
  // configuration" function.
  void _nextTest()
  {
    _currentTest++;
  }

private:
  uint8_t _currentTest;
  std::string _currentTestName;
  std::vector<GETest> _tests;
  std::map<uint8_t, GERecords> _records;
};

// Generates a test to avoid boilerplate using the following semantics:
// test("name" { ... })
// the GravingEnv& is in the scope as env
#define test_f(Name, Function) \
{\
 Name, [&](GradingEnv& env) Function \
}
