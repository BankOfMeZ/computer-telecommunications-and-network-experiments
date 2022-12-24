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

//发送应用层下来的Message，由NetworkService调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待确认状态或发送窗口已满而拒绝发送Message，则返回false
bool GBNSender::send(const Message& message)
{
	if (getWaitingState())
	{//发送窗口内报文段发完，等待ACK
		printf("\n(Sender::send)发送窗口内报文段发完，等待ACK\n");
		return false;
	}
	else
	{
		buffer[nextSeqNum].acknum = -1;//忽略该字段
		buffer[nextSeqNum].seqnum = nextSeqNum;
		memcpy(buffer[nextSeqNum].payload, message.data, sizeof(message.data));
		buffer[nextSeqNum].checksum = pUtils->calculateCheckSum(buffer[nextSeqNum]);
		pUtils->printPacket("\n(Sender::send)发送方报文：\n", buffer[nextSeqNum]);
		printf("\n(Sender::send)发送前窗口：\n");
		printWindow();
		if (base == nextSeqNum)
		{
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		pns->sendToNetworkLayer(RECEIVER, buffer[nextSeqNum]);
		nextSeqNum = (nextSeqNum + 1) % bitsNum;
		printf("\n(Sender::send)发送后窗口：\n");
		printWindow();
		return true;
	}
}

//接受确认Ack，将被NetworkService调用
void GBNSender::receive(const Packet& ackPkt)
{
	if (pUtils->calculateCheckSum(ackPkt) != ackPkt.checksum)
	{
		pUtils->printPacket("\n(Sender::receive)ACK报文损坏\n", ackPkt);
	}
	else
	{
		printf("\n(Sender::receive)接收到ACK前的窗口：\n");
		printWindow();
		base = (ackPkt.acknum + 1) % bitsNum;//GBN为累计确认
		if (base == nextSeqNum)
		{//发送的全部收到
			pns->stopTimer(SENDER, 0);
		}
		else
		{//基序号改变，重启定时器
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
		printf("\n(Sender::receive)接收到ACK后的窗口：\n");
		printWindow();
	}
}

//Timeout handler，将被NetworkService调用
void GBNSender::timeoutHandler(int seqNum)
{
	printf("\n(Sender::timeoutHandler)发送超时\n");
	if (nextSeqNum != base)
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		for (int i = base; i != nextSeqNum; i = (i + 1) % bitsNum)
		{//超时重传
			pns->sendToNetworkLayer(RECEIVER, buffer[i]);
			pUtils->printPacket("\n(Sender::timeoutHandler)重传分组：\n", buffer[i]);
		}
	}
	printf("\n(Sender::timeoutHandler)重传完成\n");
}

//返回RdtSender是否处于等待状态，如果发送方正等待确认或者发送窗口已满，返回true
bool GBNSender::getWaitingState()
{
	return (base + windowSize) % bitsNum == nextSeqNum % bitsNum;
}

//打印滑动窗口
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
