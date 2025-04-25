#include "Packet.h"

using namespace tmms::mm;

PacketPtr Packet::NewPacket(int32_t size)
{
    auto block_size = size + sizeof(Packet);
    
    // 分配内存并初始化
    Packet *packet = (Packet *)new char[block_size];
    memset((void *)packet, 0x00, block_size);
    
    // 设置包的基本属性
    packet->index_ = -1;
    packet->type_ = kPacketTypeUnknowed;
    packet->capacity_ = size;
    packet->ext_.reset();
    
    // 返回带自定义删除器的智能指针
    return PacketPtr(packet, [](Packet *p) {
        delete[] (char *)p;
    });
}