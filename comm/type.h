#pragma once

#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <list>
#include <stack>
#include <queue>
#include <deque>
#include <type_traits>

namespace jhin
{
namespace comm
{

/* is_string */
template <class T>
constexpr bool is_string = std::is_same_v<typename std::decay_t<T>,
                                          std::string>;

/* is_vector */
template <typename T>
struct is_container_vector_helper
{
    constexpr static bool is_vector = false;
};
template <typename... U>
struct is_container_vector_helper<std::vector<U...>>
{
    constexpr static bool is_vector = true;
};

template <typename T>
struct is_container_vector
{
    using DT = typename std::decay_t<T>;
    constexpr static bool is_vector = is_container_vector_helper<DT>::is_vector;
};
template <typename T>
constexpr bool is_container_vector_v = is_container_vector<T>::is_vector;

/* is_vector */
template <typename T>
constexpr bool is_vector = std::is_same_v<typename std::decay_t<T>,
                                          std::vector<typename std::decay_t<T>::value_type,
                                                      typename std::decay_t<T>::allocator_type>>;

/* is_container */
template <typename T, typename _ = void>
struct is_container: std::false_type {};

template <typename... Ts>
struct is_container_helper {};

template <typename T>
struct is_container<T,
                    std::conditional_t<false,
                                       is_container_helper<typename T::value_type,
                                                           typename T::allocator_type,
                                                           typename T::size_type,
                                                           typename T::iterator,
                                                           typename T::const_iterator,
                                                           decltype(std::declval<T>().size()),
                                                           decltype(std::declval<T>().begin()),
                                                           decltype(std::declval<T>().end()),
                                                           decltype(std::declval<T>().cbegin()),
                                                           decltype(std::declval<T>().cend())
                                                           >,
                                       void
                                       >
                    >: std::true_type {};

/* which container */
enum EContainerType
{
    ECT_ERROR,
    ECT_SET,
    ECT_UNORDERED_SET,
    ECT_MAP,
    ECT_UNORDERED_MAP,
    ECT_LIST,
    ECT_VECTOR,
    ECT_STACK,
    ECT_QUEUE,
    ECT_DEQUE,
};
template <typename T>
struct container_type_helper
{
    constexpr static EContainerType value = ECT_ERROR;
};

template <typename... U>
struct container_type_helper<std::set<U...>>
{
    constexpr static EContainerType value = ECT_SET;
};

template <typename... U>
struct container_type_helper<std::unordered_set<U...>>
{
    constexpr static EContainerType value = ECT_UNORDERED_SET;
};

template <typename... U>
struct container_type_helper<std::map<U...>>
{
    constexpr static EContainerType value = ECT_MAP;
};

template <typename... U>
struct container_type_helper<std::unordered_map<U...>>
{
    constexpr static EContainerType value = ECT_UNORDERED_MAP;
};

template <typename... U>
struct container_type_helper<std::list<U...>>
{
    constexpr static EContainerType value = ECT_LIST;
};

template <typename... U>
struct container_type_helper<std::vector<U...>>
{
    constexpr static EContainerType value = ECT_VECTOR;
};

template <typename... U>
struct container_type_helper<std::stack<U...>>
{
    constexpr static EContainerType value = ECT_STACK;
};

template <typename... U>
struct container_type_helper<std::queue<U...>>
{
    constexpr static EContainerType value = ECT_QUEUE;
};

template <typename... U>
struct container_type_helper<std::deque<U...>>
{
    constexpr static EContainerType value = ECT_DEQUE;
};

template <typename T>
struct container_type
{
    using DT = typename std::decay_t<T>;
    constexpr static EContainerType value = container_type_helper<DT>::value;
};
template <typename T>
constexpr EContainerType container_type_v = container_type<T>::value;

/* value type */
template <typename T, typename U>
constexpr bool is_val_type_same = std::is_same_v<typename std::decay_t<T>::value_type,
                                                 typename std::decay_t<T>::value_type>;


};  /* namespace comm */
};  /* namespace jhin */

