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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINMTMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAMTS OR OTHER
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
#include <memory>
#include <optional>

// Represents a record for a test
using MTRecord = struct Record
{
  bool pass;
  bool printable;
  std::optional<std::string> reason;
  friend std::ostream &operator<<(std::ostream &os, const struct Record &rec);
};

using MTRecords = std::vector<Record>;

std::ostream &operator<<(std::ostream &os, const struct Record &rec)
{
  std::stringstream ss;
  if (!rec.pass && rec.printable)
  {
    ss << "  \u001b[31m[TEST CASE FAILED]" << std::endl
       << "    Reason: " << rec.reason.value_or("no provided")
       << "\033[0m";
  }

  os << ss.str();
  return os;
}

// Class Forwarding
class MTEnv;

//
using MTFunction = std::function<void(MTEnv &env)>;

// Represents a test
using MTTest = struct MTTest
{
  int id;
  std::string name;
  std::string feedback;
  MTFunction f;
};

class MTEnv
{
public:
  MTEnv(bool showComp = false) : _currentTest(nullptr)
  {
  }

  ~MTEnv() = default;

  // Non-copyable and non-movable
  MTEnv(MTEnv &) = delete;
  MTEnv(MTEnv &&) = delete;
  MTEnv &operator=(MTEnv &) = delete;
  MTEnv &&operator=(MTEnv &&) = delete;

  // Adds a test to the environment. A test is a tuple containing:
  // (name, feedback, function)
  void add_test(std::tuple<std::string, std::string, MTFunction> t)
  {
    auto [name, feedback, func] = t;
    _tests.push_back(MTTest{MTEnv::_idCounter, name, feedback, func});
    MTEnv::_idCounter++;
  }

  // Same as above but takes a list of tuples in the following form:
  // { (name, feedback, function)* }
  void add_test(
      const std::initializer_list<std::tuple<std::string, std::string, MTFunction>> &tests)
  {
    for (const auto &test : tests)
    {
      add_test(test);
    }
  }

  // Run all the test in the environment
  void run_all(bool report = true, bool verbose = false)
  {
    std::for_each(
        _tests.begin(),
        _tests.end(),
        [this](MTTest &it) {
          _currentTest.release();
          _currentTest = std::make_unique<MTTest>(it);
          // Each function needs the environment, thus *this
          it.f(*this);
        });

    if (report)
      _report(verbose);
  }

  template <typename... Types,
            typename Comp = std::function<bool(const Types &...)>>
  void expect(Types... args, std::string reason, Comp c, bool printable)
  {
    Record rec;
    rec.printable = printable;
    if (c(args...) == true)
    {
      rec.pass = true;
      rec.reason = {};
    }
    else
    {
      rec.pass = false;

      std::stringstream ss;
      ss << "\u001b[31m" << reason << "\033[0m" << std::endl;
      rec.reason = ss.str();
    }
    _insertRecord(rec);
  }

  // Assert equality within the environment
  template <typename T>
  void expect_eq(const T &l, const T &r, bool printable = true)
  {
    std::stringstream reason;
    reason << l << " != " << r;

    expect<T, T>(l, r, reason.str(),
                 [](const T &l, const T &r) {
                   return l == r;
                 }, printable);
  }

  template <typename T>
  void expect_neq(const T &l, const T &r, bool printable = true)
  {
    std::stringstream reason;
    reason << l << " == " << r;

    expect<T, T>(l, r, reason.str(),
                 [](const T &l, const T &r) {
                   return l != r;
                 }, printable);
  }

  void expect_true(bool val, bool printable = true)
  {
    std::stringstream reason;
    reason << "value is false";

    expect<bool>(val, reason.str(),
                 [](bool val) {
                   return val == true;
                 }, printable);
  }

  void expect_false(bool val, bool printable = true)
  {
    std::stringstream reason;
    reason << "value is true";

    expect<bool>(val, reason.str(),
                 [](bool val) {
                   return val == false;
                 }, printable);
  }

private:
  // Inserts a list of records in the record map if it doesn't exist, otherwise
  // appends a the given record to the record lists associated with the current
  // test.
  void _insertRecord(const Record &rec)
  {
    auto record = _records.find(_currentTest->id);

    if (record != _records.end())
    {
      record->second.push_back(rec);
    }
    else
    {
      _records.emplace(_currentTest->id, MTRecords{rec});
    }
  }

  void _report(bool verbose)
  {
    float passedCounter = 0;
    float testCounter = 0;

    std::for_each(
        _records.begin(),
        _records.end(),
        [&](const auto &pair) {
          int failedRecord = 0;
          testCounter++;

          MTTest test = _tests.at(pair.first);
          std::cout << "\u001b[32m[RUNNING " << test.name << "]\033[0m" << std::endl;
          // For each record in the test
          std::for_each(
              std::begin(pair.second),
              std::end(pair.second),
              [&](const MTRecord &rec) {
                if (rec.pass == false)
                {
                  failedRecord++;
                }

                std::cout << rec;
              });

          // If verbose is on and test passed, print
          if (failedRecord == 0)
          {
            passedCounter++;
            std::cout << "  \u001b[32m[PASSED]\033[0m" << std::endl;
          } else {
            std::cout << "  \u001b[31mFeedback: "
                      << test.feedback
                      << "\033[0m"
                      << std::endl;
          }
        });

    int percentage = std::ceil((passedCounter / testCounter) * 100);
    std::cout << std::endl
              << "\u001b[32m"
              << percentage
              << "% of test passed\033[0m"
              << std::endl;
  }

private:
  static int _idCounter;
  std::unique_ptr<MTTest> _currentTest;
  std::vector<MTTest> _tests;
  std::map<int, MTRecords> _records;
};

int MTEnv::_idCounter = 0;

// Generates a test to avoid boilerplate using the following semantics:
// test_f("name" { ... })
// the GravingEnv& is in the scope as env
#define test_f(Name, Feedback, Scope)           \
  {                                             \
    Name, Feedback, [&](MTEnv & env) Scope \
  }\
