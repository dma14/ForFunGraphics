#include "Utils.h"
#include "Object.h"
D2D1_POINT_2F Convert2DCoords(D2D1_POINT_2F Orig,
                              D2D1_POINT_2F OrigMin,
                              D2D1_POINT_2F OrigMax,
                              D2D1_POINT_2F NewMin,
                              D2D1_POINT_2F NewMax) {
    D2D1_POINT_2F OrigCentre = { (OrigMax.x + OrigMin.x) / 2.0f,
                                 (OrigMax.y + OrigMin.y) / 2.0f };
    D2D1_POINT_2F NewCentre = { (NewMax.x + NewMin.x) / 2.0f, (NewMax.y + NewMin.y) / 2.0f };
    float NewX =
        (Orig.x - OrigCentre.x) / (OrigMax.x - OrigMin.x) * (NewMax.x - NewMin.x) + NewCentre.x;
    float NewY =
        (Orig.y - OrigCentre.y) / (OrigMax.y - OrigMin.y) * (NewMax.y - NewMin.y) + NewCentre.y;
    return { NewX, NewY };
};

void ProjectCoord(Mat4x4& Mat, Coord& In, Coord& Out) {
    Out.X = In.X * Mat[0][0] + In.Y * Mat[1][0] + In.Z * Mat[2][0] + In.W * Mat[3][0];
    Out.Y = In.X * Mat[0][1] + In.Y * Mat[1][1] + In.Z * Mat[2][1] + In.W * Mat[3][1];
    Out.Z = In.X * Mat[0][2] + In.Y * Mat[1][2] + In.Z * Mat[2][2] + In.W * Mat[3][2];
    Out.W = In.X * Mat[0][3] + In.Y * Mat[1][3] + In.Z * Mat[2][3] + In.W * Mat[3][3];
};


void PopulateRotationMatrix(Mat4x4& Mat, float Alpha, float Beta, float Gamma) {
    Mat[0][0] = cos(Beta) * cos(Gamma);
    Mat[0][1] = sin(Alpha) * sin(Beta) * cos(Gamma) - cos(Alpha) * sin(Gamma);
    Mat[0][2] = cos(Alpha) * sin(Beta) * cos(Gamma) + sin(Alpha) * sin(Gamma);
    Mat[1][0] = cos(Beta) * sin(Gamma);
    Mat[1][1] = sin(Alpha) * sin(Beta) * sin(Gamma) + cos(Alpha) * cos(Gamma);
    Mat[1][2] = cos(Alpha) * sin(Beta) * sin(Gamma) - sin(Alpha) * cos(Gamma);
    Mat[2][0] = -1.f * sin(Beta);
    Mat[2][1] = sin(Alpha) * cos(Beta);
    Mat[2][2] = cos(Alpha) * cos(Beta);
    Mat[3][3] = 1.0f;
};