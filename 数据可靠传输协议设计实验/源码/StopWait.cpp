// StopWait.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
#include "GBNSender.h"
#include "GBNReceiver.h"
#include "SRSender.h"
#include "SRReceiver.h"
#include "TCPSender.h"


int main(int argc, char* argv[])
{
	int m;
	printf("1.StopWait\t2.GBN\n3.SR\t4.TCP\n");
	scanf("%d", &m);
	RdtSender* ps;
	RdtReceiver* pr;
	switch (m)
	{
	default:
	case 1:
		ps = new StopWaitRdtSender();
		pr = new StopWaitRdtReceiver();
		break;
	case 2:
		ps = new GBNSender(4, 8);
		pr = new GBNReceiver(8);
		break;
	case 3:
		ps = new SRSender(4, 8);
		pr = new SRReceiver(4, 8);
		break;
	case 4:
		ps = new TCPSender(4, 8);
		pr = new GBNReceiver(8);
	}
	//pns->setRunMode(0);  //VERBOSģʽ
	pns->setRunMode(1);  //����ģʽ
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("D:\\netExp\\ʵ���\\������ѧ����RDT����\\Windows VS2017\\input.txt");
	pns->setOutputFile("D:\\netExp\\ʵ���\\������ѧ����RDT����\\Windows VS2017\\output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	
	return 0;
}

