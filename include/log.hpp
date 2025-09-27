/**
 * @file log.hpp
 * @brief Thread-safe logging utility for chat application.
 */

#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <iostream>
#include <mutex>

using namespace std;

/**
 * @brief Simple thread-safe logger class.
 *
 * Provides methods to open, close, and log messages to a file.
 */
class ChatLogger {
  private:
    ofstream logFile;
    mutex mtx;

  public:
    bool open(const string &filename);
    void close();
    bool log(const string &msg);
};

/**
 * @brief Log an informational message.
 * @param msg The message to log.
 * @param logger Reference to the ChatLogger instance.
 */
inline void LOG_INFO(const string &msg, ChatLogger &logger) {
    cout << "[INFO] " << msg << endl;
    if (!logger.log("[INFO] " + msg)) {
        cerr << "[ERROR] Failed to log INFO message." << endl;
    }
}

/**
 * @brief Log an error message.
 * @param msg The message to log.
 * @param logger Reference to the ChatLogger instance.
 */
inline void LOG_ERROR(const string &msg, ChatLogger &logger) {
    cerr << "[ERROR] " << msg << endl;
    if (!logger.log("[ERROR] " + msg)) {
        cerr << "[ERROR] Failed to log ERROR message." << endl;
    }
}

/**
 * @brief Log a debug message (only if DEBUG is defined).
 * @param msg The message to log.
 * @param logger Reference to the ChatLogger instance.
 */
inline void LOG_DEBUG(const string &msg, ChatLogger &logger) {
#ifdef DEBUG
    cout << "[DEBUG] " << msg << endl;
    if (!logger.log("[DEBUG] " + msg)) {
        cerr << "[ERROR] Failed to log DEBUG message." << endl;
    }
#else
    (void)msg;
    (void)logger;
#endif
}

extern ChatLogger logger;

#endif