#include "stdafx.h"
#include "RdtReceiver.h"
#include "Global.h"

class GBNReceiver :public RdtReceiver
{
	int expectedSeqNum;
	int bitsNum;//报文序号大小
	Packet lastAckPkt;
public:
	GBNReceiver(int bitsNum);
	void receive(const Packet& packet);
};
