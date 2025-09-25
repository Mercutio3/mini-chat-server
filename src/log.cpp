/*
log.cpp - Chat logging function and file management
*/

#include "../include/log.hpp"
#include <ctime>
#include <fstream>
#include <mutex>
#include <sstream>
#include <cstring>
#include <cerrno>

using namespace std;

bool ChatLogger::open(const string &filename) {
    lock_guard<mutex> lock(mtx);
    logFile.open(filename, ios::app);
    if (!logFile.is_open()) {
        cerr << "Error opening log file: " << strerror(errno) << endl;
        return false;
    }
    return true;
}

void ChatLogger::close() {
    lock_guard<mutex> lock(mtx);
    if (logFile.is_open()) {
        logFile.close();
    }
}

bool ChatLogger::log(const string &msg) {
    lock_guard<mutex> lock(mtx);
    if (!logFile.is_open()) {
        cerr << "Log file is not open. Cannot log message." << endl;
        return false;
    }
    time_t now = time(nullptr);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S UTC", gmtime(&now));
    logFile << "[" << timebuf << "] " << msg << endl;
    if(!logFile) {
        cerr << "Error writing to log file: " << strerror(errno) << endl;
        return false;
    }
    return true;
}