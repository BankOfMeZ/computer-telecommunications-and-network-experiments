#pragma once
#include "stdafx.h"
#include "SRReceiver.h"
#include "Global.h"

SRReceiver::SRReceiver(int winSize, int bNum) :base(0), windowSize(winSize), bitsNum(bNum),
buffer(new Packet[bitsNum]), isBufReceived(new bool[bitsNum])
{
	for (int i = 0; i < bNum; i++)
	{
		isBufReceived[i] = false;
	}
	lastAckPkt.acknum = -1;
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
	{
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

SRReceiver::~SRReceiver()
{
	delete[] buffer;
	delete[] isBufReceived;
}

//�жϷ����Ƿ��ڴ�����
bool SRReceiver::isInWindow(int seqNum)
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

//��ӡ��������
void SRReceiver::printWindow()
{
	for (int i = 0; i < bitsNum; i++)
	{
		if (i == base)
		{
			printf("[");
		}
		if (isInWindow(i))
		{
			if (isBufReceived[i])
			{
				printf("%drcv ", i);
			}
			else
			{
				printf("%dexp ", i);
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

void SRReceiver::receive(const Packet& packet)
{
	if(pUtils->calculateCheckSum(packet) != packet.checksum)
	{
		pUtils->printPacket("\n(Receiver::receive)���շ����𻵣�\n", packet);
	}
	else if (isInWindow(packet.seqnum))
	{//�����ڴ��ķ���
		buffer[packet.seqnum] = packet;
		isBufReceived[packet.seqnum] = true;
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("\n(Receiver::receive)���շ�����ACK��\n", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
		printf("\n(Receiver::receive)���ܷ���ǰ���ڣ�\n");
		printWindow();
		while (isBufReceived[base])
		{//�ƶ�����
			Message message;
			memcpy(message.data, buffer[base].payload, sizeof(buffer[base].payload));
			pns->delivertoAppLayer(RECEIVER, message);
			pUtils->printPacket("\n(Receiver::receive)�����ύ��Ӧ�ò㣺\n", buffer[base]);
			isBufReceived[base] = false;
			base = (base + 1) % bitsNum;
		}
		printf("\n(Receiver::receive)���ܷ���󴰿ڣ�\n");
		printWindow();
	}
	else
	{//�����ܲ������ķ���
		pUtils->printPacket("\n(Receiver::receive)���������ķ��飺\n", packet);
		//���ڴ��������ȻҪ����һ��ACK
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
		pUtils->printPacket("\n(Receiver::receive)���շ����Ͳ������������ACK��\n", lastAckPkt);
	}
}
