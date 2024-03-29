#pragma once

#include <d2d1.h>
#include <vector>
#include <string>
#include <cmath>
#include <tuple>
#include "Utils.h"
#include "Object.h"

using std::vector;
using std::string;
using std::tuple;
using std::get;

class Scene {
public:
    Scene() {
        SetPoVLocation({ 0., 0., 10., 1. });
        SetPoVAngles({ 0., 0., 0. });
        SetFoV(90.);

        // TODO: performance for this huge obj is so bad lol
        //Object* Cooper = AddObject("Cooper.obj");
        Object* Skyscraper = AddObject("Skyscraper.obj");
        Skyscraper->SetDisplacement({ 0.f, 0.f, -100.f, 0.f });
        Skyscraper->SetColor(D2D1::ColorF::AntiqueWhite);
        Object* Teapot = AddObject("Teapot.obj");
        Teapot->SetDisplacement({ 0.f, 0.f, -10.f, 0.f });
        Teapot->SetColor(D2D1::ColorF::Violet);
        Teapot->AddTimeRotation({ 0.f, -0.0000005f, 0.f, 0.f });
        Object* Top = AddObject("Cube.obj");
        Top->SetDisplacement({ 0.f, 42.f, -100.f, 0.f });
        Top->SetColor(D2D1::ColorF::LawnGreen);
        Top->AddTimeRotation({ 0.f, 0.000001f, 0.000001f, 0.f });

        for (int Row = 0; Row < 2; Row++) {
            for (int Col = 0; Col < 40; Col++) {
                Object* Cube = AddObject("Cube.obj");
                Cube->SetDisplacement(
                    { (float)((Row * 2 - 1) * 20), -5.f, (float)-2 * Col, 0.f });
                Cube->SetColor(D2D1::ColorF::LawnGreen);
                Cube->AddTimeRotation({0.0000005f, 0.000001f, 0.f, 0.f});
            }
        }
    }
    void Render(ID2D1HwndRenderTarget* RenderTarget, ID2D1SolidColorBrush* Brush);
    Object* AddObject(string FilePath) {
        Object Obj(FilePath);
        Objects.push_back(Obj);
        return &Objects[Objects.size()-1];
    };

    void StepPoV(Coord Step) {
        Coord RotatedStep;
        ProjectCoord(RotateMat, Step, RotatedStep);
        PoVLoc.X += RotatedStep.X;
        PoVLoc.Y += RotatedStep.Y;
        PoVLoc.Z += RotatedStep.Z;
        PoVLoc.W += RotatedStep.W;
    };
    void SetPoVLocation(Coord Location) { PoVLoc = Location; };
    // Angles are spherical, in radians
    void SetPoVAngles(tuple<float, float, float> Angles){
        PoVAlpha = get<0>(Angles);
        PoVBeta = get<1>(Angles);
        PoVGamma = get<2>(Angles);

        // Update the rotation matrix
        float Alpha = PI / 180. * PoVAlpha;
        float Beta = PI / 180. * PoVBeta;
        float Gamma = PI / 180. * PoVGamma;
        PopulateRotationMatrix(RotateMat, Alpha, Beta, Gamma);
        PopulateRotationMatrix(RotateMatReverse, Alpha * -1., Beta * -1., Gamma * -1.);
    };
    void SetFoV(float NewValue) {
        Fov = NewValue;

        // Update the FoV matrix
        float CoTanFov = 1.f / tan(Fov / 2.f * PI / 180.f);
        float Q = ZFar / (ZFar - ZNear);
        ToFovMat[0][0] = CoTanFov;
        ToFovMat[1][1] = CoTanFov;
        ToFovMat[2][2] = -1.f * Q;
        ToFovMat[2][3] = -1.f;
        ToFovMat[3][2] = -1.f * ZNear * Q;
    };
    Coord GetPovLocation() { return PoVLoc; };
    tuple<float, float, float> GetPoVAngles() { return { PoVAlpha, PoVBeta, PoVGamma }; };
    float GetFoV() { return Fov; };
    void UpdateInTime(TIME_MS_T TimeElapsed) {
        for (auto& Obj : Objects)
            Obj.UpdateInTime(TimeElapsed);
    }

private:
    vector<Object> Objects;
    Coord PoVLoc;
    tuple<float, float, float> DefaultFoVRotation = { -90.f, 180.f, 90.f }; // TODO: don't need?
    float PoVAlpha, PoVBeta, PoVGamma; // Rotational angle coordinates for the PoV, degrees
    float Fov;
    float ZFar = -1000.f;
    float ZNear = -0.1f;
    // Transformation matrices that we need to draw the objects
    Mat4x4 RotateMat, RotateMatReverse, ToFovMat;
};
