// file: datacluster.h
// author: Aleksandar Abu-Samra

#pragma once

#include "cluster.h"

class Drive;

class DataCluster : public Cluster {
	ClusterNo prevCluster, nextCluster;
	char rawData[EfficientClusterSize]; // TOOPT redundantno, dinamicki

public:

	DataCluster(Drive *drive, ClusterNo clusterNo = 0, ClusterNo prev = 0, ClusterNo next = 0);
	~DataCluster();

	ClusterNo getPrevCluster() const;
	ClusterNo getNextCluster() const;
	void setPrevCluster(ClusterNo);
	void setNextCluster(ClusterNo);
	void setRawData(char *buff, BytesCnt position = 0, BytesCnt amount = EfficientClusterSize);
	void getRawData(char *buff, BytesCnt position = 0, BytesCnt amount = EfficientClusterSize);

	void resetDataCluster();

	void writeToDisk(ClusterNo amount = EfficientClusterSize); // TOOPT amount?
	void readFromDisk(ClusterNo amount = EfficientClusterSize);
};