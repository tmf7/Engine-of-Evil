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
#include "ErrorLogger.h"

using namespace evil;

std::ofstream eErrorLogger::logStream;
std::string	eErrorLogger::logFilepath("EngineOfEvil(");

//******************
// eErrorLogger::Shutdown
//******************
void eErrorLogger::Shutdown() {
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
void evil::AutoLogSDLError(void * userdata, int category, SDL_LogPriority priority, const char * message) {
	eErrorLogger::logStream << "\n\n[SDL ERROR] Category: " << category << "\tPriority: " << priority << "\nMessage: " << message << "\n(See SDL Documentation for category/priority enums)";
}