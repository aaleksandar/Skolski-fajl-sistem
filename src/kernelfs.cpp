// file: kernelfs.cpp
// author: Aleksandar Abu-Samra

#include "kernelfs.h"
#include "file.h"
#include "kernelfile.h"
#include "drive.h"
#include "rootcluster.h"
#include "semrw.h"

/**
  * Vraca Drive po zadatom slovu
  */
Drive* KernelFS::getDrive(char driveLetter) const {
	if (driveLetter < 'A' || driveLetter > 'Z') return 0;
	return drives[driveLetter - 'A'];
}

KernelFS::KernelFS() {
	for (int i=0; i<26; i++) drives[i] = 0; // initialize drives to 0
}

KernelFS::~KernelFS() {
	for (int i=0; i<26; i++) if (drives[i]) delete drives[i];
}

/**
  * Montiranje particije u fajl sistem
  */
char KernelFS::mount(Partition* partition) {
	int i = 0;
	while (drives[i]!=0 && i<26) {
		// ako je particija vec mountovana vraca njeno slovo
		if (partition == drives[i]->getPartition()) return drives[i]->getDriveLetter();
		i++;
	}
	if (i >= 26) return '0'; // exit if full

	char driveLetter = i + 'A';

	Drive *drive = new Drive(partition, driveLetter);
	drives[i] = drive;

	
	return driveLetter;
}

/**
  * Demontiranje particije iz fajl sistema
  */
char KernelFS::unmount(char part) {
	// TODO: obrisati sve pri umountu?

	Drive *drive = getDrive(part);
	if (0 == drive) return 0;
	
	// onaj ko demontira particiju se blokira 
	// sve dok se ne zatvore svi fajlovi sa date particije;
	WAIT(drive->allFilesClosed);

	drives[part - 'A'] = 0;
	if (drive) delete drive;
	
	return 1;
}

/**
  * Formatiranje particije zadate dodeljenim slovom
  */
char KernelFS::format(char part) {
	Drive *drive = getDrive(part);
	if (0 == drive) return 0;

	drive->format();

	return 1;
}

/**
  * Citanje sadrzaja korenog direktorijuma
  */
char KernelFS::readRootDir(char driveLetter, EntryNum n, Directory &d) {
	Drive *drive = getDrive(driveLetter);
	if (0 == drive) return 0;
 
	drive->getRootCluster()->readFromDisk();
	// dodaje sve entry-je u Directory, dok se ne prekine niz
	int i = 0;
	for ( ; i < ENTRYCNT; i++) {
		Entry *entry = drive->getRootCluster()->getEntry(n+i); // od broja n
		if (0 == entry->name[0]) break;
		d[i] = *entry;
	}

	int numberOfEntries = i;

	// 65 - uspeh, niz je pun i nije procitan ostatak ulaza u direktorijumu
	if (0 != drive->getRootCluster()->getEntry(n+i+1)->name[0]) numberOfEntries = 65;

	return numberOfEntries;
}

/**
  * Ispitivanje postojanja zadatog fajla
  */
char KernelFS::doesExist(char* fname) {
	Drive *drive = getDrive(fname[0]);
	if (0 == drive) return 0;
	if (fname[1] != ':' || fname[2] != '\\') return 0;
	
	if (drive->getRootCluster()->getFilePosition(fname+3) < 0) return 0;

	return 1;
}

File* KernelFS::open(char* fname, char mode) {
	Drive *drive = getDrive(fname[0]);
	if (0 == drive) return 0;
	if (fname[1] != ':' || fname[2] != '\\') return 0;
	if (!('r' == mode || 'w' == mode || 'a' == mode)) return 0;

	drive->semRW.wait(fname, mode); // semafor pre svega

	int position = drive->getRootCluster()->getFilePosition(fname+3);

	Entry *entry = new Entry;

	if (position < 0) {
		// file DOES NOT exist
		if ('a' == mode || 'r' == mode) return 0;

		entry = makeEntry(fname + 3);
		drive->getRootCluster()->putEntry(*entry);
	}
	else {
		// file EXISTS
		entry = drive->getRootCluster()->getEntry(position);
	}

	File *file = new File();
	file->myImpl = new KernelFile(fname, mode, *entry, drive);
	
	return file;
}

char KernelFS::deleteFile(char* fname) {
	Drive *drive = getDrive(fname[0]);
	if (0 == drive) return 0;
	if (fname[1] != ':' || fname[2] != '\\') return 0;

	drive->semRW.wait(fname, 'w');
	
	int position = drive->getRootCluster()->getFilePosition(fname+3);
	if (position < 0) return 0;

	drive->getRootCluster()->deleteEntry(position);

	drive->semRW.signal(fname, 'w');

	return 1;
}

/**
  * Pravi i inicijalizuje novi entry
  */
Entry* KernelFS::makeEntry(char *fname) {	
	Entry *entry = new Entry;

	int i=0, j=0;
	if (fname[0] >= 'A' && fname[0] <= 'Z' && fname[1] == ':' && fname[2] == '\\') i = 3; // preskace A\ dir

	while (fname[i] != '.' && i < 8) {
		entry->name[j] = fname[i];
		i++; j++;
	}
	if (j < 8) entry->name[j] = '\0';

	i++; j=0;
	
	while (fname[i] != '\0' && j < 3) {
		entry->ext[j] = fname[i];
		i++; j++;
	}
	if (j < 3) entry->ext[j] = '\0';

	entry->reserved = 0;
	entry->firstCluster = 0;
	entry->size = 0;

	return entry;
}