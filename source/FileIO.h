#ifndef EVIL_FILEIO_H
#define EVIL_FILEIO_H

#include <fstream>

//*****************
// VerifyRead 
// global file I/O utility
// tests for unrecoverable read error or improperly formatted file
//*****************
inline bool VerifyRead(std::ifstream & read) {
	if (read.bad() || read.fail()) {
		read.clear();
		read.close();
		return false;
	}
	return true;
}

#endif /* EVIL_FILEIO_H */