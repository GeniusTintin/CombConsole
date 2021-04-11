#pragma once
#include <string>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>

namespace CombFilter {
	class combFilter {
	public:
		combFilter();
		~combFilter();

	private:
		bool initialised_ = false;
		uint32_t buffer_length_ = 0;
		uint32_t buffer_index_ = 0;
	};
}