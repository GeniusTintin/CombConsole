#pragma once
#include <string>
#include <stdio.h>
#include <fstream>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

namespace CombFilter {
	class combFilter {
	public:

		combFilter();
		virtual ~combFilter();
		void set_parameters();
		void eventsCallback();

	private:

		void initialise_image_states(const uint32_t& rows, const uint32_t& columns);
		void initialise_buffer(const uint32_t& rows, const uint32_t& columns);
		void integral_tracking(const int x, const int y, const bool polarity);
		void store2buffer(const cv::Mat& figx, const cv::Mat& figy);
		void grab_delay(cv::Mat& sel, const int i1, const int which_buffer);
		void exp_of_log(cv::Mat& converted_image);
		void user_size_input();
		inline void wait_on_enter();
		void write_log(std::string filename, std::string content);

		bool initialised_ = false;
		int filtering_method_ = 0;
		
		// ring buffer
		cv::Mat* ring_buffer1_ = nullptr;
		cv::Mat* ring_buffer2_ = nullptr;
		uint32_t buffer_length_ = 0;
		uint32_t buffer_index_ = 0;

		uint64_t img_height_ = 0;
		uint64_t img_width_ = 0;

		// delayed integrated events
		cv::Mat x0_ = cv::Mat(1,1,CV_64FC1,0);
		cv::Mat x_d1_ = cv::Mat(1, 1, CV_64FC1, 0);
		cv::Mat x_d2_ = cv::Mat(1, 1, CV_64FC1, 0);
		cv::Mat x_d12_ = cv::Mat(1, 1, CV_64FC1, 0);

		// delayed output
		cv::Mat y0_ = cv::Mat(1, 1, CV_64FC1, 0);
		cv::Mat y_d1_ = cv::Mat(1, 1, CV_64FC1, 0);
		cv::Mat y_d2_ = cv::Mat(1, 1, CV_64FC1, 0);
		cv::Mat y_d12_ = cv::Mat(1, 1, CV_64FC1, 0);

		// user defined size
		bool user_defined_size_ = false;
		uint32_t height_user_defined_ = 0;
		uint32_t width_user_defined_ = 0;

		// time delay
		double d1_ = 0;
		double d2_ = 0;
		double d12_ = 0;

		// delay gain
		double rho1_ = 0;
		double rho2_ = 0;

		// minimul time resolution
		double mtr_ = 1e5;
		double t_next_store_ = 0.0;

		// publish parameters
		double publish_framerate_ = 1e4;
		double t_next_publish_ = 0.0;
		
		// cutoff frequency
		double cutoff_freq_global_ = 0;
		double cutoff_freq_per_event_component_ = 0;
		double event_count_cutoff_freq_ = 0;

		// contrast threshold 
		double contrast_threshold_on_user_defined_ = 0.1;
		double contrast_threshold_off_user_defined_ = 0.1;
	};

} // CombFilter 

// Dingran Yuan