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

//发送应用层下来的Message，由NetworkService调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待确认状态或发送窗口已满而拒绝发送Message，则返回false
bool SRSender::send(const Message& message)
{
	if (getWaitingState())
	{//发送窗口内报文段发完，等待ACK
		printf("\n(Sender::send)发送窗口内报文段发完，等待ACK\n");
		return false;
	}
	else
	{
		buffer[nextSeqNum].acknum = -1;
		buffer[nextSeqNum].seqnum = nextSeqNum;
		memcpy(buffer[nextSeqNum].payload, message.data, sizeof(message.data));
		buffer[nextSeqNum].checksum = pUtils->calculateCheckSum(buffer[nextSeqNum]);
		pUtils->printPacket("\n(Sender::send)发送方报文：\n", buffer[nextSeqNum]);
		printf("\n(Sender::send)发送前窗口：\n");
		printWindow();
		pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqNum);
		pns->sendToNetworkLayer(RECEIVER, buffer[nextSeqNum]);
		nextSeqNum = (nextSeqNum + 1) % bitsNum;
		printf("\n(Sender::send)发送后窗口：\n");
		printWindow();
		return true;
	}
}

//接受确认Ack，将被NetworkService调用
void SRSender::receive(const Packet& ackPkt)
{
	if (pUtils->calculateCheckSum(ackPkt) == ackPkt.checksum)
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
		if (isInWindow(ackPkt.acknum))
		{//更新窗口
			printf("\n(Sender::receive)接收到ACK前的窗口：\n");
			printWindow();
			isBufACK[ackPkt.acknum] = true;
			while (isBufACK[base] == true)
			{//移动窗口
				isBufACK[base] = false;
				base = (base + 1) % bitsNum;
			}
			printf("\n(Sender::receive)接收到ACK后的窗口：\n");
			printWindow();
		}
	}
	else
	{
		pUtils->printPacket("\n(Sender::receive)ACK报文损坏\n", ackPkt);
	}
}

//Timeout handler，将被NetworkService调用
void SRSender::timeoutHandler(int seqNum)
{
	printf("\n(Sender::timeoutHandler)发送超时\n");
	pns->stopTimer(SENDER, seqNum);
	pns->sendToNetworkLayer(RECEIVER, buffer[seqNum]);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	pUtils->printPacket("\n(Sender::timeoutHandler)重传分组：\n", buffer[seqNum]);
	printf("\n(Sender::timeoutHandler)重传分组%d完成\n", seqNum);
}

//返回RdtSender是否处于等待状态，如果发送方正等待确认或者发送窗口已满，返回true
bool SRSender::getWaitingState()
{
	return (base + windowSize) % bitsNum == nextSeqNum % bitsNum;
}

//打印滑动窗口
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
			printf("%d不可用 ", i);
		}
		if (i == (base + windowSize - 1) % bitsNum)
		{
			printf("]");
		}
	}
	printf("\n");
}

//判断分组是否在窗口内
bool SRSender::isInWindow(int seqNum)
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
