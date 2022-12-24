#include "stdafx.h"
#include "RdtReceiver.h"
#include "Global.h"

class SRReceiver :public RdtReceiver
{
	int base;//�����
	int windowSize;//���մ��ڴ�С
	int bitsNum;//������Ŵ�С
	Packet lastAckPkt;
	Packet* buffer;//���Ļ���
	bool* isBufReceived;//���Ļ���״̬
	bool isInWindow(int seqNum);//�жϷ����Ƿ��ڴ�����
	void printWindow();//��ӡ��������
public:
	SRReceiver(int windowSize, int bitsNum);
	~SRReceiver();
	void receive(const Packet& packet);
};
