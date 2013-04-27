#ifndef TRACKER_H
#define TRACKER_H
#include <vector>
#include <iostream>
#include <string>
#include <map>

#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/nonfree/gpu.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <boost/lexical_cast.hpp>

#include "hungarian/hungarian.h"

using namespace std;
typedef vector<cv::Point> Contour;
typedef vector<Contour> ContourVector;


struct FlyState
{
    FlyState() : ID(-1) {}
    FlyState(int _ID, cv::Point2f _obs_pos, cv::RotatedRect _ellipse)
        : ID(_ID), pos(_obs_pos), ellipse( _ellipse) {}
    virtual ~FlyState() {}

    // Fly ID
    int ID;
    // Observed position
    cv::Point2f pos;
    // Observed ellipse
    cv::RotatedRect ellipse;
};

struct TrackerState
{
    TrackerState() {}
    TrackerState(int _framenum, int _numobs)
        : framenum(_framenum), numobs(_numobs) {}
    virtual ~TrackerState() {}

    // frame number of state
    int framenum;
    // number of observed objects in state
    int numobs;
    // vector of current observations
    vector<FlyState> obs;
};

class Background
{
public:
    Background() : nframes_(0) {}
    virtual ~Background() {}
    void init(cv::Mat& frame) { frame.convertTo(background_,CV_32FC1); }
    void addFrame(cv::Mat& frame);
    void getBackground(cv::Mat& mat);

private:
    float nframes_;
    cv::Mat background_;
};

//
// Tracker
// Step 1: Add all frames to be analyzed
// Step 2: Run trainClassifier to train classifier on all fg/bg segmented images
// Step 3:
// Step 3: Run compute
class Tracker
{
public:
    Tracker(bool useVIBE = true);
    virtual ~Tracker() {}

    // Compute background image
    void setBackground(cv::Mat& bg) { background_ = bg; }

    // Add another frame to the tracker
    void addFrame(cv::Mat& currimg);

    // Draw the locations of objects in the current frame of the tracker
    void drawCurrent(cv::Mat& img);

    // Draw state on frame
    void drawFrame(const TrackerState& state, cv::Mat& img);


    // Compute trajectories for each object, starting from the first frame
    void computeTrajectories();

    // Accessor methods dealing with current frame
    vector<cv::Point2f> getCurrentPositions();
    vector<cv::RotatedRect> getCurrentEllipses();
    int getCurrentNumObjects() { return currstate_.numobs; }
    int getTotalNumObjects() { return nobjects_; }
    TrackerState getCurrentState() { return currstate_; }
    TrackerState getState(int i) { return history_[i]; }

    void assignIdentities();
    // DON'T USE -- SLOW
    //void computeMedianBackgroundModel();

private:
    void findCentroids(const ContourVector& contours, vector<cv::Point2f> &currpos);
    void findEllipses(const ContourVector& contours, vector<cv::RotatedRect> &currellipse);
    void findFeatures(const cv::Mat &currimg, const cv::Mat& fmask);
    void filterContours(ContourVector& contours);

    void updateState(const vector<cv::Point2f>& currpos, const vector<cv::RotatedRect>& currellipse);

    // Try to assign current flies to flies in previous frame

    // Try to predict positions and anglesfor each fly, given previous positions
    void predictFlyState();

private:
    // use vibe background subtractor
    bool useVIBE_;

    // current frame
    int nframes_;
    // total number of objects being tracked
    int nobjects_;

    // background subtractors
    cv::BackgroundSubtractorMOG2 mog_subtractor_;
    cv::gpu::VIBE_GPU vibe_subtractor_;

    // Current state
    TrackerState currstate_;

    // Past states
    vector<TrackerState> history_;

    // predicted position of each fly in next frame
    vector<FlyState> prediction_;

    cv::Mat background_;

    // Size threshold for object rejection
    double sizeThreshold_;
};



#endif // TRACKER_H
