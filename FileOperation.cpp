#include "FileOperation.h"
//#include <utility>

FileOperation::FileOperation(const std::string& src, const std::string& dst, unsigned long sz, const time_t& t)
    : sourcePathName(src), destinationPathName(dst), initialFileSize(sz), initialModificationTime(t) {
}

FileOperation::FileOperation(FileOperation&& op) {
    std::swap(sourcePathName, op.sourcePathName);
    std::swap(destinationPathName, op.destinationPathName);
    std::swap(initialFileSize, op.initialFileSize);
    std::swap(initialModificationTime, op.initialModificationTime);
}

FileOperation::~FileOperation() {
}