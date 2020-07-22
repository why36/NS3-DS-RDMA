
#include <iomanip>
#include <iostream>
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ib-header.h"

NS_LOG_COMPONENT_DEFINE("IBHeader");

namespace ns3
{

    NS_OBJECT_ENSURE_REGISTERED(IBHeader);

    OpCodeType OpCode::GetOpCodeType() //opcode[7-5]
    {
        return (OpCodeType)(data >> 5);
    }

    OpCodeOperation OpCode::GetOpCodeOperation() //opcode[4-0]
    {
        return (OpCodeOperation)(data & 0x1f);
    }

    TypeId IBHeader::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::IBHeader").SetParent<Header>().AddConstructor<IBHeader>();
        return tid;
    }

    TypeId IBHeader::GetInstanceTypeId(void) const
    {
        return GetTypeId();
    }

    ~IBHeader()
    {
    }

    OpCode IBHeader::&GetOpCode()
    {
        return m_opcode;
    }

    uint32_t IBHeader::GetSerializedSize(void) const //ib-header-opcode-byte
    {
        return 1;
    }

    void IBHeader::Serialize(Buffer::Iterator start) const
    {
        Buffer::Iterator i = start;

        i.WriteU8(i, data);
    }

    uint32_t IBHeader::Deserialize(Buffer::Iterator start)
    {
        Buffer::Iterator i = start;

        data = i.ReadU8();

        return GetSerializedSize();
    }

    void IBHeader::Print(std::ostream &os) const
    {
        os << "data=" << std::dec << data;
    }

} //namespace ns3

// TO DO GYY: implement this
