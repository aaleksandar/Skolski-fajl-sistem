// file: drive.h
// author: Aleksandar Abu-Samra

#pragma once

#include "fs.h"
#include "part.h"
#include "semrw.h"

class RootCluster;


class Drive {
	Partition *partition;
	char driveLetter;

	ClusterNo freeClusterNo;
	RootCluster *rootCluster;

	int numOfFiles;

public:
	Drive(Partition*, char);
	~Drive();

	char getDriveLetter() const;
	Partition* getPartition() const;

	void format();

	RootCluster* getRootCluster() const;
	ClusterNo getAndSetFreeClusterNo();
	ClusterNo getLastFreeCluster();

	int incNumOfFiles();
	int decNumOfFiles();

	HANDLE allFilesClosed;
	SemRW semRW;
};