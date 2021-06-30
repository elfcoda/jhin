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
    KKK k1, k2;
    std::map<int, std::vector<KKK>> ve;
    ve[1] = {k1};
    ve[2] = {k2};
    std::map<int, std::vector<std::vector<int>>> mvv;
    mvv[5] = {{12, 123}, {45, 567, 76}};
    mvv[1] = {{12, 3}, {45, 76}};
    std::set<int> se = {1, 2};
    std::string s = "das";
    std::unordered_map<std::string, int> ma;
    ma["qqq"] = 111;
    ma["ccc"] = 222;
    int i = 90;

    KKK k;

    Log::singleton() >> se >> newline >> s >> newline >> i >> newline >> "hello" >> newline >> std::string("HelloWorld") >> newline >> ma >> newline >> 777 >> newline;
    Log::singleton() >> 1234567 >> newline;

    return 0;
}

