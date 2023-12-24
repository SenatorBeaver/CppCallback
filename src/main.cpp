#include "callback.hpp"
#include <iostream>

class ErrorPolicyIostream
{
  public:
    static void OnBadCall()
    {
        std::cout << "BAD CALL" << std::endl;
    }
};

template <typename Signature> using MyCallback = Callback<32, ErrorPolicyIostream, Signature>;

int main(int, char*[])
{
    int foobar  = 0;
    auto lambda = [&]() {
        std::cout << "foobar: " << foobar << std::endl;
        ++foobar;
    };

    MyCallback<void()> cb;
    MyCallback<void()> cb2;

    cb();

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
