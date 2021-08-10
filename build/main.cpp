#include <iostream>
using namespace std;

extern "C"
{
    float func();
}

int main()
{
    cout << func() << endl;
    return 0;
}
