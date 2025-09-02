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
    void open(const string &filename);
    void close();
    void log(const string &msg);
};

inline void LOG_INFO(const string &msg, ChatLogger &logger) {
    cout << "[INFO] " << msg << endl;
    logger.log("[INFO] " + msg);
}
inline void LOG_ERROR(const string &msg, ChatLogger &logger) {
    cerr << "[ERROR] " << msg << endl;
    logger.log("[ERROR] " + msg);
}

inline void LOG_DEBUG(const string &msg, ChatLogger &logger) {
#ifdef DEBUG
    cout << "[DEBUG] " << msg << endl;
    logger.log("[DEBUG] " + msg);
#else
    (void)msg;
    (void)logger;
#endif
}

#endif