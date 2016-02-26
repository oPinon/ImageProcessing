ImageProcessing
===============

Various C++ image processing functions

# Gimp Supersampling Plugin

The folder GimpPlugin/ contains a plugin for Gimp that upscales images while preserving edges. <br>
The algorithm is from G.Freeman and R.Fattal (2010) : www.cs.huji.ac.il/~raananf/projects/lss_upscale/ <br>
You can find precompiled binaries here : https://github.com/oPinon/ImageProcessing/releases/tag/1.0

# Current Functions :

- Image filtering (blur, adaptive histograms..) using a fast method (updating the borders of the kernel)
- Image Upscaling (Gimp Plugin, and standalone C++ versions)
- Image extrapolation / interpolation using a Neural Network

# Dependencies

Currently uses LodePNG to import/export images
