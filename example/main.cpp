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
#include <iostream>
#include "../grading.hpp"

void exampleTest(GradingEnv& env)
{
  // The third parameter is a message that is only displayed iff the test fails.
  env.expect_eq<std::string>("Test1", "Test1");
}

int main(int argc, char const *argv[])
{
  GradingEnv ge;

  // A test is a name and a function, e.g. {"name", "feedback", f1}.
  ge.add_test({"Example", "Error message", exampleTest});

  // A lambda can also be used.
  ge.add_test({"Test", "Error message",
    [](GradingEnv& env) {
      env.expect_eq<int>(1, 2);
      env.expect_false(1 == 2);
    }
  });

  // You can add multiple tests at the same time
  ge.add_test({
    {
      "Example2",
      "Error message",
      exampleTest
    },
    {
      "Addition",
      "Error message",
      [](GradingEnv& env) {
        env.expect_eq<int>(1, 2);
        env.expect_true(1 == 1);
      }
    },
    {
      "Division",
      "Error message",
      [](GradingEnv& env) {
        env.expect_eq<int>(2, 2);
      }
    },
  });

  // Add tests with some macro magic
  ge.add_test({
    test_f("Test name", "Feedback", {
      env.expect_false(9 == 2);
    }),
    test_f("Test name", "Feedback", {
      env.expect_eq<std::string>("a", "b");
    }),
  });

  ge.add_test(
    test_f(
      "Testing using loops", 
      "We know there are some numbers that are equal :)", {
      for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
          env.expect_neq(i, j);
        }
      }
    })
  );

  // Run all tests
  ge.run_all(true, true);

  return 0;
}
