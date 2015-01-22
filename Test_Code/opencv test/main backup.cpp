#include "opencv2/opencv.hpp"
//#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <iostream>
#include <ctime>
#include <string>
#include <fstream>

using namespace cv;
using namespace std;

int main(int, char**)
{
//-----------------------------INITIALIZATIONS---------------

    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat edges; // delete after testing edge detection
    Mat frame;
    Mat img;
    vector<Mat> channels;


    // Initializing video output file
    VideoCapture inputVideo=cap;
    VideoWriter outputVideo;
    const string NAME ="tracking.avi";
    // TODO: get the right codec
    int ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC));
    Size S = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));


    // Open the output
    // TODO Get the right FPS
//    outputVideo.open(NAME, ex=-1, 20, S, true);
//    if (!outputVideo.isOpened())
//    {
//        std::cout  <<ex ;
//        return -1;
//    }

    // Opening result file
    std::ofstream storage;
    storage.open("data.txt");
    storage<< "Frame Width: "<< inputVideo.get(CV_CAP_PROP_FRAME_WIDTH) <<"\n";
    storage<< "Frame Height: "<< inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT) <<"\n";
    storage<< "------------------\n";
    storage<< "Timestamp   X     Y\n";

    clock_t start_time=clock();
    //tracbars
    int H_low=70;
    int S_low=240;
    int V_low=130;

    int H_high=100;
    int S_high=255;
    int V_high=244;
   namedWindow ("Detected",WINDOW_AUTOSIZE);
//    namedWindow ("frame thresholded",WINDOW_AUTOSIZE);
//    createTrackbar( "H_low", "frame thresholded", &H_low,255);
//    createTrackbar( "S_low", "frame thresholded", &S_low,255);
//    createTrackbar( "V_low", "frame thresholded", &V_low,255);
//
//    createTrackbar( "H_high", "frame thresholded", &H_high,255);
//    createTrackbar( "S_high", "frame thresholded", &S_high,255);
//    createTrackbar( "V_high", "frame thresholded", &V_high,255);
//------------------------------------------------------
    for(;;)
    {

        cap >> frame; // get a new frame from camera
        clock_t frame_time=clock();

        img=frame.clone();
        // equalization
//        cvtColor(frame, img, CV_RGB2YCrCb);
//        split(img, channels);
//        merge(channels, img);
//        cvtColor(img, img, CV_YCrCb2RGB);


        // HSV thresholding
        cvtColor(img, img, CV_RGB2HSV);
        //70 100 awesome
        //between (75,50,15) an (100, 200, 256)
        //inRange(img, Scalar( 80, 0, 150), Scalar(100, 200, 256), img);
        //inRange(img, Scalar( 70, 100, 20 ), Scalar(100, 256, 256), img);
        inRange(img, Scalar( H_low, S_low, V_low ), Scalar(H_high, S_high, V_high), img);
        Mat frameThresholded=img.clone();

//        // morphological opening (remove small objects from the foreground)
//        int size_ellipse= 1;
//        erode(img, img, getStructuringElement(MORPH_ELLIPSE, Size(size_ellipse, size_ellipse)));
//        dilate(img, img, getStructuringElement(MORPH_ELLIPSE, Size(size_ellipse, size_ellipse)));
//
//        //morphological closing (fill small holes in the foreground)
//        dilate(img, img, getStructuringElement(MORPH_ELLIPSE, Size(size_ellipse, size_ellipse)));
//        erode(img, img, getStructuringElement(MORPH_ELLIPSE, Size(size_ellipse, size_ellipse)));
        //Mat frameDilated=img.clone();


        // edge detection ( from example code)
//        cvtColor(frame, edges, CV_BGR2GRAY);
//        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
//        Canny(edges, edges, 0, 30, 3);


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


        if (largest_area>150) // minimal area
        {
            // Find the bounding rectangle for biggest contour
            minEnclosingCircle(contours[largest_contour_index], center, radius);
            circle(frameDetected, center, (int)radius, Scalar(256, 0, 2),4);
            Moments M = moments(contours[largest_contour_index],false);
            Point2f mc = Point2f(M.m10 / M.m00, M.m01 / M.m00);
            int r_eq = (int)(sqrt(largest_area/ 3.14) );
            circle(frameDetected, mc, 4,Scalar(0,0,0),6);
            circle(frameDetected, mc,r_eq,Scalar(0,0,256),4 );
         //   std::cout<<largest_area<<"\n";
        }
        else{

        }
        //clock_t frame_time=clock();
        clock_t processed_time=clock();
        imshow("Detected", frameDetected);
        cout<<"process time:  "<< processed_time- frame_time<<"\n";

//        imshow("frame dilated", frameDilated);
//        imshow("frame thresholded", frameThresholded);

//        storage<< frame_time-start_time<<"   "  <<center.y<< "   " <<center.x << "\n" ;

        //outputVideo << frameDetected ;

        // TODO proper cropping ( not relevant now)
//        Mat ROI;
//        int Lx = 123;
//        int Ly = 100;
//        ROI = source(Rect(mc.x-Lx, mc.y-Ly,2*Lx,2*Ly ));
//        imwrite("beacon_ROI.jpg", ROI);



        if(waitKey(1
                   ) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
























//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <iostream>
//
//using namespace cv;
//using namespace std;
//
//int main( )
//{
//
//       Mat image;
//       vector<Mat> channels;
//       VideoCapture cap(0);
//       if(!cap.isOpened())
//            return 10;
//
//
//       // LOAD image
//       image = imread("/home/pi/Test code/opencv test/image.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file "image.jpg".
//              //This file "image.jpg" should be in the project folder.
//              //Else provide full address : "D:/images/image.jpg"
//
//       if(! image.data )  // Check for invalid input
//       {
//              cout <<  "Could not open or find the image" << std::endl ;
//              return -1;
////       }
//       split(image,channels);
//
//       //DISPLAY image
//
//       imshow( "window", channels[1] ); // Show our image inside it.
//    namedWindow( "window", CV_WINDOW_AUTOSIZE ); // Create a window for display.
//
//       for(;;)
//
//       {
//        cap>>image;
//
//       imshow("window",image);
//       if(waitKey(1) >=0) break;                       // Wait for a keystroke in the window
//       }
//
//
//       SAVE image
//
//
//
//       imwrite("/home/pi/Test code/opencv test/result.jpg",image);// it will store the image in name "result.jpg"
//       return 0;
//}
