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
            Coord Displacement = Obj.GetDisplacement();
            vector<Coord> Vertices = Obj.GetVerticesForFace(F);
            vector<D2D1_POINT_2F> Vertices2D(Vertices.size());
            vector<bool> SkipDrawingVertex(Vertices.size(), false);
            for (unsigned Idx = 0; Idx < Vertices.size(); Idx++) {
                Coord Vertex = Vertices.at(Idx);
                Coord VertexRelative, VertexRotated, VertexFovCone;
                // Translate the object to match with the camera location, as well as any
                // inherent translation.
                VertexRelative.X = Vertex.X - PoVLoc.X + Displacement.X;
                VertexRelative.Y = Vertex.Y - PoVLoc.Y + Displacement.Y;
                VertexRelative.Z = Vertex.Z - PoVLoc.Z + Displacement.Z;

                // Rotate the object to match the FoV angle (need to rotate the opposite way).
                ProjectCoord(RotateMatReverse, VertexRelative, VertexRotated);

                // Check if the vertex is between the (real) Z planes we wish to draw
                if (VertexRotated.Z > ZNear || VertexRotated.Z < ZFar) {
                    SkipDrawingVertex[Idx] = true;
                    continue;
                }
                
                // Project the coordinate from real space into FoV cone
                VertexRotated.W = 1.f;
                ProjectCoord(ToFovMat, VertexRotated, VertexFovCone);
                // Need an extra aspect ratio adjustment for the X coord
                VertexFovCone.X *= AspectRatio;
                // Need to finish by dividing by original Z coord (which is now stored in W of
                // the new vector)
                if (VertexFovCone.W != 0.0) {
                    VertexFovCone.X /= VertexFovCone.W;
                    VertexFovCone.Y /= VertexFovCone.W;
                    VertexFovCone.Z /= VertexFovCone.W;
                }

                // Check if the vertex is within the FoV, if not we shouldn't draw it...
                if (VertexFovCone.X < -1.f || VertexFovCone.X > 1.f ||
                    VertexFovCone.Y < -1.f || VertexFovCone.Y > 1.f) {
                    SkipDrawingVertex[Idx] = true;
                    continue;
                }

                // Now convert the FoV cone into screen coordinates
                D2D1_POINT_2F Vertex2D =
                    Convert2DCoords({ VertexFovCone.X, VertexFovCone.Y },
                                    SpaceMin,
                                    SpaceMax,
                                    WindowMin,
                                    WindowMax);
                // Real space is bottom-left oriented, while the window coords are top-left
                // oriented.
                Vertex2D.y = WindowSize.height - Vertex2D.y;
                Vertices2D.at(Idx) = Vertex2D;
            }

            // Draw each line (that we should) that makes up the face
            for (unsigned Idx = 0; Idx < Vertices2D.size(); Idx++) {
                unsigned NextIdx = (Idx + 1) % Vertices2D.size();
                if (!SkipDrawingVertex[Idx] && !SkipDrawingVertex[NextIdx])
                    RenderTarget->DrawLine(
                        Vertices2D[Idx], Vertices2D[NextIdx], Brush, StrokeWidth);
            }
        }
    }
};