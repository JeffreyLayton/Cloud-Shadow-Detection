#include "Imageio.h"

#include "ImageOperations.h"
#include "boilerplate/Log.h"
#include "tiffio.h"

std::shared_ptr<ImageFloat> Imageio::ReadSingleChannelFloat(const Path path) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    std::shared_ptr<ImageFloat> ret;
    TIFF *tif = TIFFOpen(path.string().c_str(), "r");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        std::vector<float> retVal;
        uint32_t width, height;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        retVal.resize(size_t(width) * size_t(height));

        for (size_t y = 0; y < height; y++)
            if (TIFFReadScanline(tif, &retVal[y * size_t(width)], y) == -1) {
                TIFFClose(tif);
                throw std::runtime_error("Falure when reading file");
            }
        ret = std::make_shared<ImageFloat>(
            Eigen::Map<ImageFloat>(reinterpret_cast<float *>(retVal.data()), height, width)
                .cast<float>()
        );
        TIFFClose(tif);
    }
    return std::make_shared<ImageFloat>(ret->colwise().reverse());
}

std::shared_ptr<ImageUint> Imageio::ReadSingleChannelUint8(const Path path) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    std::shared_ptr<ImageUint> ret;
    TIFF *tif = TIFFOpen(path.string().c_str(), "r");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        std::vector<uint8_t> retVal;
        uint32_t width, height;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        retVal.resize(size_t(width) * size_t(height));

        for (size_t y = 0; y < height; y++)
            if (TIFFReadScanline(tif, &retVal[y * size_t(width)], y) == -1) {
                TIFFClose(tif);
                throw std::runtime_error("Falure when reading file");
            }
        std::vector<unsigned int> tmp(retVal.begin(), retVal.end());
        ret = std::make_shared<ImageUint>(
            Eigen::Map<ImageUint>(reinterpret_cast<unsigned int *>(tmp.data()), height, width)
                .cast<unsigned int>()
        );

        TIFFClose(tif);
    }
    return std::make_shared<ImageUint>(ret->colwise().reverse());
}

std::shared_ptr<ImageUint> Imageio::ReadSingleChannelUint16(const Path path) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    std::shared_ptr<ImageUint> ret;
    TIFF *tif = TIFFOpen(path.string().c_str(), "r");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        std::vector<uint16_t> retVal;
        uint32_t width, height;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        retVal.resize(size_t(width) * size_t(height));

        for (size_t y = 0; y < height; y++)
            if (TIFFReadScanline(tif, &retVal[y * size_t(width)], y) == -1) {
                TIFFClose(tif);
                throw std::runtime_error("Falure when reading file");
            }
        std::vector<unsigned int> tmp(retVal.begin(), retVal.end());
        ret = std::make_shared<ImageUint>(
            Eigen::Map<ImageUint>(reinterpret_cast<unsigned int *>(tmp.data()), height, width)
                .cast<unsigned int>()
        );

        TIFFClose(tif);
    }
    return std::make_shared<ImageUint>(ret->colwise().reverse());
}

std::shared_ptr<ImageUint> Imageio::ReadSingleChannelUint32(const Path path) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    std::shared_ptr<ImageUint> ret;
    TIFF *tif = TIFFOpen(path.string().c_str(), "r");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        std::vector<uint32_t> retVal;
        uint32_t width, height;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        retVal.resize(size_t(width) * size_t(height));

        for (size_t y = 0; y < height; y++)
            if (TIFFReadScanline(tif, &retVal[y * width], y) == -1) {
                TIFFClose(tif);
                throw std::runtime_error("Falure when reading file");
            }
        std::vector<unsigned int> tmp(retVal.begin(), retVal.end());
        ret = std::make_shared<ImageUint>(
            Eigen::Map<ImageUint>(reinterpret_cast<unsigned int *>(tmp.data()), height, width)
                .cast<unsigned int>()
        );
        TIFFClose(tif);
    }
    return std::make_shared<ImageUint>(ret->colwise().reverse());
}

std::shared_ptr<ImageUint> Imageio::ReadRGBA(const Path path) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    std::shared_ptr<ImageUint> ret;
    TIFF *tif = TIFFOpen(path.string().c_str(), "r");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        std::vector<uint32_t> retVal;
        uint32_t width, height;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        retVal.resize(size_t(width) * size_t(height));
        if (!TIFFReadRGBAImage(tif, width, height, retVal.data(), 0)) {
            TIFFClose(tif);
            throw std::runtime_error("Falure when reading file");
        } else {
            std::vector<unsigned int> tmp(retVal.begin(), retVal.end());
            ret = std::make_shared<ImageUint>(
                Eigen::Map<ImageUint>(reinterpret_cast<unsigned int *>(tmp.data()), height, width)
                    .cast<unsigned int>()
            );
        }
        TIFFClose(tif);
    }
    return ret;
}

void quietHandler(const char *module, const char *fmt, va_list args) { return; }
void Imageio::SupressLibTIFF() {
    TIFFSetErrorHandler(quietHandler);
    TIFFSetWarningHandler(quietHandler);
}

void Imageio::WriteSingleChannelFloat(const Path path, std::shared_ptr<ImageFloat> image) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    if (std::filesystem::exists(path)) std::filesystem::remove(path);
    TIFF *tif = TIFFOpen(path.string().c_str(), "w");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        image = std::make_shared<ImageFloat>(image->colwise().reverse());
        std::vector<float> imageData
            = std::vector<float>(image->data(), image->data() + image->size());

        // Set values to use
        uint32_t width           = image->cols();
        uint32_t height          = image->rows();
        uint32_t bitsPerSample   = 32;
        uint32_t bytesPerSample  = 4;
        uint32_t samplesPerPixel = 1;
        uint32_t lineSize        = samplesPerPixel * width;
        uint32_t lineBytes       = lineSize * bytesPerSample;
        uint32_t lineBufferSize  = std::max(lineSize, uint32_t(TIFFScanlineSize(tif)));
        uint32_t stripSize       = TIFFDefaultStripSize(tif, width * samplesPerPixel);

        // Set image fields
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, stripSize);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

        std::vector<float> lineBufferData = std::vector<float>(lineBufferSize);

        for (size_t y = 0; y < height; y++) {
            memcpy(lineBufferData.data(), &imageData[y * lineSize], lineBytes);
            if (TIFFWriteScanline(tif, lineBufferData.data(), y) == -1) {
                TIFFClose(tif);
                throw std::runtime_error("Falure when writing file");
            }
        }
        TIFFClose(tif);
    }
}

void Imageio::WriteSingleChannelUint8(const Path path, std::shared_ptr<ImageUint> image) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    if (std::filesystem::exists(path)) std::filesystem::remove(path);
    TIFF *tif = TIFFOpen(path.string().c_str(), "w");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        image = std::make_shared<ImageUint>(image->colwise().reverse());
        std::vector<uint8_t> imageData
            = std::vector<uint8_t>(image->data(), image->data() + image->size());

        // transform(imageData.begin(), imageData.end(), imageData.begin(), [](uint8_t in_v)
        // { return 50u * in_v; });

        // Set values to use
        uint32_t width           = image->cols();
        uint32_t height          = image->rows();
        uint32_t bitsPerSample   = 8;
        uint32_t bytesPerSample  = 1;
        uint32_t samplesPerPixel = 1;
        uint32_t lineSize        = samplesPerPixel * width;
        uint32_t lineBytes       = lineSize * bytesPerSample;
        uint32_t lineBufferSize  = std::max(lineSize, uint32_t(TIFFScanlineSize(tif)));
        uint32_t stripSize       = TIFFDefaultStripSize(tif, width * samplesPerPixel);

        // Set image fields
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, stripSize);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

        std::vector<uint8_t> lineBufferData = std::vector<uint8_t>(lineBufferSize);

        for (size_t y = 0; y < height; y++) {
            memcpy(lineBufferData.data(), &imageData[y * lineSize], lineBytes);
            if (TIFFWriteScanline(tif, lineBufferData.data(), y) == -1) {
                TIFFClose(tif);
                throw std::runtime_error("Falure when writing file");
            }
        }
        TIFFClose(tif);
    }
}

void Imageio::WriteSingleChannelUint16(const Path path, std::shared_ptr<ImageUint> image) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    if (std::filesystem::exists(path)) std::filesystem::remove(path);
    TIFF *tif = TIFFOpen(path.string().c_str(), "w");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        image = std::make_shared<ImageUint>(image->colwise().reverse());
        std::vector<uint16_t> imageData
            = std::vector<uint16_t>(image->data(), image->data() + image->size());

        // Set values to use
        uint32_t width           = image->cols();
        uint32_t height          = image->rows();
        uint32_t bitsPerSample   = 16;
        uint32_t bytesPerSample  = 2;
        uint32_t samplesPerPixel = 1;
        uint32_t lineSize        = samplesPerPixel * width;
        uint32_t lineBytes       = lineSize * bytesPerSample;
        uint32_t lineBufferSize  = std::max(lineSize, uint32_t(TIFFScanlineSize(tif)));
        uint32_t stripSize       = TIFFDefaultStripSize(tif, width * samplesPerPixel);

        // Set image fields
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, stripSize);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

        std::vector<uint16_t> lineBufferData = std::vector<uint16_t>(lineBufferSize);

        for (size_t y = 0; y < height; y++) {
            memcpy(lineBufferData.data(), &imageData[y * lineSize], lineBytes);
            if (TIFFWriteScanline(tif, lineBufferData.data(), y) == -1) {
                TIFFClose(tif);
                throw std::runtime_error("Falure when writing file");
            }
        }
        TIFFClose(tif);
    }
}

void Imageio::WriteSingleChannelUint32(const Path path, std::shared_ptr<ImageUint> image) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    if (std::filesystem::exists(path)) std::filesystem::remove(path);
    TIFF *tif = TIFFOpen(path.string().c_str(), "w");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        image = std::make_shared<ImageUint>(image->colwise().reverse());
        std::vector<uint32_t> imageData
            = std::vector<uint32_t>(image->data(), image->data() + image->size());

        // transform(imageData.begin(), imageData.end(), imageData.begin(), [](uint32_t in_v) {
        // return uint64_t(858993459u) * uint64_t(in_v); });

        // Set values to use
        uint32_t width           = image->cols();
        uint32_t height          = image->rows();
        uint32_t bitsPerSample   = 32;
        uint32_t bytesPerSample  = 4;
        uint32_t samplesPerPixel = 1;
        uint32_t lineSize        = samplesPerPixel * width;
        uint32_t lineBytes       = lineSize * bytesPerSample;
        uint32_t lineBufferSize  = std::max(lineSize, uint32_t(TIFFScanlineSize(tif)));
        uint32_t stripSize       = TIFFDefaultStripSize(tif, width * samplesPerPixel);

        // Set image fields
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, stripSize);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

        std::vector<uint32_t> lineBufferData = std::vector<uint32_t>(lineBufferSize);

        for (size_t y = 0; y < height; y++) {
            memcpy(lineBufferData.data(), &imageData[y * lineSize], lineBytes);
            if (TIFFWriteScanline(tif, lineBufferData.data(), y) == -1) {
                TIFFClose(tif);
                throw std::runtime_error("Falure when writing file");
            }
        }
        TIFFClose(tif);
    }
}

void Imageio::WriteRGBA(const Path path, std::shared_ptr<ImageUint> image) {
    if (path.extension() != Path(".tif")) throw std::runtime_error("Extention must be tif");
    if (std::filesystem::exists(path)) std::filesystem::remove(path);
    TIFF *tif = TIFFOpen(path.string().c_str(), "w");
    if (!tif) throw std::runtime_error("Cant open file");
    else {
        image                          = std::make_shared<ImageUint>(image->colwise().reverse());
        std::vector<uint8_t> imageData = ImageOperations::decomposeRBGA256(image);

        // Set values to use
        uint32_t width           = image->cols();
        uint32_t height          = image->rows();
        uint32_t bitsPerSample   = 8;
        uint32_t bytesPerSample  = 1;
        uint32_t samplesPerPixel = 4;
        uint32_t lineSize        = samplesPerPixel * width;
        uint32_t lineBytes       = lineSize * bytesPerSample;
        uint32_t lineBufferSize  = std::max(lineSize, uint32_t(TIFFScanlineSize(tif)));
        uint32_t stripSize       = TIFFDefaultStripSize(tif, width * samplesPerPixel);

        // Set image fields
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
        TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, stripSize);
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

        std::vector<uint8_t> lineBufferData = std::vector<uint8_t>(lineBufferSize);

        for (size_t y = 0; y < height; y++) {
            memcpy(lineBufferData.data(), &imageData[y * lineSize], lineBytes);
            if (TIFFWriteScanline(tif, lineBufferData.data(), y) == -1) {
                TIFFClose(tif);
                throw std::runtime_error("Falure when writing file");
            }
        }
        TIFFClose(tif);
    }
}