#include <iostream>
#include <string>
#include <memory>
#include <limits>
#include "converter.h"
#include "image_converter.h"
#include "audio_converter.h"
#include "video_converter.h"
#include "document_converter.h"
#include "data_converter.h"
#include "detector.h"
#include "validator.h"
#include "history.h"
#include "utils.h"

using namespace ufc;

void displayMainMenu() {
    std::cout << "\n";
    utils::printSeparator('=', 50);
    std::cout << "        Universal File Converter" << std::endl;
    utils::printSeparator('=', 50);
    std::cout << std::endl;
    std::cout << "  1. Image Converter" << std::endl;
    std::cout << "  2. Audio Converter" << std::endl;
    std::cout << "  3. Video Converter" << std::endl;
    std::cout << "  4. Document Converter" << std::endl;
    std::cout << "  5. Data Converter" << std::endl;
    std::cout << "  6. Conversion History" << std::endl;
    std::cout << "  7. Help" << std::endl;
    std::cout << "  8. Exit" << std::endl;
    std::cout << std::endl;
    utils::printSeparator('-', 50);
    std::cout << "  Enter your choice: ";
}

void displayHelp() {
    std::cout << "\n";
    utils::printSeparator('=', 50);
    std::cout << "        Help & Supported Conversions" << std::endl;
    utils::printSeparator('=', 50);

    std::cout << "\n  IMAGE CONVERSIONS:" << std::endl;
    std::cout << "    PNG <-> JPG" << std::endl;
    std::cout << "    BMP <-> PNG" << std::endl;

    std::cout << "\n  AUDIO CONVERSIONS:" << std::endl;
    std::cout << "    MP3 <-> WAV" << std::endl;

    std::cout << "\n  VIDEO CONVERSIONS:" << std::endl;
    std::cout << "    MP4 <-> AVI" << std::endl;
    std::cout << "    MKV <-> MP4" << std::endl;

    std::cout << "\n  DOCUMENT CONVERSIONS:" << std::endl;
    std::cout << "    DOCX -> PDF" << std::endl;
    std::cout << "    PDF  -> TXT" << std::endl;

    std::cout << "\n  DATA CONVERSIONS:" << std::endl;
    std::cout << "    CSV  <-> JSON" << std::endl;
    std::cout << "    JSON <-> XML" << std::endl;
    std::cout << "    CSV  <-> XML" << std::endl;

    std::cout << "\n  USAGE:" << std::endl;
    std::cout << "    1. Select a converter from the main menu" << std::endl;
    std::cout << "    2. Enter the full path to your input file" << std::endl;
    std::cout << "    3. Choose the desired output format" << std::endl;
    std::cout << "    4. Specify the output folder" << std::endl;
    std::cout << "    5. The converted file will be saved automatically" << std::endl;
    std::cout << std::endl;
}

std::string getOutputFormatChoice(Converter& converter) {
    auto formats = converter.supportedFormats();

    std::cout << "\n  Available output formats:" << std::endl;
    for (size_t i = 0; i < formats.size(); i++) {
        std::cout << "    " << (i + 1) << ". "
                  << Detector::fileTypeToString(formats[i].first) << " -> "
                  << Detector::fileTypeToString(formats[i].second) << std::endl;
    }
    std::cout << "    0. Back to main menu" << std::endl;

    std::cout << "\n  Select conversion: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice == 0 || choice < 0 || choice > static_cast<int>(formats.size())) {
        return "";
    }

    return Detector::fileTypeToString(formats[choice - 1].second);
}

void runConverter(Converter& converter, History& history) {
    std::cout << "\n";
    utils::printSeparator('=', 50);
    std::cout << "        " << converter.name() << std::endl;
    utils::printSeparator('=', 50);

    converter.displaySupportedFormats();

    std::cout << "\n  Enter input file path: ";
    std::string inputPath;
    std::getline(std::cin, inputPath);

    if (inputPath.empty()) {
        std::cout << "\n  Error: No file path provided." << std::endl;
        return;
    }

    if (inputPath.front() == '\'' || inputPath.front() == '"') {
        inputPath = inputPath.substr(1);
    }
    if (!inputPath.empty() && (inputPath.back() == '\'' || inputPath.back() == '"')) {
        inputPath = inputPath.substr(0, inputPath.size() - 1);
    }

    FileType inputType = Detector::detect(inputPath);
    if (inputType == FileType::UNKNOWN) {
        std::cout << "\n  Error: Unrecognized file format." << std::endl;
        return;
    }

    std::cout << "  Detected format: " << Detector::fileTypeToString(inputType) << std::endl;

    std::string outputFormatStr = getOutputFormatChoice(converter);
    if (outputFormatStr.empty()) return;

    FileType outputType = Detector::stringToFileType(outputFormatStr);

    std::string validationError = converter.validate(inputPath, outputType);
    if (!validationError.empty()) {
        std::cout << "\n  " << validationError << std::endl;
        history.record(utils::extractFilename(inputPath),
                       Detector::fileTypeToString(inputType),
                       outputFormatStr, false);
        return;
    }

    std::cout << "  Enter output folder path: ";
    std::string outputFolder;
    std::getline(std::cin, outputFolder);

    if (outputFolder.empty()) {
        outputFolder = ".";
    }

    if (outputFolder.front() == '\'' || outputFolder.front() == '"') {
        outputFolder = outputFolder.substr(1);
    }
    if (!outputFolder.empty() && (outputFolder.back() == '\'' || outputFolder.back() == '"')) {
        outputFolder = outputFolder.substr(0, outputFolder.size() - 1);
    }

    std::string outputPath = utils::getOutputPath(outputFolder, inputPath,
                                                   utils::toLowerExtension(outputFormatStr));

    std::cout << "\n  Converting..." << std::endl;
    std::cout << "  " << Detector::fileTypeToString(inputType) << " -> " << outputFormatStr << std::endl;

    bool success = converter.convert(inputPath, outputPath, outputType);

    if (success) {
        std::cout << "\n  Conversion successful!" << std::endl;
        std::cout << "  Output: " << outputPath << std::endl;
    } else {
        std::cout << "\n  Conversion failed." << std::endl;
    }

    history.record(utils::extractFilename(inputPath) + "." + utils::extractExtension(inputPath),
                   Detector::fileTypeToString(inputType),
                   outputFormatStr, success);
}

int main() {
    History history("history");

    ImageConverter imageConverter;
    AudioConverter audioConverter;
    VideoConverter videoConverter;
    DocumentConverter documentConverter;
    DataConverter dataConverter;

    bool running = true;

    while (running) {
        displayMainMenu();

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "\n  Invalid input. Please enter a number." << std::endl;
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                runConverter(imageConverter, history);
                break;
            case 2:
                runConverter(audioConverter, history);
                break;
            case 3:
                runConverter(videoConverter, history);
                break;
            case 4:
                runConverter(documentConverter, history);
                break;
            case 5:
                runConverter(dataConverter, history);
                break;
            case 6:
                history.display();
                break;
            case 7:
                displayHelp();
                break;
            case 8:
                std::cout << "\n  Thank you for using Universal File Converter!" << std::endl;
                std::cout << "  Goodbye.\n" << std::endl;
                running = false;
                break;
            default:
                std::cout << "\n  Invalid choice. Please select 1-8." << std::endl;
                break;
        }
    }

    return 0;
}
