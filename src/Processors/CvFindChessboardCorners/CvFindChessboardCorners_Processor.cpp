/*
 * CvFindChessboardCorners_Processor.cpp
 *
 *  Created on: 16-10-2010
 *      Author: mateusz
 */

#include "CvFindChessboardCorners_Processor.hpp"

namespace Processors {

namespace CvFindChessboardCorners {

using namespace std;
using namespace boost;
using namespace cv;
using namespace Types::Objects3D;

CvFindChessboardCorners_Processor::CvFindChessboardCorners_Processor(const std::string & name) :
	Component(name),
	prop_subpix("subpix", true),
	prop_subpix_window("subpix_window", 9, "range"),
	prop_scale("scale", true),
	prop_scale_factor("scale_factor", 2, "range"),
	prop_width("chessboard.width", 9),
	prop_height("chessboard.height", 6),
	prop_square_width("chessboard.square_width", 20),
	prop_square_height("chessboard.square_height", 20)
{
	findChessboardCornersFlags = CV_CALIB_CB_ADAPTIVE_THRESH + CV_CALIB_CB_NORMALIZE_IMAGE + CV_CALIB_CB_FAST_CHECK;


	registerProperty(prop_subpix);

	prop_subpix_window.addConstraint("3");
	prop_subpix_window.addConstraint("11");
	registerProperty(prop_subpix_window);

	registerProperty(prop_scale);

	prop_scale_factor.addConstraint("1");
	prop_scale_factor.addConstraint("8");
	registerProperty(prop_scale_factor);

	registerProperty(prop_width);

	registerProperty(prop_height);

	registerProperty(prop_square_width);

	registerProperty(prop_square_height);
}

CvFindChessboardCorners_Processor::~CvFindChessboardCorners_Processor()
{
}

bool CvFindChessboardCorners_Processor::onFinish()
{
	return true;
}

bool CvFindChessboardCorners_Processor::onStop()
{
	return true;
}

bool CvFindChessboardCorners_Processor::onInit()
{
	h_onNewImage.setup(this, &CvFindChessboardCorners_Processor::onNewImage);
	registerHandler("onNewImage", &h_onNewImage);

	registerStream("in_img", &in_img);
	registerStream("out_chessboard", &out_chessboard);

	chessboardFound = registerEvent("chessboardFound");
	chessboardNotFound = registerEvent("chessboardNotFound");

	/*
	LOG(LINFO) << "CvFindChessboardCorners_Processor: width: " << props.patternSize.width << "\n";
	LOG(LINFO) << "CvFindChessboardCorners_Processor: height: " << props.patternSize.height << "\n";
	LOG(LINFO) << "CvFindChessboardCorners_Processor: squareSize: " << props.squareSize << "\n";

	chessboard = boost::shared_ptr <Chessboard>(new Chessboard(props.patternSize, props.squareSize));

	vector <Point3f> modelPoints;
	for (int i = 0; i < props.patternSize.height; ++i) {
		for (int j = 0; j < props.patternSize.width; ++j) {
			modelPoints.push_back(Point3f(-j * props.squareSize, -i * props.squareSize, 0));
		}
	}
	*/

	initChessboard();

	LOG(LTRACE) << "component initialized\n";
	return true;
}

void CvFindChessboardCorners_Processor::initChessboard() {
	LOG(LINFO) << "CvFindChessboardCorners_Processor: width: " << prop_width << "\n";
	LOG(LINFO) << "CvFindChessboardCorners_Processor: height: " << prop_height << "\n";
	LOG(LINFO) << "CvFindChessboardCorners_Processor: squareSize: " << prop_square_width << "x" << prop_square_height << "mm\n";

	chessboard = boost::shared_ptr <Chessboard>(new Chessboard(cv::Size(prop_height, prop_width), prop_square_width));

	vector <Point3f> modelPoints;
	for (int i = 0; i < prop_height; ++i) {
		for (int j = 0; j < prop_width; ++j) {
			modelPoints.push_back(Point3f(-j * prop_square_height * 0.001, -i * prop_square_width * 0.001, 0));
		}
	}

	chessboard->setModelPoints(modelPoints);
}

bool CvFindChessboardCorners_Processor::onStart()
{
	return true;
}

bool CvFindChessboardCorners_Processor::onStep()
{
	return true;
}

void CvFindChessboardCorners_Processor::onNewImage()
{
	LOG(LTRACE) << "void CvFindChessboardCorners_Processor::onNewImage() begin\n";
	try {
		Mat image = in_img.read();

		timer.restart();

		cv::resize(image, sub_img, Size(), 1.0 / prop_scale_factor, 1.0 / prop_scale_factor, INTER_NEAREST);

		bool found;

		if (prop_scale) {
			found = findChessboardCorners(sub_img, cv::Size(prop_height, prop_width), corners, findChessboardCornersFlags);
			for (int i = 0; i < corners.size(); ++i) {
				corners[i] *= prop_scale_factor;
}
		} else {
			found = findChessboardCorners(image, cv::Size(prop_height, prop_width), corners, findChessboardCornersFlags);
		}

		LOG(LINFO) << "findChessboardCorners() execution time: " << timer.elapsed() << " s\n";

		if (found) {
			LOG(LTRACE) << "chessboard found\n";

			if (prop_subpix) {
				cornerSubPix(image, corners, Size(prop_subpix_window, prop_subpix_window), Size(1, 1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 50, 1e-3));
			}

			chessboard->setImagePoints(corners);
			out_chessboard.write(*chessboard);

			chessboardFound->raise();
		} else {
			LOG(LTRACE) << "chessboard not found\n";

			chessboardNotFound->raise();
		}
	} catch (const Exception& e) {
		LOG(LERROR) << e.what() << "\n";
	}
	LOG(LTRACE) << "void CvFindChessboardCorners_Processor::onNewImage() end\n";
}

} // namespace CvFindChessboardCorners {
} // namespace Processors {
