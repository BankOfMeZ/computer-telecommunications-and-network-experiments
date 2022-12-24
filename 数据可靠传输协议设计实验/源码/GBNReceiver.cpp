#pragma once
#include "stdafx.h"
#include "GBNReceiver.h"
#include "Global.h"

GBNReceiver::GBNReceiver(int bNum) :bitsNum(bNum), expectedSeqNum(0)
{
	lastAckPkt.seqnum = -1;//���Ը��ֶ�
	lastAckPkt.checksum = 0;
	lastAckPkt.acknum = -1;//��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
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
		pUtils->printPacket("\n(Receiver::receive)���շ����𻵣�\n", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
	}
	else if (packet.seqnum != expectedSeqNum)
	{
		pUtils->printPacket("\n(Receiver::receive)�����ڴ��ķ��飺\n", packet);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
	}
	else
	{
		Message message;
		memcpy(message.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, message);
		pUtils->printPacket("\n(Receiver::receive)���յ����鲢����Ӧ�ò㣺\n", packet);
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("\n(Receiver::receive)���շ�����ACK��\n", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);
		expectedSeqNum = (expectedSeqNum + 1) % bitsNum;
	}
}
