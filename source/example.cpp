// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.
// Authored in 2015 by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
// https://github.com/ddiakopoulos/tinyply
// Version 2.0

#include <thread>
#include <chrono>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>

#include "tinyply.h"

using namespace tinyply;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;
std::chrono::high_resolution_clock c;

inline std::chrono::time_point<std::chrono::high_resolution_clock> now()
{
    return c.now();
}

inline double difference_millis(timepoint start, timepoint end)
{
    return (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void write_ply_example(const std::string & filename)
{
    std::vector<float> verts;
    std::vector<float> norms;
    std::vector<uint8_t> colors;

    std::vector<int32_t> vertexIndicies;
    std::vector<float> faceTexcoords;

    // Per-vertex elements
    verts = {
        0.f, 1.f, 2.f,
        3.f, 4.f, 5.f,
        6.f, 7.f, 8.f,
        9.f, 10.f, 11.f,
        12.f, 13.f, 14.f,
        15.f, 16.f, 17.f,
        18.f, 19.f, 20.f,
        21.f, 22.f, 23.f,
        24.f, 25.f, 26.f,
        27.f, 28.f, 29.f,
        30.f, 31.f, 32.f,
        33.f, 34.f, 35.f,
    };

    norms = {
        0.5f, 1.5f, 2.5f,
        3.5f, 4.5f, 5.5f,
        6.5f, 7.5f, 8.5f,
        9.5f, 10.5f, 11.5f,
        12.5f, 13.5f, 14.5f,
        15.5f, 16.5f, 17.5f,
        18.5f, 19.5f, 20.5f,
        21.5f, 22.5f, 23.5f,
        24.5f, 25.5f, 26.5f,
        27.5f, 28.5f, 29.5f,
        30.5f, 31.5f, 32.5f,
        33.5f, 34.5f, 35.5f,
    };

    colors = {
        10, 20, 30, 244,
        15, 25, 35, 245,
        20, 30, 40, 246,
        25, 35, 45, 247,
        30, 40, 50, 248,
        35, 45, 55, 249,
        40, 50, 60, 250,
        45, 55, 65, 251,
        50, 60, 70, 252,
        55, 65, 75, 253,
        60, 70, 80, 254,
        65, 75, 85, 255
    };

    // Per-face elements
    vertexIndicies = { 
        0, 1, 2, 
        3, 4, 5, 
        6, 7, 8, 
        9, 10, 11 
    };

    faceTexcoords = {
        0.25f, 0.55f, 
        0.26f, 0.56f,
        0.27f, 0.57f,
        0.28f, 0.58f, 
        0.29f, 0.59f, 
        0.30f, 0.60f, 
        0.31f, 0.61f, 
        0.32f, 0.62f,
        0.33f, 0.63f, 
        0.34f, 0.64f,
        0.35f, 0.65f, 
        0.36f, 0.66f 
    };

    // Tinyply does not perform any file i/o internally
    std::filebuf fb;
    fb.open(filename, std::ios::out | std::ios::binary);
    std::ostream outputStream(&fb);

    PlyFile exampleOutFile;

    exampleOutFile.add_properties_to_element("vertex", { "x", "y", "z" }, Type::FLOAT32, verts.size(), reinterpret_cast<uint8_t*>(verts.data()), Type::INVALID, 0);
    exampleOutFile.add_properties_to_element("vertex", { "nx", "ny", "nz" }, Type::FLOAT32, verts.size(), reinterpret_cast<uint8_t*>(norms.data()), Type::INVALID, 0);
    exampleOutFile.add_properties_to_element("vertex", { "red", "green", "blue", "alpha" }, Type::UINT8, verts.size(), reinterpret_cast<uint8_t*>(colors.data()), Type::INVALID, 0);

    exampleOutFile.add_properties_to_element("face", { "vertex_indices" }, Type::UINT32, vertexIndicies.size(), reinterpret_cast<uint8_t*>(vertexIndicies.data()), Type::UINT16, 3);
    exampleOutFile.add_properties_to_element("face", { "texcoord" }, Type::FLOAT32, faceTexcoords.size(), reinterpret_cast<uint8_t*>(faceTexcoords.data()), Type::UINT16, 6);

    exampleOutFile.get_comments().push_back("generated by tinyply");
    exampleOutFile.write(outputStream, false);

    fb.close();
}

void read_ply_file(const std::string & filename)
{
    try
    {
        // Read the file and create a std::istringstream suitable
        // for the lib -- tinyply does not perform any file i/o.
        std::ifstream ss(filename, std::ios::binary);

        if (ss.fail()) 
        {
            throw std::runtime_error("failed to open " + filename);
        }

        PlyFile file;

        file.parse_header(ss);

        std::cout << "================================================================\n";

        for (auto c : file.get_comments()) std::cout << "Comment: " << c << std::endl;

        for (auto e : file.get_elements())
        {
            std::cout << "element - " << e.name << " (" << e.size << ")" << std::endl;
            for (auto p : e.properties)
            {
                std::cout << "\tproperty - " << p.name << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;
            }
        }

        std::cout << "================================================================\n";

        // Tinyply 2.0 treats incoming data as untyped byte buffers. It's now
        // up to users to treat this data as they wish. See below for examples.
        std::shared_ptr<PlyData> vertices, normals, colors, faces, texcoords;

        // The header information can be used to programmatically extract properties on elements
        // known to exist in the file header prior to reading the data. For brevity of this sample, properties 
        // like vertex position are hard-coded: 
        try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
        catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
        catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { colors = file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha" }); }
        catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { faces = file.request_properties_from_element("face", { "vertex_indices" }); }
        catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { texcoords = file.request_properties_from_element("face", { "texcoord" }); }
        catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        timepoint before = now();
        file.read(ss);
        timepoint after = now();

        // Good place to put a breakpoint!
        std::cout << "Parsing took " << difference_millis(before, after) << " ms: " << std::endl;
        if (vertices) std::cout << "\tRead " << vertices->count << " total vertices "<< std::endl;
        if (normals) std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
        if (colors) std::cout << "\tRead " << colors->count << " total vertex colors "<< std::endl;
        if (faces) std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;
        if (texcoords) std::cout << "\tRead " << texcoords->count << " total texcoords " << std::endl;

        // Example: type 'conversion' to your own native types - Option A
        {
            const size_t numVerticesBytes = vertices->buffer.size_bytes();
            struct float3 { float x, y, z; };
            std::vector<float3> verts(vertices->count);
            std::memcpy(verts.data(), vertices->buffer.get(), numVerticesBytes);
        }

        // Example: type 'conversion' to your own native types - Option B
        {
            const size_t numVerticesBytes = vertices->buffer.size_bytes();
            struct float3 { float x, y, z; };
            struct double3 { double x, y, z; };

            std::vector<float3> verts_floats;
            std::vector<double3> verts_doubles;

            if (vertices->t == tinyply::Type::FLOAT32) { /* as floats ... */ }
            if (vertices->t == tinyply::Type::FLOAT64) { /* as doubles ... */ }
        }
    }
    catch (const std::exception & e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    write_ply_example("example_junk.ply");
    read_ply_file("example_junk.ply");
    return 0;
}
