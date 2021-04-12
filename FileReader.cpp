#include "FileReader.h"

namespace FileReader {

    fileReader::fileReader() {
        
        int userPath;
        std::cout << "Whether user input path? (0: Default path, 1: User input path)" << std::endl;
        std::cin >> userPath;

        switch (userPath) {
        case 0: {
            userInputPath_ = false;
            break;
        }
        default:
            userInputPath_ = true;
        }

        if (userInputPath_) {
            std::cout << "Enter file path: " << std::endl;
            std::cin >> path_;
        }
        else{
            path_ = "D:\\Home\\MyProjects\\VisualStudio\\repos\\CombConsole\\";
        }

        std::cout << "Enter file name: " << std::endl;
        std::cin >> fileName_;
        fileInitialised_ = false;
    }

    void fileReader::readOneLine(bool& iseof) {

        if (!fileInitialised_) {
            std::string fullPath = path_ + fileName_;
            initialiseFile(fullPath);
        }

        if (myFile_.eof()) {
            iseof = true;
        }

        std::string str;
        std::getline(myFile_, str);
        getNumber(myNumber_, str);
        

    }

    void fileReader::getValue(uint64_t& return_value, int i) {
        // get value from myNumber_
        // [0,1,2,3]: [ts, x, y, polarity]
        if (i == 0) {
            return_value = uint64_t(timeResolution_ * myNumber_[i]);
        }
        else {
            return_value = uint64_t(myNumber_[i]);
        }
    }

    void fileReader::initialiseFile(std::string fullPath) {
         
        myFile_.open(fullPath, std::ios::in);
        timeResolution_ = 1e6;
        fileInitialised_ = true;
    }

    void fileReader::getNumber(double* numArray, std::string str) {

        std::string word;
        int i = 0;
        for (auto x : str) {
            if (x == ' ') {
                numArray[i] = atof(word.c_str());
                word = "";
                i++;
            }
            else {
                word = word + x;
            }
        }
        numArray[i] = atof(word.c_str());
    }
}