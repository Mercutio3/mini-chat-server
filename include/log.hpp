/*
log.h - ChatLogger class definition
*/

#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <iostream>
#include <mutex>

using namespace std;

class ChatLogger {
  private:
    ofstream logFile;
    mutex mtx;

  public:
    bool open(const string &filename);
    void close();
    bool log(const string &msg);
};

inline void LOG_INFO(const string &msg, ChatLogger &logger) {
    cout << "[INFO] " << msg << endl;
    if (!logger.log("[INFO] " + msg)) {
        cerr << "[ERROR] Failed to log INFO message." << endl;
    }
}
inline void LOG_ERROR(const string &msg, ChatLogger &logger) {
    cerr << "[ERROR] " << msg << endl;
    if (!logger.log("[ERROR] " + msg)) {
        cerr << "[ERROR] Failed to log ERROR message." << endl;
    }
}

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