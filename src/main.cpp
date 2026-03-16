#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <webp/encode.h>

struct Param {
    std::vector<std::string> inputs;
    std::string output;
};

void showHelp() {
    std::cout << "Usage: imgconv [options]\n" <<
        "Options:\n"
        " --inputs, -i <file1> [file2 ...]\tInput image files (PNG/JPEG)\n"
        " --output, -o <dir>\tOutput directory\n"
        " --help\tShow this help message\n";
}

Param getParams(int argc, char** argv) {
    Param param;

    enum class ArgType {HELP, INPUTS, OUTPUT, UNKNOWN};

    auto classIfArg = [](const std::string& a) {
        if (a == "--help" || a == "-h") return ArgType::HELP;
        if (a == "--input" || a == "-i") return ArgType::INPUTS;
        if (a == "--output" || a == "-o") return ArgType::OUTPUT;
        return ArgType::UNKNOWN;
    };

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        switch (classIfArg(arg)) {
            case ArgType::HELP:
                showHelp();
                std::exit(EXIT_SUCCESS);
                break;
            case ArgType::INPUTS:
                break;
            case ArgType::OUTPUT:
                break;
            case ArgType::UNKNOWN:
            default:
                std::cerr << "Unknown options: " << arg << std::endl;
                std::cerr << "Usage: imgconv --help" << std::endl;
                std::exit(EXIT_FAILURE);
        }
    }
};

int main(int argc, char** argv) {
    std::vector<std::string> inputs;
    std::string outputDir;
    Param param;

    param = getParams(argc, argv);

    int width, height, channels;

    unsigned char* image = stbi_load("../src/test.png", &width, &height, &channels, 4);
    if (!image) { std::cerr << "Failed to load image\n"; return EXIT_FAILURE; }

    std::cout << "Loaded " << width << "x" << height << std::endl;

    WebPConfig config;
    if (!WebPConfigInit(&config)) { std::cerr << "Failed to init config\n"; return EXIT_FAILURE; }

    config.quality = 90;        // качество 0-100
    config.method = 5;           // 0-6, больше — лучше сжатие
    config.lossless = 0;         // 0 = lossy
    config.sns_strength = 50;    // шумоподавление
    config.filter_strength = 60; // deblocking
    config.pass = 3;            // мультипассы

    WebPPicture picture;
    WebPPictureInit(&picture);
    picture.width = width;
    picture.height = height;

    if (!WebPPictureImportRGBA(&picture, image, width * 4)) {
        std::cerr << "Failed to import image to WebPPicture\n";
        stbi_image_free(image);
        return EXIT_FAILURE;
    }

    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    picture.writer = WebPMemoryWrite;
    picture.custom_ptr = &writer;

    if (!WebPEncode(&config, &picture)) {
        std::cerr << "Failed to encode\n";
        WebPPictureFree(&picture);
        WebPMemoryWriterClear(&writer);
        stbi_image_free(image);
        return EXIT_FAILURE;
    }

    std::ofstream out("output2.webp", std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file\n";
    } else {
        out.write(reinterpret_cast<char*>(writer.mem), writer.size);
        std::cout << "Saved output.webp (" << writer.size << " bytes)" << std::endl;
    }

    WebPMemoryWriterClear(&writer);
    WebPPictureFree(&picture);
    stbi_image_free(image);

    return EXIT_SUCCESS;
}