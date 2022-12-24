#include "stdafx.h"
#include "RdtSender.h"
#include "Global.h"

class GBNSender :public RdtSender
{
	int base;//�����
	int nextSeqNum;//��һ�����
	int windowSize;//���ڳ���
	int bitsNum;//������ŵĴ�С
	Packet* buffer;//���Ļ���
	void printWindow();//��ӡ��������
public:
	GBNSender(int windowSize, int bitsNum);
	~GBNSender();
	bool send(const Message& message);//����Ӧ�ò�������Message����NetworkService����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ�ȷ��״̬���ʹ����������ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);//����ȷ��Ack������NetworkService����
	void timeoutHandler(int seqNum);//Timeout handler������NetworkService����
	bool getWaitingState();//����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true
};
