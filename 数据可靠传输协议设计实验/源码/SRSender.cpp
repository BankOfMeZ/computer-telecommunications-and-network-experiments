#pragma once
#include "stdafx.h"
#include "SRSender.h"
#include "Global.h"

SRSender::SRSender(int winSize, int bNum) :base(0), nextSeqNum(0), windowSize(winSize),
bitsNum(bNum), buffer(new Packet[bNum]), isBufACK(new bool[bNum])
{
	for (int i = 0; i < bNum; i++)
	{
		isBufACK[i] = false;
	}
}

SRSender::~SRSender()
{
	delete[] buffer;
	delete[] isBufACK;
}

//����Ӧ�ò�������Message����NetworkService����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ�ȷ��״̬���ʹ����������ܾ�����Message���򷵻�false
bool SRSender::send(const Message& message)
{
	if (getWaitingState())
	{//���ʹ����ڱ��Ķη��꣬�ȴ�ACK
		printf("\n(Sender::send)���ʹ����ڱ��Ķη��꣬�ȴ�ACK\n");
		return false;
	}
	else
	{
		buffer[nextSeqNum].acknum = -1;
		buffer[nextSeqNum].seqnum = nextSeqNum;
		memcpy(buffer[nextSeqNum].payload, message.data, sizeof(message.data));
		buffer[nextSeqNum].checksum = pUtils->calculateCheckSum(buffer[nextSeqNum]);
		pUtils->printPacket("\n(Sender::send)���ͷ����ģ�\n", buffer[nextSeqNum]);
		printf("\n(Sender::send)����ǰ���ڣ�\n");
		printWindow();
		pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqNum);
		pns->sendToNetworkLayer(RECEIVER, buffer[nextSeqNum]);
		nextSeqNum = (nextSeqNum + 1) % bitsNum;
		printf("\n(Sender::send)���ͺ󴰿ڣ�\n");
		printWindow();
		return true;
	}
}

//����ȷ��Ack������NetworkService����
void SRSender::receive(const Packet& ackPkt)
{
	if (pUtils->calculateCheckSum(ackPkt) == ackPkt.checksum)
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
		if (isInWindow(ackPkt.acknum))
		{//���´���
			printf("\n(Sender::receive)���յ�ACKǰ�Ĵ��ڣ�\n");
			printWindow();
			isBufACK[ackPkt.acknum] = true;
			while (isBufACK[base] == true)
			{//�ƶ�����
				isBufACK[base] = false;
				base = (base + 1) % bitsNum;
			}
			printf("\n(Sender::receive)���յ�ACK��Ĵ��ڣ�\n");
			printWindow();
		}
	}
	else
	{
		pUtils->printPacket("\n(Sender::receive)ACK������\n", ackPkt);
	}
}

//Timeout handler������NetworkService����
void SRSender::timeoutHandler(int seqNum)
{
	printf("\n(Sender::timeoutHandler)���ͳ�ʱ\n");
	pns->stopTimer(SENDER, seqNum);
	pns->sendToNetworkLayer(RECEIVER, buffer[seqNum]);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	pUtils->printPacket("\n(Sender::timeoutHandler)�ش����飺\n", buffer[seqNum]);
	printf("\n(Sender::timeoutHandler)�ش�����%d���\n", seqNum);
}

//����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true
bool SRSender::getWaitingState()
{
	return (base + windowSize) % bitsNum == nextSeqNum % bitsNum;
}

//��ӡ��������
void SRSender::printWindow()
{
	for (int i = 0; i < bitsNum; i++)
	{
		if (i == base)
		{
			printf("[");
		}
		if (isInWindow(i))
		{
			if (i == nextSeqNum)
			{
				printf("->%d ", i);
			}
			else if (i < nextSeqNum)
			{
				if (isBufACK[i] == true)
				{
					printf("%dACKed ", i);
				}
				else
				{
					printf("%dnotACK ", i);
				}
			}
			else
			{
				printf("%d ", i);
			}
		}
		else
		{
			printf("%d������ ", i);
		}
		if (i == (base + windowSize - 1) % bitsNum)
		{
			printf("]");
		}
	}
	printf("\n");
}

//�жϷ����Ƿ��ڴ�����
bool SRSender::isInWindow(int seqNum)
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
