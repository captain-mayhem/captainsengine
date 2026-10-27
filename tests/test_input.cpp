#include <gtest/gtest.h>

#include <input/touch.h>

using namespace Input;

// The touch screen's fingers (Input::Touch), without an engine: positions
// stay window pixels and nothing goes to an application or the mouse.

namespace{

class TouchTest : public ::testing::Test{
protected:
  void SetUp() override{
    Touch::init();
    touch_ = Touch::instance();
  }

  void TearDown() override{
    Touch::release();
  }

  Touch* touch_;
};

}

TEST_F(TouchTest, FingersAreTrackedByTheirIds){
  touch_->down(7, 10, 20);
  touch_->down(3, 30, 40);
  ASSERT_EQ(touch_->getFingers().size(), 2u);
  EXPECT_EQ(touch_->getFingers()[0].id, 7);
  EXPECT_EQ(touch_->getFingers()[1].id, 3);
  touch_->move(3, 35, 45);
  EXPECT_EQ(touch_->getFingers()[1].pos, CGE::Vec2i(35, 45));
  EXPECT_EQ(touch_->getFingers()[0].pos, CGE::Vec2i(10, 20));
  touch_->up(7, 11, 21);
  EXPECT_FALSE(touch_->isDown(7));
  EXPECT_TRUE(touch_->isDown(3));
  //unknown fingers are ignored
  touch_->move(5, 1, 1);
  touch_->up(5, 1, 1);
  touch_->cancel(5);
  EXPECT_EQ(touch_->getFingers().size(), 1u);
  touch_->cancel(3);
  EXPECT_TRUE(touch_->getFingers().empty());
}

TEST_F(TouchTest, ThePrimaryFingerIsTheFirstOnAnEmptyScreen){
  EXPECT_EQ(touch_->getPrimary(), -1);
  touch_->down(1, 0, 0);
  touch_->down(2, 0, 0);
  EXPECT_EQ(touch_->getPrimary(), 1);
  //no new primary finger while others are still down
  touch_->up(1, 0, 0);
  EXPECT_EQ(touch_->getPrimary(), -1);
  touch_->down(3, 0, 0);
  EXPECT_EQ(touch_->getPrimary(), -1);
  touch_->cancelAll();
  EXPECT_TRUE(touch_->getFingers().empty());
  touch_->down(3, 0, 0);
  EXPECT_EQ(touch_->getPrimary(), 3);
}

TEST_F(TouchTest, AFingerDownAgainReplacesItself){
  touch_->down(4, 1, 2);
  touch_->down(4, 5, 6);
  ASSERT_EQ(touch_->getFingers().size(), 1u);
  EXPECT_EQ(touch_->getFingers()[0].pos, CGE::Vec2i(5, 6));
  EXPECT_EQ(touch_->getPrimary(), 4);
}

TEST_F(TouchTest, MouseEmulationIsOnByDefault){
  EXPECT_TRUE(touch_->isMouseEmulation());
  touch_->setMouseEmulation(false);
  EXPECT_FALSE(touch_->isMouseEmulation());
}
