#include "stdafx.h"
#include "LogFile.h"

LogFiles::LogFiles(std::string filename)
{
	this->filename = filename;
	//_f.open(filename + ".txt");
}

LogFiles::LogFiles() {

}


LogFiles::~LogFiles()
{
	//_f.close();
}

void LogFiles::shared_log(std::string msg) {
	//std::lock_guard<std::mutex> guard(_mu);
	//_f.write(msg.c_str(), msg.length());
}