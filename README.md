# CADventory

CADventory is a 3D CAD file, categorical tagging, and associated
metadata inventory management system.

It is tested on Mac, Linux, and Windows, includes code coverage
testing, and code security testing.


## Installation

1) Install: CMake 3.25+, SQLite3, and Qt6
2) Clone/Download CADventory from source
3) Compile (see .gitlab-ci.yml for variations):
     mkdir .build
     cd .build
     cmake .. -DCMAKE_INSTALL_PREFIX=/path -DCMAKE_BUILD_TYPE=Release -DQt6_DIR=/path/to/qt6
     cmake --build . --config Release

## Usage

Run (Mac and Linux):
  ./bin/cadventory
Run (Windows):
  .\Release\bin\cadventory.exe

Click + in GUI and navigate to a folder to index.

To clear all settings, run with --no-gui command-line option.

## Support

Christopher Sean Morrison
brlcad@gmail.com


## Roadmap

1) Implement automatic PDF report generation (1-page per library model).
2) Integrate metadata support into GUI for tagging primary models.
3) Graphically depict models and iconography during GUI browsing.
4) Implement support for creating & modifying collections manually.
5) Implement keyword search filtering for custom inventory reporting.


## Design

Here's an architecture diagram:

```
┌──────────────────────────────────────────────────────────────────────┐
│                               CADventory                             │
│                                                                      │
│    ┌──────────────────────────────────────────────────────────────┐  │
│    │                      User Interface (Qt)                     │  │
│    └─────────────┬───────────────────────┬─────────────────┬──────┘  │
│                  │                       │                 │         │
│    ┌─────────────▼───────────┐  ┌────────▼─────────┐  ┌────▼──────┐  │
│    │  Filesystem Processor   │  │ SQLite or JSON   │◄─│ Report    │  │
│    └─────────────┬───────────┘  │ Storage Manager  │  │ Generator │  │
│                  │              └──────────────────┘  └────┬──────┘  │
│                  │                                         │         │
│    ┌─────────────▼───────────┐                             │         │
│    │ Geometry/Image/Document │                             │         │
│    │ Handler                 │                             │         │
│    └─────────────┬───────────┘                             │         │
│                  │                                         │         │
│    ┌─────────────▼────────┐                                │         │
│    │     CAD Libraries    │◄───────────────────────────────┘         │
│    └──────────────────────┘                                          │
└──────────────────────────────────────────────────────────────────────┘
```


## Authors and Acknowledgments

Original Author
Christopher Sean Morrison

CADventory was initiated under RSEG-127, Software Engineering Studio,
Brandeis University, under the guidance of Erik Hemdal.


## License

MIT

