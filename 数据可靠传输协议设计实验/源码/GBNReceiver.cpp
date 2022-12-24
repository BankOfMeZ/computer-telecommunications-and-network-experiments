#pragma once
#include "stdafx.h"
#include "GBNReceiver.h"
#include "Global.h"

GBNReceiver::GBNReceiver(int bNum) :bitsNum(bNum), expectedSeqNum(0)
{
	lastAckPkt.seqnum = -1;//忽略该字段
	lastAckPkt.checksum = 0;
	lastAckPkt.acknum = -1;//初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
	{
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

void GBNReceiver::receive(const Packet& packet)
{
	if (pUtils->calculateCheckSum(packet) != packet.checksum)
	{
		pUtils->printPacket("\n(Receiver::receive)接收分组损坏：\n", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
	}
	else if (packet.seqnum != expectedSeqNum)
	{
		pUtils->printPacket("\n(Receiver::receive)不是期待的分组：\n", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
	}
	else
	{
		Message message;
		memcpy(message.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, message);
		pUtils->printPacket("\n(Receiver::receive)接收到分组并交给应用层：\n", packet);
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("\n(Receiver::receive)接收方发送ACK：\n", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
		expectedSeqNum = (expectedSeqNum + 1) % bitsNum;
	}
}
