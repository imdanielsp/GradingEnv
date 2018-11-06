# Minimal Testing Framework
This is a header-only lightweight test framework that can be used by just including `mtest.hpp` into your source code.

**Note**: this requires C++17. What does this mean? When compiling your project, add `--std=c++17` to your compiler invocation command. If C++17 is available, you should be all set.

How to use:
```cpp
#include "mtest.hpp"

int main()
{
  MTEnv mtEnv;

  mtEnv.add_test({
    test_f("Test Name", "Message if fails", {
      env.expect_eq<int>(3, 3);
      env.expect_true(9 == 9);
    }),
    test_f("Another Test Name", "Message if fails", {
      env.expect_neq<std::string>("a", "b");
    })
  });

  mtEnv.run_all();
  return 0;
}
```

For more, look into the example folder.