#include "stdafx.h"
#include "RdtReceiver.h"
#include "Global.h"

class GBNReceiver :public RdtReceiver
{
	int expectedSeqNum;
	int bitsNum;//������Ŵ�С
	Packet lastAckPkt;
public:
	GBNReceiver(int bitsNum);
	void receive(const Packet& packet);
};
