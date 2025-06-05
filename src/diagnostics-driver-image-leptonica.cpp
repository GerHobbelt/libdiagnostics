
#include <diagnostics/diagnostics.h>

#include <spdlog/spdlog.h>
#include <fmt/format.h>






1

Well I could solve the problem this way:

Leptonica offers a function

l_int32     pixWriteMemBmp (l_uint8 **pdata, size_t *psize, PIX *pix)
With this function you can write into the memory instead of a filestream. Still (in this example) the Bmp Header and format persists (there are the same functions for other image formats too).

The corresponding function from QT is this one:

bool QImage::loadFromData (const uchar * data, int len, const char * format = 0)
Since the the Header persits I just need to pass the data ptr and the size to the loadFromData function and QT does the rest.

So all together it would be like this:

	PIX *m_pix;
FILE * pFile;
pFile = fopen("PathToFile", "r");
m_pix = pixReadStreamBmp(pFile); // If other file format use the according function
fclose(pFile);
// Now we have a Pix object from leptonica

l_uint8* ptr_memory;
size_t len;
pixWriteMemBmp(&ptr_memory, &size, m_pix);
// Now we have the picture somewhere in the memory

QImage testimage;
QPixmap pixmap;
testimage.loadFromData((uchar *)ptr_memory, len);
pixmap.convertFromImage(testimage);
// Now we have the image as a pixmap in Qt
This actually works for me, tho I don't know if there is a way to do this backwards so easy. ( If there is, please let me know )





















PIX* TessTools::qImage2PIX(QImage& qImage) {
	PIX * pixs;
	l_uint32 *lines;

	qImage = qImage.rgbSwapped();
	int width = qImage.width();
	int height = qImage.height();
	int depth = qImage.depth();
	int wpl = qImage.bytesPerLine() / 4;

	pixs = pixCreate(width, height, depth);
	pixSetWpl(pixs, wpl);
	pixSetColormap(pixs, NULL);
	l_uint32 *datas = pixs->data;

	for (int y = 0; y < height; y++) {
		lines = datas + y * wpl;
		QByteArray a((const char*)qImage.scanLine(y), qImage.bytesPerLine());
		for (int j = 0; j < a.size(); j++) {
			*((l_uint8 *)lines + j) = a[j];
		}
	}
	return pixEndianByteSwapNew(pixs);
}
And this for conversion PIX to QImage:

QImage TessTools::PIX2QImage(PIX *pixImage) {
	int width = pixGetWidth(pixImage);
	int height = pixGetHeight(pixImage);
	int depth = pixGetDepth(pixImage);
	int bytesPerLine = pixGetWpl(pixImage) * 4;
	l_uint32 * s_data = pixGetData(pixEndianByteSwapNew(pixImage));

	QImage::Format format;
	if (depth == 1)
		format = QImage::Format_Mono;
	else if (depth == 8)
		format = QImage::Format_Indexed8;
	else
		format = QImage::Format_RGB32;

	QImage result((uchar*)s_data, width, height, bytesPerLine, format);

	// Handle pallete
	QVector<QRgb> _bwCT;
	_bwCT.append(qRgb(255, 255, 255));
	_bwCT.append(qRgb(0, 0, 0));

	QVector<QRgb> _grayscaleCT(256);
	for (int i = 0; i < 256; i++) {
		_grayscaleCT.append(qRgb(i, i, i));
	}
	if (depth == 1) {
		result.setColorTable(_bwCT);
	} else if (depth == 8) {
		result.setColorTable(_grayscaleCT);

	} else {
		result.setColorTable(_grayscaleCT);
	}

	if (result.isNull()) {
		static QImage none(0, 0, QImage::Format_Invalid);
		qDebug() << "***Invalid format!!!";
		return none;
	}

	return result.rgbSwapped();
}


