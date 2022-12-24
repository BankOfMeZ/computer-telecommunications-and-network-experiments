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

//判断分组是否在窗口内
bool SRReceiver::isInWindow(int seqNum)
{
	if (base < (base + windowSize) % bitsNum)
	{//窗口内分组序号为连续增序
		return seqNum >= base && seqNum < (base + windowSize) % bitsNum;
	}
	else
	{
		return seqNum >= base || seqNum < (base + windowSize) % bitsNum;
	}
}

//打印滑动窗口
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
			printf("%d不可用 ", i);
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
		pUtils->printPacket("\n(Receiver::receive)接收分组损坏：\n", packet);
	}
	else if (isInWindow(packet.seqnum))
	{//接收期待的分组
		buffer[packet.seqnum] = packet;
		isBufReceived[packet.seqnum] = true;
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("\n(Receiver::receive)接收方发送ACK：\n", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
		printf("\n(Receiver::receive)接受分组前窗口：\n");
		printWindow();
		while (isBufReceived[base])
		{//移动窗口
			Message message;
			memcpy(message.data, buffer[base].payload, sizeof(buffer[base].payload));
			pns->delivertoAppLayer(RECEIVER, message);
			pUtils->printPacket("\n(Receiver::receive)分组提交给应用层：\n", buffer[base]);
			isBufReceived[base] = false;
			base = (base + 1) % bitsNum;
		}
		printf("\n(Receiver::receive)接受分组后窗口：\n");
		printWindow();
	}
	else
	{//不接受不期望的分组
		pUtils->printPacket("\n(Receiver::receive)不是期望的分组：\n", packet);
		//但在此情况下依然要产生一个ACK
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
		pUtils->printPacket("\n(Receiver::receive)接收方发送不是期望分组的ACK：\n", lastAckPkt);
	}
}
