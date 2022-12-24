#include "stdafx.h"
#include "RdtSender.h"
#include "Global.h"

class GBNSender :public RdtSender
{
	int base;//基序号
	int nextSeqNum;//下一个序号
	int windowSize;//窗口长度
	int bitsNum;//报文序号的大小
	Packet* buffer;//报文缓存
	void printWindow();//打印滑动窗口
public:
	GBNSender(int windowSize, int bitsNum);
	~GBNSender();
	bool send(const Message& message);//发送应用层下来的Message，由NetworkService调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待确认状态或发送窗口已满而拒绝发送Message，则返回false
	void receive(const Packet& ackPkt);//接受确认Ack，将被NetworkService调用
	void timeoutHandler(int seqNum);//Timeout handler，将被NetworkService调用
	bool getWaitingState();//返回RdtSender是否处于等待状态，如果发送方正等待确认或者发送窗口已满，返回true
};
