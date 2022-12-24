#pragma once
#include "stdafx.h"
#include "RdtSender.h"

class TCPSender :public RdtSender
{
	int base;//�����
	int nextSeqNum;//��һ�����
	int windowSize;//���ڴ�С
	int bitsNum;//������Ŵ�С
	Packet* buffer;//���黺��
	int dupACKNum;//�յ�����ACK��Ŀ
	void printWindow();//��ӡ��������
	bool isInWindow(int seqNum);//�жϷ����Ƿ��ڴ�����
public:
	TCPSender(int windowSize, int bitsNum);
	~TCPSender();
	bool send(const Message& message);//����Ӧ�ò�������Message����NetworkService����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ�ȷ��״̬���ʹ����������ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);//����ȷ��Ack������NetworkService����
	void timeoutHandler(int seqNum);//Timeout handler������NetworkService����
	bool getWaitingState();//����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true
};