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
    std::swap(restoreEncrypted, op.restoreEncrypted);
}

FileOperation& FileOperation::operator=(const FileOperation& op) {
    this->sourcePathName = op.sourcePathName;
    this->destinationPathName = op.destinationPathName;
    this->initialFileSize = op.initialFileSize;
    this->initialModificationTime = op.initialModificationTime;
    this->restoreEncrypted = op.restoreEncrypted;

    return *this;
}

FileOperation::~FileOperation() {
}
