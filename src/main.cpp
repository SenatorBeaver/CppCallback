#include "callback.hpp"
#include <chrono>
#include <functional>
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
    int foobar      = 0;
    auto lambda     = [&]() { ++foobar; };
    using Signature = void();

    MyCallback<Signature> cb;
    MyCallback<Signature> cb2;
    std::function<Signature> f;

    cb();

    cb  = lambda;
    cb2 = cb;
    f   = lambda;

    { // Start measuring time
        auto start_time = std::chrono::high_resolution_clock::now();

        // Your code to be measured goes here
        for (int i = 0; i < 100'000'000; ++i) {
            cb();
        }

        // Stop measuring time
        auto end_time = std::chrono::high_resolution_clock::now();

        // Calculate the duration
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        // Output the duration
        std::cout << "Execution time MyCallback:    " << duration.count() << " microseconds" << std::endl;
    }
    { // Start measuring time
        auto start_time = std::chrono::high_resolution_clock::now();

        // Your code to be measured goes here
        for (int i = 0; i < 100'000'000; ++i) {
            f();
        }

        // Stop measuring time
        auto end_time = std::chrono::high_resolution_clock::now();

        // Calculate the duration
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        // Output the duration
        std::cout << "Execution time std::function: " << duration.count() << " microseconds" << std::endl;
    }
    return 0;
}
