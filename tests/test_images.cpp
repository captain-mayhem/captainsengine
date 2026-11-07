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

class TiffLayouts : public ::testing::TestWithParam<std::string>{};

// the picture in every way TIFF stores RGB (tests/data/make_fixtures.py), and from GDI+'s encoder
TEST_P(TiffLayouts, LoadsThePicture){
  std::unique_ptr<Image> img(ImageLoader::load(("tiff/" + GetParam() + ".tif").c_str()));
  ASSERT_NE(img, nullptr);
  expectPicture(*img);
}

INSTANTIATE_TEST_SUITE_P(Images, TiffLayouts, ::testing::Values("rgb", "rgb_be_lzw_predictor",
  "rgb_packbits_planar", "rgb_tiled", "palette4", "gdiplus"));

TEST(Images, TiffWithAlpha){
  //unassociated alpha, Deflate with the predictor (how Photoshop writes RGBA)
  std::unique_ptr<Image> img(ImageLoader::load("tiff/rgba_deflate_predictor.tif"));
  ASSERT_NE(img, nullptr);
  ASSERT_EQ(img->getNumChannels(), 4u);
  ASSERT_EQ(img->getWidth(), 3u);
  ASSERT_EQ(img->getHeight(), 2u);
  for (int y = 0; y < 2; ++y){
    for (int x = 0; x < 3; ++x){
      const unsigned char* p = img->getData() + y*img->getRowSpan() + x*4;
      EXPECT_EQ(p[0], picture[y][x].r) << "pixel " << x << "," << y;
      EXPECT_EQ(p[1], picture[y][x].g) << "pixel " << x << "," << y;
      EXPECT_EQ(p[2], picture[y][x].b) << "pixel " << x << "," << y;
      EXPECT_EQ(p[3], 255) << "pixel " << x << "," << y;
    }
  }
}

TEST(Images, TiffPremultipliedAlphaBecomesStraight){
  std::unique_ptr<Image> img(ImageLoader::load("tiff/premultiplied.tif"));
  ASSERT_NE(img, nullptr);
  ASSERT_EQ(img->getNumChannels(), 4u);
  const unsigned char* p = img->getData();
  EXPECT_EQ(p[0], 255);
  EXPECT_EQ(p[1], 0);
  EXPECT_EQ(p[3], 128);
  //fully transparent stays black
  EXPECT_EQ(p[4], 0);
  EXPECT_EQ(p[7], 0);
}

TEST(Images, TiffGray){
  //16 bits, big-endian, Deflate with the predictor: the high bytes
  std::unique_ptr<Image> gray(ImageLoader::load("tiff/gray16.tif"));
  ASSERT_NE(gray, nullptr);
  ASSERT_EQ(gray->getNumChannels(), 1u);
  const unsigned char expected[6] = {0x00, 0x80, 0xFF, 0x12, 0xFE, 0x40};
  for (int i = 0; i < 6; ++i)
    EXPECT_EQ(gray->getData()[i], expected[i]) << "pixel " << i;
  //1 bit with 0 as white
  std::unique_ptr<Image> bilevel(ImageLoader::load("tiff/bilevel.tif"));
  ASSERT_NE(bilevel, nullptr);
  ASSERT_EQ(bilevel->getNumChannels(), 1u);
  const unsigned char bits[6] = {0, 255, 0, 255, 0, 255};
  for (int i = 0; i < 6; ++i)
    EXPECT_EQ(bilevel->getData()[i], bits[i]) << "pixel " << i;
}

TEST(Images, TiffLzwWithWideCodes){
  //64x64 bytes: the LZW codes grow to 10 and 11 bits
  std::unique_ptr<Image> img(ImageLoader::load("tiff/gradient.tif"));
  ASSERT_NE(img, nullptr);
  ASSERT_EQ(img->getWidth(), 64u);
  ASSERT_EQ(img->getHeight(), 64u);
  ASSERT_EQ(img->getNumChannels(), 1u);
  int wrong = 0;
  for (int y = 0; y < 64; ++y){
    for (int x = 0; x < 64; ++x){
      int value = ((x / 3) * 7 + y * 13 + (x * y) % 5) & 0xFF;
      if (img->getData()[y*64 + x] != value)
        ++wrong;
    }
  }
  EXPECT_EQ(wrong, 0);
}

TEST(Images, TiffBrokenOrUnsupportedGivesNull){
  std::vector<uint8> data;
  ASSERT_TRUE(CGE::Assets::load("tiff/rgb.tif", data));
  //cut off in the directory
  EXPECT_EQ(ImageLoader::load(data.data(), 20, ImageLoader::TIFF), nullptr);
  //JPEG compression (7) isn't supported: patch the compression tag's value
  std::vector<uint8> jpeg = data;
  for (size_t i = 0; i + 12 <= jpeg.size(); ++i){
    if (jpeg[i] == 0x03 && jpeg[i+1] == 0x01 && jpeg[i+2] == 3 && jpeg[i+3] == 0){
      jpeg[i+8] = 7;
      break;
    }
  }
  EXPECT_EQ(ImageLoader::load(jpeg.data(), (unsigned)jpeg.size(), ImageLoader::TIFF), nullptr);
  //not a TIFF
  std::vector<uint8> png;
  ASSERT_TRUE(CGE::Assets::load("image.png", png));
  EXPECT_EQ(ImageLoader::load(png.data(), (unsigned)png.size(), ImageLoader::TIFF), nullptr);
}

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
  EXPECT_EQ(ImageLoader::determineType("b.tif"), ImageLoader::TIFF);
  EXPECT_EQ(ImageLoader::determineType("b.TIFF"), ImageLoader::TIFF);
  EXPECT_EQ(ImageLoader::determineType("b.txt"), ImageLoader::UNKNOWN);
}

TEST(Images, MissingOrUnknownFilesGiveNull){
  EXPECT_EQ(ImageLoader::load("missing.png"), nullptr);
  EXPECT_EQ(ImageLoader::load("folder/a.txt"), nullptr);
}
