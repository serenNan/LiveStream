#include "gtest/gtest.h"
#include <fstream>
#include <json/json.h>
#include <string>

TEST(TestJsonCpp, BasicParsing)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::ifstream config_file("../bin/config/config.json");
    std::string errs;

    bool parsingSuccessful = Json::parseFromStream(builder, config_file, &root, &errs);
    ASSERT_FALSE(parsingSuccessful) << "Failed to parse JSON: " << errs;

    EXPECT_TRUE(root.isMember("log_info")) << "Missing log_info in config";

    Json::Value logInfo = root["log_info"];
    EXPECT_TRUE(logInfo.isMember("level")) << "Missing log level";
    EXPECT_TRUE(logInfo.isMember("path")) << "Missing log path";
    EXPECT_TRUE(logInfo.isMember("name")) << "Missing log name";
}

TEST(TestJsonCpp, BasicGeneration)
{
    Json::Value root;
    root["test_key"] = "test_value";

    Json::StreamWriterBuilder writer;
    std::string jsonString = Json::writeString(writer, root);

    EXPECT_FALSE(jsonString.empty());
    EXPECT_NE(jsonString.find("test_key"), std::string::npos);
    EXPECT_NE(jsonString.find("test_value"), std::string::npos);
}