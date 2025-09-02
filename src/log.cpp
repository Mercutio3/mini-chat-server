/*
log.cpp - Chat logging function and file management
*/

#include "../include/log.hpp"
#include <ctime>
#include <fstream>
#include <mutex>
#include <sstream>

using namespace std;

void ChatLogger::open(const string &filename) {
    lock_guard<mutex> lock(mtx);
    if (!logFile.is_open()) {
        logFile.open(filename, ios::app);
    }
}

void ChatLogger::close() {
    lock_guard<mutex> lock(mtx);
    if (logFile.is_open()) {
        logFile.close();
    }
}

void ChatLogger::log(const string &msg) {
    lock_guard<mutex> lock(mtx);
    if (!logFile.is_open()) {
        return;
    }
    time_t now = time(nullptr);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S UTC", gmtime(&now));
    logFile << "[" << timebuf << "] " << msg << endl;
}