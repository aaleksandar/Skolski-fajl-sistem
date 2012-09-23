// file: kernelfile.cpp
// author: Aleksandar Abu-Samra

#include "kernelfile.h"

#include "semaphores.h"
#include "drive.h"
#include "rootcluster.h"
#include "datacluster.h"

HANDLE partitionAccessMutex = CreateSemaphore(NULL, 1, 32, NULL);
HANDLE numOfFilesMutex = CreateSemaphore(NULL, 1, 32, NULL);

KernelFile::KernelFile(char *fname, char mode, Entry entry, Drive *drive) {
	strcpy(this->fname, fname);
	this->mode = mode;
	this->entry = entry;
	this->drive = drive;

	cluster = new DataCluster(drive);

	switch (mode) {
		case 'r': pointer = 0;
			break;
		case 'w': pointer = 0;
			truncate();
			break;
		case 'a': pointer = entry.size;
			seek(pointer);
			break;
	}

	WAIT(numOfFilesMutex);
	drive->incNumOfFiles();
	SIGNAL(numOfFilesMutex);
}

/**
  * Zatvaranje i brisanje fajla
  * Oslobadjaju se svi semafori
  */ 
KernelFile::~KernelFile() {
	WAIT(numOfFilesMutex);
	drive->decNumOfFiles();

	if (mode != 'r') {
		cluster->writeToDisk();
		drive->getRootCluster()->updateEntry(entry);
	}

	delete cluster;

	drive->semRW.signal(fname, mode);
	SIGNAL(numOfFilesMutex);
}

/**
  * Upis na tekucu poziciju u fajl zadatog broja bajtova sa zadate memorijske adrese
  * TOOPT uzasno ruzna metoda
  */
char KernelFile::write(BytesCnt bytesCnt, char* buffer) {
	if ('r' == mode) return 0;
	if (0 == bytesCnt) return 0;

	WAIT(partitionAccessMutex);

	// TODO ukoliko vec nije dostugnuta maksimalna velicina fajla
	if (pointer + bytesCnt > entry.size) entry.size += bytesCnt;

	BytesCnt
		amount,
		bufferPointer = 0,
		localPointer = pointer % EfficientClusterSize;
	ClusterNo
		prevCluster = cluster->getPrevCluster(), 
		currentCluster = cluster->getClusterNo(),
		nextCluster = cluster->getNextCluster();

	if (0 == entry.firstCluster) { // fajl tek napravljen, postavi FirstCluster
		currentCluster = drive->getAndSetFreeClusterNo();
		entry.firstCluster = currentCluster;
		cluster->setClusterNo(currentCluster);
	}
	else if (0 == currentCluster) {
		currentCluster = entry.firstCluster;
		cluster->setClusterNo(currentCluster);
		
	}

	if (localPointer) { // ako trenutni klaster nije popunjen
		amount = EfficientClusterSize - localPointer;
		if (amount > bytesCnt) amount = bytesCnt;

		cluster->setRawData(buffer, localPointer, amount);

		bytesCnt -= amount;
		pointer += amount;
		bufferPointer += amount;
	}

	char toWr[EfficientClusterSize + 1]; //
	while (bytesCnt > 0) {

		localPointer = pointer % EfficientClusterSize;
		if (0 == localPointer && pointer > 0) {
			prevCluster = cluster->getClusterNo();
			currentCluster = cluster->getNextCluster();

			if (0 == currentCluster) { // ako ne postoji sledeci klaster, napravi ga i postavi
				currentCluster = drive->getAndSetFreeClusterNo();

				cluster->setNextCluster(currentCluster);
				cluster->writeToDisk();

				cluster->setClusterNo(currentCluster);
				cluster->setPrevCluster(prevCluster);
				cluster->setNextCluster(0);
			}
			else { // ako vec postoji sledeci klaster
				cluster->writeToDisk();

				cluster->setClusterNo(currentCluster);
				cluster->readFromDisk();
			}

			drive->getRootCluster()->updateEntry(entry); // TODEL ??? 
		}
		
		amount = EfficientClusterSize;
		if (amount > bytesCnt) amount = bytesCnt;

		cluster->setRawData(buffer + bufferPointer, localPointer, amount); // pozicija
		cluster->writeToDisk(amount); // upisi klaster na disk

		pointer += amount;
		bufferPointer += amount;
		bytesCnt -= amount;
	}

	SIGNAL(partitionAccessMutex);

	return 1;
}

/**
  * Citanje sa tekuce pozicije iz fajla zadatog broja bajtova na zadatu adresu u memoriji
  */
BytesCnt KernelFile::read(BytesCnt bytesCnt, char* buffer) {
	if (pointer > getFileSize()) return 0; // pointer vec na kraju fajla
	if (0 == bytesCnt) return 0;

	WAIT(partitionAccessMutex);

	// ako zadati bytesCnt premasuje velicinu fajla
	if (bytesCnt + pointer > entry.size) bytesCnt = entry.size - pointer;

	BytesCnt
		amount,
		bufferPointer = 0,
		localPointer = pointer % EfficientClusterSize;
	ClusterNo currentCluster = entry.firstCluster; // first

	if (localPointer) { // ako je pointer negde u sredini klastera
		amount = EfficientClusterSize - localPointer;
		if (amount > bytesCnt) amount = bytesCnt;

		cluster->getRawData(buffer + bufferPointer, localPointer, amount);

		bytesCnt -= amount;
		pointer += amount;
		bufferPointer += amount;
	}

	while (bytesCnt > 0) { // regularno citanja vise blokova
		if (EfficientClusterSize > bytesCnt) amount = bytesCnt; // poslednji klaster
		else amount = EfficientClusterSize;

		localPointer = pointer % EfficientClusterSize;
		if (0 == localPointer) {
			if (0 == pointer) currentCluster = entry.firstCluster;
			else currentCluster = cluster->getNextCluster();
		}

		cluster->setClusterNo(currentCluster);
		cluster->readFromDisk();
		cluster->getRawData(buffer + bufferPointer, 0, amount);

		bytesCnt -= amount;
		pointer += amount;
		bufferPointer += amount;
	}

	SIGNAL(partitionAccessMutex);

	return bufferPointer;
}

/**
  * Pomeranje tekuce pozicije u fajlu
  */
char KernelFile::seek(BytesCnt bytesCnt) {
	if (getFileSize() == 0) return 0; // ako je prazan fajl
	if (bytesCnt > getFileSize()) return 0; // ako premasuje velicinu

	WAIT(partitionAccessMutex);

	pointer = bytesCnt;

	if ('r' != mode) cluster->writeToDisk();

	BytesCnt i = 0;
	ClusterNo clusterToFetch = entry.firstCluster;

	do {
		cluster->setClusterNo(clusterToFetch);
		cluster->readFromDisk();
		clusterToFetch = cluster->getNextCluster();
		i += EfficientClusterSize;
	} while (i < pointer);

	SIGNAL(partitionAccessMutex);

	return 1;
}

/**
  * Dohvatanje tekuce pozicije u fajlu
  */
BytesCnt KernelFile::filePos() const {
	return pointer;
}

/**
  * Provera da li je tekuca pozicija u fajlu kraj tog fajla
  */
char KernelFile::eof() const {
	if (getFileSize() == 0) return 1; // ako je prazan fajl, greska
	if (getFileSize() == pointer) return 2; // eof
	return 0;
}

/**
  * Dohvatanje trenutne velicine fajla u bajtovima
  */
BytesCnt KernelFile::getFileSize() const {
	return entry.size;
}

/**
  * Brisanje dela fajla, cime se fajl oslobadja za koriscenje od strane drugih niti
  */
char KernelFile::truncate() {
	if ('r' == mode) return 0;
	if (pointer >= entry.size) return 0;

	WAIT(partitionAccessMutex);

	ClusterNo
		currentCluster = cluster->getClusterNo(),
		nextCluster = cluster->getNextCluster(),
		lastFreeCluster = drive->getLastFreeCluster(); 

	int entrySize = pointer;

	// odseci klaster
	cluster->setNextCluster(0);
	cluster->writeToDisk();

	// ulancaj ostale klastere sa Free clusterima
	cluster->setClusterNo(lastFreeCluster); // TOOPT
	cluster->setNextCluster(nextCluster);
	cluster->writeToDisk();

	// vrati tekuci klaster u cache memoriju fajla
	cluster->setClusterNo(currentCluster);
	cluster->readFromDisk();

	entry.size = entrySize;
	drive->getRootCluster()->updateEntry(entry);

	SIGNAL(partitionAccessMutex);

	return 1;
}


char KernelFile::getMode() const {
	return mode;
}