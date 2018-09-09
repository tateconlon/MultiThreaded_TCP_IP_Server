#pragma once
#include <mutex>
#include <fstream>
#include <string>
class LogFiles
{
	std::mutex _mu;
	std::ofstream _f;
	std::string filename;
public:
	LogFiles(std::string filename);
	LogFiles();
	~LogFiles();
	void shared_log(std::string msg);
};

