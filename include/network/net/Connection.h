#pragma once

#include "Event.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>

namespace tmms
{
    namespace network
    {

        enum
        {
            kNormalContext = 0, ///< 普通连接上下文，默认类型
            kRtmpContext = 1,   ///< RTMP协议连接上下文
            kHttpContext = 2,   ///< HTTP协议连接上下文
            kUserContext = 3,   ///< 用户自定义上下文
            kFlvContext = 4,    ///< FLV协议连接上下文
        };
        using ContextPtr =
            std::shared_ptr<void>; ///< 上下文智能指针类型，用于存储任意类型的上下文对象
        class Connection; ///< Connection类前向声明
        using ConnectionPtr =
            std::shared_ptr<Connection>; ///< Connection智能指针类型，用于管理Connection对象生命周期
        using ActiveCallback = std::function<void(const ConnectionPtr &)>;
        struct BufferNode
        {
            BufferNode(void *buf, size_t s) : addr(buf), size(s)
            {
            }
            void *addr;
            size_t size{0};
        };
        using BufferNodePtr = std::shared_ptr<BufferNode>;
        /**
         * @brief 网络连接基类，继承自Event
         * 封装了网络连接的基本操作和状态管理
         */
        class Connection : public Event
        {
          public:
            /**
             * @brief 构造函数
             * @param loop 所属的事件循环
             * @param fd 连接的文件描述符
             * @param localAddr 本地地址
             * @param peerAddr 对端地址
             */
            Connection(EventLoop *loop, int fd, const InetAddress &localAddr,
                       const InetAddress &peerAddr);
            virtual ~Connection() = default;

            /**
             * @brief 设置本地地址
             * @param local 新的本地地址
             */
            void SetLocalAddr(const InetAddress &local);

            /**
             * @brief 设置对端地址
             * @param peer 新的对端地址
             */
            void SetPeerAddr(const InetAddress &peer);

            /**
             * @brief 获取本地地址
             * @return const InetAddress& 本地地址引用
             */
            const InetAddress &LocalAddr() const;

            /**
             * @brief 获取对端地址
             * @return const InetAddress& 对端地址引用
             */
            const InetAddress &PeerAddr() const;

            /**
             * @brief 设置连接上下文(左值引用版本)
             * @param type 上下文类型
             * @param context 上下文对象
             */
            void SetContext(int type, const std::shared_ptr<void> &context);

            /**
             * @brief 设置连接上下文(右值引用版本)
             * @param type 上下文类型
             * @param context 上下文对象
             */
            void SetContext(int type, std::shared_ptr<void> &&context);

            /**
             * @brief 获取指定类型的连接上下文
             * @tparam T 上下文类型
             * @param type 上下文类型枚举值
             * @return std::shared_ptr<T> 上下文对象指针，如果不存在返回nullptr
             */
            template <typename T> std::shared_ptr<T> GetContext(int type) const
            {
                auto iter = contexts_.find(type);
                if (iter != contexts_.end())
                {
                    return std::dynamic_pointer_cast<T>(iter->second);
                }
                return std::shared_ptr<T>();
            }
            /**
             * @brief 清除指定类型的上下文
             * @param type 上下文类型
             */
            void ClearContext(int type);

            /**
             * @brief 清除所有上下文
             */
            void ClearContext();

            /**
             * @brief 设置活动回调
             * @param cb 回调函数
             */
            void setActiveCallback(const ActiveCallback &cb);

            /**
             * @brief 设置活动回调(右值引用版本)
             * @param cb 回调函数
             */
            void setActiveCallback(ActiveCallback &&cb);

            /**
             * @brief 激活连接
             */
            void Active();

            /**
             * @brief 取消激活连接
             */
            void Deactive();

            /**
             * @brief 强制关闭连接(纯虚函数，由子类实现)
             */
            virtual void ForceClose() = 0;

          protected:
            InetAddress local_addr_; ///< 本地地址
            InetAddress peer_addr_;  ///< 对端地址

          private:
            std::unordered_map<int, ContextPtr> contexts_; ///< 上下文存储映射表
            ActiveCallback active_cb_;                     ///< 活动状态回调函数
            std::atomic<bool> active_{false};              ///< 连接是否处于活动状态
        };
    } // namespace network
} // namespace tmms