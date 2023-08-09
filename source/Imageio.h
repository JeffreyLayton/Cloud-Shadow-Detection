#pragma once
#include "types.h"

namespace Imageio {
std::shared_ptr<ImageFloat> ReadSingleChannelFloat(const Path path);
std::shared_ptr<ImageUint> ReadSingleChannelUint8(const Path path);
std::shared_ptr<ImageUint> ReadSingleChannelUint16(const Path path);
std::shared_ptr<ImageUint> ReadSingleChannelUint32(const Path path);
std::shared_ptr<ImageUint> ReadRGBA(const Path path);

void SupressLibTIFF();

void WriteSingleChannelFloat(const Path path, std::shared_ptr<ImageFloat> image);
void WriteSingleChannelUint8(const Path path, std::shared_ptr<ImageUint> image);
void WriteSingleChannelUint16(const Path path, std::shared_ptr<ImageUint> image);
void WriteSingleChannelUint32(const Path path, std::shared_ptr<ImageUint> image);
void WriteRGBA(const Path path, std::shared_ptr<ImageUint> image);
}  // namespace Imageio
