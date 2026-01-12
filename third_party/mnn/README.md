# MNN Third Party Library

## Directory Structure

```
third_party/mnn/
├── Linux/
│   └── x86_64/
│       ├── include/          # MNN header files
│       │   ├── MNN/
│       │   │   ├── Interpreter.hpp
│       │   │   ├── Tensor.hpp
│       │   │   ├── MNNDefine.h
│       │   │   └── ImageProcess.hpp
│       │   └── ...
│       └── lib/              # MNN library files
│           ├── libMNN.so
│           └── libMNNTrain.so
└── Windows/
    └── AMD64/
        ├── include/          # MNN header files (same as Linux)
        └── lib/              # MNN library files
            ├── MNN.lib
            └── MNNTrain.lib
```

## Installation Instructions

### Linux
1. Download MNN prebuilt libraries from [MNN releases](https://github.com/alibaba/MNN/releases)
2. Extract the libraries to `Linux/x86_64/lib/`
3. Extract the header files to `Linux/x86_64/include/`

### Windows
1. Download MNN prebuilt libraries for Windows
2. Extract the libraries to `Windows/AMD64/lib/`
3. Extract the header files to `Windows/AMD64/include/`

## Required Libraries

- **libMNN.so** / **MNN.lib**: Core MNN inference library
- **libMNNTrain.so** / **MNNTrain.lib**: MNN training library (optional)

## Header Files

The following header files are required:
- `MNN/Interpreter.hpp`: Main interpreter interface
- `MNN/Tensor.hpp`: Tensor operations
- `MNN/MNNDefine.h`: Basic definitions
- `MNN/ImageProcess.hpp`: Image processing utilities

## Notes

- Make sure the library version is compatible with your system
- For GPU acceleration, additional backend libraries may be required
- The cmake configuration will automatically detect the platform and link appropriate libraries 