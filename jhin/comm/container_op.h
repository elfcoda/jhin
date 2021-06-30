#pragma once

#include <set>
#include <unordered_set>
#include <tuple>
#include <stack>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <cassert>
#include <type_traits>
#include "algorithm.h"
#include "type.h"
#include "jhin_assert.h"

namespace jhin
{
namespace comm
{
    /* union second set to the first set */
    template <template<typename...> typename S,
              typename T = unsigned>
    void unionSet2Set(S<T>& fst, const S<T>&& snd)
    {
        static_assert(std::is_same_v<S<T>, std::unordered_set<T>> || std::is_same_v<S<T>, std::set<T>>, "S should be [unordered_]set type");
        for (T e: snd) fst.insert(e);
    }

    template <template<typename...> typename S,
              typename T = unsigned>
    void unionSet2Set(S<T>& fst, const S<T>& snd)
    {
        static_assert(std::is_same_v<S<T>, std::unordered_set<T>> || std::is_same_v<S<T>, std::set<T>>, "S should be [unordered_]set type");
        for (T e: snd) fst.insert(e);
    }

    /* map */
    template <template<typename...> typename M,
              typename M1,
              typename M2,
              template<typename...> typename S>
    void unionSet2Map(M<M1, M2>& fst, const S<M1>&& snd, M2 m2)
    {
        static_assert((std::is_same_v<M<M1, M2>, std::unordered_map<M1, M2>> || std::is_same_v<M<M1, M2>, std::map<M1, M2>>) &&
                      (std::is_same_v<S<M1>, std::unordered_set<M1>> || std::is_same_v<S<M1>, std::set<M1>>),
                      "M should be [unordered_]map type and S should be [unordered_]set type");
        for (const M1 e: snd) fst[e] = m2;
    }

    template <template<typename...> typename M,
              typename M1,
              typename M2,
              template<typename...> typename S>
    void unionSet2Map(M<M1, M2>& fst, const S<M1>& snd, M2 m2)
    {
        static_assert((std::is_same_v<M<M1, M2>, std::unordered_map<M1, M2>> || std::is_same_v<M<M1, M2>, std::map<M1, M2>>) &&
                      (std::is_same_v<S<M1>, std::unordered_set<M1>> || std::is_same_v<S<M1>, std::set<M1>>),
                      "M should be [unordered_]map type and S should be [unordered_]set type");
        for (const M1 e: snd) fst[e] = m2;
    }

    /* map to map */
    template <template<typename...> typename M,
              typename M1,
              typename M2>
    void unionMap2Map(M<M1, M2>& fst, const M<M1, M2>&& snd)
    {
        static_assert((std::is_same_v<M<M1, M2>, std::unordered_map<M1, M2>> || std::is_same_v<M<M1, M2>, std::map<M1, M2>>),
                      "M should be [unordered_]map type");
        for (const auto& e: snd) fst[e.first] = e.second;
    }

    template <template<typename...> typename M,
              typename M1,
              typename M2>
    void unionMap2Map(M<M1, M2>& fst, const M<M1, M2>& snd)
    {
        static_assert((std::is_same_v<M<M1, M2>, std::unordered_map<M1, M2>> || std::is_same_v<M<M1, M2>, std::map<M1, M2>>),
                      "M should be [unordered_]map type");
        for (const auto& e: snd) fst[e.first] = e.second;
    }

    /* stack */
    template <typename T>
    void stackPopN(std::stack<T>& st, unsigned n)
    {
        while (n--) {
            JHIN_ASSERT_BOOL(!st.empty());
            st.pop();
        }
    }

    template <typename T>
    std::vector<T> stackPopGetN(std::stack<T>& st, unsigned n)
    {
        std::vector<T> vs;

        while (n--) {
            JHIN_ASSERT_BOOL(!st.empty());
            vs.push_back(st.top());
            st.pop();
        }

        std::reverse(vs.begin(), vs.end());

        return vs;
    }

    template <typename T, typename... Ts>
    std::vector<T> stackPopNGetTupleFst(std::stack<std::tuple<T, Ts...>>& st, unsigned n)
    {
        std::vector<T> vIds;
        while (n--) {
            JHIN_ASSERT_BOOL(!st.empty());
            vIds.push_back(std::get<0>(st.top()));
            st.pop();
        }

        std::reverse(vIds.begin(), vIds.end());

        return vIds;
    }

    /* vector as stack */
    template <typename T>
    std::enable_if_t<is_vector<T>, void>
    stackPop(T&& vec)
    {
        JHIN_ASSERT_BOOL(!vec.empty());
        vec.pop_back();
    }

    template <typename T>
    std::enable_if_t<is_vector<T>,
                     typename std::decay_t<T>::value_type>
    stackTop(T&& vec)
    {
        JHIN_ASSERT_BOOL(!vec.empty());
        unsigned size = vec.size();
        return vec[size - 1];
    }

    template <typename T, typename Val>
    std::enable_if_t<is_vector<T>, void>
    stackPush(T&& vec, Val&& v)
    {
        // static_assert(std::is_same_v<typename std::decay_t<T>::value_type,
        //                              typename std::decay_t<Val>>,
        //               "value type error!");
        vec.push_back(std::forward<Val>(v));
    }

    /* vector */
    template <typename T, typename U>
    std::enable_if_t<is_vector<T> &&
                     is_vector<U> &&
                     is_val_type_same<T, U>,
                     void>
    mergeVec2Vec(T&& v1, const U&& v2)
    {
        for (const auto& it: v2) {
            /* r/l value of element follows container's */
            v1.push_back(std::forward<U>(it));
        }
    }

    template <typename T>
    void mergeVec2Vec_P(std::vector<T>* v1, const std::vector<T>* v2)
    {
        for (const auto& it: *v2) {
            /* r/l value of element follows container's */
            v1->push_back(it);
        }
    }

    template <typename T>
    std::vector<T> element2Vec(const T&& e)
    {
        std::vector<T> vec;
        vec.push_back(std::forward<T>(e));
        return vec;
    }

};  /* namespace comm */
};  /* namespace jhin */


