// file: drive.cpp
// author: Aleksandar Abu-Samra

#include "drive.h"

#include "rootcluster.h"
#include "datacluster.h"

Drive::Drive(Partition *partition, char driveLetter) {
	this->partition = partition;
	this->driveLetter = driveLetter;

	numOfFiles = 0;
	rootCluster = new RootCluster(this);

	allFilesClosed = CreateSemaphore(NULL, 1, 32, NULL);
}

Drive::~Drive() {
	// TODO ?
//	delete partition;
	delete rootCluster;
}

char Drive::getDriveLetter() const {
	return driveLetter;
}

Partition* Drive::getPartition() const {
	return partition;
}

void Drive::format() {
	rootCluster->resetRootCluster();
	rootCluster->writeToDisk();
	freeClusterNo = 1;
	
	ClusterNo numOfClusters = partition->getNumOfClusters();

	DataCluster cluster(this, 1, 0, 2);
	for (unsigned i=1; i<numOfClusters; i++) {
		cluster.setClusterNo(i);
		cluster.setPrevCluster(i-1);
		cluster.setNextCluster(i+1);
		if (i+1 == numOfClusters) cluster.setNextCluster(0);

		cluster.writeToDisk();
	}
}

RootCluster* Drive::getRootCluster() const {
	return rootCluster;
}

/**
  * Vraca poziciju prvog slobodnog klastera
  * TOOPT
  */
ClusterNo Drive::getAndSetFreeClusterNo() {
	ClusterNo ret = freeClusterNo;

	DataCluster cluster(this, freeClusterNo); // napravi klaster sa pozicijom prvog free clustera
	cluster.readFromDisk(); // procitaj njegove podatke sa diska

	freeClusterNo = cluster.getNextCluster(); // postavi free cluster na sledeceg
	rootCluster->setFreeClusterNo(freeClusterNo);

	cluster.setPrevCluster(0); // inicijalizuj clusterov prev i next
	cluster.setNextCluster(0);
	cluster.writeToDisk(); // vrati na disk

	return ret;
}

ClusterNo Drive::getLastFreeCluster() {
	ClusterNo next = freeClusterNo;	
	DataCluster cluster(this, next);

	while (next != 0) {
		cluster.setClusterNo(next);
		cluster.readFromDisk();
		next = cluster.getNextCluster();
	}

	return cluster.getClusterNo();
}

int Drive::incNumOfFiles() {
	numOfFiles++;
	if (1 == numOfFiles) WAIT(allFilesClosed);
	
	return numOfFiles;
}

int Drive::decNumOfFiles() {
	numOfFiles--;
	if (0 == numOfFiles) SIGNAL(allFilesClosed);
	
	return numOfFiles;
}