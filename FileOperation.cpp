#include "FileOperation.h"
//#include <utility>

FileOperation::FileOperation(const std::wstring& src, const std::wstring& dst, unsigned long sz, const time_t& t, const std::wstring& rel)
    : sourcePathName(src), destinationPathName(dst), initialFileSize(sz), initialModificationTime(t), relativePath(rel) {
}

FileOperation::FileOperation(FileOperation&& op) {
    std::swap(sourcePathName, op.sourcePathName);
    std::swap(destinationPathName, op.destinationPathName);
    std::swap(initialFileSize, op.initialFileSize);
    std::swap(initialModificationTime, op.initialModificationTime);
    std::swap(processItem, op.processItem);
    std::swap(relativePath, op.relativePath);
}

FileOperation& FileOperation::operator=(const FileOperation& op) {
    this->sourcePathName = op.sourcePathName;
    this->destinationPathName = op.destinationPathName;
    this->initialFileSize = op.initialFileSize;
    this->initialModificationTime = op.initialModificationTime;
    this->processItem = op.processItem;
    this->relativePath = op.relativePath;

    return *this;
}
/*
FileOperation& FileOperation::operator=(FileOperation&& op) {
    std::swap(sourcePathName, op.sourcePathName);
    std::swap(destinationPathName, op.destinationPathName);
    std::swap(initialFileSize, op.initialFileSize);
    std::swap(initialModificationTime, op.initialModificationTime);
    std::swap(processItem, op.processItem);
    std::swap(relativePath, op.relativePath);

    return *this;
}
*/
FileOperation::~FileOperation() {
}
