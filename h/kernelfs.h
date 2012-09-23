// file: kernelfs.h
// author: Aleksandar Abu-Samra

#pragma once

#include "fs.h"

class Drive;

class KernelFS {
	Drive* drives[26];

public: 

	KernelFS();
	~KernelFS(); 
 
	char mount(Partition*);
		// montira particiju 
		// vraca dodeljeno slovo

	char unmount(char); 
		// demontira particiju oznacenu datim slovom
		// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha

	char format(char);
		// formatira particiju sa datim slovom;
		// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha 
 
	char readRootDir(char, EntryNum, Directory &d); 
		// prvim argumentom se zadaje particija 
		// drugim redni broj validnog ulaza od kog se pocinje citanje 
		// treci argument je adresa na kojoj se smesta procitani niz ulaza
 
	char doesExist(char*);
		// argument je naziv fajla sa apsolutnom putanjom

	File* open(char*, char); 
	char deleteFile(char*); 
 
protected:

	Drive* getDrive(char driveLetter) const;
	Entry* makeEntry(char *fname);
};

