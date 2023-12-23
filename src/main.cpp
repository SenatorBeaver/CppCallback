#include "callback.hpp"
#include <iostream>

int main(int, char*[])
{
    int foobar  = 0;
    auto lambda = [&]() {
        std::cout << "foobar: " << foobar << std::endl;
        ++foobar;
    };

    Callback<void()> cb;
    Callback<void()> cb2;

    cb  = lambda;
    cb2 = cb;

    for (unsigned i = 0; i < 10; ++i) {
        cb();
    }
    for (unsigned i = 0; i < 10; ++i) {
        cb2();
    }
    return 0;
}
