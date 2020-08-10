#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <fstream>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>
#include <mutex>



namespace jhin
{
    enum ELogLevel
    {
        DEBUG = 1,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };

    const std::string sDEBUG    = "DEBUG: ";
    const std::string sINFO     = "INFO: ";
    const std::string sWARN     = "WARN: ";
    const std::string sERROR    = "ERROR: ";
    const std::string sFATAL    = "FATAL: ";
    const std::unordered_map<ELogLevel, std::string> mLevel = {{ELogLevel::DEBUG, sDEBUG},
                                                               {ELogLevel::INFO, sINFO},
                                                               {ELogLevel::WARN, sWARN},
                                                               {ELogLevel::ERROR, sERROR},
                                                               {ELogLevel::FATAL, sFATAL}};

    /* current log level settings */
    const ELogLevel WRITE_LEVEL = INFO;
    const std::string WRITE_FILE_INFOMATION = "WRITE_LEVEL is: " + std::to_string(static_cast<unsigned>(WRITE_LEVEL)) + "\n";

};  /* namespace jhin */

namespace jhin
{
namespace comm
{
    template <class T>
    constexpr bool isString = std::is_same_v<typename std::decay<T>::type, std::string>;

    template <class T>
    constexpr bool isShowType = std::is_arithmetic_v<T> || isString<T>;

    const std::string sLogFilename = "log.log";

    struct LogNewLine {} newline;

    class Log
    {
        private:
            Log(const std::string& writefilename) {}

        public:
            static std::mutex mtx;
            static Log log;
            static bool isInited;
            static ELogLevel logLevel;
            static Log& singleton(ELogLevel level, bool showLevel, std::string writefilename);


            bool init()
            {
                file.open(filename);
                return true;
            }

        public:
            /* get write string */
            /* trivial case */
            template <class T>
            std::string getWriteString(T&& t)
            {
                std::string s = std::to_string(t);
                return s;
            }

            /* string case */
            // template <>
            // std::string getWriteString<const std::string&>(const std::string& s)
            // {
            //     return s;
            // }

            /* new line case */
            template <>
            std::string getWriteString<const LogNewLine&>(const LogNewLine& n)
            {
                std::string s = "\n";
                return s;
            }

            /* const char* case */
            std::string getWriteString(const char* c)
            {
                std::string s(c);
                return s;
            }

            /* container case */
            template <template<class...> class C,
                      class... As>
            std::string getWriteString(const C<As...>& container)
            {
                std::string s = genContainerString(container);
                return s;
            }

            /* generate container string */
            template <template<class...> class C,
                      class... As>
            std::string genContainerString(const C<As...>& container)
            {
                std::string s = "{" + sERROR + "container can not display!}";

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

                return s;
            }

            template <class T>
            std::string genContainerString(const T& data)
            {
                std::string s = "{" + sERROR + "data can not display!}";
                return s;
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
                    s = "{\n";
                    for (const A& subContainer: se) {
                        s += genContainerString(subContainer);
                    }
                    s += "}\n";
                }

                return s;
            }

            template <template<class...> class M,
                      class K,
                      class V>
            std::string genMapStr(const M<K, V>& ma)
            {
                std::string s = "";

                /* K and V are arithmetic types */
                s += "{";
                for (const auto& item: ma) {
                    /* Key */
                    if constexpr (isString<K>) {
                        s += item.first + " => ";
                    } else if constexpr (std::is_arithmetic_v<K>) {
                        s += std::to_string(item.first) + " => ";
                    } else {
                        s += genContainerString(item.first) + " => ";
                    }

                    /* Value */
                    if constexpr (isString<V>) {
                        s += item.second + ", ";
                    } else if constexpr (std::is_arithmetic_v<V>) {
                        s += std::to_string(item.second) + ", ";
                    } else {
                        s += genContainerString(item.second);
                    }
                }

                /* pop back redundant chars */
                if constexpr (isShowType<V>) {
                    if (!ma.empty()) {
                        s.pop_back();
                        s.pop_back();
                    }
                }

                s += "}\n";

                return s;
            }

            /* write to file */
            static std::ostream& write(const std::string& s);

            /* switch to another file */
            static bool switchFile(const std::string& writefilename);

            /* op >> */
            template <class T>
            Log& operator >>(T&& t)
            {
                if (Log::logLevel < WRITE_LEVEL) return *this;

                /* decay type */
                using DT = typename std::decay<T>::type;
                std::string s = getWriteString(static_cast<const DT&>(t));

                write(s);

                return *this;
            }

            ~Log() { file.close(); }

        private:
            static std::string filename;
            static std::ofstream file;

    };
    std::mutex Log::mtx;
    Log Log::log(sLogFilename);
    bool Log::isInited = false;
    ELogLevel Log::logLevel = DEBUG;
    std::ofstream Log::file;
    std::string Log::filename = sLogFilename;

    /* write to file */
    std::ostream& Log::write(const std::string& s)
    {
        return file.write(s.c_str(), s.length());
    }

    /* switch to another file */
    bool Log::switchFile(const std::string& writefilename)
    {
        if (writefilename == filename) return true;

        file.close();
        filename = writefilename;
        file.open(filename);

        /* write infomation */
        write(WRITE_FILE_INFOMATION);

        return true;
    }

    Log& Log::singleton(ELogLevel level = DEBUG, bool showLevel = true, std::string writefilename = sLogFilename)
    {
        Log::logLevel = level;

        if (!isInited) {
            const std::lock_guard<std::mutex> lock(mtx);
            if (!isInited) {
                log.init();
                isInited = true;
                /* write infomation */
                write(WRITE_FILE_INFOMATION);
            }
        }

        if (writefilename != filename) {
            switchFile(writefilename);
        }

        if (Log::logLevel >= WRITE_LEVEL && showLevel) {
            std::string s = mLevel.at(Log::logLevel);
            write(s);
        }

        return log;
    }

};  /* namespace comm */
};  /* namespace jhin */

#endif


