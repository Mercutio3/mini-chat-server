/**
 * @file log.cpp
 * @brief Chat logging function and file management.
 */

#include "../include/log.hpp"
#include <cerrno>
#include <cstring>
#include <ctime>
#include <fstream>
#include <mutex>
#include <sstream>

using namespace std;

/**
 * @brief Opens the log file for appending log messages.
 * 
 * If file cannot be opened, error message is printed to stderr.
 * 
 * @param filename The name of log file to open.
 * @return true if file opened successfully, false otherwise.
 */
bool ChatLogger::open(const string &filename) {
    lock_guard<mutex> lock(mtx);
    logFile.open(filename, ios::app);
    if (!logFile.is_open()) {
        cerr << "Error opening log file: " << strerror(errno) << endl;
        return false;
    }
    return true;
}

/**
 * @brief Closes the log file, if it's open. Otherwise does nothing.
 */
void ChatLogger::close() {
    lock_guard<mutex> lock(mtx);
    if (logFile.is_open()) {
        logFile.close();
    }
}

/**
 * @brief Logs message to log file with timestamp.
 * 
 * Prepends a UTC timestamp to the message and writes it to the log file.
 * If log file isn't open or a writing error occurs, error message is
 * printed to stderr.
 *
 * @param msg Message to log.
 * @return true if message logged successfully, false otherwise.
 */
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
    if (!logFile) {
        cerr << "Error writing to log file: " << strerror(errno) << endl;
        return false;
    }
    return true;
}