/*
===========================================================================

Engine of Evil GPL Source Code
Copyright (C) 2016-2017 Thomas Matthew Freehill 

This file is part of the Engine of Evil GPL game engine source code. 

The Engine of Evil (EOE) Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EOE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EOE Source Code.  If not, see <http://www.gnu.org/licenses/>.


If you have questions concerning this license, you may contact Thomas Freehill at tom.freehill26@gmail.com

===========================================================================
*/
#ifndef EVIL_ERROR_LOGGER_H
#define EVIL_ERROR_LOGGER_H

#include <memory>
#include <string>
#include <fstream>
#include <SDL.h>

#define EVIL_ERROR_LOG (eErrorLogger::errorLog)

//*******************************************
//			eErrorLogger
// "singleton" class for output to error log file
// in project directory and displaying runtime
// popup error messages
//*******************************************
class eErrorLogger {
public:
									

	bool							Init();
	void							ErrorPopupWindow(const char * message);
	void							LogError(const char * message, const char * sourceFilepath, int lineOfCode);
	void							CheckSDLError(const char * sourceFilepath, int lineOfCode);

	friend void						AutoLogSDLError(void * userdata, int category, SDL_LogPriority priority, const char * message);

private:

									eErrorLogger();
								   ~eErrorLogger();

									eErrorLogger(const eErrorLogger & other) = delete;
									eErrorLogger(eErrorLogger && other) = delete;

	eErrorLogger &					operator=(const eErrorLogger & other) = delete;
	eErrorLogger					operator=(eErrorLogger && other) = delete;

public:

	static eErrorLogger				errorLog;

private:

	std::ofstream					logStream;
	std::string						logFilepath;

};

//******************
// SkipFileKey (global)
// skips "KeyName:" and whitepace b/t ':' and the labelled value
//******************
inline void SkipFileKey(std::ifstream & read) {
	read.ignore(std::numeric_limits<std::streamsize>::max(), ':');
	while (read.peek() == ' ' || read.peek() == '\t')						
		read.ignore();
}

//******************
// VerifyWrite (global)
// closes the stream if it's corrupted
//******************
inline bool VerifyWrite(std::ofstream & write) {
	if (write.bad() || write.fail()) {
		write.close();
		return false;
	}
	return true;
}

//******************
// VerifyRead (global)
// clears and closes the stream if it's corrupted
//******************
inline bool VerifyRead(std::ifstream & read) {
	if (read.bad() || read.fail()) {
		read.clear();
		read.close();
		return false;
	}
	return true;
}

#endif /* EVIL_ERROR_LOGGER_H */