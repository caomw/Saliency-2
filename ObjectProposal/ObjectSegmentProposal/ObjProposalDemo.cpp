#include "ObjProposalDemo.h"


ObjProposalDemo::ObjProposalDemo()
{
	frameid = 0;
	DATADIR = "e:\\res\\kinectvideos\\1\\";
}

//////////////////////////////////////////////////////////////////////////

bool ObjProposalDemo::RunVideoDemo(SensorType stype, DemoType dtype)
{
	bool tosave = false;
	visualsearch::io::camera::OpenCVCameraIO cam;
	if(stype == SENSOR_CAMERA)
	{
		if( !cam.InitCamera() )
			return false;
	}
	/*KinectDataMan kinect;
	if(stype == SENSOR_KINECT)
	{
	if( !kinect.InitKinect() )
	return false;
	}*/

	//if( !kinectDM.InitKinect() )
	//return false;

	char str[100];
	frameid = 0;

	while(1)
	{
		Mat cimg, dmap;
		if(stype == SENSOR_CAMERA) {
			if( !cam.QueryNextFrame(visualsearch::io::camera::STREAM_COLOR, cimg) )
				continue;
		}
		/*if(stype == SENSOR_KINECT) {
		if( !kinect.GetColorDepth(cimg, dmap) )
		continue;
		}*/

		// downsample cimg to have same size as dmap
		Size newsz;
		visualsearch::common::tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
		resize(cimg, cimg, newsz);
		// show input
		imshow("color", cimg);
		if (!dmap.empty()) {
			resize(dmap, dmap, newsz);
			ImgVisualizer::DrawFloatImg("depth", dmap);
		}

		if(tosave) {
			sprintf_s(str, "frame_%d.jpg", frameid);
			imwrite(DATADIR+string(str), cimg);
			sprintf_s(str, "frame_%d_d.png", frameid);
			imwrite(DATADIR+string(str), dmap);
		}
		
		frameid++;

		/*if(dtype == DEMO_OBJECT_WIN)
			RunObjWinProposal(cimg, dmap);*/
		if(dtype == DEMO_OBJECT_SEG)
			RunObjSegProposal("", cimg, dmap, Mat());
		if(dtype == DEMO_SAL)
			RunSaliency(cimg, dmap, visualsearch::processors::attention::SAL_HC);

		if( waitKey(10) == 'q' )
			break;
	}

	return true;
}

bool ObjProposalDemo::RunObjSegProposal(string fn, Mat& cimg, Mat& dmap, Mat& oimg)
{
	// propose
	vector<VisualObject> sps;
	seg_proposal.Run(cimg, dmap, 5, sps);

	//return true;
	
	// display results
	vector<ImgWin> boxes;
	for (auto val : sps) {
		boxes.push_back(val.visual_data.bbox);
	}
	imshow("mask", sps[0].visual_data.mask * 255);
	cout << "contour length: " << sps[0].visual_data.original_contour.size() << endl;
	cout << sps[0].visual_data.area*1.0f / (sps[0].visual_data.mask.rows*sps[0].visual_data.mask.cols) << endl;
	Mat contour_pts = Mat::zeros(cimg.rows, cimg.cols, CV_8U);
	for (auto p : sps[0].visual_data.original_contour) {
		contour_pts.at<uchar>(p) = 255;
	}
	imshow("contour pts", contour_pts);
	cout << ImgVisualizer::DrawShapes(cimg, sps, oimg, true) << endl;

	// save result image
	imwrite(fn + "_res.png", oimg);
	// convert to 3d point cloud and output to file
	visualsearch::features::Feature3D feat3d;
	Mat pts3d;
	feat3d.ComputeKinect3DMap(dmap, pts3d);
	ofstream out(fn + ".obj");
	for (int r = 0; r < pts3d.rows; r++)
	{
		for (int c = 0; c < pts3d.cols; c++)
		{
			if (sps[0].visual_data.mask.at<uchar>(r, c) > 0) {
				Vec3f curval = pts3d.at<Vec3f>(r, c);
				out << "v " << curval.val[0] << " " << curval.val[1] << " " << curval.val[2] << std::endl;
			}
		}
	}


	//ImgVisualizer::DrawWinsOnImg("objects", cimg, boxes, oimg);
	//resize(oimg, oimg, Size(oimg.cols*2, oimg.rows*2));
	/*char str[30];
	sprintf_s(str, "%d_0.jpg", frameid);
	imwrite(DATADIR + string(str), cimg);
	imgvis.DrawWinsOnImg("input", cimg, drawwins, oimg);
	sprintf_s(str, "%d_1.png", frameid);
	dmap.copyTo(oimg);
	imwrite(DATADIR + string(str), oimg);
	imgvis.DrawCroppedWins("obj", cimg, drawwins, 5, oimg);
	sprintf_s(str, "%d_2.jpg", frameid);
	imwrite(DATADIR + string(str), oimg);*/
	
	// check lock file
	/*string lockfn = DATADIR + "demo.lock";
	ifstream in(lockfn);
	if( in.is_open() )
		return true;*/

	// save object
	//for (size_t i=0; i<drawwins.size(); i++)
	//{
	//	// color
	//	sprintf_s(str, "obj_%d_%d_c.png", frameid, i);
	//	imwrite(DATADIR+string(str), cimg(drawwins[i]));
	//	if( dmap.empty() )
	//		continue;
	//	// depth
	//	sprintf_s(str, "obj_%d_%d_d.png", frameid, i);
	//	imwrite(DATADIR+string(str), dmap(drawwins[i]));
	//}

	// write lock file
	/*ofstream out(lockfn);
	out.close();*/

	return true;
}

//bool ObjProposalDemo::RunObjWinProposal(Mat& cimg, Mat& dmap)
//{
//	// resize image
//	Size newsz;
//	tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 400, newsz);
//	resize(cimg, cimg, newsz);
//
//	/*Mat salmap;
//	salcomputer.ComputeSaliencyMap(cimg, SAL_HC, salmap);
//	salmap.convertTo(salmap, CV_8U, 255);
//	cvtColor(salmap, salmap, CV_GRAY2BGR);
//	imshow("salcolor", salmap);
//	waitKey(10);
//	*/
//
//	// get objects
//	vector<ImgWin> objwins, salwins;
//	if( !bing.GetProposals(cimg, objwins, 1000) )
//		return false;
//
//	// rank
//	vector<int> sorted_ids;
//	ranker.RankWindowsBySaliency(cimg, objwins, sorted_ids);
//
//	// nms
//	vector<ImgWin> drawwins = visualsearch::processors::nms(objwins, 0.4f);
//	drawwins.resize(MIN(drawwins.size(), 10));
//
//	//for (size_t i=0; i<MIN(sorted_ids.size(), 10); i++)
//		//drawwins.push_back(objwins[sorted_ids[i]]);
//	//objwins.erase(objwins.begin()+10, objwins.end());
//	
//	Mat oimg;
//	char str[30];
//	imgvis.DrawWinsOnImg("input", cimg, drawwins);
//	sprintf_s(str, "e:\\res\\objectness\\%d_1.jpg", frameid);
//	imwrite(str, cimg);
//	imgvis.DrawCroppedWins("obj", cimg, drawwins, 5, oimg);
//	sprintf_s(str, "e:\\res\\objectness\\%d_2.jpg", frameid);
//	imwrite(str, oimg);
//
//	return true;
//}

bool ObjProposalDemo::RunSaliency(Mat& cimg, Mat& dmap, visualsearch::processors::attention::SaliencyType saltype)
{
	// resize image
	Size newsz;
	tools::ToolFactory::compute_downsample_ratio(Size(cimg.cols, cimg.rows), 300, newsz);
	resize(cimg, cimg, newsz);

	Mat salmap;
	salcomputer.ComputeSaliencyMap(cimg, saltype, salmap);

	imshow("color", cimg);
	imshow("sal", salmap);

	return true;
}