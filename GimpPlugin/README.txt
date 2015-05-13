============================
Gimp SuperSampling Plugin
============================

	This plugin upscales images, while keeping sharp edges (compared to traditional techniques).
	It is example-based, and matches patches within the image itself.

----------
ALGORITHM
----------

	Based on the publication from Gilad Freeman and Raanan Fattal [2010]:
	http://www.cs.huji.ac.il/~raananf/projects/lss_upscale/

	Features not yet implemented :
	- Use of the article's biorthogonal filters
	- GPU implementation

-----------
PARAMETERS
-----------

	The plugin's dialog asks for 4 parameters :

	- WIDTH and HEIGHT, in pixels, of the desired size. The algorithm uses scaling steps of x1.25,
	so the result size will be slightly bigger than these parameters. To correct this, use the "adjust" parameter.

	- ADJUST. If it is checked, will down-sample the result size to the target size, with a classical Lanczos2 resampling.

	- CUBIC(Cleaner) or LANCZOS(Details) is the interpolation kernel for the main algorithm.
	Lanczos will give best results in most cases, but Cubic works better for artificial images, such as vector graphics.

-----------
INSTALLING
-----------

	Windows and Linux binaries are included, respectively "SuperSampleGimp_Win32.exe" and "SuperSampleGimp_Lin64.exe".
	Just put the correct ".exe" file in the plugin folder ".gimp-2.8/plug-ins/".

----------
COMPILING
----------

	If you wish to compile the plugin yourself, I have included the source code.
	For Linux, I have included a script (install.sh) to compile it.
	For Windows, I have included a Visual Studio 2013 project, but you will have to download the dependencies (listed in the project's property pages).

--------
CREDITS
--------

	The original algorithm was designed by Gilad Freeman and Raanan Fattal. The current version still lacks some elements from their publication.
	The plugin was implemented by me, Olivier Pinon. Please, feel free to contact me if you have any suggestion (improvements, bugs, etc...).

You can find the plugin's latest version on my Github page : https://github.com/oPinon/ImageProcessing/tree/master/GimpPlugin
