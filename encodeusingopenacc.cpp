#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// Define block size
const int block_size = 4;

// Function to divide the image into blocks
vector<Mat> divideImageIntoBlocks(const Mat& image) {
    vector<Mat> blocks;
    int rows = image.rows;
    int cols = image.cols;
    for (int y = 0; y < rows; y += block_size) {
        for (int x = 0; x < cols; x += block_size) {
            Mat block = image(Rect(x, y, block_size, block_size));
            blocks.push_back(block.clone());
        }
    }
    return blocks;
}

// Function to rotate a 4x4 block
void rotateBlock(const Mat& src, Mat& dst, int angle) {
    Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
    Mat rot = getRotationMatrix2D(center, angle, 1.0);
    warpAffine(src, dst, rot, src.size());
}

// Function to find the closest match for each block
vector<pair<Point, int>> findClosestMatch(const vector<Mat>& blocks, const vector<Mat>& compressedBlocks) {
    vector<pair<Point, int>> closestMatches(blocks.size());

    #pragma acc parallel loop present(blocks, compressedBlocks, closestMatches)
    for (int i = 0; i < blocks.size(); ++i) {
        int bestMatch = -1;
        int bestOrientation = -1;
        double minDifference = numeric_limits<double>::max();
        int compressedCols = 64 / 2; // Number of columns in compressed image
        for (int j = 0; j < compressedBlocks.size(); ++j) {
            for (int angle = 0; angle < 360; angle += 45) {
                Mat rotatedBlock;
                int angleIndex = angle / 45;
                rotateBlock(compressedBlocks[j], rotatedBlock, angle);
                double difference = norm(blocks[i], rotatedBlock, NORM_L1);
                if (difference < minDifference) {
                    minDifference = difference;
                    bestMatch = j;
                    bestOrientation = angleIndex;
                }
            }
        }
        int x = (bestMatch % compressedCols) * block_size;
        int y = (bestMatch / compressedCols) * block_size;
        if (x >= 64) x = 64 - block_size; // Ensure x is within 0 to 64 range
        if (y >= 64) y = 64 - block_size; // Ensure y is within 0 to 64 range
        closestMatches[i] = make_pair(Point(x, y), bestOrientation);
    }
    return closestMatches;
}

int main() {
    // Load the original image and resize to 128x128
    Mat originalImage = imread("input.jpeg");
    resize(originalImage, originalImage, Size(128, 128));

    imwrite("resized_original_image.jpg", originalImage);

    // Load the compressed image and resize to 64x64
    Mat compressedImage;
    resize(originalImage, compressedImage, Size(64, 64));


    // Divide the images into blocks
    vector<Mat> originalBlocks = divideImageIntoBlocks(originalImage);
    vector<Mat> compressedBlocks = divideImageIntoBlocks(compressedImage);

    // Find closest matches
    vector<pair<Point, int>> closestMatches = findClosestMatch(originalBlocks, compressedBlocks);

    // Store the information in a text file
    ofstream outFile("ffractal.txt");
    if (!outFile.is_open()) {
        cerr << "Error: Unable to open file." << endl;
        return -1;
    }

    for (int i = 0; i < closestMatches.size(); ++i) {
        int x = closestMatches[i].first.x;
        int y = closestMatches[i].first.y;
        int orientation = closestMatches[i].second;
        outFile << x << " " << y << " " << orientation << endl;
    }

    outFile.close();
    return 0;
}


