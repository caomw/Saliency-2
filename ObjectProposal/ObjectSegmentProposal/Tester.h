//////////////////////////////////////////////////////////////////////////
// various testing function: higher level task
// jiefeng@2014-10-04
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "common3d.h"
#include "Common/common_libs.h"

#include "ObjectRanker.h"
using namespace visualsearch;
using namespace visualsearch::io;
using namespace visualsearch::processors;

class ObjectProposalTester {

public:
	void TestRankerLearner();

	void ComputeNormals(SuperPixel& sp);

private:


};

//////////////////////////////////////////////////////////////////////////

