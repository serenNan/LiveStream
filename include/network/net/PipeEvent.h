#pragma once
#include "Event.h"
#include <memory>
namespace tmms
{
    namespace network
    {
        /**
         * @brief 管道事件类
         * 封装了管道读写事件的处理逻辑
         * 负责管理管道的读写操作和事件回调
         */
        class PipeEvent : public Event
        {
          public:
            /**
             * @brief 构造函数
             * @param loop 关联的事件循环
             * @note 初始化时会创建管道并设置非阻塞模式
             */
            PipeEvent(EventLoop *loop);
            
            /**
             * @brief 析构函数
             * 关闭管道文件描述符并清理资源
             */
            ~PipeEvent();

            /**
             * @brief 读事件回调
             * 当管道可读时被事件循环调用
             * @note 内部会处理数据读取和错误情况
             */
            void OnRead() override;
            
            /**
             * @brief 关闭事件回调
             * 当管道关闭时被事件循环调用
             * @note 会清理相关资源
             */
            void OnClose() override;
            
            /**
             * @brief 错误事件回调
             * @param message 错误信息
             * @note 会记录错误日志并尝试恢复
             */
            void OnError(const std::string &message) override;
            
            /**
             * @brief 写入数据到管道
             * @param data 要写入的数据指针
             * @param len 数据长度
             * @note 线程安全，支持多线程调用
             * @return 实际写入的字节数，-1表示错误
             */
            void Write(const char *data, size_t len);

          private:
            int write_fd_{-1}; ///< 管道写端文件描述符，-1表示未初始化
        };
        
        /// 智能指针类型定义
        using PipeEventPtr = std::shared_ptr<PipeEvent>;
    }
}