#pragma once
#include "stdafx.h"
#include "TCPSender.h"
#include "Global.h"

TCPSender::TCPSender(int winSize, int bNum) :base(0), nextSeqNum(0), windowSize(winSize), bitsNum(bNum),
buffer(new Packet[bNum]), dupACKNum(0)
{

}

TCPSender::~TCPSender()
{
	delete[] buffer;
}

//��ӡ��������
void TCPSender::printWindow()
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

//�жϷ����Ƿ��ڴ�����
bool TCPSender::isInWindow(int seqNum)
{
	if (base < (base + windowSize) % bitsNum)
	{//�����ڷ������Ϊ��������
		return seqNum >= base && seqNum < (base + windowSize) % bitsNum;
	}
	else
	{
		return seqNum >= base || seqNum < (base + windowSize) % bitsNum;
	}
}

//����Ӧ�ò�������Message����NetworkService����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ�ȷ��״̬���ʹ����������ܾ�����Message���򷵻�false
bool TCPSender::send(const Message& message)
{
	if (getWaitingState())
	{
		printf("\n(Sender::send)���ʹ����ڱ��Ķη��꣬�ȴ�ACK\n");
		return false;
	}
	else
	{
		buffer[nextSeqNum].acknum = -1;
		buffer[nextSeqNum].seqnum = nextSeqNum;
		memcpy(buffer[nextSeqNum].payload, message.data, sizeof(message.data));
		buffer[nextSeqNum].checksum = pUtils->calculateCheckSum(buffer[nextSeqNum]);
		printf("\n(Sender::send)���ͷ���ǰ���ڣ�\n");
		printWindow();
		if (base == nextSeqNum)
		{//�ۼ�ȷ�ϣ���һ����ʱ��
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		pns->sendToNetworkLayer(RECEIVER, buffer[nextSeqNum]);
		pUtils->printPacket("\n(Sender::send)���ͱ��ģ�\n", buffer[nextSeqNum]);
		nextSeqNum = (nextSeqNum + 1) % bitsNum;
		printf("\n(Sender::send)���ͷ���󴰿ڣ�\n");
		printWindow();
		return true;
	}
}

//����ȷ��Ack������NetworkService����
void TCPSender::receive(const Packet& ackPkt)
{
	if (pUtils->calculateCheckSum(ackPkt) == ackPkt.checksum)
	{
		if (isInWindow(ackPkt.acknum))
		{
			pns->stopTimer(SENDER, 0);
			printf("\n(Sender::receive)����ACKǰ���ڣ�\n");
			printWindow();
			base = (ackPkt.acknum + 1) % bitsNum;
			if (base != nextSeqNum)
			{//����û��ACK�ķ��飬�������ö�ʱ��
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
			}
			printf("\n(Sender::receive)����ACK�󴰿ڣ�\n");
			printWindow();
			dupACKNum = 0;
		}
		else
		{//�յ�����ACK
			dupACKNum++;
			if (dupACKNum == 3)
			{
				dupACKNum = 0;
				pns->sendToNetworkLayer(RECEIVER, buffer[base]);
				pUtils->printPacket("\n(Sender::receive)�յ�3������ACK�������ش���\n", buffer[base]);
			}
		}
	}
	else
	{
		pUtils->printPacket("\n(Sender::receive)�յ���ACK�𻵣�\n", ackPkt);
	}
}

//Timeout handler������NetworkService����
void TCPSender::timeoutHandler(int seqNum)
{
	printf("\n(Sender::timeoutHandler)���ͳ�ʱ\n");
	//ֹͣ��ʱ�������¼�ʱ
	pns->stopTimer(SENDER, 0);
	pns->sendToNetworkLayer(RECEIVER, buffer[base]);
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
	pUtils->printPacket("\n(Sender::timeoutHandler)�ش����飺\n", buffer[base]);
}

//����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true
bool TCPSender::getWaitingState()
{
	return (base + windowSize) % bitsNum == nextSeqNum % bitsNum;
}
