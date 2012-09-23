// file: cluster.h
// author: Aleksandar Abu-Samra

#pragma once

#include "part.h"
#include "fs.h"

class Drive;
class Partition;

class Cluster {
protected:
	ClusterNo clusterNo;
	char cluster[ClusterSize];

	Drive *drive;
	Partition *partition;

public:
	Cluster(Drive *, ClusterNo);
	~Cluster();

//	void resetCluster(); // TODO virtual?
	void setClusterNo(ClusterNo cn); // TODO: delete
	ClusterNo getClusterNo() const;
};