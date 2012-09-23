// file: fs.cpp
// author: Aleksandar Abu-Samra

#include "fs.h"
#include "kernelfs.h"

KernelFS *FS::myImpl = new KernelFS();

FS::~FS() {
	delete FS::myImpl;
}

char FS::mount(Partition* partition) {
	return FS::myImpl->mount(partition);
}

char FS::unmount(char part) {
	return FS::myImpl->unmount(part);
}

char FS::format(char part) {
	return FS::myImpl->format(part);
}

char FS::readRootDir(char part, EntryNum n, Directory &d) {
	return FS::myImpl->readRootDir(part, n, d);
}

char FS::doesExist(char* fname) {
	return FS::myImpl->doesExist(fname);
}

File* FS::open(char* fname, char mode) {
	return FS::myImpl->open(fname, mode);
}

char FS::deleteFile(char* fname) {
	return FS::myImpl->deleteFile(fname);
}