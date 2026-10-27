#include "uniformstream.h"

#include "renderdevice.h"
#include "../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL(CGE_UniformStream);

UniformStream::UniformStream(RenderDevice& device, size_t blockSize) : device_(device), blockSize_(blockSize),
block_(0), offset_(0), frame_(device.getFrameCount()), generation_(0){
}

UniformStream::~UniformStream(){
}

void UniformStream::checkFrame(){
  if (device_.getFrameCount() == frame_)
    return;
  frame_ = device_.getFrameCount();
  block_ = 0;
  offset_ = 0;
  ++generation_;
}

UniformRange UniformStream::push(const void* data, size_t size){
  TR_USE(CGE_UniformStream);
  checkFrame();
  UniformRange range;
  if (size == 0 || size > blockSize_){
    TR_ERROR("push: %d bytes don't fit a block of %d bytes", (int)size, (int)blockSize_);
    return range;
  }
  size_t alignment = (size_t)device_.getCaps().uniformBufferOffsetAlignment;
  if (alignment == 0)
    alignment = 1;
  size_t offset = (offset_+alignment-1)/alignment*alignment;
  if (block_ < blocks_.size() && offset+size > blockSize_){
    ++block_;
    offset = 0;
  }
  if (block_ == blocks_.size()){
    std::unique_ptr<Buffer> buffer = device_.createBuffer({BufferType::Uniform, Usage::Stream, blockSize_});
    if (!buffer)
      return range;
    blocks_.push_back(std::move(buffer));
  }
  Buffer* buffer = blocks_[block_].get();
  device_.updateBuffer(buffer, offset, size, data);
  offset_ = offset+size;
  range.buffer = buffer;
  range.offset = offset;
  range.size = size;
  return range;
}

bool UniformStream::bind(int slot, const void* data, size_t size){
  UniformRange range = push(data, size);
  if (!range.buffer)
    return false;
  device_.setUniformBuffer(slot, range.buffer, range.offset, range.size);
  return true;
}
