#include "opencv2/opencv.hpp"
#include <ctime>
#include <string>
#include <fstream>

#define TRACKBAR true
#define EQUALIZATION false
#define MORPHOLOGICAL false
#define SAVE_VIDEO false
#define SAVE_RESULTS true
#define SHOW_THRESHOLDED true
#define SHOW_DILATED false
#define SHOW_DETECTED true


using namespace cv;
using namespace std;

int main(int, char**)
{
//-----------------------------INITIALIZATIONS---------------
    system("sudo modprobe bcm2835-v4l2");

    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;
    VideoCapture inputVideo=cap;

    Mat frame;
    Mat img;
    vector<Mat> channels;


#if SAVE_VIDEO
    // Initializing video output file
    VideoWriter outputVideo;
    const string NAME ="tracking.avi";
    // TODO: get the right codec
    int ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC));
    Size S = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));

    // Open the output
    // TODO Get the right FPS
    outputVideo.open(NAME, ex=-1, 20, S, true);
    if (!outputVideo.isOpened())
    {
        std::cout  <<ex ;
        return -1;
    }
#endif

#if SAVE_RESULTS
    // Opening result file
    std::ofstream storage;
    storage.open("data.txt");
    storage<< "Frame Width: "<< inputVideo.get(CV_CAP_PROP_FRAME_WIDTH) <<"\n";
    storage<< "Frame Height: "<< inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT) <<"\n";
    storage<< "------------------\n";
    storage<< "Timestamp   X     Y\n";
#endif
    clock_t start_time=clock();



    // trackbars for determining treshold

    //yellow cap
//    int H_low=70;
//    int S_low=240;
//    int V_low=130;
//
//    int H_high=100;
//    int S_high=255;
//    int V_high=244;

// red light
    int H_low=110;
    int S_low=110;
    int V_low=145;

    int H_high=145;
    int S_high=255;
    int V_high=255;

#ifdef SHOW_THRESHOLDED
    namedWindow( "frame thresholded", WINDOW_AUTOSIZE );
#ifdef TRACKBAR
    createTrackbar( "H_low", "frame thresholded", &H_low, 255 );
    createTrackbar( "S_low", "frame thresholded", &S_low, 255 );
    createTrackbar( "V_low", "frame thresholded", &V_low, 255 );

    createTrackbar(" H_high", "frame thresholded", &H_high, 255 );
    createTrackbar( "S_high", "frame thresholded", &S_high, 255 );
    createTrackbar( "V_high", "frame thresholded", &V_high, 255 );
#endif
#endif
//------------------------------------------------------
    for(;;)
    {

        cap >> frame; // get a new frame from camera

        clock_t frame_time=clock();
        img=frame.clone();

#if EQUALIZATION
        // equalization
        cvtColor(frame, img, CV_RGB2YCrCb);
        split(img, channels);
        merge(channels, img);
        cvtColor(img, img, CV_YCrCb2RGB);
#endif


        // HSV thresholding
        cvtColor(img, img, CV_RGB2HSV);
        //70 100 awesome
        //between (75,50,15) an (100, 200, 256)
        //inRange(img, Scalar( 80, 0, 150), Scalar(100, 200, 256), img);
        //inRange(img, Scalar( 70, 100, 20 ), Scalar(100, 255, 255), img);
        inRange(img, Scalar( H_low, S_low, V_low ), Scalar(H_high, S_high, V_high), img);
        Mat frameThresholded=img.clone();

#if MORPHOLOGICAL
        // morphological opening (remove small objects from the foreground)
        int size_ellipse= 3;
        erode(img, img, getStructuringElement(MORPH_ELLIPSE, Size(size_ellipse, size_ellipse)));
        dilate(img, img, getStructuringElement(MORPH_ELLIPSE, Size(size_ellipse, size_ellipse)));

        //morphological closing (fill small holes in the foreground)
        dilate(img, img, getStructuringElement(MORPH_ELLIPSE, Size(size_ellipse, size_ellipse)));
        erode(img, img, getStructuringElement(MORPH_ELLIPSE, Size(size_ellipse, size_ellipse)));

        Mat frameDilated=img.clone();
#endif


        // Contour analysis
        int largest_area = 0;
        int largest_contour_index = 0;
        Point2f center;
        float radius;
        vector<vector<Point> > contours; // Vector for storing contour
        vector<Vec4i> hierarchy;
        findContours(img, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
        for (int i = 0; i< contours.size(); i++)
        {
            //  Find the area of contour
            double a = contourArea(contours[i], false);
            if (a>largest_area)
            {
                largest_area = a;
                // Store the index of largest contour
                largest_contour_index = i;
            }
        }
        Mat frameDetected=frame.clone();

        //drawing the circles and timestamps

//TODO update the compiler so you can use to_string and timestamp
//        putText(frameDetected,"aaa", cvPoint(30,30),
//        FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);

        Point2f mc;
        if (largest_area>400) // minimal area
        {
            // Find the bounding rectangle for biggest contour
            minEnclosingCircle(contours[largest_contour_index], center, radius);
            Moments M = moments(contours[largest_contour_index],false);
            Point2f mc = Point2f(M.m10 / M.m00, M.m01 / M.m00);
            #if SHOW_DETECTED
            int r_eq = (int)(sqrt(largest_area/ 3.14) );
            circle(frameDetected, center, (int)radius, Scalar(256, 0, 2),4);
            circle(frameDetected, mc, 4,Scalar(0,0,0),6);
            circle(frameDetected, mc,r_eq,Scalar(0,0,256),4 );
            #endif
            std::cout<<largest_area<<"    x:"<< mc.x<< "    y:" <<mc.y<<"\n";
        }
        else
        {
            mc= Point2f(0,0);
        }

#if SHOW_DETECTED
        imshow("Detected", frameDetected);
#endif

#if SHOW_DILATED && MORPHOLOGICAL
        imshow("frame dilated", frameDilated);
#endif

#if SHOW_THRESHOLDED
        imshow("frame thresholded", frameThresholded);
#endif

#if SAVE_RESULTS
        storage<< frame_time-start_time<<"   "  <<mc.y<< "   " <<mc.x << "\n" ;
#endif

#if SAVE_VIDEO
        outputVideo << frameDetected ;
#endif


        // TODO proper cropping
//        Mat ROI;
//        int Lx = 123;
//        int Ly = 100;
//        ROI = source(Rect(mc.x-Lx, mc.y-Ly,2*Lx,2*Ly ));
//        imwrite("beacon_ROI.jpg", ROI);

//------------------------------------------------------------------------------------------------------------------------------


        if(waitKey(1) >= 0) break;

    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
