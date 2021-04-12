#define _USE_MATH_DEFINES
#include "CombFilter.h"
#include <iostream>
#include <cmath>

namespace CombFilter {

	combFilter::combFilter() {

		static FileReader::fileReader eventReader;
		myReaderPtr_ = &eventReader;
	}

	combFilter::~combFilter() {

		// free the buffer space before exit
		delete[] ring_buffer1_;
		delete[] ring_buffer2_;
	}

	void combFilter::eventsCallback() {
		
		//FileReader::fileReader myReader;
		// the first line of the dataset is designed to be the image information state
		
		bool iseof = false;
		myReaderPtr_->readOneLine(iseof);
		if (iseof) {
			std::cout << "ERROR: Void package." << std::endl;
			return;
		}

		uint8_t title = myReaderPtr_->eData_.polarity;

		if (title >= 2) {

			img_height_ = myReaderPtr_->eData_.y;
			img_width_ = myReaderPtr_->eData_.x;
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

		while (!iseof) {
			
			myReaderPtr_->readOneLine(iseof);

			// declear variable
			uint32_t x = myReaderPtr_->eData_.x;
			uint32_t y = myReaderPtr_->eData_.y;

			if (x >= 0 && x < img_width_ && y >= 0 && y < img_height_) {
				
				uint64_t ts = myReaderPtr_->eData_.ts;
				uint8_t polarity = myReaderPtr_->eData_.polarity;

				// integral tracking
				integral_tracking(x, y, polarity);

				// grab delay and calculate y0_
				while (ts >= t_next_store_) {
					if (t_next_store_ == 0) {
						t_next_store_ = ts;
					}

					cv::Mat x0_e = cv::Mat::zeros(img_height_, img_width_, CV_64FC1);
					exp_of_log(x0_e);

					grab_delay(x_d1_, int(d1_), 1);
					grab_delay(x_d2_, int(d2_), 1);
					grab_delay(x_d12_, int(d12_), 1);

					grab_delay(y_d1_, int(d1_), 2);
					grab_delay(y_d2_, int(d2_), 2);
					grab_delay(y_d12_, int(d12_), 2);

					// calculate new y0_
					switch (filtering_method_) {
					case 1: {
						y0_ = x0_e; // direct integration
						break;
					}
					case 2: {
						y0_ = x0_e - x_d1_; // simple version of comb (no dc)
						break;
					}
					default: {
						y0_ = x0_e - x_d1_ - rho2_ * x_d2_ + rho2_ * x_d12_ + rho1_ * y_d1_ + y_d2_ - rho1_ * y_d12_;
					}
					}
					// FIXME:
						// std::cout << ts << std::endl;
						// if(ts >= 4.48){
						//     std::string message = "time: ";
						//     message += std::to_string(ts);
						//     message += ", value at (10,10): ";
						//     message += std::to_string(y0_.at<double>(10,10));
						//     message += ", value at (20,20): ";
						//     message += std::to_string(y0_.at<double>(20,20));
						//     write_log("log.txt", message);
						// }

					/*
					if (ts >= 4.47 && (y0_.at<double>(10, 10) > 0.59 || y0_.at<double>(10, 10) < 0.45)) {
						std::cout << ts << std::endl;
						// std::cout << y0_ << std::endl;
						std::string message = "time: ";
						message += std::to_string(ts);
						message += "value at (10,10): ";
						message += std::to_string(y0_.at<double>(10, 10));
						write_log("delay_value.txt", message);

						message = "y_d1_: ";
						message += std::to_string(y_d1_.at<double>(10, 10));
						write_log("delay_value.txt", message);

						message = "y_d2_: ";
						message += std::to_string(y_d2_.at<double>(10, 10));
						write_log("delay_value.txt", message);

						message = "y_d12_: ";
						message += std::to_string(y_d12_.at<double>(10, 10));
						write_log("delay_value.txt", message);

						message = "x_d1_: ";
						message += std::to_string(x_d1_.at<double>(10, 10));
						write_log("delay_value.txt", message);

						message = "x_d2_: ";
						message += std::to_string(x_d2_.at<double>(10, 10));
						write_log("delay_value.txt", message);

						message = "x_d12_: ";
						message += std::to_string(x_d12_.at<double>(10, 10));
						write_log("delay_value.txt", message);

						message = "x0_e: ";
						message += std::to_string(x0_e.at<double>(10, 10));
						write_log("delay_value.txt", message);

						wait_on_enter();
					}
					*/
					store2buffer(x0_e, y0_);
					t_next_store_ += myReaderPtr_->timeResolution_ / mtr_;


					if (publish_framerate_ > 0 && ts >= t_next_publish_) {

						publish_intensity_estimate();
						t_next_publish_ = ts + myReaderPtr_->timeResolution_ / publish_framerate_;
					}
				}
			}
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

		d1_ = 1 * myReaderPtr_->timeResolution_ / base_freq;
		d2_ = d1_ / 10;
		d12_ = d1_ + d2_;

		// delay gain
		rho1_ = 0.9;
		rho2_ = 0.99;

		cv::namedWindow(window_name_, cv::WINDOW_NORMAL);

		initialise_buffer(rows, columns);

		initialised_ = true;

		std::cout << "Initialised! " << std::endl;
	}

	void combFilter::initialise_buffer(const uint32_t& rows, const uint32_t& columns) {

		// minimum time resolution
		mtr_ = 1e5; // NOTE: this should be 1e-5, but due to the accuracy of the floating point number we use positive index here!
		t_next_store_ = 0.0;
		buffer_length_ = int(d12_ * mtr_ / myReaderPtr_->timeResolution_ + 1);
		buffer_index_ = 0;

		// FIXME: validate that the buffer size is correctly constructed
		std::cout << "tao1:" << d1_ / myReaderPtr_->timeResolution_ << " tao2:" << d2_ / myReaderPtr_->timeResolution_ << " tao12:" << d12_ / myReaderPtr_->timeResolution_ << std::endl;
		std::cout << "mtr_ = " << 1 / mtr_ << std::endl;
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

	void combFilter::grab_delay(cv::Mat& sel, const int delay, const int which_buffer) {

		int index_offset = delay * mtr_ / myReaderPtr_->timeResolution_;
		int index = buffer_index_ - index_offset;

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

	void combFilter::publish_intensity_estimate() {

		cv::Mat display_image;
		y0_.convertTo(display_image, CV_8UC1, 255.0);
		cv::imshow(window_name_, display_image);
		cv::waitKey(1);
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

// Dingran Yuan