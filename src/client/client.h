#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <vector>
#include <stack>
#include <string>
#include "../lex/keywords.h"
#include "../../comm/dfa.h"

namespace jhin
{
namespace client
{

template <typename T>
void stackPopN(std::stack<T>& st, unsigned n)
{
    while (n--) {
        assert(!st.empty());
        st.pop();
    }
}

template <class T>
std::string stack2String(std::stack<T> st)
{
    std::string s = "";
    while (!st.empty()) {
        s += std::to_string(st.top());
        st.pop();
    }
    s += "\n";

    return s;
}

std::string stack2String(std::stack<std::pair<unsigned, jhin::comm::pSyntaxDFA>> st)
{
    std::string s = "stack: #\n";
    while (!st.empty()) {
        std::pair<unsigned, jhin::comm::pSyntaxDFA> pa = st.top();
        s += std::to_string(pa.first) + ": " + std::to_string(pa.second->id) + "\n";
        st.pop();
    }
    s += "$\n";

    return s;
}

};  /* namespace client */
};  /* namespace jhin */

#endif

