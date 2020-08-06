#ifndef __CONTAINER_OP_H__
#define __CONTAINER_OP_H__

#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <cassert>
#include <type_traits>

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


};  /* namespace comm */
};  /* namespace jhin */

#endif

