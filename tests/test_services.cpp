#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include <platform/services.h>

using CGE::Platform;

// The tests run without a window; what needs one (the clipboard, visible
// cursors) is checked in the applications.

TEST(Platform, TickClockAdvancesInMilliseconds){
  uint64_t t0 = Platform::getTickMs();
  std::this_thread::sleep_for(std::chrono::milliseconds(30));
  uint64_t t1 = Platform::getTickMs();
  EXPECT_GE(t1, t0 + 25);
  EXPECT_LT(t1, t0 + 2000);
}

TEST(Platform, CursorIsRememberedWithoutAWindow){
  Platform::setCursor(Platform::CURSOR_WAIT);
  EXPECT_EQ(Platform::getCursor(), Platform::CURSOR_WAIT);
  Platform::setCursor(Platform::CURSOR_ARROW);
  EXPECT_EQ(Platform::getCursor(), Platform::CURSOR_ARROW);
}

TEST(Platform, ClipboardNeedsAWindow){
  EXPECT_FALSE(Platform::setClipboardText("text"));
}
