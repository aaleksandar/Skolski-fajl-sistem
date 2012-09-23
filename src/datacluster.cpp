// file: datacluster.cpp
// author: Aleksandar Abu-Samra

#include "datacluster.h"
#include <iomanip>
#include <fstream>

DataCluster::DataCluster(Drive *drive, ClusterNo clusterNo, ClusterNo prev, ClusterNo next): Cluster(drive, clusterNo) {
	prevCluster = prev;
	nextCluster = next;
}

DataCluster::~DataCluster() {
	// writeToDisk(); // ?
}
	
ClusterNo DataCluster::getPrevCluster() const {
	return prevCluster;
}

ClusterNo DataCluster::getNextCluster() const {
	return nextCluster;
}

void DataCluster::setPrevCluster(ClusterNo pos) {
	prevCluster = pos;
}

void DataCluster::setNextCluster(ClusterNo pos) {
	nextCluster = pos;
}

void DataCluster::setRawData(char *buff, BytesCnt position, BytesCnt amount) {
	memcpy(rawData + position, buff, amount);
}

void DataCluster::getRawData(char *buff, BytesCnt position, BytesCnt amount) {
	memcpy(buff, rawData + position, amount);
}

void DataCluster::resetDataCluster() {
	// TODO
}

void DataCluster::writeToDisk(ClusterNo amount) {
	memcpy(cluster, &prevCluster, 4);
	memcpy(cluster + 4, &nextCluster, 4);
	memcpy(cluster + 8, rawData, amount);

	partition->writeCluster(clusterNo, cluster);
}

void DataCluster::readFromDisk(ClusterNo amount) {
	partition->readCluster(clusterNo, cluster);

	memcpy(&prevCluster, cluster, 4);
	memcpy(&nextCluster, cluster + 4, 4);
	memcpy(rawData, cluster + 8, amount);
}