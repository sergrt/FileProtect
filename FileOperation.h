#pragma once
#include <string>

struct FileOperation {
//public:
    FileOperation(const std::wstring& src, const std::wstring& dst, unsigned long sz, const time_t& t);
    ~FileOperation();
    FileOperation(FileOperation&& op);
    FileOperation& operator=(const FileOperation& op);

//private:
    std::wstring sourcePathName;
    std::wstring destinationPathName;
    unsigned long initialFileSize;
    time_t initialModificationTime;

    bool processItem = false;
};

