Mini Diagnostic CT Viewer Pro
=============================

A desktop medical imaging utility developed in C++ using the Qt 6 framework. 
This application provides basic 3D CT volume loading capabilities, interactive Hounsfield windowing optimizations (contrast and brightness adjustment), and real-time slice annotation tracking.

Key Functionalities:
- Folder-based loading pipeline for local PNG slice datasets.
- Pair-synchronized contrast (Window Width) and brightness (Window Center) controls.
- Dynamic linear measurement updates and region-of-interest indicators.
- Smooth mouse wheel multi-scale canvas scaling.

System Architecture:
- Qt 6 Core, GUI, and Widgets libraries.
- Model-View-Controller framework patterns separation.
- QGraphicsView rendering architecture.

Build Requirements:
- Ubuntu Linux layout / Desktop Environment
- Qt Creator 6.x / qmake setup toolchains
- C++17 compliant compiler engine
