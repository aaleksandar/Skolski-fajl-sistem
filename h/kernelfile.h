// file: kernelfile.h
// author: Aleksandar Abu-Samra

#pragma once

#include "fs.h"

class Drive;
class DataCluster;

class KernelFile {
	char fname[16];
	char mode;
	Drive *drive;

	Entry entry;
	BytesCnt pointer;

	DataCluster *cluster;

public:
	KernelFile(char *fname, char mode, Entry entry, Drive *drive);
	~KernelFile();

	char write(BytesCnt, char* buffer);
	BytesCnt read(BytesCnt, char* buffer);
	char seek(BytesCnt);

	BytesCnt filePos() const;
	char eof() const;
	BytesCnt getFileSize() const;
	char truncate();

	// custom
	char getMode() const;
};