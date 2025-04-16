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
    InetAddress addr("192.168.1.1:8080");
    EXPECT_EQ(addr.Port(), 8080);
}