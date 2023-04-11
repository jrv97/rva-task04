#include <opencv2/opencv.hpp>
#include "rva.h"
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"

using namespace cv;
using namespace std;

// TASK 1
Mat rva_compute_homography(vector<Point2f> points_image1, vector<Point2f> points_image2)
{
    return findHomography(points_image1, points_image2, RANSAC);
}

void rva_draw_contour(Mat image, vector<Point2f> points, Scalar color, int thickness)
{
    line(image, points[0], points[1], color, thickness);
    line(image, points[1], points[2], color, thickness);
    line(image, points[2], points[3], color, thickness);
    line(image, points[3], points[0], color, thickness);
}

void rva_deform_image(const Mat &im_input, Mat &im_output, Mat homography)
{
    warpPerspective(im_input, im_output, homography, Size(im_output.cols - 1, im_output.rows - 1));
}

// TASK 2
void rva_calculaKPsDesc(const Mat &img, vector<KeyPoint> &keypoints, Mat &descriptors)
{
    auto detector = SIFT::create();
    detector->detectAndCompute(img, noArray(), keypoints, descriptors);
    Mat img_keypoints;
}

void rva_matchDesc(const Mat &descriptors1, const Mat &descriptors2, vector<DMatch> &matches)
{
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    vector<vector<DMatch>> knn_matches;
    matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);
    //-- Filter matches using the Lowe's ratio test
    const float ratio_thresh = 0.75f;
    for (size_t i = 0; i < knn_matches.size(); i++)
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
            matches.push_back(knn_matches[i][0]);
}

void rva_dibujaMatches(Mat &img1, Mat &img2, vector<KeyPoint> &keypoints1,
                       vector<KeyPoint> &keypoints2, vector<DMatch> &matches, Mat &img_matches)
{
    drawMatches(img1, keypoints1, img2, keypoints2, matches, img_matches);
}

// TASK 3
void rva_localizaObj(const Mat &img1, const Mat &img2, const vector<KeyPoint> &keypoints1,
                     const vector<KeyPoint> &keypoints2,
                     const vector<DMatch> &matches, Mat &homography, vector<Point2f> &pts_im2)
{
    vector<Point2f> obj;
    vector<Point2f> scene;

    for (size_t i = 0; i < matches.size(); i++)
    {
        obj.push_back(keypoints1[matches[i].queryIdx].pt);
        scene.push_back(keypoints2[matches[i].trainIdx].pt);
    }

    homography = rva_compute_homography(obj, scene);

    vector<Point2f> obj_corners(4);
    obj_corners[0] = Point2f(0, 0);
    obj_corners[1] = Point2f((float)img1.cols, 0);
    obj_corners[2] = Point2f((float)img1.cols, (float)img1.rows);
    obj_corners[3] = Point2f(0, (float)img1.rows);

    perspectiveTransform(obj_corners, pts_im2, homography);
}

// TASK 4
/*
 * Generates a binary mask that can be used to control the blending of the warped patch image with the scene image.
 * The mask has the same dimensions as the warped patch image and contains 255 in the regions where the warped patch
 * image should be blended with the scene image, and 0 in the regions where it should not
 */
Mat getMask(const Mat &warpedPatch)
{
    Mat mask, grayWarpedPatch;
    cvtColor(warpedPatch, grayWarpedPatch, COLOR_BGR2GRAY);
    threshold(grayWarpedPatch, mask, 0, 255, THRESH_BINARY);
    return mask;
}

void rva_dibujaPatch(const Mat &scene, const Mat &patch, const Mat &H, Mat &output)
{
    auto size = scene.size();
    Mat warpedPatch;
    warpPerspective(patch, warpedPatch, H, size);

    Mat mask = getMask(warpedPatch);

    scene.copyTo(output);
    warpedPatch.copyTo(output, mask);
}
