// file: semrw.h
// author: Aleksandar Abu-Samra

#pragma once

#include "semaphores.h"

class SemRW {
	struct Node {
		char fname[16];
		HANDLE turnstile, roomEmpty, mutex;
		int numOfReaders;
		Node *next;

		Node(char *fname, Node *n = 0) {
			strcpy(this->fname, fname);
			turnstile = CreateSemaphore(NULL, 1, 32, NULL);
			roomEmpty = CreateSemaphore(NULL, 1, 32, NULL);
			mutex = CreateSemaphore(NULL, 1, 32, NULL);
			numOfReaders = 0;
			next = n;
		}
	} *first;

	void insert(char *fname);

public:

	SemRW();
	~SemRW();

	void wait(char *fname, char mode);
	void signal(char *fname, char mode);
};