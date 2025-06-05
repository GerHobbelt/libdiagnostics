
#include <diagnostics/diagnostics.h>

#include <spdlog/spdlog.h>
#include <fmt/format.h>



// -------------------------------------------------
// https://stackoverflow.com/questions/851679/saving-an-image-in-opencv
// https://opencv.org/blog/read-display-and-write-an-image-using-opencv/




#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/version.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/core/cuda_types.hpp>
#include <opencv2/core/cuda.inl.hpp>
#include <opencv2/core/cuda_stream_accessor.hpp>
#include <opencv2/core/cuda_stream.hpp>




using namespace cv;
// Capture the Image from the webcam
VideoCapture cap(0);

// Get the frame
Mat save_img; cap >> save_img;

if (save_img.empty())
{
	std::cerr << "Something is wrong with the webcam, could not get frame." << std::endl;
}
// Save the frame into a file
imwrite("test.jpg", save_img); // A JPG FILE IS BEING SAVED











In my experience OpenCV writes a black image when SaveImage is given a matrix with bit depth different from 8 bit. In fact, this is sort of documented:

Only 8-bit single-channel or 3-channel (with ‘BGR’ channel order) images can be saved using this function. If the format, depth or channel order is different, use cvCvtScale and cvCvtColor to convert it before saving, or use universal cvSave to save the image to XML or YAML format.

In your case you may first investigate what kind of image is captured, change capture properties (I suppose CV_CAP_PROP_CONVERT_RGB might be important) or convert it manually afterwards.

This is an example how to convert to 8-bit representation with OpenCV. cc here is an original matrix of type CV_32FC1, cc8u is its scaled version which is actually written by SaveImage:

# I want to save cc here
cc8u = CreateMat(cc.rows, cc.cols, CV_8U)
ccmin, ccmax, minij, maxij = MinMaxLoc(cc)
ccscale, ccshift = 255.0/(ccmax-ccmin), -ccmin
CvtScale(cc, cc8u, ccscale, ccshift)
SaveImage("cc.png", cc8u)
(sorry, this is Python code, but it should be easy to translate it to C/C++)


















I had similar problem with my Microsoft WebCam. I looked in the image aquisition toolbox in Matlab and found that the maximum supported resolution is 640*480.

I just changed the code in openCV and added

cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 352);
cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 288);
before cvQueryFrame function which was the next supported resolution and changed skipped some initial frames before saving the image and finally got it working.

I am sharing my working Code

#include "cv.h" 
#include "highgui.h" 
#include <stdio.h> 



using namespace cv;
using namespace std;

int main()
{
	CvCapture* capture = cvCaptureFromCAM(CV_CAP_ANY);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 352);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 288)


		// Get one frame
		IplImage* frame;

	for (int i = 0; i < 25; i++)
	{
		frame = cvQueryFrame(capture);
	}


	printf("Image captured \n");
	//IplImage* RGB_frame = frame;
	//cvCvtColor(frame,RGB_frame,CV_YCrCb2BGR);
	//cvWaitKey(1000);
	cvSaveImage("test.jpg", frame);
	//cvSaveImage("cam.jpg" ,RGB_frame);

	printf("Image Saved \n");

	//cvWaitKey(10);

	// Release the capture device housekeeping
	cvReleaseCapture(&capture);
	//cvDestroyWindow( "mywindow" );
	return 0;
}
My Suggestions:

Dont grab frame with maximum resolution
Skip some frames for correct camera initialisation



















Use cvSetCaptureProperty to set the frame width and height to a supported resolution














1

hopefully this will save images form your webcam

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main()
{
	VideoCapture cap(0);

	Mat save_img;

	cap >> save_img;

	char Esc = 0;

	while (Esc != 27 && cap.isOpened()) {
		bool Frame = cap.read(save_img);
		if (!Frame || save_img.empty()) {
			cout << "error: frame not read from webcam\n";
			break;
		}
		namedWindow("save_img", CV_WINDOW_NORMAL);
		imshow("imgOriginal", save_img);
		Esc = waitKey(1);
	}
	imwrite("test.jpg", save_img);
}











