// file: file.cpp
// author: Aleksandar Abu-Samra

#include "file.h"
#include "fs.h"
#include "kernelfile.h"

File::File() {
	// empty
}

File::~File() {
	if (myImpl) delete myImpl;
}

char File::write(BytesCnt bytesCnt, char* buffer) {
	return myImpl->write(bytesCnt, buffer);
}

BytesCnt File::read(BytesCnt bytesCnt, char* buffer) {
	return myImpl->read(bytesCnt, buffer);
}

char File::seek(BytesCnt bytesCnt) {
	return myImpl->seek(bytesCnt);
}

BytesCnt File::filePos() {
	return myImpl->filePos();
}

char File::eof() {
	return myImpl->eof();
}

BytesCnt File::getFileSize() {
	return myImpl->getFileSize();
}

char File::truncate() {
	return myImpl->truncate();
}