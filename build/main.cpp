#include <iostream>
using namespace std;

extern "C"
{
    double func();
}

int main()
{
    cout << func() << endl;
    return 0;
}
