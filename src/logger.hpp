#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <sstream>

#define ENHANCER_POS (__FILE__ ":" + std::to_string(__LINE__))

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
            DEBUG_LOG,
            DEBUG_PER_FRAME_LOG
        };

        /**
         * @brief Set maximal level of logging. By default, INFO_LOG
         *
         * @param level new maxima
         */
        void setMaximumLevel(LogLevel level);

        /// Get currently set maximum
        LogLevel getMaximumLevel() const;

        /// Debug Per-Frame: Increment current frame number
        void incrementFrameNumber();

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

        /// Force flush
        void flush();


        inline static const std::string ToString() { return ""; }
        inline static const std::string ToString(const char* str) { return str; }
        inline static const std::string ToString(const std::string str) { return str; }
        inline static const std::string ToString(int arg) { return std::to_string(arg); }
        inline static const std::string ToString(unsigned int arg) { return std::to_string(arg); }
        inline static const std::string ToString(unsigned long int arg) { return std::to_string(arg); }
        inline static const std::string ToString(double arg) { return std::to_string(arg); }
        inline static const std::string ToString(void* ptr)
        {
            std::ostringstream out;
            out << ptr;
            return out.str();
        }

        inline static const std::string ToStringVariadic() { return ""; };

        template<typename T, typename ...ARGS>
        inline static const std::string ToStringVariadic(T arg, ARGS... args)
        {
            return ToString(arg) + " " + ToStringVariadic(args...);
        }

        template<typename ...ARGS, unsigned _Level = LogLevel::INFO_LOG>
        inline static void log(ARGS... msgParts)
        {
            const auto logResult = ToStringVariadic(msgParts...)+"\n";
            getInstance().printLog(logResult, static_cast<LogLevel>(_Level));
        }

        template<typename ...ARGS>
        inline static void logError(ARGS... msgParts)
        {
            const auto logResult = ToStringVariadic(msgParts...)+"\n";
            getInstance().printLog(logResult, static_cast<LogLevel>(LogLevel::ERROR_LOG));
            getInstance().flush();
        }

        template<typename ...ARGS>
        inline static void logDebug(ARGS... msgParts)
        {
            const auto logResult = "Debug: "+ToStringVariadic(msgParts...)+"\n";
            getInstance().printLog(logResult, static_cast<LogLevel>(LogLevel::DEBUG_LOG));
        }
        template<typename ...ARGS>
        inline static void logDebugPerFrame(ARGS... msgParts)
        {
            const auto logResult = "Debug: "+ToStringVariadic(msgParts...)+"\n";
            getInstance().printLog(logResult, static_cast<LogLevel>(LogLevel::DEBUG_PER_FRAME_LOG));
        }

        private:
        void printLogBanner(LogLevel level);

        LogLevel m_maximalLogLevel = DEBUG_PER_FRAME_LOG;

        size_t m_currentFrameID = 0;
    };
} //namespace ve

#endif
