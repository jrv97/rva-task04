#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include "rva.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace cv;
using namespace std;

const String keys =
    "{help h usage ? |      | print this message   }"
    "{@model         |<none>| Path to image model.}"
    "{@video         |<none>| Path to video scene.}"
    "{patch         |<none>| Path to image patch.}"
    "{video2         |<none>| Path to a second video.}";

// Main function
int main(int argc, char **argv)
{
    // Get the arguments: model, video and patch using OpenCv parser
    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    string model_path = parser.get<string>(0);
    string video_path = parser.get<string>(1);

    // Patch argument is available?
    string patch_path = parser.get<string>("patch");

    // Second video argument is available?
    string video2_path = parser.get<string>("video2");

    // Count for ScreenShots Name
    int screenshots_cnt = 0;

    // Video2 has priority over patch
    bool use_video2 = !video2_path.empty();
    bool use_patch = !patch_path.empty() && !use_video2;

    if (!use_video2 && !use_patch)
    {
        cout << "Error: no patch or video2" << endl;
        return -1;
    }

    // Load the images in color
    Mat img_model = imread(model_path, IMREAD_COLOR);

    Mat img_patch;
    if (use_patch)
        img_patch = imread(patch_path, IMREAD_COLOR);

    // Check if the images are loaded (TODO ok)
    try
    {
        if (img_model.empty())
        {
            throw runtime_error("Error: failed to load model image from file: " + model_path);
        }
        if (use_patch && img_patch.empty())
        {
            throw runtime_error("Error: failed to load patch image from file: " + patch_path);
        }
    }
    catch (const exception &ex)
    {
        cout << "Error: " << ex.what() << endl;
        return -1;
    }

    // Change resolution of the image model to half
    resize(img_model, img_model, Size(), 0.5, 0.5);

    // Resize the patch to the size of the model
    if (use_patch)
        resize(img_patch, img_patch, img_model.size());

    // Create the video capture
    VideoCapture cap(video_path);
    if (!cap.isOpened())
    {
        cout << "Error: video not loaded: " + video_path << endl;
        return -1;
    }

    // If you use video2, create the video capture (TODO ok)
    VideoCapture cap2;
    if (use_video2)
    {
        cap2.open(video2_path);
        // Check if the input was successfully opened
        if (!cap2.isOpened())
        {
            // If the input is a webcam or a video file, show an appropriate error message
            if (isdigit(video2_path[0]))
            {
                cout << "Error: Failed to open webcam with index " << video2_path
                     << ". Please check that the webcam is connected and try again." << endl;
            }
            else
            {
                cout << "Error: Failed to open video file: " << video2_path
                     << ". Please check that the file exists and try again." << endl;
            }
            return -1;
        }
    }

    // Pre-compute keypoints and descriptors for the model image
    vector<KeyPoint> keypoints_model;
    Mat descriptors_model;
    rva_calculaKPsDesc(img_model, keypoints_model, descriptors_model);

    // For each video frame, detect the object and overlay the patch
    Mat img_scene;
    Size frameSize;
    vector<Mat> frames;

    while (cap.read(img_scene))
    {
        // To speed up processing, resize the image to half
        resize(img_scene, img_scene, img_model.size(), 0.5, 0.5);

        // Compute keypoints and descriptors for the scene image
        vector<KeyPoint> keypoints_scene;
        Mat descriptors_scene;
        rva_calculaKPsDesc(img_scene, keypoints_scene, descriptors_scene);

        // Match the descriptors
        vector<DMatch> matches;
        rva_matchDesc(descriptors_model, descriptors_scene, matches);

        // Compute the bounding-box of the model in the scene
        Mat H;
        vector<Point2f> pts_obj_in_scene;
        rva_localizaObj(img_model, img_scene, keypoints_model, keypoints_scene, matches, H, pts_obj_in_scene);

        Mat patch;
        // If use_video2, read the frame and resize it to the size of the patch (TODO ok)
        if (use_video2)
        {
            // Read the next frame from the input stream and resize it so that it fits in our model
            cap2 >> patch;
            resize(patch, patch, img_scene.size());
        }
        // Otherwise, use img_patch as patch
        else if (!img_patch.empty())
        {
            patch = img_patch;
        }
        if (!patch.empty())
        {
            // Warp the patch to the object using OpenCV
            rva_dibujaPatch(img_scene, patch, H, img_scene);
        }
        // Draw the bounding-box
        rva_draw_contour(img_scene, pts_obj_in_scene, Scalar(0, 255, 0), 4);

        // Show the result
        imshow("AugmentedReality", img_scene);

        // Save each frame wot collect them all, we will write them later to an output video using a VideoWriter object
        Mat frame = img_scene.clone();
        // Save them with full/original scale
        resize(frame, frame, img_model.size(), 1, 1);
        frames.push_back(frame);
        // Set the frame size if it hasn't been set yet
        if (frameSize.empty())
        {
            frameSize = frame.size();
        }

        // Check pressed keys to take action (TODO ok)
        // Check for user input
        int key = waitKey(1);
        // Exit the program if the user presses the 'q' or 'Esc' key
        if (key == 27 || key == 'q')
        {
            cout << "Exiting program." << endl;
            break;
        }
        // Take a screenshot of the current scene if the user presses the 's' key
        if (key == 's')
        {
            auto now = chrono::system_clock::now();
            auto now_c = chrono::system_clock::to_time_t(now);
            auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

            ostringstream oss;
            oss << put_time(localtime(&now_c), "%Y-%m-%d_%H-%M-%S") << "_" << setfill('0') << setw(3) << ms.count();
            string filename = "../data/screenshots/screenshot_" + oss.str() + ".jpg";
            imwrite(filename, img_scene);
            cout << "Screenshot saved as " << filename << endl;
        }
    }

    const string codec = "MJPG";                    // Or "mp4v" for .mp4 files
    const string outputFile = "../data/output.avi"; // Or "output.mp4" for .mp4 files
    const int frameRate = 30;
    VideoWriter videoWriter;
    videoWriter.open(outputFile, VideoWriter::fourcc(codec[0], codec[1], codec[2], codec[3]), frameRate, frameSize, true);
    if (!videoWriter.isOpened())
    {
        cerr << "Could not open the output video file for writing" << endl;
        return -1;
    } // Write all frames to the video file
    for (const auto &frame : frames)
    {
        videoWriter.write(frame);
    }
    cout << "Video saved to " << outputFile << endl; // Release the video writer to close the output video file
    videoWriter.release();

    // The camera will be de-initialized automatically in VideoCapture destructor
    return 0;
}
