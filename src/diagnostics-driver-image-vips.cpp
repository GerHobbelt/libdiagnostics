
#include <diagnostics/diagnostics.h>

#include <spdlog/spdlog.h>
#include <fmt/format.h>





// https://github.com/libvips/libvips







#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

// C++ API
#include <vips/vips>

// Get the C API too
#include <vips/vips.h>

int
main(int argc, char **argv)
{
	char *contents;
	size_t length;
	GError *error = NULL;

	// start up vips
	if (vips_init(argv[0]))
		vips_error_exit("unable to start vips");

	if (!g_file_get_contents(argv[1], &contents, &length, &error)) {
		fprintf(stderr, "unable to read file: %s\n", error->message);
		g_error_free(error);
		exit(1);
	}

	/* Use the C API to decode to a VipsImage.
	 */
	VipsImage *c_im;

	/* You can put load options before the NULL as name/value pairs, eg.
	 * ("shrink", 2,") to shrink by x2 during load.
	 */
	if (vips_jpegload_buffer(contents, length, &c_im, NULL))
		vips_error_exit("unable to decode jpeg");

	/* Now wrap the C VipsImage up as a C++ VImage.
	 */
	vips::VImage cpp_im(c_im);

	/* The C++ API does NOT assume responsibility for the C image, you
	 * will need to unref that yourself.
	 *
	 * Don't unref it while the C++ image that wraps it is still active
	 * (obviously).
	 */

	printf("image average is: %g\n", cpp_im.avg());

	/* Unref the C image.
	 */
	g_object_unref(c_im);

	return 0;
}



// see also: https://github.com/jcupitt/libvips/blob/master/tools/vipsthumbnail.c



