#include "Object.h"

Object::Object(string FilePath){
    // If no file specified, init an empty object.
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
            string Values[3];
            InSS >> Values[0] >> Values[1] >> Values[2];
            unsigned VId[3];
            for (unsigned Idx = 0; Idx < 3; Idx++)
                VId[Idx] = stoi(Values[Idx].substr(0, Values[Idx].find("/")));

            // We should check that the Vertices given actually exist
            for (unsigned Idx = 0; Idx < 3; Idx++)
                if (VId[Idx] > Vertices.size())
                    std::cout << "Error: couldn't parse face due to illegal verted id: "
                              << VId[Idx] << std::endl;
            AddFace(VId[0], VId[1], VId[2]);
        } else {
            std::cout << "Error: couldn't parse .obj file due to unrecognized starter (" << Type
                      << ")" << std::endl;
        }
    }
};
