#include "Object.h"

Object::Object(string FilePath, D2D1::ColorF LineColor) {
    // If no file specified, init an empty object.
    SetColor(LineColor);
    if (FilePath == "")
        return;

    // Initialize from a .obj file, see: https://en.wikipedia.org/wiki/Wavefront_.obj_file
    // Read the file out line-by-line
    std::fstream InFile;
    InFile.open(FilePath);
    bool Success = InFile.is_open();
    string Line;
    while (std::getline(InFile, Line)) {
        std::istringstream InSS(Line);

        // Parsing depends on the first string in the line
        string Type;
        InSS >> Type;
        if (Type == "" || Type == "#")
            continue;

        if (Type == "v") {
            float X, Y, Z, W;
            InSS >> X >> Y >> Z;
            if (!(InSS >> W))
                W = 1.0;
            AddVertex({ X, Y, Z, W });
        } else if (Type == "f") {
            string Value;
            Face& NewFace = AddFace();
            while (!InSS.eof()) {
                InSS >> Value;
                unsigned VIdx = stoi(Value.substr(0, Value.find("/")));
                if (VIdx > Vertices.size())
                    std::cout << "Error: couldn't parse face due to illegal vertex id: " << VIdx
                              << std::endl;
                NewFace.push_back(VIdx);
            }
        } else {
            std::cout << "Error: couldn't parse .obj file due to unrecognized starter (" << Type
                      << ")" << std::endl;
        }
    }
};
