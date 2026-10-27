#include <string>

#include <gtest/gtest.h>

#include <io/json.h>

using CGE::JsonValue;

TEST(Json, ParsesAllTypes){
  JsonValue v;
  std::string error;
  ASSERT_TRUE(JsonValue::parse(R"( {"n": null, "t": true, "f": false, "i": -12, "x": 2.5e2,
    "s": "text", "a": [1, [2], {}], "o": {"k": "v"}} )", v, &error)) << error;
  ASSERT_TRUE(v.isObject());
  EXPECT_EQ(v.size(), 8u);
  EXPECT_TRUE(v["n"].isNull());
  EXPECT_TRUE(v["n"].getBool(true));
  EXPECT_TRUE(v["t"].getBool());
  EXPECT_TRUE(v["f"].isBool());
  EXPECT_FALSE(v["f"].getBool(true));
  EXPECT_EQ(v["i"].getInt(), -12);
  EXPECT_DOUBLE_EQ(v["x"].getNumber(), 250);
  EXPECT_EQ(v["s"].getString(), "text");
  ASSERT_TRUE(v["a"].isArray());
  EXPECT_EQ(v["a"].size(), 3u);
  EXPECT_EQ(v["a"][1][0].getInt(), 2);
  EXPECT_TRUE(v["a"][2].isObject());
  EXPECT_EQ(v["o"]["k"].getString(), "v");
}

TEST(Json, MissingValuesAreNullAndGiveTheDefault){
  JsonValue v;
  ASSERT_TRUE(JsonValue::parse(R"({"a": [1], "s": "x"})", v));
  EXPECT_TRUE(v["missing"].isNull());
  EXPECT_TRUE(v["missing"]["deeper"][3].isNull());
  EXPECT_TRUE(v["a"][5].isNull());
  EXPECT_FALSE(v.has("missing"));
  EXPECT_TRUE(v.has("a"));
  EXPECT_EQ(v["s"].getInt(7), 7);
  EXPECT_FLOAT_EQ(v["missing"].getFloat(1.5f), 1.5f);
  EXPECT_TRUE(v["a"].getString().empty());
}

TEST(Json, KeepsTheMembersOrder){
  JsonValue v;
  ASSERT_TRUE(JsonValue::parse(R"({"z": 1, "a": 2, "m": 3})", v));
  ASSERT_EQ(v.getMembers().size(), 3u);
  EXPECT_EQ(v.getMembers()[0].first, "z");
  EXPECT_EQ(v.getMembers()[1].first, "a");
  EXPECT_EQ(v.getMembers()[2].first, "m");
}

TEST(Json, DecodesEscapesToUtf8){
  JsonValue v;
  ASSERT_TRUE(JsonValue::parse(R"(["a\"\\\/\n\t", "ä€", "😀"])", v));
  EXPECT_EQ(v[0].getString(), "a\"\\/\n\t");
  EXPECT_EQ(v[1].getString(), "\xC3\xA4\xE2\x82\xAC");
  EXPECT_EQ(v[2].getString(), "\xF0\x9F\x98\x80");
}

TEST(Json, NumbersDontDependOnTheLocale){
  JsonValue v;
  ASSERT_TRUE(JsonValue::parse("[0.5, -0, 1E3, 12]", v));
  EXPECT_DOUBLE_EQ(v[0].getNumber(), 0.5);
  EXPECT_DOUBLE_EQ(v[2].getNumber(), 1000);
  EXPECT_EQ(v[3].getInt(), 12);
}

TEST(Json, RefusesInvalidDocuments){
  const char* invalid[] = {
    "", "{", "[1,]", "{\"a\" 1}", "{a: 1}", "[01]", "[1.]", "\"open", "tru", "[1] 2", "'x'", "[\"\t\"]", "-",
  };
  for (const char* text : invalid){
    JsonValue v;
    std::string error;
    EXPECT_FALSE(JsonValue::parse(text, v, &error)) << text;
    EXPECT_FALSE(error.empty()) << text;
    EXPECT_TRUE(v.isNull()) << text;
  }
  //the message names the line
  JsonValue v;
  std::string error;
  EXPECT_FALSE(JsonValue::parse("{\n\"a\": 1,\n\"b\" 2}", v, &error));
  EXPECT_NE(error.find("line 3"), std::string::npos) << error;
}

TEST(Json, RefusesNestingTooDeep){
  std::string deep(1000, '[');
  deep += std::string(1000, ']');
  JsonValue v;
  EXPECT_FALSE(JsonValue::parse(deep, v));
  std::string ok(100, '[');
  ok += std::string(100, ']');
  EXPECT_TRUE(JsonValue::parse(ok, v));
}
