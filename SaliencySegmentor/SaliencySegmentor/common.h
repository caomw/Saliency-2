//////////////////////////////////////////////////////////////////////////
//	common structures
//////////////////////////////////////////////////////////////////////////


#pragma once


#include <opencv2/opencv.hpp>
#include <set>
using namespace std;
using namespace cv;


namespace Saliency
{

	/************************************************************************/
	/* Structures                                                           */
	/************************************************************************/

	// information of another segment
	struct TPair
	{
		TPair(): id(-1), appdist(0), spadist(0), saliency(0)	{}

		int id;	// compared sp id
		float appdist;	// appearance distance
		float spadist;	// spatial distance
		float saliency;	// pair composition cost

		static bool comp_by_saliency(const TPair& a, const TPair& b) 
		{
			return a.saliency < b.saliency;	
		}
	};


	// superpixel dynamic features only used for composition
	struct SegSuperPixelComposeFeature
	{
		SegSuperPixelComposeFeature() : composition_cost(0.f)
			,extendedArea(0), currentArea(0), leftInnerArea(0), leftOuterArea(0)
			,dist_to_win(0), importWeight(1)
		{}

		//void Init(const ImageFloatSimple& saliencyMatrix);	

		vector<TPair> pairs;	// list for all other superpixels
		float leftInnerArea, leftOuterArea;
		float extendedArea;	// area after boundary extension
		float composition_cost;	// cost for composing current superpixel
		float currentArea;	// area need to be filled currently
		float importWeight;	// importance weight: may affected by relative position, background likelihood...	

#ifdef RECORD_AUXILIARY_INFO
		vector<TPair> composers;	 // segments composing current sp
#endif

		float dist_to_win;	// distance between centroid to window center

		static bool comp_by_dist(const SegSuperPixelComposeFeature* a, const SegSuperPixelComposeFeature* b)	{	return a->dist_to_win > b->dist_to_win;	}

		static bool comp_by_cost(const SegSuperPixelComposeFeature* a, const SegSuperPixelComposeFeature* b)	{	return a->composition_cost < b->composition_cost;	}

		// x: id, y: area
		static bool comp_by_area(const Point a, const Point b) { return a.y > b.y; }

	};



	// superpixel feature
	struct SegSuperPixelFeature
	{
		SegSuperPixelFeature() : bnd_pixels(0), perimeter(0), 
			area(0), id(-1), saliency(0)
		{ box_pos[0].x = 100000; box_pos[0].y = 100000; box_pos[1].x = 0; box_pos[1].y = 0; }

		vector<float> feat;	// feature vector

		Point box_pos[2];	// bounding box: top-left, bottom-right
		Rect box;	// bounding box
		float bnd_pixels;	// boundary pixel numbers
		float perimeter;
		float area;
		Point2f centroid;
		set<int> neighbor_ids;
		unsigned int id;
		Mat mask;	// currently, same size as image for each superpixel
		Mat mask_integral;
		vector<bool> components;	// true: member
		float saliency;

		static bool InsideSegment(const Point& pt, const SegSuperPixelFeature& sp_feat)
		{
			if(sp_feat.mask.empty() || !sp_feat.box.contains(pt))
				return false;
			
			if( sp_feat.mask.at<uchar>(pt) > 0 )
				return true;
			else
				return false;
		}

		static float FeatureIntersectionDistance(const SegSuperPixelFeature& a, const SegSuperPixelFeature& b)
		{
			// do normalization first
			vector<float> feat1 = a.feat;
			vector<float> feat2 = b.feat;
			for(size_t i=0; i<feat1.size(); i++)
			{
				feat1[i] /= (a.area*3);
				feat2[i] /= (b.area*3);
			}

			float dist = 0;
			for(size_t i = 0; i < feat1.size(); i++)
				dist += ((feat1[i] < feat2[i]) ? feat1[i] : feat2[i]);
			return 1 - dist;
		}


		// composition feature
		SegSuperPixelComposeFeature compose_feat;

	};


	struct ScoredRect : public Rect
	{
		ScoredRect() : score(0){}
		ScoredRect(const Rect& r) : Rect(r), score(0){}
		ScoredRect(const Rect& r, float s) : Rect(r), score(s){}
		float score;

		static bool comp_by_score(const ScoredRect& a, const ScoredRect& b)
		{
			return a.score < b.score;
		}
	};

	//used in paper: L_A_B 4 8 8
	// R_G_B: 8 8 8
	const int quantBins[3] = {4, 8, 8}; 

	const bool use4Neighbor = true;


	//////////////////////////////////////////////////////////////////////////
	inline float l2_dist(float x1, float y1, float x2, float y2)
	{
		return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
	}

	inline float l2_dist(const Point2f& p1, const Point2f& p2)
	{
		return l2_dist(p1.x, p1.y, p2.x, p2.y);
	}



	template<class ScoreRectType>
	vector<ScoreRectType> nms(vector<ScoreRectType>& input, float overlapRate)
	{
		int wincount = 0;
		vector<bool> flags(input.size(), false);

		// sort windows by scores (low->high)
		sort(input.begin(), input.end(), ScoredRect::comp_by_score);

		for(size_t i=0; i<input.size(); i++)
		{
			for(size_t j=i+1; j<input.size(); j++)
			{
				int xx1 = max(input[i].x, input[j].x);
				int yy1 = max(input[i].y, input[j].y);
				int xx2 = min(input[i].br().x, input[j].br().x);
				int yy2 = min(input[i].br().y, input[j].br().y);
				float width = xx2-xx1+1;
				float height = yy2-yy1+1;
				if(width>0 && height>0)
				{
					float overlap = 
						(width*height) / 
						(input[i].width*input[i].height+input[j].width*input[j].height-width*height);
					if(overlap > overlapRate)
					{
						flags[i] = true;	//suppressed
						wincount++;
						break;
					}
				}
			}
		}

		vector<ScoredRect> res;
		res.reserve(wincount);
		for(int i=input.size()-1; i>=0; i--)
		{
			if(!flags[i])
				res.push_back(input[i]);
		}

		return res;
	}

}
