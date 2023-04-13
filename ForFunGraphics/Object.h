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

typedef vector<unsigned> Face;

class Object {
public:
    Object(string FilePath = "", D2D1::ColorF LineColor = D2D1::ColorF::White);
    void AddVertex(Coord V) { Vertices.push_back(V); };
    Face& AddFace() {
        Faces.push_back(Face());
        return Faces.back();
    };
    vector<Coord>& GetVertices() { return Vertices; };
    vector<Face>& GetFaces() { return Faces; };
    vector<Coord> GetVerticesForFace(Face& F) {
        vector<Coord> FaceVertices(F.size());
        for (unsigned Idx = 0; Idx < F.size(); Idx++)
            FaceVertices[Idx] = Vertices.at(F[Idx] - 1);
        return FaceVertices;
    };
    void SetColor(D2D1::ColorF C) { Color = C; }
    D2D1::ColorF GetColor() { return Color; }

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
    D2D1::ColorF Color = D2D1::ColorF::White;
};
