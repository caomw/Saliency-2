//////////////////////////////////////////////////////////////////////////
// tool function for rgbd video object segmentation
// jiefeng@2014-1-4
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <opencv2/opencv.hpp>
#include <fstream>
#include <string>
#include "Common/common_libs.h"
using namespace cv;


namespace visualsearch
{
	namespace common
	{
		namespace tools
		{

			class RGBDTools
			{
			public:

				RGBDTools() {}

				//////////////////////////////////////////////////////////////////////////
				// I/O

				static bool OutputMaskToFile(std::ofstream& out, const cv::Mat& color_img, const cv::Mat& mask, bool hasProb = false);

				static bool LoadBinaryDepthmap(const std::string& filename, cv::Mat& dmap, int w, int h);

				static bool LoadMat(const std::string& filename, cv::Mat& rmat, int w, int h);

				static bool SavePointsToOBJ(const string& filename, const Mat& pts);

				//////////////////////////////////////////////////////////////////////////
				// visualization

				//////////////////////////////////////////////////////////////////////////
				// transformation

				static bool Proj2Dto3D(const cv::Mat& fg_mask, const cv::Mat& dmap, const cv::Mat& w2c_mat, std::vector<cv::Vec3f>& pts3d);

				// convert kinect depth map to 3d points
				// depth in mm unit
				static bool KinectDepthTo3D(const Mat& dmap, Mat& pts3d);
			};
		}
	}
	
}