// file: semrw.cpp
// author: Aleksandar Abu-Samra

#include "semrw.h"
#include <iostream>
using namespace std;
SemRW::SemRW() {
	first = NULL;
}

SemRW::~SemRW() {
	while (first) {
		Node *old = first;
		first = first->next;

		delete old;
	}
}

void SemRW::insert(char *fname) {
	Node *newNode = new Node(fname, first);
	first = newNode;
}

void SemRW::wait(char *fname, char mode) {
	Node *i = first;

	while (i) {
		if (!strcmp(fname, i->fname)) {
			
			if ('w' == mode || 'a' == mode) { // writer
				WAIT(i->turnstile);
				WAIT(i->roomEmpty);
			}
			else if ('r' == mode) { // reader
				WAIT(i->turnstile);
				SIGNAL(i->turnstile);

				WAIT(i->mutex);
				i->numOfReaders++;
				if (1 == i->numOfReaders) WAIT(i->roomEmpty);
				SIGNAL(i->mutex);
			}

			return;
		}
		i = i->next;
	}

	insert(fname);
	
	// moze ovo i lepse
	i = first;
	if ('w' == mode || 'a' == mode) { // writer
		WAIT(i->turnstile);
		WAIT(i->roomEmpty);
	}
	else if ('r' == mode) { // reader
		WAIT(i->turnstile);
		SIGNAL(i->turnstile);

		WAIT(i->mutex);
		i->numOfReaders++;
		if (1 == i->numOfReaders) WAIT(i->roomEmpty);
		SIGNAL(i->mutex);
	}
}

void SemRW::signal(char *fname, char mode) {
	Node *i = first;

	while (i) {
		if (!strcmp(fname, i->fname)) {
			
			if ('w' == mode || 'a' == mode) { // writer
				SIGNAL(i->turnstile);
				SIGNAL(i->roomEmpty);
			}
			else if ('r' == mode) { // reader
				WAIT(i->mutex);
				i->numOfReaders--;
				if (0 == i->numOfReaders) SIGNAL(i->roomEmpty);
				SIGNAL(i->mutex);
			}

			return;
		}
		i = i->next;
	}
}