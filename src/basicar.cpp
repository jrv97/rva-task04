#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include "rva.h"

using namespace cv;
using namespace std;

const String keys =
    "{help h usage ? |      | print this message   }"
    "{@model         |<none>| Path to image model.}"
    "{@video         |<none>| Path to video scene.}"
    "{patch          |<none>| Path to image patch.}"
    "{video2         |<none>| Path to a second video.}"
    "{index-cam      |<none>| Webcam index to use.}";

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
    string webcam_path = parser.get<string>("index-cam");

    // Count for ScreenShots Name
    int screenshots_cnt = 0;

    // Video2 has priority over patch
    bool use_video2 = !video2_path.empty();
    bool use_webcam = !webcam_path.empty();
    bool use_patch = !patch_path.empty() && !(use_video2 || use_webcam);

    if (!use_video2 && !use_webcam && !use_patch)
    {
        cout << "Error: no patch or video2" << endl;
        return -1;
    }

    // Load the images in color
    Mat img_model = imread(model_path, IMREAD_COLOR);

    Mat img_patch;
    if (use_patch)
        img_patch = imread(patch_path, IMREAD_COLOR);

    // Check if the images are loaded
    if (img_model.empty())
    {
        cerr << "Error: failed to load model image from file: " << model_path;
        return -1;
    }
    if (use_patch && img_patch.empty())
    {
        cerr << "Error: failed to load patch image from file: " << patch_path;
        return -1;
    }

    // Change resolution of the image model to half
    cv::resize(img_model, img_model, Size(), 0.5, 0.5);

    // cv::resize the patch to the size of the model
    if (use_patch)
        cv::resize(img_patch, img_patch, img_model.size());

    // Create the video capture
    VideoCapture cap(video_path);
    if (!cap.isOpened())
    {
        cerr << "Error: video not loaded: " + video_path << endl;
        return -1;
    }

    // If you use video2, create the video capture
    VideoCapture video2_source;
    if (use_video2)
    {
        video2_source.open(video2_path);
        if (!video2_source.isOpened())
        {
            cerr << "Error: Failed to open video file: " << video2_path
                 << ". Please check that the file exists and you have read privileges to open it." << endl;
            return -1;
        }
    }
    else if (use_webcam)
    {
        int webcam_idx = stoi(webcam_path);
        video2_source.open(webcam_idx);

        if (!video2_source.isOpened())
        {
            cerr << "Failed to open the camera " << webcam_path
                 << ". Please check that the webcam is properly connected." << endl;
            return -1;
        }
    }

    // Pre-compute keypoints and descriptors for the model image
    vector<KeyPoint> keypoints_model;
    Mat descriptors_model;
    rva_calculaKPsDesc(img_model, keypoints_model, descriptors_model);

    // For each video frame, detect the object and overlay the patch
    Mat img_scene;
    vector<Mat> frames;
    Mat patch;
    if (!img_patch.empty())
    {
        patch = img_patch;
    }
    while (cap.read(img_scene))
    {
        // To speed up processing, cv::resize the image to half
        cv::resize(img_scene, img_scene, img_model.size(), 0.5, 0.5);

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

        // When using a video2_source, read the frame and resize it to the size of the patch
        if (use_video2 || use_webcam)
        {
            // Read the next frame from the input stream and cv::resize it so that it fits in our model
            video2_source >> patch;
            cv::resize(patch, patch, img_scene.size());
        }
        if (!patch.empty())
        {
            // Warp the patch to the object using OpenCV
            rva_dibujaPatch(img_scene, patch, H, img_scene);
        }
        // Draw the bounding-box
        rva_draw_contour(img_scene, pts_obj_in_scene, Scalar(0, 255, 0), 4);

        // Show the result
        cv::imshow("AugmentedReality", img_scene);

        // Save each frame wot collect them all, we will write them later to an output video using a VideoWriter object
        Mat frame = img_scene.clone();
        // Save them with full/original scale
        cv::resize(frame, frame, img_model.size(), 1, 1);
        frames.push_back(frame);

        // Check pressed keys to take action
        // Check for user input
        int onKeyPress = waitKey(1);
        // Exit the program if the user presses the 'q' or 'Esc' key
        if (onKeyPress == 27 || onKeyPress == 'q')
        {
            cout << "Execution terminated. Exiting..." << endl;
            break;
        }
        // Take a screenshot of the current scene if the user presses the 's' key
        if (onKeyPress == 's')
        {
            string filename = "../data/screenshots/screenshot_" + to_string(++screenshots_cnt) + ".jpg";
            imwrite(filename, frame);
            cout << "Screenshot saved as " << filename << endl;
        }
    }

    const string codec = "MJPG";                    // Or "mp4v" for .mp4 files
    const string outputFile = "../data/output.avi"; // Or "output.mp4" for .mp4 files
    const int frameRate = 30;
    VideoWriter videoWriter;
    videoWriter.open(outputFile, VideoWriter::fourcc(codec[0], codec[1], codec[2], codec[3]), frameRate, frames[0].size(), true);
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

    // The camera will be disposed automatically in VideoCapture destructor
    return 0;
}
