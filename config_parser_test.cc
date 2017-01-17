#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "config_parser.h"

// Basic test of a newly generated config file
TEST(NginxConfigTest, ToString) {
  NginxConfigStatement statement;
  statement.tokens_.push_back("foo");
  statement.tokens_.push_back("bar");
  EXPECT_EQ(statement.ToString(0), "foo bar;\n");
}

class NginxStringConfigTest : public ::testing::Test {
  protected:
    bool ParseString(const std::string config_string) {
      std::stringstream config_stream(config_string);
      return parser_.Parse(&config_stream, &out_config_);
    }
    NginxConfigParser parser_;
    NginxConfig out_config_;
};

TEST_F(NginxStringConfigTest, SimpleConfig) {
  bool success = parser_.Parse("example_config", &out_config_);
  EXPECT_TRUE(success) << "Failed to parse a basic config file";
}

TEST_F(NginxStringConfigTest, AnotherSimpleConfig) {
  EXPECT_TRUE(ParseString("foo bar;"));
  EXPECT_EQ(1, out_config_.statements_.size())
    << "Config has one statements";
  EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0));
}

TEST_F(NginxStringConfigTest, VhostsConfig) {
  bool success = parser_.Parse("./testconfigs/vhosts_config", &out_config_);
  EXPECT_TRUE(success) << "Failed to parse config file with separate blocks";
}

TEST_F(NginxStringConfigTest, ListenDefaultConfig) {
  bool success = parser_.Parse("./testconfigs/listen_default_config",
                               &out_config_);
  EXPECT_TRUE(success) << "Failed to parse basic config file";
}

TEST_F(NginxStringConfigTest, InvalidConfig) {
  EXPECT_FALSE(ParseString("foo bar"))
    << "Failed to detect unfinished statement";
}

TEST_F(NginxStringConfigTest, NestedConfig) {
  EXPECT_TRUE(parser_.Parse("./testconfigs/nested_config", &out_config_))
    << "Failed to parse config file with singly nested blocks";
}

TEST_F(NginxStringConfigTest, DoubleNestedConfig) {
  EXPECT_TRUE(parser_.Parse("./testconfigs/double_nested_config", &out_config_))
    << "Failed to parse config file with doubly nested blocks";
}

TEST_F(NginxStringConfigTest, UnbalancedConfig) {
  EXPECT_FALSE(ParseString("server { listen 80; "))
    << "Failed to detect unclosed server block";
  EXPECT_FALSE(ParseString("server listen 80; }"))
    << "Failed to detect unopened server block";
}

TEST_F(NginxStringConfigTest, EmptyConfig) {
  EXPECT_FALSE(ParseString(""))
    << "Failed to detect empty config file";
  EXPECT_FALSE(ParseString("# im the only one in here"))
    << "Failed to parse single line comment";
  EXPECT_FALSE(ParseString("##\n"))
    << "Failed to parse commented pound symbol";
}

TEST_F(NginxStringConfigTest, OnlyCurlyConfig) {
  EXPECT_FALSE(ParseString("{}")) << "Failed to detect empty config file";
}

TEST_F(NginxStringConfigTest, NoBlockNameConfig) {
  EXPECT_FALSE(ParseString("{ listen 80; }"));
}

TEST_F(NginxStringConfigTest, CommentConfig) {
  EXPECT_TRUE(ParseString("# hello world\n server { listen 80; }"));
  EXPECT_TRUE(ParseString("server {\n\
                            # hello world\n\
                            listen 80;\n\
                          }"));
  EXPECT_TRUE(ParseString("server { # simple reverse-proxy\n\
                             listen 80;\n\
                           }"));
}

TEST_F(NginxStringConfigTest, FullConfig) {
  EXPECT_TRUE(parser_.Parse("./testconfigs/full_config", &out_config_))
    << "Failed to parse a complete example nginx config file";
}
