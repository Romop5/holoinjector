#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fmt/format.h>

namespace ve
{
    class Logger
    {
        Logger() = default;
        public:
        static Logger& getInstance();

        /**
         * @brief Defines levels of message. Lower the level,
         * more important the message is for being logged.
         */
        enum LogLevel
        {
            ERROR_LOG = 0,
            INFO_LOG,
            DEBUG_LOG
        };

        /**
         * @brief Set maximal level of logging. By default, INFO_LOG
         *
         * @param level new maxima
         */
        void setMaximumLevel(LogLevel level);
        /**
         * @brief Writes message into log stream
         *
         * @param msg Raw message string
         * @param level Message level
         *
         * log() method serves for logging message into log stream. Each
         * message has a level, which is then used by logger to prevent 
         * logging excessive amount of messages. This is can controlled 
         * by setMaximumLevel(). By default, INFO logs are dumped at least.
         */
        void printLog(const std::string& msg, LogLevel level = LogLevel::INFO_LOG);

        template<typename ...ARGS, unsigned _Level = LogLevel::INFO_LOG>
        inline static void log(const std::string& msg, ARGS... types)
        {
            const auto logResult = fmt::format(msg, types...);
            getInstance().printLog(logResult, static_cast<LogLevel>(_Level));
        }
        private:
        LogLevel m_maximalLogLevel = INFO_LOG;
    };
} //namespace ve

#endif
