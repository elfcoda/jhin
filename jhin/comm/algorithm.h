#pragma once

#include <vector>
#include "type.h"
#include "jhin_assert.h"

namespace jhin
{
namespace comm
{

/* 这个算法目前没有用到，先废弃
 * for production E -> A B C D C, if ParseTree has deduction E -> C D, function return {2, 3}, means other symbols may be cut by AST.
 * if the number of C in PArseTree is exactly the same as that in production, we just match from the start symbol.
 * if not, this indicate that C may be EPSILON, so other non-EPSILON symbols must appear between Cs. in this case, we can also match C with first C of production
 *
 * @std::pair: symbol -> v1_Idx */
template <typename T>
std::vector<std::pair<unsigned, unsigned>> matchProductionIdx(const std::vector<T>& v1, const std::vector<T>& v2)
{
    std::vector<std::pair<unsigned, unsigned>> ans;
    unsigned len1 = v1.size(), len2 = v2.size();
    for (unsigned i1 = 0, i2 = 0; i2 < len2; i2++) {
        while (i1 < len1 && v1[i1] != v2[i2]) i1++;
        JHIN_ASSERT_BOOL(i1 < len1);
        ans.push_back(std::make_pair(v2[i2], i1++));
    }

    return ans;
}

// template <typename T, typename U>
// std::enable_if_t<is_vector<T> && is_vector<U> && std::is_same_v<typename T::value_type, typename U::value_type>, std::vector<unsigned>> matchProductionIdx(const T&& v1, const U&& v2)
// {
//     std::vector<unsigned> ans;
//     unsigned len1 = v1.size(), len2 = v2.size();
//     for (unsigned i1 = 0, i2 = 0; i2 < len2; i2++) {
//         while (i1 < len1 && v1[i1] != v2[i2]) i1++;
//         JHIN_ASSERT_BOOL(i1 < len1);
//         ans.push_back(i1++);
//     }
//
//     return ans;
// }


/* util functions */
template <typename T>
std::string getPtrString(const T* const t)
{
    if (t == nullptr) return "nullptr_str";
    return t->toString();
}


};  /* namespace comm */
};  /* namespace jhin */

