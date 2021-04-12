#include "CombFilter.h"
#include <iostream>

namespace CombFilter {

	combFilter::combFilter() {

	}

	combFilter::~combFilter() {

	}

	void combFilter::user_size_input() {
		
		std::cout << "Enter user defined height and width: " << std::endl;
		std::cout << "Enter height: " << std::endl;
		std::cin >> height_user_defined_;
		std::cout << "Enter width: " << std::endl;
		std::cin >> width_user_defined_;

		user_defined_size_ = true;
	}
	
	inline void combFilter::wait_on_enter() {

		std::cout << "Enter to continue..." << std::endl;
		std::cin.ignore().get();
	}

	void combFilter::write_log(std::string filename, std::string content) {

		std::string path = "D:\\Home\\MyProjects\\VisualStudio\\repos\\CombConsole\\";
		path += filename;
		std::ofstream file2write;
		file2write.open(path, std::ios_base::app); // append instead of overwrite
		file2write << content << std::endl;
	}
}