#include "stdafx.h"
#include "RdtReceiver.h"
#include "Global.h"

class SRReceiver :public RdtReceiver
{
	int base;//基序号
	int windowSize;//接收窗口大小
	int bitsNum;//报文序号大小
	Packet lastAckPkt;
	Packet* buffer;//报文缓存
	bool* isBufReceived;//报文缓存状态
	bool isInWindow(int seqNum);//判断分组是否在窗口内
	void printWindow();//打印滑动窗口
public:
	SRReceiver(int windowSize, int bitsNum);
	~SRReceiver();
	void receive(const Packet& packet);
};
