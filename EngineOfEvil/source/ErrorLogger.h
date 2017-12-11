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