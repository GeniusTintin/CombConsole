#pragma once
#include <iostream>
#include <string>
#include <fstream>

namespace FileReader {
	class fileReader {

	public:
		fileReader();
		void readOneLine(bool& iseof);

		struct eventData {

			uint64_t ts;
			uint32_t x;
			uint32_t y;
			uint8_t polarity;
		};

		eventData eData_;
		int timeResolution_ = 1e6;

	private:
		void initialiseFile(std::string fullPath);
		void getNumber_Legacy(double* numArray, std::string str);
		void getNumber(std::string str);
		
		std::string path_ = "";
		std::string fileName_ = "";
		std::ifstream myFile_;
		bool fileInitialised_ = false;
		bool userInputPath_ = true;

	};
}