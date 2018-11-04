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
  env.expect_eq<std::string>("Test1", "Test1", "The string aren't equal");
}

int main(int argc, char const *argv[])
{
  GradingEnv ge;

  // A test is a name and a function, e.g. {"name", f1}.
  ge.add_test({"Example", exampleTest});

  // A lambda can also be used.
  ge.add_test({"Test",
    [](GradingEnv& env) {
      env.expect_eq<int>(1, 2, "The test failed!");
    }
  });

  // You can add multiple tests at the same time
  ge.add_test({
    {
      "Example2",
      exampleTest
    },
    {
      "Addition",
      [](GradingEnv& env) {
        env.expect_eq<int>(1, 2, "The test failed!");
      }
    },
    {
      "Division",
      [](GradingEnv& env) {
        env.expect_eq<int>(2, 2, "The test failed!");
      }
    },
  });

  // Run all tests
  ge.run_all();

  return 0;
}
