#pragma once
#include "Event.h"
#include <memory>
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>
namespace tmms
{
    namespace network
    {
        using EventPtr = std::shared_ptr<Event>;
        /**
         * @brief 事件循环类
         * 基于epoll实现的事件循环，负责管理IO事件的分发和处理
         */
        class EventLoop
        {
          public:
            /**
             * @brief 构造函数，初始化epoll实例
             * @throws std::runtime_error 如果epoll创建失败
             */
            EventLoop();

            /**
             * @brief 析构函数，关闭epoll实例
             */
            ~EventLoop();

            /**
             * @brief 开始事件循环
             * 进入事件循环，持续监听和处理IO事件
             */
            void Loop();

            /**
             * @brief 停止事件循环
             * 设置停止标志，将在下一次事件循环时退出
             */
            void Quit();

            /**
             * @brief 添加事件
             * @param event 要添加的事件
             * @throws std::runtime_error 如果epoll_ctl添加失败
             */
            void AddEvent(const EventPtr &event);

            /**
             * @brief 删除事件
             * @param event 要删除的事件
             * @throws std::runtime_error 如果epoll_ctl删除失败
             */
            void DelEvent(const EventPtr &event);

            /**
             * @brief 允许或禁止事件的读取
             * @param event 要更新的事件
             * @param enable 是否启用事件
             * @return 是否更新成功
             * @throws std::runtime_error 如果epoll_ctl修改失败
             */
            bool EnableEventReading(const EventPtr &event, bool enable);

            /**
             * @brief 允许或禁止事件的写入
             * @param event 要更新的事件
             * @param enable 是否启用事件
             * @return 是否更新成功
             * @throws std::runtime_error 如果epoll_ctl修改失败
             */
            bool EnableEventWriting(const EventPtr &event, bool enable);

          private:
            bool lopping_{false};                          ///< 循环运行标志
            int epoll_fd_{-1};                             ///< epoll文件描述符
            std::vector<struct epoll_event> epoll_events_; ///< 存储epoll事件的容器
            std::unordered_map<int, EventPtr> events_;     ///< 存储事件的映射表
        };
    } // namespace network
} // namespace tmms