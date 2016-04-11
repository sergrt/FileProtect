#pragma once
#include <string>
//#include <time.h>

class FileOperation {
public:
    FileOperation(const std::string& src, const std::string& dst, unsigned long sz, const time_t& t);
    ~FileOperation();
    FileOperation(FileOperation&& op);
//private:
    std::string sourcePathName;
    std::string destinationPathName;
    unsigned long initialFileSize;
    time_t initialModificationTime;

    bool restoreEncrypted = false;
};
