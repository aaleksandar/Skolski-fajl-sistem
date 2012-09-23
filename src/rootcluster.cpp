// file: cluster.cpp
// author: Aleksandar Abu-Samra

#include "rootcluster.h"

#include "drive.h"
#include "semrw.h"
#include "semaphores.h"
#include <iostream>

HANDLE rootClusterMutex = CreateSemaphore(NULL, 1, 32, NULL);

RootCluster::RootCluster(Drive *drive, ClusterNo clusterNo, ClusterNo free, ClusterNo next) : Cluster(drive, clusterNo) {
	freeClusterNo = free;
	nextRootCluster = next;
	loaded = 0;

	for (int i=0; i<ClusterEntrySize; i++) entries[i].name[0] = 0;
}

RootCluster::~RootCluster() {
	// nothing to do
}

/**
  * Resetuje Root Klaster
  */
void RootCluster::resetRootCluster() {
	WAIT(rootClusterMutex);
	freeClusterNo = 1;
	nextRootCluster = 0;

	for (int i=0; i<ClusterEntrySize; i++) entries[i].name[0] = 0;
	SIGNAL(rootClusterMutex);
}


/**
  * Ucitava klaster u kome se nalazi pozicija
  * Dohvata Entry i vraca pokazivac na njega
  */
Entry* RootCluster::getEntry(unsigned position) {
	WAIT(rootClusterMutex);

	// kesira od nultog klastera ako se trazi pozicija koja je ranija u nizu klasera
	if (position / ClusterEntrySize < loaded) {
		clusterNo = 0;
		readFromDisk();
		loaded = 0;
	}

	// ucitava sledece klastere dok ne dodje do relevantnog
	while (position / ClusterEntrySize > loaded) {

		if (nextRootCluster == 0) { // ne postoji sledeci root klaster
			nextRootCluster = drive->getAndSetFreeClusterNo();
			
			writeToDisk();

			clusterNo = nextRootCluster;
			resetRootCluster();

			writeToDisk();
		}
		else { // postoji sledeci root klaster
			clusterNo = nextRootCluster;
			readFromDisk();
		}

		loaded++;
		position -= ClusterEntrySize;
	}

	SIGNAL(rootClusterMutex);
	return &entries[position % ClusterEntrySize];
}

/**
  * Stavlja novi entry i vraca njegovu poziciju u root-u
  */
int RootCluster::putEntry(Entry entry) {
	int i = 0;

	while (getEntry(i)->name[0] != 0) i++;
	entries[i % ClusterEntrySize] = entry;

	writeToDisk();
	return i;
}

/**
  * Update-uje zadati entry
  */
int RootCluster::updateEntry(Entry entry) {
	bool breakLoop;
	unsigned loadMore = 1;

	// trazi entry po name i ext
	for (unsigned i = 0; i < ClusterEntrySize * loadMore; i++) {
		breakLoop = false;

		for (int j = 0; j < FNAMELEN; j++) {
			if (entry.name[j] != getEntry(i)->name[j]) breakLoop = true;
		}
		for (int j = 0; j < FEXTLEN; j++) {
			if (entry.ext[j] != getEntry(i)->ext[j]) breakLoop = true;
		}
	
		if (nextRootCluster != 0) loadMore = loaded + 2;
		if (breakLoop) continue;
		
		getEntry(i)->firstCluster = entry.firstCluster;
		getEntry(i)->size = entry.size;

		writeToDisk(); 
		return 1;
	}

	return 0;
}

/**
  * Brise entry
  */
void RootCluster::deleteEntry(unsigned position) {
	getEntry(position)->name[0] = 0;
	writeToDisk();
}

/**
  * Vraca poziciju zadatog fajla
  * TOOPT: ako se zna poslednja pozicija u nizu koja sadrzi fajl, prekinuti pretragu
  */
int RootCluster::getFilePosition(char* fname) {
	unsigned loadMore = 1;
	for (unsigned i = 0; i < ClusterEntrySize * loadMore; i++) {
		if (getEntry(i)->name[0] == 0) {
			if (nextRootCluster != 0) loadMore = loaded + 2;
			continue;
		}
		if (!strcmp(fname, getFilename(getEntry(i)))) return i;
		if (nextRootCluster != 0) loadMore = loaded + 2;
	}

	return -1;
}

/**
  * Vraca filename (entry.name + '.' + entry.ext)
  */
char* RootCluster::getFilename(Entry *entry) {
	if (entry->name[0] == 0) return 0;

	char *name = entry->name;
	char *ext  = entry->ext;
	char *filename = new char[strlen(name) + strlen(ext) + 3];

	unsigned i = 0;
	for (; i < strlen(name); i++) filename[i] = name[i];
	filename[i++] = '.';
	for (unsigned j = 0; j < strlen(ext); j++) {
		filename[i++] = ext[j];
	}
	filename[i] = '\0';

	return filename;
}


/**
  * Postavlja indikator pozicije na prvi slobodan klaster
  * TOOPT: oladiti malo sa pristupanjem disku
  */
void RootCluster::setFreeClusterNo(ClusterNo pos) {
	WAIT(rootClusterMutex);
	ClusterNo retClusterNo = clusterNo;

	writeToDisk();

	clusterNo = 0;
	readFromDisk();
	freeClusterNo = pos;
	writeToDisk();

	clusterNo = retClusterNo;
	readFromDisk();

	SIGNAL(rootClusterMutex);
}

/**
  * Vraca poziciju prvog slobodnog klastera
  * TOTEST
  */
ClusterNo RootCluster::getFreeClusterNo() {
	WAIT(rootClusterMutex);
	ClusterNo retClusterNo = clusterNo;
	
	writeToDisk();

	clusterNo = 0;
	readFromDisk();
	ClusterNo ret = freeClusterNo;

	clusterNo = retClusterNo;
	readFromDisk();

	SIGNAL(rootClusterMutex);
	return freeClusterNo;
}


void RootCluster::writeToDisk() {
	memcpy(cluster, &freeClusterNo, 4);
	memcpy(cluster + 4, &nextRootCluster, 4);
	memcpy(cluster + 8, entries, EfficientClusterSize);

	partition->writeCluster(clusterNo, cluster);
}

void RootCluster::readFromDisk() {
	partition->readCluster(clusterNo, cluster);

	memcpy(&freeClusterNo, cluster, 4);
	memcpy(&nextRootCluster, cluster + 4, 4);
	memcpy(entries, cluster + 8, EfficientClusterSize);
}