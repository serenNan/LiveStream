#pragma once
#include "mmedia/base/MMediaHandler.h"

namespace tmms
{
    namespace mm
    {
        /**
         * @brief RTMP 协议处理器类
         *
         * 该类继承自 MMediaHandler，用于处理 RTMP 协议相关的操作，
         * 包括播放、发布、暂停、快进/回退等功能。
         */
        class RtmpHandler : public MMediaHandler
        {
          public:
            /**
             * @brief 处理 RTMP 播放请求
             *
             * @param conn TCP 连接对象指针
             * @param session_name 会话名称
             * @param param 播放参数
             * @return bool 处理成功返回 true，否则返回 false
             */
            virtual bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name,
                                const std::string &param)
            {
                // 默认返回 false，表示未处理
                return false;
            }

            /**
             * @brief 处理 RTMP 发布请求
             *
             * @param conn TCP 连接对象指针
             * @param session_name 会话名称
             * @param param 发布参数
             * @return bool 处理成功返回 true，否则返回 false
             */
            virtual bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name,
                                   const std::string &param)
            {
                // 默认返回 false，表示未处理
                return false;
            }

            /**
             * @brief 处理 RTMP 暂停请求
             *
             * @param conn TCP 连接对象指针
             * @param pause true 表示暂停，false 表示继续播放
             */
            virtual void OnPause(const TcpConnectionPtr &conn, bool pause)
            {
            }

            /**
             * @brief 处理 RTMP 快进/回退请求
             *
             * @param conn TCP 连接对象指针
             * @param time 目标时间点（单位：秒）
             */
            virtual void OnSeek(const TcpConnectionPtr &conn, double time)
            {
            }

            /**
             * @brief 处理 RTMP 发布准备请求
             *
             * @param conn TCP 连接对象指针
             */
            virtual void OnPublishPrepare(const TcpConnectionPtr &conn)
            {
            }
        };
    } // namespace mm
} // namespace tmms