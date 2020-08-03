#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <fstream>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>

namespace jhin
{
namespace comm
{
    template <class T>
    constexpr bool isString = std::is_same_v<typename std::decay<T>::type, std::string>;

    template <class T>
    constexpr bool isShowType = std::is_arithmetic_v<T> || isString<T>;

    const std::string sErr = "ERROR: ";

    struct LogNewLine {} newline;

    class Log
    {
        public:
            Log(const std::string& filename): filename(filename) {}

            bool init()
            {
                file.open(filename);
                return true;
            }

            /* write */
            /* trivial case */
            template <class T>
            std::ostream& write(T&& t)
            {
                std::string s = std::to_string(t);
                return file.write(s.c_str(), s.length());
            }

            /* string case */
            // template <>
            // std::ostream& write<const std::string&>(const std::string& s)
            // {
            //     return file.write(s.c_str(), s.length());
            // }

            /* new line case */
            template <>
            std::ostream& write<const LogNewLine&>(const LogNewLine& n)
            {
                std::string s = "\n";
                return file.write(s.c_str(), s.length());
            }

            /* const char* case */
            std::ostream& write(const char* c)
            {
                std::string s(c);
                return file.write(s.c_str(), s.length());
            }

            /* container case */
            template <template<class...> class C,
                      class... As>
            std::ostream& write(const C<As...>& container)
            {
                std::string s = sErr + "container can not display!";

                /* test */
                // static_assert(sizeof...(As) == 3, "err");

                if constexpr (sizeof...(As) == 2) {
                    if constexpr (std::is_same_v<C<As...>, std::vector<As...>>) {
                        s = genSingleStr(container);
                    }
                } else if constexpr (sizeof...(As) == 3) {
                    if constexpr (std::is_same_v<C<As...>, std::set<As...>>) {
                        s = genSingleStr(container);
                    } else if constexpr (isString<C<As...>>) {
                        s = container;
                    }
                } else if constexpr (sizeof...(As) == 4) {
                    if constexpr (std::is_same_v<C<As...>, std::unordered_set<As...>>) {
                        s = genSingleStr(container);
                    } else if constexpr (std::is_same_v<C<As...>, std::map<As...>>) {
                        s = genMapStr(container);
                    }
                } else if constexpr (sizeof...(As) == 5) {
                    if constexpr (std::is_same_v<C<As...>, std::unordered_map<As...>>) {
                        s = genMapStr(container);
                    }
                }

                return file.write(s.c_str(), s.length());
            }

            /* single parameter
             * std::set, std::unordered_set, std::vector...
             * */
            template <template<class...> class S,
                      class A>
            std::string genSingleStr(const S<A>& se)
            {
                std::string s = "";

                if constexpr (isShowType<A>) {
                    /* A is arithmetic type */
                    s += "{";
                    for (A item: se) {
                        if constexpr (isString<A>) {
                            s += item + ", ";
                        } else {
                            s += std::to_string(item) + ", ";
                        }
                    }

                    /* pop back redundant chars */
                    if (!se.empty()) {
                        s.pop_back();
                        s.pop_back();
                    }

                    s += "}\n";
                } else {
                    s = sErr + "element is not arithmetic type!";
                }

                return s;
            }

            template <template<class...> class M,
                      class K,
                      class V>
            std::string genMapStr(const M<K, V>& ma)
            {
                std::string s = "";

                if constexpr (isShowType<K> && isShowType<V>) {
                    /* K and V are arithmetic types */
                    s += "{";
                    for (const auto& item: ma) {
                        /* Key */
                        if constexpr (isString<K>) {
                            s += item.first + ": ";
                        } else {
                            s += std::to_string(item.first) + ": ";
                        }
                        /* Value */
                        if constexpr (isString<V>) {
                            s += item.second + ", ";
                        } else {
                            s += std::to_string(item.second) + ", ";
                        }
                    }

                    /* pop back redundant chars */
                    if (!ma.empty()) {
                        s.pop_back();
                        s.pop_back();
                    }

                    s += "}\n";
                } else if constexpr (!isShowType<K>) {
                    s = "Key is not arithmetic type!";
                } else {
                    s = "Value is not arithmetic type!";
                }

                return s;
            }

            /* op >> */
            template <class T>
            Log& operator >>(T&& t)
            {
                /* decay type */
                using DT = typename std::decay<T>::type;
                write(static_cast<const DT&>(t));

                return *this;
            }

            ~Log() { file.close(); }

        private:
            std::string filename;
            std::ofstream file;

    };

};  /* namespace comm */
};  /* namespace jhin */

#endif

