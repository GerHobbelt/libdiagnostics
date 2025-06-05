
#include <diagnostics/diagnostics.h>

#include <spdlog/spdlog.h>
#include <fmt/format.h>




// --------------------------------------------------
// https://openimageio.readthedocs.io/en/latest/imageoutput.html



void diagnostics::driver::image::OpenImageIO::run()
{
	spdlog::info("Running OpenImageIO diagnostics...");
	// Check if OpenImageIO is available
	if (!OpenImageIO::exists()) {
		spdlog::error("OpenImageIO is not available.");
		return;
	}
	// Create an image output object
	auto output = OpenImageIO::create_output("test_image.exr");
	if (!output) {
		spdlog::error("Failed to create image output.");
		return;
	}
	// Set image parameters
	OpenImageIO::ImageSpec spec(800, 600, 3, OpenImageIO::TypeDesc::FLOAT);
	output->open("test_image.exr", spec);
	// Write a simple gradient image
	for (int y = 0; y < 600; ++y) {
		for (int x = 0; x < 800; ++x) {
			float r = static_cast<float>(x) / 800.0f;
			float g = static_cast<float>(y) / 600.0f;
			float b = 0.5f; // Constant blue channel
			output->write_scanline(y, 0, OpenImageIO::TypeDesc::FLOAT, &r);
			output->write_scanline(y, 1, OpenImageIO::TypeDesc::FLOAT, &g);
			output->write_scanline(y, 2, OpenImageIO::TypeDesc::FLOAT, &b);
		}
	}
	// Close the image output
	output->close();
	
	spdlog::info("OpenImageIO diagnostics completed successfully.");
}









#include <OpenImageIO/imageio.h>
using namespace OIIO;

void
simple_write()
{
	const char* filename = "simple.tif";
	const int xres = 320, yres = 240, channels = 3;
	std::vector<unsigned char> pixels(xres * yres * channels);

	std::unique_ptr<ImageOutput> out = ImageOutput::create(filename);
	if (!out)
		return;  // error
	ImageSpec spec(xres, yres, channels, TypeDesc::UINT8);
	out->open(filename, spec);
	out->write_image(make_cspan(pixels));
	out->close();
}




/*

What happens when the file format doesn’t support the spec?

The open() call will fail (returning an empty pointer and set an appropriate error message) if the output format cannot accommodate what is requested by the ImageSpec. This includes:

Dimensions (width, height, or number of channels) exceeding the limits supported by the file format. [1]

Volumetric (depth > 1) if the format does not support volumetric data.

Tile size >1 if the format does not support tiles.

Multiple subimages or MIP levels if not supported by the format.

[1]
One exception to the rule about number of channels is that a file format that supports only RGB, but not alpha, is permitted to silently drop the alpha channel without considering that to be an error.

However, several other mismatches between requested ImageSpec and file format capabilities will be silently ignored, allowing open() to succeed:

If the pixel data format is not supported (for example, a request for half pixels when writing a JPEG/JFIF file), the format writer may substitute another data format (generally, whichever commonly-used data format supported by the file type will result in the least reduction of precision or range).

If the ImageSpec requests different per-channel data formats, but the format supports only a single format for all channels, it may just choose the most precise format requested and use it for all channels.

If the file format does not support arbitrarily-named channels, the channel names may be lost when saving the file.

Any other metadata in the ImageSpec may be summarily dropped if not supported by the file format.


*/













ImageSpec spec(xres, yres, channels, TypeDesc::UINT8);
std::vector<unsigned char> scanline(xres * channels);
out->open(filename, spec);
for (int y = 0; y < yres; ++y) {
	// ... generate data in scanline[0..xres*channels-1] ...
	out->write_scanline(y, make_span(scanline));
}
out->close();






std::unique_ptr<ImageOutput> out = ImageOutput::create (filename);
if (! out->supports ("tiles")) {
	// Tiles are not supported
}



std::unique_ptr<ImageOutput> out = ImageOutput::create(filename);
if (!out)
return;  // error: could not create output at all
if (!out->supports("tiles")) {
	// Tiles are not supported
}













std::unique_ptr<ImageOutput> out = ImageOutput::create (filename);
if (! out->supports ("rectangles")) {
	// Rectangles are not supported
}



unsigned int rect[...];
// ... generate data in rect[] ...
out->write_rectangle (xbegin, xend, ybegin, yend, zbegin, zend,
					  TypeDesc::UINT8, rect);













/*
Converting pixel data types
The code examples of the previous sections all assumed that your internal pixel data is stored as unsigned 8-bit integers (i.e., 0-255 range). But OpenImageIO is significantly more flexible.

You may request that the output image pixels be stored in any of several data types. This is done by setting the format field of the ImageSpec prior to calling open. You can do this upon construction of the ImageSpec, as in the following example that requests a spec that stores pixel values as 16-bit unsigned integers:

ImageSpec spec (xres, yres, channels, TypeDesc::UINT16);

*/


ImageSpec spec(...);
spec.set_format(TypeDesc::UINT16);
















Flip an image vertically upon writing, by using negative y stride:

unsigned char pixels[xres*yres*channels];
int scanlinesize = xres * channels * sizeof(pixels[0]);
...
out->write_image (TypeDesc::UINT8,
				  (char *)pixels+(yres-1)*scanlinesize, // offset to last
				  AutoStride,                  // default x stride
				  -scanlinesize,               // special y stride
				  AutoStride);                 // default z stride
Write a tile that is embedded within a whole image of pixel data, rather than having a one-tile-only memory layout:

unsigned char pixels[xres*yres*channels];
int pixelsize = channels * sizeof(pixels[0]);
int scanlinesize = xres * pixelsize;
...
out->write_tile (x, y, 0, TypeDesc::UINT8,
				 (char *)pixels + y*scanlinesize + x*pixelsize,
				 pixelsize,
				 scanlinesize);
Write only a subset of channels to disk. In this example, our internal data layout consists of 4 channels, but we write just channel 3 to disk as a one-channel image:

// In-memory representation is 4 channel
const int xres = 640, yres = 480;
const int channels = 4;  // RGBA
const int channelsize = sizeof(unsigned char);
unsigned char pixels[xres*yres*channels];

// File representation is 1 channel
std::unique_ptr<ImageOutput> out = ImageOutput::create (filename);
ImageSpec spec (xres, yres, 1, TypeDesc::UINT8);
out->open (filename, spec);

// Use strides to write out a one-channel "slice" of the image
out->write_image (TypeDesc::UINT8,
				  (char *)pixels+3*channelsize, // offset to chan 3
				  channels*channelsize,         // 4 channel x stride
				  AutoStride,                   // default y stride
				  AutoStride);                  // default z stride
...








