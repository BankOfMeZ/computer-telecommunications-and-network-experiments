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

//打印滑动窗口
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

//判断分组是否在窗口内
bool TCPSender::isInWindow(int seqNum)
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

//发送应用层下来的Message，由NetworkService调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待确认状态或发送窗口已满而拒绝发送Message，则返回false
bool TCPSender::send(const Message& message)
{
	if (getWaitingState())
	{
		printf("\n(Sender::send)发送窗口内报文段发完，等待ACK\n");
		return false;
	}
	else
	{
		buffer[nextSeqNum].acknum = -1;
		buffer[nextSeqNum].seqnum = nextSeqNum;
		memcpy(buffer[nextSeqNum].payload, message.data, sizeof(message.data));
		buffer[nextSeqNum].checksum = pUtils->calculateCheckSum(buffer[nextSeqNum]);
		printf("\n(Sender::send)发送分组前窗口：\n");
		printWindow();
		if (base == nextSeqNum)
		{//累计确认，设一个计时器
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		pns->sendToNetworkLayer(RECEIVER, buffer[nextSeqNum]);
		pUtils->printPacket("\n(Sender::send)发送报文：\n", buffer[nextSeqNum]);
		nextSeqNum = (nextSeqNum + 1) % bitsNum;
		printf("\n(Sender::send)发送分组后窗口：\n");
		printWindow();
		return true;
	}
}

//接受确认Ack，将被NetworkService调用
void TCPSender::receive(const Packet& ackPkt)
{
	if (pUtils->calculateCheckSum(ackPkt) == ackPkt.checksum)
	{
		if (isInWindow(ackPkt.acknum))
		{
			pns->stopTimer(SENDER, 0);
			printf("\n(Sender::receive)接收ACK前窗口：\n");
			printWindow();
			base = (ackPkt.acknum + 1) % bitsNum;
			if (base != nextSeqNum)
			{//还有没有ACK的分组，重新设置定时器
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
			}
			printf("\n(Sender::receive)接收ACK后窗口：\n");
			printWindow();
			dupACKNum = 0;
		}
		else
		{//收到冗余ACK
			dupACKNum++;
			if (dupACKNum == 3)
			{
				dupACKNum = 0;
				pns->sendToNetworkLayer(RECEIVER, buffer[base]);
				pUtils->printPacket("\n(Sender::receive)收到3个冗余ACK，快速重传：\n", buffer[base]);
			}
		}
	}
	else
	{
		pUtils->printPacket("\n(Sender::receive)收到的ACK损坏：\n", ackPkt);
	}
}

//Timeout handler，将被NetworkService调用
void TCPSender::timeoutHandler(int seqNum)
{
	printf("\n(Sender::timeoutHandler)发送超时\n");
	//停止计时器，重新计时
	pns->stopTimer(SENDER, 0);
	pns->sendToNetworkLayer(RECEIVER, buffer[base]);
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
	pUtils->printPacket("\n(Sender::timeoutHandler)重传分组：\n", buffer[base]);
}

//返回RdtSender是否处于等待状态，如果发送方正等待确认或者发送窗口已满，返回true
bool TCPSender::getWaitingState()
{
	return (base + windowSize) % bitsNum == nextSeqNum % bitsNum;
}
