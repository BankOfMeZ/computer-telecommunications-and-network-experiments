// StopWait.cpp : 定义控制台应用程序的入口点。
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
	//pns->setRunMode(0);  //VERBOS模式
	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("D:\\netExp\\实验二\\发布给学生的RDT代码\\Windows VS2017\\input.txt");
	pns->setOutputFile("D:\\netExp\\实验二\\发布给学生的RDT代码\\Windows VS2017\\output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	
	return 0;
}

