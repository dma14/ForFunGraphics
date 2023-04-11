#pragma once

#include <d2d1.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Utils.h"

using std::vector;
using std::string;

typedef struct {
    unsigned VId[3];
} Face;

class Object {
public:
    Object(string FilePath = "");
    void AddVertex(Coord V) { Vertices.push_back(V); };
    void AddFace(unsigned VId1, unsigned VId2, unsigned VId3) {
        Faces.push_back({ VId1, VId2, VId3 });
    };
    vector<Coord>& GetVertices() { return Vertices; };
    vector<Face>& GetFaces() { return Faces; };
    vector<Coord> GetVerticesForFace(Face& F) {
        vector<Coord> FaceVertices;
        for (unsigned Idx = 0; Idx < 3; Idx++)
            FaceVertices.push_back(Vertices.at(F.VId[Idx] - 1));
        return FaceVertices;
    };

    // Rotation applied before displacement, so user needs to manaully add the Displacement
    // vector, while rotation is applied automatically.
    void SetDisplacement(Coord Vec) { Displacement = Vec; };
    Coord GetDisplacement() { return Displacement; };
    void Rotate(Coord Angles) {
        PopulateRotationMatrix(RotationMat, Angles.X, Angles.Y, Angles.Z);
        for (auto& Vertex : Vertices) {
            Coord Rotated;
            ProjectCoord(RotationMat, Vertex, Rotated);
            Vertex.X = Rotated.X;
            Vertex.Y = Rotated.Y;
            Vertex.Z = Rotated.Z;
        }
    };
    void AddTimeRotation(Coord AngleVelocities) {
        RotationSpeeds = AngleVelocities;
        MovesInTime = true;
    };
    void UpdateInTime(TIME_MS_T TimeElapsed) {
        if (!MovesInTime)
            return;

        Coord Angles = { RotationSpeeds.X * TimeElapsed,
                         RotationSpeeds.Y * TimeElapsed,
                         RotationSpeeds.Z * TimeElapsed,
                         1.f };
        PopulateRotationMatrix(RotationMat, Angles.X, Angles.Y, Angles.Z);
        for (auto& Vertex : Vertices) {
            Coord Rotated;
            ProjectCoord(RotationMat, Vertex, Rotated);
            Vertex.X = Rotated.X;
            Vertex.Y = Rotated.Y;
            Vertex.Z = Rotated.Z;
        }
    };


private:
    vector<Coord> Vertices;
    vector<Face> Faces;
    Coord Displacement = { 0. };
    Coord RotationSpeeds = { 0. };
    Mat4x4 RotationMat = { 0. };
    bool MovesInTime = false;
};
