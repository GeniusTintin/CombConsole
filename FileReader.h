#pragma once
#include <iostream>
#include <string>
#include <fstream>

namespace FileReader {
	class fileReader {

	public:
		fileReader();
		void readOneLine();
		void getValue(uint64_t& return_value, const int i);

	private:
		void initialiseFile(std::string fullPath);
		void getNumber(double* numArray, std::string str);

		int timeResolution_ = 1e6;
		std::string path_ = "";
		std::string fileName_ = "";
		std::ifstream myFile_;
		bool fileInitialised_ = false;
		bool userInputPath_ = true;
		double myNumber_[4] = { 0.0 };
	};
}