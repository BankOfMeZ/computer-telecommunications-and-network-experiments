#pragma once
#include "stdafx.h"
#include "GBNSender.h"
#include "Global.h"

GBNSender::GBNSender(int winSize, int bNum) :base(0), nextSeqNum(0), windowSize(winSize), 
bitsNum(bNum), buffer(new Packet[bNum])
{

}

GBNSender::~GBNSender()
{
	delete[] buffer;
}

//����Ӧ�ò�������Message����NetworkService����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ�ȷ��״̬���ʹ����������ܾ�����Message���򷵻�false
bool GBNSender::send(const Message& message)
{
	if (getWaitingState())
	{//���ʹ����ڱ��Ķη��꣬�ȴ�ACK
		printf("\n(Sender::send)���ʹ����ڱ��Ķη��꣬�ȴ�ACK\n");
		return false;
	}
	else
	{
		buffer[nextSeqNum].acknum = -1;//���Ը��ֶ�
		buffer[nextSeqNum].seqnum = nextSeqNum;
		memcpy(buffer[nextSeqNum].payload, message.data, sizeof(message.data));
		buffer[nextSeqNum].checksum = pUtils->calculateCheckSum(buffer[nextSeqNum]);
		pUtils->printPacket("\n(Sender::send)���ͷ����ģ�\n", buffer[nextSeqNum]);
		printf("\n(Sender::send)����ǰ���ڣ�\n");
		printWindow();
		if (base == nextSeqNum)
		{
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		pns->sendToNetworkLayer(RECEIVER, buffer[nextSeqNum]);
		nextSeqNum = (nextSeqNum + 1) % bitsNum;
		printf("\n(Sender::send)���ͺ󴰿ڣ�\n");
		printWindow();
		return true;
	}
}

//����ȷ��Ack������NetworkService����
void GBNSender::receive(const Packet& ackPkt)
{
	if (pUtils->calculateCheckSum(ackPkt) != ackPkt.checksum)
	{
		pUtils->printPacket("\n(Sender::receive)ACK������\n", ackPkt);
	}
	else
	{
		printf("\n(Sender::receive)���յ�ACKǰ�Ĵ��ڣ�\n");
		printWindow();
		base = (ackPkt.acknum + 1) % bitsNum;//GBNΪ�ۼ�ȷ��
		if (base == nextSeqNum)
		{//���͵�ȫ���յ�
			pns->stopTimer(SENDER, 0);
		}
		else
		{//����Ÿı䣬������ʱ��
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		printf("\n(Sender::receive)���յ�ACK��Ĵ��ڣ�\n");
		printWindow();
	}
}

//Timeout handler������NetworkService����
void GBNSender::timeoutHandler(int seqNum)
{
	printf("\n(Sender::timeoutHandler)���ͳ�ʱ\n");
	if (nextSeqNum != base)
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		for (int i = base; i != nextSeqNum; i = (i + 1) % bitsNum)
		{//��ʱ�ش�
			pns->sendToNetworkLayer(RECEIVER, buffer[i]);
			pUtils->printPacket("\n(Sender::timeoutHandler)�ش����飺\n", buffer[i]);
		}
	}
	printf("\n(Sender::timeoutHandler)�ش����\n");
}

//����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true
bool GBNSender::getWaitingState()
{
	return (base + windowSize) % bitsNum == nextSeqNum % bitsNum;
}

//��ӡ��������
void GBNSender::printWindow()
{
	for (int i = 0; i < bitsNum; i++)
	{
		if (i == base)
		{
			printf("[");
		}
		if (i == nextSeqNum)
		{
			printf("->");
		}
		printf("%d ", i);
		if (i == (base + windowSize - 1) % bitsNum)
		{
			printf("]");
		}
	}
	printf("\n");
}
