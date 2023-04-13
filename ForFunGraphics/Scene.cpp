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
    for (Object& Obj : Objects) {
        Brush->SetColor(Obj.GetColor());
        Coord Displacement = Obj.GetDisplacement();

        Coord PrevVertex, Vertex = { 0 };
        D2D1_POINT_2F PrevVertex2D, Vertex2D = { 0 };
        bool PrevSkipVertex, SkipVertex = false;
        for (Face& F : Obj.GetFaces()) {

            // Handle adjacent vertices in pairs, so we can draw the outline. To do this we need
            // to loop one extra time.
            vector<Coord> Vertices = Obj.GetVerticesForFace(F);
            for (unsigned Idx = 0; Idx < Vertices.size() + 1; Idx++) {
                // Current vertex is now the previous vertex
                PrevVertex = Vertex;
                PrevVertex2D = Vertex2D;
                PrevSkipVertex = SkipVertex;

                Vertex = Vertices.at(Idx % Vertices.size());
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
                    SkipVertex = true;
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
                    SkipVertex = true;
                    continue;
                }

                // Now convert the FoV cone into screen coordinates
                Vertex2D = Convert2DCoords({ VertexFovCone.X, VertexFovCone.Y },
                                           SpaceMin,
                                           SpaceMax,
                                           WindowMin,
                                           WindowMax);
                // Real space is bottom-left oriented, while the window coords are top-left
                // oriented.
                Vertex2D.y = WindowSize.height - Vertex2D.y;

                // Draw a line between the vertices, just not for the first iteration.
                if (Idx != 0 && !PrevSkipVertex && !SkipVertex) {
                    RenderTarget->DrawLine(
                        PrevVertex2D, Vertex2D, Brush, StrokeWidth);
                }
            }
        }
    }
};