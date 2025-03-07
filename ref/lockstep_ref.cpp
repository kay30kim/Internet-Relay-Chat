#include "lockstep.hpp"

#include <array>
#include <vector>
#include <iostream>

int main()
{
    std::array<int, 3> a {42, 99, -1};
    std::cout << "ARRAY: ";
    for (auto& m : a)
        std::cout << m << ", ";
    std::cout << std::endl;

    std::vector<double> v {2.1, 3.2, 4.3};
    std::cout << "VECTOR: ";
    for (auto& m : v)
        std::cout << m << ", ";
    std::cout << std::endl;

    auto x = lockstep::lockstep(a, v);

    int i = 0;
    for (auto b = x.begin(); i < std::min(a.size(), v.size()); ++b, ++i)
    {
        const auto& [first, second] = *b;
        std::cout << "a[" << i << "]: " << first << "\tv[" << i << "]: " << second << std::endl;
    }

    return 0;
}