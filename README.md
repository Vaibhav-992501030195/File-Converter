# Universal File Converter

A modular, CLI-based file conversion tool built with C++17 that supports converting between multiple file formats across five categories: images, audio, video, documents, and structured data.

## Folder Structure

```
UniversalFileConverter/
├── include/
│   ├── converter.h
│   ├── image_converter.h
│   ├── audio_converter.h
│   ├── video_converter.h
│   ├── document_converter.h
│   ├── data_converter.h
│   ├── validator.h
│   ├── detector.h
│   ├── history.h
│   └── utils.h
├── src/
│   ├── main.cpp
│   ├── converter.cpp
│   ├── image_converter.cpp
│   ├── audio_converter.cpp
│   ├── video_converter.cpp
│   ├── document_converter.cpp
│   ├── data_converter.cpp
│   ├── validator.cpp
│   ├── detector.cpp
│   ├── history.cpp
│   └── utils.cpp
├── data/
├── history/
├── CMakeLists.txt
└── README.md
```

## Compilation

Make sure you have CMake (3.16+) and a C++17-compatible compiler installed.

```bash
cd UniversalFileConverter
mkdir build
cd build
cmake ..
make
```

The executable `ufc` will be generated inside the `build` directory.

## Usage

Run the converter from the build directory:

```bash
./ufc
```

You will see an interactive menu:

```
==================================================
        Universal File Converter
==================================================

  1. Image Converter
  2. Audio Converter
  3. Video Converter
  4. Document Converter
  5. Data Converter
  6. Conversion History
  7. Help
  8. Exit
```

Select a converter, provide the input file path, choose the output format, and specify the output folder. The tool handles validation, conversion, and history logging automatically.

## Supported Conversions

| Category  | Input  | Output |
|-----------|--------|--------|
| Image     | PNG    | JPG    |
| Image     | JPG    | PNG    |
| Image     | BMP    | PNG    |
| Image     | PNG    | BMP    |
| Audio     | MP3    | WAV    |
| Audio     | WAV    | MP3    |
| Video     | MP4    | AVI    |
| Video     | AVI    | MP4    |
| Video     | MKV    | MP4    |
| Video     | MP4    | MKV    |
| Document  | DOCX   | PDF    |
| Document  | PDF    | TXT    |
| Data      | CSV    | JSON   |
| Data      | JSON   | CSV    |
| Data      | JSON   | XML    |
| Data      | XML    | JSON   |
| Data      | CSV    | XML    |
| Data      | XML    | CSV    |

## Architecture

The project follows an object-oriented design with an abstract base class `Converter` that defines the interface for all converters. Each converter type inherits from this base class and implements:

- `validate()` — Checks if the conversion is valid
- `convert()` — Performs the actual conversion
- `supportedFormats()` — Returns the list of supported format pairs

Supporting modules:

- **Detector** — Identifies file types by extension
- **Validator** — Validates file existence, readability, and conversion support
- **History** — Logs all conversion attempts to `history/history.txt`
- **Utils** — Helper functions for string manipulation, timestamps, and file operations

## Future Improvements

- Integrate FFmpeg for real audio/video transcoding
- Add stb_image for actual image format conversion
- Support batch conversion of multiple files
- Add progress bars for large file conversions
- Implement drag-and-drop file path support
- Add configuration file for default output directories
- Support additional formats (WEBP, FLAC, OGG, XLSX)
