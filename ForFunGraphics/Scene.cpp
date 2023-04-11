#include "Scene.h"

void Scene::Render(ID2D1HwndRenderTarget* RenderTarget, ID2D1SolidColorBrush *Brush) {
    // We will transform all rendered ojects from ture (x,y,z) coordinates to the FOV "cone"
    // coordinates (x', y', z'). Perspective projection described here:
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix.html
    D2D1_SIZE_F WindowSize = RenderTarget->GetSize();
    float AspectRatio = WindowSize.width / WindowSize.height;
    float StrokeWidth = 2.;

    // The FoV "cone" is built so that the FoV captures pbjects between -1 and 1 in both x and
    // y. We'll need to convert these to screen coordinates after.
    D2D1_POINT_2F SpaceMin = { -1., -1. };
    D2D1_POINT_2F SpaceMax = { 1., 1. };
    D2D1_POINT_2F WindowMin = { 0., 0. };
    D2D1_POINT_2F WindowMax = { WindowSize.width, WindowSize.height };
    for (Object &Obj : Objects) {
        for (Face& F : Obj.GetFaces()) {
            vector<Coord> Vertices = Obj.GetVerticesForFace(F);
            Coord Displacement = Obj.GetDisplacement();
            Coord VerticesRelative[3], VerticesRotated[3], VerticesFovCone[3];
            D2D1_POINT_2F Vertices2D[3];
            bool SkipDrawingVertex[3] = { false };
            for (unsigned Idx = 0; Idx < 3; Idx++) {
                // Translate the object to match with the camera location, as well as any
                // inherent translation.
                VerticesRelative[Idx].X = Vertices[Idx].X - PoVLoc.X + Displacement.X;
                VerticesRelative[Idx].Y = Vertices[Idx].Y - PoVLoc.Y + Displacement.Y;
                VerticesRelative[Idx].Z = Vertices[Idx].Z - PoVLoc.Z + Displacement.Z;

                // Rotate the object to match the FoV angle (need to rotate the opposite way).
                ProjectCoord(RotateMatReverse, VerticesRelative[Idx], VerticesRotated[Idx]);

                // Check if the vertex is between the (real) Z planes we wish to draw
                if (VerticesRotated[Idx].Z > ZNear || VerticesRotated[Idx].Z < ZFar) {
                    SkipDrawingVertex[Idx] = true;
                    continue;
                }
                
                // Project the coordinate from real space into FoV cone
                VerticesRotated[Idx].W = 1.f;
                ProjectCoord(ToFovMat, VerticesRotated[Idx], VerticesFovCone[Idx]);
                // Need an extra aspect ratio adjustment for the X coord
                VerticesFovCone[Idx].X *= AspectRatio;
                // Need to finish by dividing by original Z coord (which is now stored in W of
                // the new vector)
                if (VerticesFovCone[Idx].W != 0.0) {
                    VerticesFovCone[Idx].X /= VerticesFovCone[Idx].W;
                    VerticesFovCone[Idx].Y /= VerticesFovCone[Idx].W;
                    VerticesFovCone[Idx].Z /= VerticesFovCone[Idx].W;
                }

                // Check if the vertex is within the FoV, if not we shouldn't draw it...
                if (VerticesFovCone[Idx].X < -1.f || VerticesFovCone[Idx].X > 1.f ||
                    VerticesFovCone[Idx].Y < -1.f || VerticesFovCone[Idx].Y > 1.f) {
                    SkipDrawingVertex[Idx] = true;
                    continue;
                }

                // Now convert the FoV cone into screen coordinates
                Vertices2D[Idx] =
                    Convert2DCoords({ VerticesFovCone[Idx].X, VerticesFovCone[Idx].Y },
                                                  SpaceMin,
                                                  SpaceMax,
                                                  WindowMin,
                                                  WindowMax);
                // Real space is bottom-left oriented, while the window coords are top-left
                // oriented.
                Vertices2D[Idx].y = WindowSize.height - Vertices2D[Idx].y;
            }
            D2D1_TRIANGLE ProjectedPoly = { Vertices2D[0], Vertices2D[1], Vertices2D[2] };
            // No Direct2D function to draw a triangle...
            if (!SkipDrawingVertex[0] && !SkipDrawingVertex[1])
                RenderTarget->DrawLine(Vertices2D[0], Vertices2D[1], Brush, StrokeWidth);
            if (!SkipDrawingVertex[1] && !SkipDrawingVertex[2])
                RenderTarget->DrawLine(Vertices2D[1], Vertices2D[2], Brush, StrokeWidth);
            if (!SkipDrawingVertex[2] && !SkipDrawingVertex[0])
                RenderTarget->DrawLine(Vertices2D[2], Vertices2D[0], Brush, StrokeWidth);
        }
    }
};