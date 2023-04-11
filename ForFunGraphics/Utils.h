#pragma once
#include <d2d1.h>
#include <queue>

typedef struct {
    float X, Y, Z, W;
} Coord;

constexpr auto PI = 3.15159265359f;
constexpr auto VK_LETTER_BASE = 0x40;
typedef unsigned long long TIME_MS_T;
typedef float Mat4x4[4][4];
typedef float Vec4[4];

using std::queue;

// Simple class to compute a running average, up to N items.
template <class T> class RunningAverage {
public:
    RunningAverage(unsigned MaxItems) : MaxItems(MaxItems){};
    void addData(T Data) {
        if (Items.size() == MaxItems) {
            Sum -= Items.front();
            Items.pop();
        }
        Sum += Data;
        Items.push(Data);
    };
    T getAverage() { return round(((double)Sum) / ((double)Items.size())); };

private:
    T        Sum = 0;
    unsigned MaxItems;
    queue<T> Items;
};

// Helper function to convert betwen 2D coordinate spaces
D2D1_POINT_2F Convert2DCoords(D2D1_POINT_2F Orig,
                              D2D1_POINT_2F OrigMin,
                              D2D1_POINT_2F OrigMax,
                              D2D1_POINT_2F NewMin,
                              D2D1_POINT_2F NewMax);

// Transform In coordinates to Out using given transformation matrix
void ProjectCoord(Mat4x4& Mat, Coord& In, Coord& Out);

// Generate a rotation matrix from angles in Rads.
void PopulateRotationMatrix(Mat4x4& Mat, float Alpha, float Beta, float Gamma);