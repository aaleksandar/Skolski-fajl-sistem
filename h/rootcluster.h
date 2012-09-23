// file: rootcluster.h
// author: Aleksandar Abu-Samra

#pragma once

#include "cluster.h"

class Drive;

const unsigned int ClusterEntrySize = 102; // (ClusterSize - 8) / 20

class RootCluster : public Cluster {
	ClusterNo freeClusterNo, nextRootCluster;
	Entry entries[ClusterEntrySize]; // TOOPT redundantno

	unsigned loaded; // redni broj ucitanog Root klastera u listi

	char* getFilename(Entry *);

public:
	RootCluster(Drive *drive, ClusterNo clusterNo = 0, ClusterNo free = 1, ClusterNo next = 0);
	~RootCluster();

	void resetRootCluster();

	Entry* getEntry(unsigned position);
	int putEntry(Entry);
	int updateEntry(Entry);
	void deleteEntry(unsigned position);

	int getFilePosition(char* fname);

	void setFreeClusterNo(ClusterNo);
	ClusterNo getFreeClusterNo();

	void writeToDisk();
	void readFromDisk();
};