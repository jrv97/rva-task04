// This is the header file for the RVA functions
// (c) Realidad Virtual y Aumentada - Universidad de Cordoba - Manuel J. Marin-Jimenez

#ifndef _RVA_H
#define _RVA_H

// Include some OpenCV headers
#include <opencv2/opencv.hpp>

cv::Mat rva_compute_homography(std::vector<cv::Point2f> points_image1, std::vector<cv::Point2f> points_image2);

void rva_draw_contour(cv::Mat image, std::vector<cv::Point2f> points, cv::Scalar color, int thickness);

void rva_deform_image(const cv::Mat &im_input, cv::Mat &im_output, cv::Mat homography);

// ======================
// Functions for task 2
// ======================

//! This function extract keypoint descriptors from an image
//! \param img: input image
//! \param keypoints: vector of keypoints
//! \param descriptors: matrix of descriptors
void rva_calculaKPsDesc(const cv::Mat &img, std::vector<cv::KeyPoint> &keypoints, cv::Mat &descriptors);

//! This function matches two sets of descriptors
//! \param descriptors1: matrix of descriptors of the first image
//! \param descriptors2: matrix of descriptors of the second image
//! \param matches: vector of matches

void rva_matchDesc(const cv::Mat &descriptors1, const cv::Mat &descriptors2, std::vector<cv::DMatch> &matches);

//! This function draws the matches between two images
//! \param img1: first image
//! \param img2: second image
//! \param keypoints1: vector of keypoints of the first image
//! \param keypoints2: vector of keypoints of the second image
//! \param matches: vector of matches
//! \param img_matches: output image with the matches

void rva_dibujaMatches(cv::Mat &img1, cv::Mat &img2, std::vector<cv::KeyPoint> &keypoints1, std::vector<cv::KeyPoint> &keypoints2, std::vector<cv::DMatch> &matches, cv::Mat &img_matches);

// ======================
// Functions for task 3
// ======================

//! This function computes the homography between two images and finds the points in the second image that are inside the contour of the first image
//! \param img1: obj image
//! \param img2: scene image
//! \param keypoints1: vector of keypoints of the obj image
//! \param keypoints2: vector of keypoints of the scene image
//! \param matches: vector of matches
//! \param homography: output homography
//! \param pts_im2: output vector of points in the scene image that are inside the contour of the obj image

void rva_localizaObj(const cv::Mat &img1, const cv::Mat &img2, const std::vector<cv::KeyPoint> &keypoints1, const std::vector<cv::KeyPoint> &keypoints2, const std::vector<cv::DMatch> &matches, cv::Mat &homography, std::vector<cv::Point2f> &pts_im2);

// =========================
// Functions for task 4
// =========================

//! This function draws the patch on the scene image
//! \param scene: scene image
//! \param patch: patch image
//! \param homography: homography
//! \param output: output image
//! \pre homography must be computed with rva_compute_homography
//! \pre patch must have the same size as the obj image (used to compute the homography)

void rva_dibujaPatch(const cv::Mat &scene, const cv::Mat &patch, const cv::Mat &homography, cv::Mat &output);

#endif