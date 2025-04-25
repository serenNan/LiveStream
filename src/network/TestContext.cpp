#include "TestContext.h"
#include <iostream>

using namespace tmms::network;

TestContext::TestContext(const TcpConnectionPtr &con) : connection_(con)
{
}

int TestContext::ParseMessage(MsgBuffer &buff)
{
    while (buff.ReadableBytes() > 0)
    {
        if (state_ == kTestContextHeader)
        {
            if (buff.ReadableBytes() >= 4)
            {
                message_length_ = buff.ReadInt32();
                state_ = kTestContextBody;
                continue;
            }
            else
            {
                return 1;
            }
        }
        else if (state_ == kTestContextBody)
        {
            std::cout << "message_length_: " << message_length_ << std::endl;
            if (buff.ReadableBytes() >= message_length_)
            {
                std::string tmp;
                tmp.assign(buff.Peek(), message_length_);
                message_body_.append(tmp);
                buff.Retrieve(message_length_);
                message_length_ = 0;

                if (cb_)
                {
                    cb_(connection_, message_body_);
                    message_body_.clear();
                }

                state_ = kTestContextHeader;
            }
        }
    }

    return 1;
}

void TestContext::SetTestMessageCallback(const TestMessageCallback &cb)
{
    cb_ = cb;
}

void TestContext::SetTestMessageCallback(TestMessageCallback &&cb)
{
    cb_ = std::move(cb);
}