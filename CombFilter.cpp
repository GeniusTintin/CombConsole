#define _USE_MATH_DEFINES
#include "CombFilter.h"
#include "FileReader.h"
#include <iostream>
#include <cmath>

namespace CombFilter {

	combFilter::combFilter() {

		
	}

	combFilter::~combFilter() {

		// free the buffer space before exit
		delete[] ring_buffer1_;
		delete[] ring_buffer2_;
	}

	void combFilter::eventsCallback() {
		
		FileReader::fileReader myReader;
		uint64_t title;
		myReader.readOneLine();
		myReader.getValue(title, 3);

		if (title >= 2) {

			myReader.getValue(img_height_, 1);
			myReader.getValue(img_width_, 2);
		}
		else{

			user_size_input();
			img_height_ = height_user_defined_;
			img_width_ = width_user_defined_;
		}

		if (!initialised_) {

			// initialise image parameters
			initialise_image_states(img_height_, img_width_);
		}

		 
	}

	void combFilter::initialise_image_states(const uint32_t& rows, const uint32_t& columns) {

		t_next_publish_ = 0.0;
		
		cutoff_freq_global_ = 0.06 * 2 * M_PI;
		cutoff_freq_per_event_component_ = 0.06;
		contrast_threshold_on_user_defined_ = 0.1;
		contrast_threshold_off_user_defined_ = -0.1;

		// delayed version of integrated events
		x0_ = cv::Mat::zeros(rows, columns, CV_64FC1);
		x_d1_ = cv::Mat::zeros(rows, columns, CV_64FC1);
		x_d2_ = cv::Mat::zeros(rows, columns, CV_64FC1);
		x_d12_ = cv::Mat::zeros(rows, columns, CV_64FC1);

		// delayed version of output
		y0_ = cv::Mat::zeros(rows, columns, CV_64FC1);
		y_d1_ = cv::Mat::zeros(rows, columns, CV_64FC1);
		y_d2_ = cv::Mat::zeros(rows, columns, CV_64FC1);
		y_d12_ = cv::Mat::zeros(rows, columns, CV_64FC1);

		// time delay
		double base_freq;
		std::cout << "Enter base frequency: " << std::endl;
		std::cin >> base_freq;
		std::cout << "Filtering method (1:direct integration, 2:comb, 3:improved comb): " << std::endl;
		std::cin >> filtering_method_;

		d1_ = 1 / base_freq;
		d2_ = d1_ / 10;
		d12_ = d1_ + d2_;

		// delay gain
		rho1_ = 0.9;
		rho2_ = 0.99;

		initialise_buffer(rows, columns);

		initialised_ = true;

		std::cout << "Initialised! " << std::endl;
	}

	void combFilter::initialise_buffer(const uint32_t& rows, const uint32_t& columns) {

		// minimum time resolution
		mtr_ = 1e5; // NOTE: this should be 1e-5, but due to the accuracy of the floating point number we use positive index here!
		t_next_store_ = 0.0;
		buffer_length_ = int(d12_ * mtr_ + 1);
		buffer_index_ = 0;

		// FIXME: validate that the buffer size is correctly constructed
		std::cout << "tao1:" << d1_ << " tao2:" << d2_ << " tao12:" << d12_ << std::endl;
		std::cout << 1 / mtr_ << std::endl;
		std::cout << "Buffer length:" << buffer_length_ << std::endl;
		wait_on_enter();

		ring_buffer1_ = new cv::Mat[buffer_length_];
		ring_buffer2_ = new cv::Mat[buffer_length_];

		// zero initialisation for all buffers
		for (int i = 0; i < buffer_length_; i++) {

			cv::Mat temp = cv::Mat::zeros(rows, columns, CV_64FC1);
			temp.copyTo(ring_buffer1_[i]);
			temp.copyTo(ring_buffer2_[i]);
		}
	}

	void combFilter::integral_tracking(const int x, const int y, const bool polarity) {

		double c_times_p;
		
		c_times_p = (polarity) ? contrast_threshold_on_user_defined_ : contrast_threshold_off_user_defined_;

		x0_.at<double>(y, x) = x0_.at<double>(y, x) + c_times_p;
	}

	void combFilter::store2buffer(const cv::Mat& figx, const cv::Mat& figy) {

		// buffer index of input and output is the same
		figx.copyTo(ring_buffer1_[buffer_index_]);
		figy.copyTo(ring_buffer2_[buffer_index_]);

		buffer_index_++;

		if (buffer_index_ == buffer_length_) {
			buffer_index_ = 0;
		}

		// after storing, the index is pointing to the next store location
	}

	void combFilter::grab_delay(cv::Mat& sel, const int i1, const int which_buffer) {

		int index = buffer_index_ - i1;

		if (index < 0) {
			index = buffer_length_ + index;
		}

		if (which_buffer == 1) {
			ring_buffer1_[index].copyTo(sel);
		}
		else {
			ring_buffer2_[index].copyTo(sel);
		}
	}

	void combFilter::exp_of_log(cv::Mat& converted_image) {

		double LOG_INTENSITY_OFFSET = std::log(1.5);

		x0_.copyTo(converted_image);

		converted_image += LOG_INTENSITY_OFFSET;
		cv::exp(converted_image, converted_image);
		converted_image -= 1;
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