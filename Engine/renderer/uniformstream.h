#ifndef CGE_UNIFORM_STREAM_H
#define CGE_UNIFORM_STREAM_H

#include <cge_export.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace CGE{
namespace Gfx{

class Buffer;
class RenderDevice;

//! where UniformStream::push put the data
struct UniformRange{
  //! nullptr if the push failed
  Buffer* buffer = nullptr;
  size_t offset = 0;
  size_t size = 0;
};

//! Uniform data that changes per draw
/*! A linear allocator in uniform buffers: every push gets a range of its own
    at the device's offset alignment, so each draw keeps the data it was
    issued with, as Vulkan and Direct3D 12 need it. The ranges are reused
    once the device begins the next frame (RenderDevice::getFrameCount);
    more buffers of blockSize bytes are added when a frame needs them. */
class CGE_API UniformStream{
public:
  explicit UniformStream(RenderDevice& device, size_t blockSize = 64*1024);
  ~UniformStream();
  UniformStream(const UniformStream&) = delete;
  UniformStream& operator=(const UniformStream&) = delete;

  //! copies size bytes (at most the block size) into the stream
  UniformRange push(const void* data, size_t size);
  //! pushes the data and binds it to a uniform buffer slot (inside a pass)
  bool bind(int slot, const void* data, size_t size);
  template<typename T> bool bind(int slot, const T& data) {return bind(slot, &data, sizeof(T));}

  //! changes whenever the ranges are reused; ranges pushed before are invalid then
  uint64_t getGeneration() const {return generation_;}
  //! the number of buffers allocated so far
  int getBlockCount() const {return (int)blocks_.size();}

private:
  //! starts over if the device began a new frame
  void checkFrame();

  RenderDevice& device_;
  size_t blockSize_;
  std::vector<std::unique_ptr<Buffer>> blocks_;
  //! the block being filled and the next free byte in it
  size_t block_;
  size_t offset_;
  uint64_t frame_;
  uint64_t generation_;
};

}
}

#endif
