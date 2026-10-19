#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <image/image.h>
#include <image/loader.h>
#include <platform/assets.h>

using CGE::Image;
using CGE::ImageLoader;

namespace{

struct Rgb{
  int r, g, b;
};

//! the picture in image.bmp, image.png and image.gif (tests/data/make_fixtures.py), rows top to bottom
const Rgb picture[2][3] = {
  {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}},
  {{255, 255, 255}, {0, 0, 0}, {128, 64, 32}},
};

void expectPicture(const Image& img){
  ASSERT_EQ(img.getWidth(), 3u);
  ASSERT_EQ(img.getHeight(), 2u);
  ASSERT_EQ(img.getNumChannels(), 3u);
  for (int y = 0; y < 2; ++y){
    for (int x = 0; x < 3; ++x){
      const unsigned char* p = img.getData() + y*img.getRowSpan() + x*3;
      EXPECT_EQ(p[0], picture[y][x].r) << "pixel " << x << "," << y;
      EXPECT_EQ(p[1], picture[y][x].g) << "pixel " << x << "," << y;
      EXPECT_EQ(p[2], picture[y][x].b) << "pixel " << x << "," << y;
    }
  }
}

}

class ImageFormats : public ::testing::TestWithParam<std::string>{};

// every decoder delivers the rows top to bottom with RGB order
TEST_P(ImageFormats, LoadsTheSamePixels){
  std::unique_ptr<Image> img(ImageLoader::load(GetParam().c_str()));
  ASSERT_NE(img, nullptr);
  expectPicture(*img);
}

INSTANTIATE_TEST_SUITE_P(Images, ImageFormats, ::testing::Values("image.bmp", "image.png", "image.gif"),
  [](const ::testing::TestParamInfo<std::string>& info){
    return info.param.substr(info.param.find('.') + 1);
  });

TEST(Images, DecodesFromMemory){
  std::vector<uint8> data;
  ASSERT_TRUE(CGE::Assets::load("image.png", data));
  std::unique_ptr<Image> img(ImageLoader::load(data.data(), (unsigned)data.size(), ImageLoader::PNG));
  ASSERT_NE(img, nullptr);
  expectPicture(*img);
}

TEST(Images, PngWithAlpha){
  std::unique_ptr<Image> img(ImageLoader::load("rgba.png"));
  ASSERT_NE(img, nullptr);
  ASSERT_EQ(img->getWidth(), 2u);
  ASSERT_EQ(img->getHeight(), 1u);
  ASSERT_EQ(img->getNumChannels(), 4u);
  const unsigned char* p = img->getData();
  EXPECT_EQ(p[0], 255);
  EXPECT_EQ(p[3], 255);
  EXPECT_EQ(p[6], 255);
  EXPECT_EQ(p[7], 0);
}

TEST(Images, Jpeg){
  //a 1x1 JPEG; lossy, so only the format is checked
  std::unique_ptr<Image> img(ImageLoader::load("image.jpg"));
  ASSERT_NE(img, nullptr);
  EXPECT_EQ(img->getWidth(), 1u);
  EXPECT_EQ(img->getHeight(), 1u);
  EXPECT_EQ(img->getNumChannels(), 3u);
}

TEST(Images, TypeFromExtension){
  EXPECT_EQ(ImageLoader::determineType("a/b.jpg"), ImageLoader::JPG);
  EXPECT_EQ(ImageLoader::determineType("b.jpeg"), ImageLoader::JPG);
  EXPECT_EQ(ImageLoader::determineType("b.png"), ImageLoader::PNG);
  EXPECT_EQ(ImageLoader::determineType("b.gif"), ImageLoader::GIF);
  EXPECT_EQ(ImageLoader::determineType("b.bmp"), ImageLoader::BMP);
  EXPECT_EQ(ImageLoader::determineType("b.txt"), ImageLoader::UNKNOWN);
}

TEST(Images, MissingOrUnknownFilesGiveNull){
  EXPECT_EQ(ImageLoader::load("missing.png"), nullptr);
  EXPECT_EQ(ImageLoader::load("folder/a.txt"), nullptr);
}
