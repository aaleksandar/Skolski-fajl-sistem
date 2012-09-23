// file: cluster.cpp
// author: Aleksandar Abu-Samra

#include "cluster.h"
#include "drive.h"

Cluster::Cluster(Drive *drive, ClusterNo cn) {
	this->drive = drive;
	clusterNo = cn;

	partition = drive->getPartition();
}

Cluster::~Cluster() {
	// TODO: write to disk?
//	delete [] cluster; TODO: delete
}

void Cluster::setClusterNo(ClusterNo cn) {
	clusterNo = cn;
}

ClusterNo Cluster::getClusterNo() const {
	return clusterNo;
}
