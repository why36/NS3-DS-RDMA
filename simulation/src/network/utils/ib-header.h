#ifndef IB_HEADER_H
#define IB_HEADER_H

#include <stdint.h>

#include <cstdio>

#include "ns3/buffer.h"
#include "ns3/header.h"

namespace ns3 {

// Note : only RC/UC + send is supported (for OpCodeOperation)
enum class OpCodeType {
    RC = 0,
    UC = 1,
    /* RD = 2, UD = 3, CNP = 4, XRC = 5 */
};

enum class OpCodeOperation {
    SEND_FIRST = 0,
    SEND_MIDDLE = 1,
    SEND_LAST = 2,
    SEND_LAST_WITH_IMM = 3,
    SEND_ONLY = 4,
    SEND_ONLY_WITH_IMM = 5,
};

struct OpCode {
    uint8_t data = 0;
    OpCodeType GetOpCodeType();
    OpCodeOperation GetOpCodeOperation();
    void SetOpCodeType(OpCodeType opCodeType);
    void SetOpCodeOperation(OpCodeOperation opCodeOperation);
};

class IBHeader final : public Header {
   public:
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual ~IBHeader() override;
    virtual uint32_t GetSerializedSize(void) const override;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start) override;
    virtual void Print(std::ostream &os) const override;

    OpCode &GetOpCode();
    OpCode m_opcode;
};

}  // namespace ns3

#endif /* INT_HEADER_H */
