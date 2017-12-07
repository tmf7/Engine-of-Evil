#include "ErrorLogger.h"

eErrorLogger eErrorLogger::errorLog;

//******************
// eErrorLogger::eErrorLogger
//******************
eErrorLogger::eErrorLogger()
	: logFilepath("EngineOfEvil(") {
}

//-------------------
// eErrorLogger::~eErrorLogger
//------------------
eErrorLogger::~eErrorLogger() {
	if (logStream.is_open()) {
		LogError("------------------------------ENDING RUN------------------------------", __FILE__, __LINE__);
		logStream.close();
	}
}

//******************
// eErrorLogger::Init
//******************
bool eErrorLogger::Init() {
	logFilepath += __DATE__;
	logFilepath += ").log";
	logStream.open(logFilepath, std::ios::out | std::ios::app);

	if (!VerifyWrite(logStream)) {
		ErrorPopupWindow("Failed to initialize error output log.");
		return false;
	}

	SDL_LogOutputFunction FileLogFn_ptr = &AutoLogSDLError;
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);	// SDL_LOG_PRIORITY_CRITICAL // 
	SDL_LogSetOutputFunction(FileLogFn_ptr, NULL);		// FIXME: SDL_LogCritical is called alot for some reason, bottleneck (eRenderer::DrawOutlineText, IMG_Load seem to be the reason)

	LogError("------------------------------STARTING RUN------------------------------", __FILE__, __LINE__);
	return true;
}

//******************
// eErrorLogger::ErrorPopupWindow
// immediatly shows a small dialog box with the intended message
// does not log the error, be sure to call LogError as needed
//******************
void eErrorLogger::ErrorPopupWindow(const char * message) {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", message, NULL);
}

//******************
// eErrorLogger::LogError
// outputs error message to the log file in project directory
//******************
void eErrorLogger::LogError(const char * message, const char * sourceFilepath, int lineOfCode) {
	if (!VerifyWrite(logStream)) {
		ErrorPopupWindow("Log output stream corrupted. Log closed.");
		return;
	}

	logStream << "\n\n" << message;
	logStream << "\nFile: " << sourceFilepath << "\nLine: " << lineOfCode;
}


//******************
// eErrorLogger::CheckSDLError
// manually check for any SDL errors, logs them, then clears them
//******************
void eErrorLogger::CheckSDLError(const char * sourceFilepath, int lineOfCode) {
	const std::string error(SDL_GetError());

	if (error != "") {
		const std::string message("\nSDL Error : ");
		LogError((message + error).c_str(), sourceFilepath, lineOfCode);
		SDL_ClearError();
	}
}

//******************
// AutoLogSDLError
// callback given to SDL to log errors
//******************
void AutoLogSDLError(void * userdata, int category, SDL_LogPriority priority, const char * message) {
	EVIL_ERROR_LOG.logStream << "\n\n[SDL ERROR] Category: " << category << "\tPriority: " << priority << "\nMessage: " << message << "\n(See SDL Documentation for category/priority enums)";
}