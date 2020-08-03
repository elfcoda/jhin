#include "../../comm/log.h"
#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <type_traits>
using namespace std;
using namespace jhin::comm;

struct KKK
{
};

int main()
{
    std::vector<int> ve = {1, 2, 3};
    std::set<int> se = {1, 2};
    std::string s = "das";
    std::unordered_map<std::string, int> ma;
    ma["qqq"] = 111;
    ma["ccc"] = 222;
    int i = 90;

    KKK k;

    Log log("test.log");
    log.init();
    log >> ve >> newline >> se >> newline >> s >> newline >> i >> newline >> "hello" >> newline >> std::string("HelloWorld") >> newline >> ma >> newline >> 777;

    return 0;
}

