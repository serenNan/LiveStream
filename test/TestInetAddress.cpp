#include "network/base/InetAddress.h"
#include <gtest/gtest.h>

using namespace tmms::network;

TEST(TestInetAddress, IPv4Parsing)
{
    InetAddress addr("192.168.1.1");
    EXPECT_EQ(addr.IP(), "192.168.1.1");
    EXPECT_TRUE(addr.IsLanIp());
    EXPECT_FALSE(addr.IsWanIp());
    EXPECT_FALSE(addr.IsLoopbackIp());
}

TEST(TestInetAddress, IPv6Parsing)
{
    InetAddress addr("172.1.1.1");
    EXPECT_EQ(addr.IP(), "172.1.1.1");
    EXPECT_FALSE(addr.IsLanIp());
    EXPECT_TRUE(addr.IsWanIp());
    EXPECT_FALSE(addr.IsLoopbackIp());
}

TEST(TestInetAddress, Loopback)
{
    InetAddress addr("127.0.0.1");
    EXPECT_TRUE(addr.IsLoopbackIp());
    EXPECT_FALSE(addr.IsWanIp());
}

TEST(TestInetAddress, PortHandling)
{
    // 测试普通IPv4地址带端口
    InetAddress addr1("192.168.1.1:8080");
    EXPECT_EQ(addr1.Port(), 8080);
    
    // 测试IPv6地址带端口（如果有需要）
    InetAddress addr2("[2001:db8::1]:8080");
    EXPECT_EQ(addr2.Port(), 8080);
    
    // 测试无端口情况
    InetAddress addr3("192.168.1.1");
    EXPECT_EQ(addr3.Port(), 0);  // 假设无端口时返回0
}