#include "stdio.h"

double f2()
{
    double x1 = 1.12;
    double x2 = 1.13;
    x1 = x1 + x2;
    x1 = x1 - x2;
    x1 = x1 * x2;
    x1 = x1 / x2;
    return 1.2;
}
float f3()
{
    float x1 = 1.12;
    float x2 = 1.13;
    x1 = x1 + x2;
    x1 = x1 - x2;
    x1 = x1 * x2;
    x1 = x1 / x2;
    return 1.2;
}
float f()
{
    float x1 = 1.12;
    float x2 = 1.13;
    x1 = x1 + x2;
    x1 = x1 - x2;
    x1 = x1 * x2;
    x1 = x1 / x2;
    return 1.2;
}

int main()
{
    f();
    return 0;
}
