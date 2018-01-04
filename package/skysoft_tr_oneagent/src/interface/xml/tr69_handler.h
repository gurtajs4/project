#ifndef __TR69_HANDLER_H__
#define __TR69_HANDLER_H__

#define _GNU_SOURCE

#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/timeb.h>
#include <time.h>
#include <sys/time.h>
#ifndef USE_X86
#include <iwinfo.h>
#endif
#include <uci.h>
#include <suci.h>

#include "tr69_handler_ext.h"
#include "tr69_handler_table.h"
#include "tr_uciconfig.h"
#include "log.h"
#include "string.h"
#include "apps.h"
#include "tr.h"
#include "tr_strings.h"
#include "system.h"
#include "session.h"

#include "ssl.h"
#include <sys/stat.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/ssl23.h>
#include <openssl/ssl2.h>
#include <openssl/asn1.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/objects.h>

extern int lib_dynamic_init_children(node_t node);
extern int lib_init_PortMapping_children(node_t node);

#if 0
//QOS template node , it's a path 
#define  QTMP_C_DestVendorSpecificInfoSubOption   "classification_template.template.DestVendorSpecificInfoSubOption"
#define  QTMP_C_EthernetPriorityMark				 "classification_template.template.EthernetPriorityMark"
#define  QTMP_C_InnerEthernetPriorityCheck            "classification_template.template.InnerEthernetPriorityCheck"




//For commit template
#define  SHAPER_TMP 			  "shaper_template"
#define  QUEUE_TMP   			  "queue_template"
#define  CLASSIFICATION_TMP  	  "classification_template" 
//for commit entry
#define  SHAPER 					  "shaper"
#define  QUEUE 					  "queue"
#define  CLASSIFICATION  		  "classification"

//QoSNumberEntries
#define QOSNUMBERENTRIES  "QoSNumberEntries"

//For set ,get ,rename and so on.    it's a path,
#define QUEUENUMBEROFENTRIES_PATH          "QoSNumberEntries.qosnumber.QueueNumberOfEntries"
#define SHAPERNUMBEROFENTRIES_PATH         "QoSNumberEntries.qosnumber.ShaperNumberOfEntries"
#define CLASSIFICATIONNUMBEROFENTRIES_PATH   "QoSNumberEntries.qosnumber.ClassificationNumberOfEntries"
#define MaxShaperEntries_PATH  				 "QoSNumberEntries.qosnumber.MaxClassificationEntries"
#endif

typedef enum
{	
    QosQueueStatus_Disabled=0,
    QosQueueStatus_Enabled=1,
    QosQueueStatus_Error_Misconfigured=2,
    QosQueueStatus_Error=3
} QosQueueStatus;

typedef enum
{	
    QosShaperStatus_Disabled=0,
    QosShaperStatus_Enabled=1,
    QosShaperStatus_Error_Misconfigured=2,
    QosShaperStatus_Error=3
} QosShaperStatus; 	

typedef enum
{	
    QosClassificationStatus_Disabled=0,
    QosClassificationStatus_Enabled=1,
    QosClassificationStatus_Error_Misconfigured=2,
    QosClassificationStatus_Error=3
} QosClassificationStatus; 	


//extern int uci_caller = UCI_CALLER_TR;
extern int connectionRequestPasswordFlag;
extern int managementServerPasswordFlag;
extern int managementServerSTUNPasswordFlag;
int dealayrebootsens = 0;
long int schedulerebootsens = 0;
int parameternum = 0;
extern a_WifimappingInfo wifi_map[];
extern a_LanmappingInfo lan_map[];

int tr_getvalue_mapfile_byinstance(char* filename,char *value, int inst)
{
    int i = 0;
	int instance;
	char szinstance[16];
	char szvalue[256];
	
	FILE *fp = fopen(filename,"r");
	if (fp)
	{
		while(!feof(fp))
		{
			fscanf(fp,"%s %*s %s",szinstance,szvalue);
			instance	= atoi(szinstance);
			if (instance == inst)
			{
				printf("value = %s \n", szvalue);
				strcpy(value,szvalue);
				fclose(fp);
				return 0;
			}
			i++;
		}
		fclose(fp);
	}
	return -1;
}

static int tr_creat_shaper_rules()
{
	char ShaperNumberOfEntries[32] = {0};
	char ShapingRate[32] = {0};
	char ShapingBurstSize[32] = {0};
	char InterfaceName[256] = {0};
	char TrafficClasses[256] = {0};
	char Interface[256] = {0};
	char Interface2[256] = {0};
	char Weight[256] = {0};
	char Precedence[256] = {0};
	char SchedulerAlgorithm[256] = {0};
	char name[256] = {0};
	char command[256] = {0};
	int i = 0;
	int j = 1;
	int classid = 1;
	int classid1 = 1;
	int wrrflag1 = 0;	
	int wrrflag2 = 0;	
	int wfqflag1 = 0;	
	int wfqflag2 = 0;	
	int setdefault = 0;
	int setdefault1 = 0;
	char value[64] = {0};
	char waninf[32] = {0};
	char shaper_enable[32]={0};

	char shaper_status[32];

	getEthInterfaceName("wan", waninf);
	
	memset(shaper_status, 0, sizeof(shaper_status));	
	memset(ShaperNumberOfEntries, 0, sizeof(ShaperNumberOfEntries));
	do_uci_get(SHAPERNUMBEROFENTRIES_PATH, ShaperNumberOfEntries);
	tr_log(LOG_NOTICE,"ShaperNumberOfEntries S: %s\n", ShaperNumberOfEntries);
	tr_log(LOG_NOTICE,"ShaperNumberOfEntries D: %d\n", atoi(ShaperNumberOfEntries)); 
	while(j < atoi(ShaperNumberOfEntries))
	{
		sprintf(name, "shaper.Device_QoS_Shaper_%d.Status", j);
		do_uci_get(name, shaper_status);
		
		tr_log(LOG_NOTICE,"in while j ====%dth\n",j);        
		sprintf(name, "shaper.Device_QoS_Shaper_%d", j); //j = int [1,7]
		tr_log(LOG_NOTICE,"Device_QoS_Shaper_%d name====%s\n",j,name);		
		do_uci_get(name,value);
		tr_log(LOG_NOTICE,"Device_QoS_Shaper_%d value====%s\n",j,value);
		if(strcmp(value, "shaper") == 0)
		{
			sprintf(name, "shaper.Device_QoS_Shaper_%d.Interface", j);	
			do_uci_get(name, Interface);
			sprintf(name, "shaper.Device_QoS_Shaper_%d.ShapingRate", j);	
			do_uci_get(name, ShapingRate);
			sprintf(name, "shaper.Device_QoS_Shaper_%d.ShapingBurstSize", j);	
			do_uci_get(name, ShapingBurstSize);
			sprintf(name, "shaper.Device_QoS_Shaper_%d.Enable", j);	
			do_uci_get(name, shaper_enable);

			tr_log(LOG_NOTICE,"Interface ====%s\n",Interface);   
			tr_log(LOG_NOTICE,"ShapingRate ====%s\n",ShapingRate);   
			tr_log(LOG_NOTICE,"ShapingBurstSize ====%s\n",ShapingBurstSize);   
			tr_log(LOG_NOTICE,"Interface ====%s\n",Interface);   
			tr_log(LOG_NOTICE,"shaper_enable ====%s\n",shaper_enable);   

			for (i = 0; i <= IP_LAN_INSTANCE_NUM; i ++){
				sprintf(Interface2, "Device.IP.Interface.%d", lan_map[i].num);
				if (strcmp(Interface, Interface2) == 0){
					strcpy(InterfaceName, lan_map[i].laninf);
					break;
				}
			}
			if(strcmp(Interface, IP_WAN_INTERFACE_PATH) == 0)
			{
				strcpy(InterfaceName, waninf);
			}

			if((strcmp(shaper_enable, "1") == 0 )&&(atoi(shaper_status)==QosShaperStatus_Disabled) )//enable=1
			{
				sprintf(command, "tc qdisc add dev %s root handle 1: nsstbl rate %s burst %d", InterfaceName, ShapingRate, atoi(ShapingBurstSize)*8);
				system(command);
				tr_log(LOG_NOTICE,"command: %s\n", command);
			}
			else if(strcmp(shaper_enable, "0") == 0)//disenable=0
			{
				sprintf(command, "tc qdisc del dev %s root handle 1: nsstbl rate %s burst %d", InterfaceName, ShapingRate, atoi(ShapingBurstSize)*8);
				system(command);
				tr_log(LOG_NOTICE,"command: %s\n", command);
			}
			memset(Interface, 0, sizeof(Interface));
			memset(ShapingRate, 0, sizeof(ShapingRate));
			memset(ShapingBurstSize, 0, sizeof(ShapingBurstSize));
			memset(command, 0, sizeof(command));
			memset(value, 0, sizeof(value));
			memset(shaper_enable, 0, sizeof(shaper_enable));
		}
		j++;
	}
}


static int tr_creat_queue_rules()
{
	char ShaperNumberOfEntries[32] = {0};
	char QueueNumberOfEntries[32] = {0};
	char ClassificationNumberOfEntries[32] = {0};
	char ShapingRate[32] = {0};
	char ShapingBurstSize[32] = {0};
	char InterfaceName[256] = {0};
	char TrafficClasses[256] = {0};
	char Interface[256] = {0};
	char Interface2[256] = {0};
	char Weight[256] = {0};
	char Precedence[256] = {0};
	char SchedulerAlgorithm[256] = {0};
	char name[256] = {0};
	char command[256] = {0};
	int i = 0;
	int j = 0;
	int classid = 1;
	int classid1 = 1;
	int wrrflag1 = 0;	
	int wrrflag2 = 0;	
	int wfqflag1 = 0;	
	int wfqflag2 = 0;	
	int setdefault = 0;
	int setdefault1 = 0;
	char value[64] = {0};
	char waninf[32] = {0};
	
	memset(QueueNumberOfEntries, 0, sizeof(QueueNumberOfEntries));
	do_uci_get(QUEUENUMBEROFENTRIES_PATH, QueueNumberOfEntries);
	tr_log(LOG_NOTICE,"QueueNumberOfEntries CCC: %s\n", QueueNumberOfEntries);
	sprintf(QueueNumberOfEntries ,"7");
	tr_log(LOG_NOTICE,"QueueNumberOfEntries=%s\n",QueueNumberOfEntries );
      tr_log(LOG_NOTICE,"atoi(QueueNumberOfEntries)  =%d\n",atoi(QueueNumberOfEntries) );     
	while(j < atoi(QueueNumberOfEntries))
	{
			j++;
			//Device_QoS_Classification_%d
			//Device_QoS_Queue_%d"
			sprintf(name, "queue.Device_QoS_Queue_%d", j);		
			memset(value, 0, sizeof(value));
			do_uci_get(name,value);
			tr_log(LOG_NOTICE,"%d name=%s, value=%s\n",j,name ,value );
			if(strcmp(value, "queue") == 0)
			{
				//i++;
				//Device_QoS_Classification_16
				//qos.cf16.SourceMas
				sprintf(name, "queue.Device_QoS_Queue_%d.TrafficClasses", j);	
				do_uci_get(name, TrafficClasses);
				tr_log(LOG_NOTICE,"name=%s, TrafficClasses=%s\n",name ,TrafficClasses );
				sprintf(name, "queue.Device_QoS_Queue_%d.Interface", j);	
				do_uci_get(name, Interface);
				sprintf(name, "queue.Device_QoS_Queue_%d.Weight", j);	
				do_uci_get(name, Weight);
				sprintf(name, "queue.Device_QoS_Queue_%d.Precedence", j);	
				do_uci_get(name, Precedence);
				sprintf(name, "queue.Device_QoS_Queue_%d.SchedulerAlgorithm", j);	
				do_uci_get(name, SchedulerAlgorithm);

				for (i = 0; i <= IP_LAN_INSTANCE_NUM; i ++){
					sprintf(Interface2, "Device.IP.Interface.%d", lan_map[i].num);
					if (strcmp(Interface, Interface2) == 0){
						strcpy(InterfaceName, lan_map[i].laninf);
						break;
					}
				}
				if(strcmp(Interface, IP_WAN_INTERFACE_PATH) == 0)
				{
					getEthInterfaceName("wan", waninf);
					strcpy(InterfaceName, waninf);
				}

				tr_log(LOG_NOTICE,"SchedulerAlgorithm====%s\n", SchedulerAlgorithm); 
				if(strcasecmp(SchedulerAlgorithm, "SP") == 0)
				{	      
					sprintf(command, "tc qdisc add dev %s parent 1:1 handle 10: nssprio bands 3", InterfaceName);
					system(command);
					tr_log(LOG_NOTICE,"command: %s\n", command);
					sprintf(command, "tc qdisc add dev %s parent 10:1 handle 101: nsspfifo", InterfaceName);
					system(command);
					tr_log(LOG_NOTICE,"command: %s\n", command);
					sprintf(command, "tc qdisc add dev %s parent 10:2 handle 102: nsspfifo", InterfaceName);
					system(command);
					tr_log(LOG_NOTICE,"command: %s\n", command);
					sprintf(command, "tc qdisc add dev %s parent 10:3 handle 103: nsspfifo set_default", InterfaceName);
					system(command);
					tr_log(LOG_NOTICE,"command: %s\n", command);
				}
				else if(strcasecmp(SchedulerAlgorithm, "WRR") == 0)
				{
					if(strcmp(InterfaceName, "br-lan") == 0)
					{
						if(wrrflag1 == 0)
						{
							sprintf(command, "tc qdisc add dev %s parent 1:1 handle 2: nsswrr", InterfaceName);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
							wrrflag1 = 1;
						}
						sprintf(command, "tc class add dev %s parent 2: classid 2:%d nsswrr quantum %s", InterfaceName, classid, Weight);
						system(command);
						tr_log(LOG_NOTICE,"command: %s\n", command);
						if(setdefault == 0)
						{
							sprintf(command, "tc qdisc add dev %s parent 2:%d handle %d00: nsspfifo set_default", InterfaceName, classid, classid);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
							setdefault = 1;
						}
						else
						{
							sprintf(command, "tc qdisc add dev %s parent 2:%d handle %d00: nsspfifo", InterfaceName, classid, classid);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
						}
						classid++;
					}
					else
					{
						if(wrrflag2 == 0)
						{
							sprintf(command, "tc qdisc add dev %s parent 1:1 handle 2: nsswrr", InterfaceName);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
							wrrflag2 = 1;
						}
						sprintf(command, "tc class add dev %s parent 2: classid 2:%d nsswrr quantum %s", InterfaceName, classid1, Weight);
						system(command);
						tr_log(LOG_NOTICE,"command: %s\n", command);
						if(setdefault1 == 0)
						{
							sprintf(command, "tc qdisc add dev %s parent 2:%d handle %d00: nsspfifo set_default", InterfaceName, classid1, classid1);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
							setdefault1 = 1;
						}
						else
						{
							sprintf(command, "tc qdisc add dev %s parent 2:%d handle %d00: nsspfifo", InterfaceName, classid1, classid1);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
						}
						classid1++;
					}		
				}
				else if(strcasecmp(SchedulerAlgorithm, "WFQ") == 0)
				{
					if(strcmp(InterfaceName, "br-lan") == 0)
					{
						if(wfqflag1 == 0)
						{
							sprintf(command, "tc qdisc add dev %s parent 1:1 handle 2: nsswfq", InterfaceName);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
							wfqflag1 = 1;
						}
						sprintf(command, "tc class add dev %s parent 2: classid 2:%d nsswfq quantum %s", InterfaceName, classid, Weight);
						system(command);
						tr_log(LOG_NOTICE,"command: %s\n", command);
						if(setdefault == 0)
						{
							sprintf(command, "tc qdisc add dev %s parent 2:%d handle %d00: nsspfifo set_default", InterfaceName, classid, classid);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
							setdefault = 1;
						}
						else
						{
							sprintf(command, "tc qdisc add dev %s parent 2:%d handle %d00: nsspfifo", InterfaceName, classid, classid);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
						}
						classid++;
					}
					else
					{
						if(wfqflag2 == 0)
						{
							sprintf(command, "tc qdisc add dev %s parent 1:1 handle 2: nsswfq", InterfaceName);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
							wfqflag2 = 1;
						}
						sprintf(command, "tc class add dev %s parent 2: classid 2:%d nsswfq quantum %s", InterfaceName, classid1, Weight);
						system(command);
						tr_log(LOG_NOTICE,"command: %s\n", command);
						if(setdefault1 == 0)
						{
							sprintf(command, "tc qdisc add dev %s parent 2:%d handle %d00: nsspfifo set_default", InterfaceName, classid1, classid1);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
							setdefault1 = 1;
						}
						else
						{
							sprintf(command, "tc qdisc add dev %s parent 2:%d handle %d00: nsspfifo", InterfaceName, classid1, classid1);
							system(command);
							tr_log(LOG_NOTICE,"command: %s\n", command);
						}
						classid1++;
					}		
				}
			}
	}
	
}

static int tr_creat_classification_rules()
{
	char Enable[32] = {0};
	char Order[32] = {0};
	char AllInterfaces[32] = {0};
	char DestIP[32] = {0};
	char DestMask[32] = {0};
	char DestIPExclude[32] = {0};
	char SourceIP[32] = {0};
	char SourceMask[32] = {0};
	char SourceIPExclude[32] = {0};
	char DestPort[32] = {0};
	char DestPortRangeMax[32] = {0};
	char DestPortExclude[32] = {0};
	char SourcePort[32] = {0};
	char SourcePortRangeMax[32] = {0};
	char SourcePortExclude[32] = {0};
	char SourceMACAddress[32] = {0};
	char SourceMACMask[32] = {0};
	char SourceMACExclude[32] = {0};
	char DestMACAddress[32] = {0};
	char DestMACMask[32] = {0};
	char DestMACExclude[32] = {0};
	char TCPACK[32] = {0};
	char TCPACKExclude[32] = {0};
	char DSCPMark[32] = {0};
	char TrafficClass[32] = {0};
	char Protocol[32] = {0};
	char ProtocolExclude[32] = {0};
	char Ethertype[32] = {0};
	char EthertypeExclude[32] = {0};
	char SSAP[32] = {0};
	char SSAPExclude[32] = {0};
	char DSCPCheck[32] = {0};
	char DSCPExclude[32] = {0};
	char EthernetPriorityCheck[32] = {0};
	char EthernetPriorityExclude[32] = {0};
	char VLANIDCheck[32] = {0};
	char VLANIDExclude[32] = {0};
	char IPLengthMin[32] = {0};
	char IPLengthMax[32] = {0};
	char IPLengthExclude[32] = {0};
	char App[32] = {0};
    char ClassificationNumberOfEntries[32] = {0};
	char iptablescommand[1024] = {0};
	char ebtablescommand[1024] = {0};
	int i;
	int j;
	char value[64] = {0};
	char name[256] = {0};
	char Interface[256] = {0};
	char Interface2[256] = {0};
	char waninf[32] = {0};
	char classification_enable[32] = {0};
	memset(ClassificationNumberOfEntries, 0, sizeof(ClassificationNumberOfEntries));
	memset(classification_enable, 0, sizeof(classification_enable));
	do_uci_get(CLASSIFICATIONNUMBEROFENTRIES_PATH, ClassificationNumberOfEntries);
	printf("ClassificationNumberOfEntries: %s\n", ClassificationNumberOfEntries);

	i = 0;
	j = 1;
	while(j < atoi(ClassificationNumberOfEntries))
	{
		
		sprintf(name, "qos.cf%d", j);
		memset(value, 0, sizeof(value));
		do_uci_get(name,value);
		if(strcmp(value, "classification") == 0)
		{
			memset(Enable, 0, sizeof(Enable));
			sprintf(name, "qos.cf%d.Enable", j);	
			do_uci_get(name, Enable);

			if(atoi(Enable) == 0)
			{
				continue;
			}
			
			memset(Order, 0, sizeof(Order));
			memset(Interface, 0, sizeof(Interface));
			memset(AllInterfaces, 0, sizeof(AllInterfaces));
			memset(DestIP, 0, sizeof(DestIP));
			memset(DestMask, 0, sizeof(DestMask));
			memset(DestIPExclude, 0, sizeof(DestIPExclude));
			memset(SourceIP, 0, sizeof(SourceIP));
			memset(SourceMask, 0, sizeof(SourceMask));
			memset(SourceIPExclude, 0, sizeof(SourceIPExclude));
			memset(DestPort, 0, sizeof(DestPort));
			memset(DestPortRangeMax, 0, sizeof(DestPortRangeMax));
			memset(DestPortExclude, 0, sizeof(DestPortExclude));
			memset(SourcePort, 0, sizeof(SourcePort));
			memset(SourcePortRangeMax, 0, sizeof(SourcePortRangeMax));
			memset(SourcePortExclude, 0, sizeof(SourcePortExclude));
			memset(SourceMACAddress, 0, sizeof(SourceMACAddress));
			memset(SourceMACMask, 0, sizeof(SourceMACMask));
			memset(SourceMACExclude, 0, sizeof(SourceMACExclude));
			memset(DestMACAddress, 0, sizeof(DestMACAddress));
			memset(DestMACMask, 0, sizeof(DestMACMask));
			memset(DestMACExclude, 0, sizeof(DestMACExclude));
			memset(TCPACK, 0, sizeof(TCPACK));
			memset(TCPACKExclude, 0, sizeof(TCPACKExclude));
			memset(DSCPMark, 0, sizeof(DSCPMark));
			memset(Protocol, 0, sizeof(Protocol));
			memset(ProtocolExclude, 0, sizeof(ProtocolExclude));
			memset(TrafficClass, 0, sizeof(TrafficClass));
			memset(Ethertype, 0, sizeof(Ethertype));
			memset(EthertypeExclude, 0, sizeof(EthertypeExclude));
			memset(SSAP, 0, sizeof(SSAP));
			memset(SSAPExclude, 0, sizeof(SSAPExclude));
			memset(DSCPCheck, 0, sizeof(DSCPCheck));
			memset(DSCPExclude, 0, sizeof(DSCPExclude));
			memset(EthernetPriorityCheck, 0, sizeof(EthernetPriorityCheck));
			memset(EthernetPriorityExclude, 0, sizeof(EthernetPriorityExclude));
			memset(VLANIDCheck, 0, sizeof(VLANIDCheck));
			memset(VLANIDExclude, 0, sizeof(VLANIDExclude));
			memset(App, 0, sizeof(App));
			memset(IPLengthMin, 0, sizeof(IPLengthMin));
			memset(IPLengthMax, 0, sizeof(IPLengthMax));
			memset(IPLengthExclude, 0, sizeof(IPLengthExclude));
			memset(classification_enable, 0, sizeof(classification_enable));
			
			sprintf(name, "qos.cf%d.Order", j);	
			do_uci_get(name, Order);
			sprintf(name, "qos.cf%d.Interface", j);	
			do_uci_get(name, Interface);
			sprintf(name, "qos.cf%d.AllInterfaces", j);	
			do_uci_get(name, AllInterfaces);
			sprintf(name, "qos.cf%d.DestIP", j);	
			do_uci_get(name, DestIP);
			sprintf(name, "qos.cf%d.DestMask", j);	
			do_uci_get(name, DestMask);
			sprintf(name, "qos.cf%d.DestIPExclude", j);	
			do_uci_get(name, DestIPExclude);
			sprintf(name, "qos.cf%d.SourceIP", j);	
			do_uci_get(name, SourceIP);
			sprintf(name, "qos.cf%d.SourceMask", j);	
			do_uci_get(name, SourceMask);
			sprintf(name, "qos.cf%d.SourceIPExclude", j);	
			do_uci_get(name, SourceIPExclude);
			sprintf(name, "qos.cf%d.DestPort", j);	
			do_uci_get(name, DestPort);
			sprintf(name, "qos.cf%d.DestPortRangeMax", j);	
			do_uci_get(name, DestPortRangeMax);
			sprintf(name, "qos.cf%d.DestPortExclude", j);	
			do_uci_get(name, DestPortExclude);
			sprintf(name, "qos.cf%d.SourcePort", j);	
			do_uci_get(name, SourcePort);
			sprintf(name, "qos.cf%d.SourcePortRangeMax", j);	
			do_uci_get(name, SourcePortRangeMax);
			sprintf(name, "qos.cf%d.SourcePortExclude", j);	
			do_uci_get(name, SourcePortExclude);
			sprintf(name, "qos.cf%d.SourceMACAddress", j);	
			do_uci_get(name, SourceMACAddress);
			sprintf(name, "qos.cf%d.SourceMACMask", j);	
			do_uci_get(name, SourceMACMask);
			sprintf(name, "qos.cf%d.SourceMACExclude", j);	
			do_uci_get(name, SourceMACExclude);
			sprintf(name, "qos.cf%d.TCPACK", j);	
			do_uci_get(name, TCPACK);
			sprintf(name, "qos.cf%d.TCPACKExclude", j);	
			do_uci_get(name, TCPACKExclude);
			sprintf(name, "qos.cf%d.TrafficClass", j);	
			do_uci_get(name, TrafficClass);
			sprintf(name, "qos.cf%d.DSCPMark", j);	
			do_uci_get(name, DSCPMark);
			sprintf(name, "qos.cf%d.Protocol", j);	
			do_uci_get(name, Protocol);
			sprintf(name, "qos.cf%d.ProtocolExclude", j);	
			do_uci_get(name, ProtocolExclude);
			sprintf(name, "qos.cf%d.DestMACAddress", j);	
			do_uci_get(name, DestMACAddress);
			sprintf(name, "qos.cf%d.DestMACMask", j);	
			do_uci_get(name, DestMACMask);
			sprintf(name, "qos.cf%d.DestMACExclude", j);	
			do_uci_get(name, DestMACExclude);
			sprintf(name, "qos.cf%d.Ethertype", j);	
			do_uci_get(name, Ethertype);
			sprintf(name, "qos.cf%d.EthertypeExclude", j);	
			do_uci_get(name, EthertypeExclude);
			sprintf(name, "qos.cf%d.SSAP", j);	
			do_uci_get(name, SSAP);
			sprintf(name, "qos.cf%d.SSAPExclude", j);	
			do_uci_get(name, SSAPExclude);
			sprintf(name, "qos.cf%d.DSCPCheck", j);	
			do_uci_get(name, DSCPCheck);
			sprintf(name, "qos.cf%d.DSCPExclude", j);	
			do_uci_get(name, DSCPExclude);
			sprintf(name, "qos.cf%d.EthernetPriorityCheck", j);	
			do_uci_get(name, EthernetPriorityCheck);
			sprintf(name, "qos.cf%d.EthernetPriorityExclude", j);	
			do_uci_get(name, EthernetPriorityExclude);
			sprintf(name, "qos.cf%d.VLANIDCheck", j);	
			do_uci_get(name, VLANIDCheck);
			sprintf(name, "qos.cf%d.VLANIDExclude", j);	
			do_uci_get(name, VLANIDExclude);
			sprintf(name, "qos.cf%d.App", j);	
			do_uci_get(name, App);
			sprintf(name, "qos.cf%d.IPLengthMin", j);	
			do_uci_get(name, IPLengthMin);
			sprintf(name, "qos.cf%d.IPLengthMax", j);	
			do_uci_get(name, IPLengthMax);
			sprintf(name, "qos.cf%d.IPLengthExclude", j);	
			do_uci_get(name, IPLengthExclude);

			char markmatch[64] = {0};
			if(Order[0] != '\0')
			{
				sprintf(markmatch, "-m mark --mark %s", Order);
			}

			char intefacematch[64] = {0};
			memset(intefacematch, 0, sizeof(intefacematch));
			if(atoi(AllInterfaces) != 1)
			{
				for (i = 0; i <= IP_LAN_INSTANCE_NUM; i ++){
					sprintf(Interface2, "Device.IP.Interface.%d", lan_map[i].num);
					if (strcmp(Interface, Interface2) == 0){
						sprintf(intefacematch, "-i %s", lan_map[i].laninf);
						break;
					}
				}
				if(strcmp(Interface, IP_WAN_INTERFACE_PATH) == 0)
				{
					getEthInterfaceName("wan", waninf);
					sprintf(intefacematch, "-i %s", waninf);
				}
			}
			
			char destipmatch[64] = {0};
			if(atoi(DestIPExclude) == 1)
			{
				if((DestIP[0] != '\0') && (DestMask[0] == '\0'))
					sprintf(destipmatch, "! -d %s", DestIP);
				if((DestIP[0] != '\0') && (DestMask[0] != '\0'))
					sprintf(destipmatch, "! -d %s/%s", DestIP, DestMask);
			}
			else
			{
				if((DestIP[0] != '\0') && (DestMask[0] == '\0'))
					sprintf(destipmatch, "-d %s", DestIP);
				if((DestIP[0] != '\0') && (DestMask[0] != '\0'))
					sprintf(destipmatch, "-d %s/%s", DestIP, DestMask);
			}
				
			char srcipmatch[64] = {0};
			if(atoi(SourceIPExclude) == 1)
			{
				if((SourceIP[0] != '\0') && (SourceMask[0] == '\0'))
					sprintf(srcipmatch, "! -s %s", SourceIP);
				if((SourceIP[0] != '\0') && (SourceMask[0] != '\0'))
					sprintf(srcipmatch, "! -s %s/%s", SourceIP, SourceMask);
			}
			else
			{
				if((SourceIP[0] != '\0') && (SourceMask[0] == '\0'))
					sprintf(srcipmatch, "-s %s", SourceIP);
				if((SourceIP[0] != '\0') && (SourceMask[0] != '\0'))
					sprintf(srcipmatch, "-s %s/%s", SourceIP, SourceMask);
			}

			char Protocolmatch[64] = {0};
			if(atoi(ProtocolExclude) == 1)
			{
				if((Protocol[0] != '\0') && (atoi(Protocol) != -1))
					sprintf(Protocolmatch, "! -p %s", Protocol);
			}
			else
			{
				if((Protocol[0] != '\0') && (atoi(Protocol) != -1))
					sprintf(Protocolmatch, "-p %s", Protocol);
			}

			char destportmatch[64] = {0};
			if(atoi(DestPortExclude) == 1)
			{
				if((DestPort[0] != '\0') && (DestPortRangeMax[0] == '\0'))
					sprintf(destportmatch, "! --dport %s", DestPort);
				if((DestPort[0] != '\0') && (DestPortRangeMax[0] != '\0'))
					sprintf(destportmatch, "! --dport %s:%s", DestPort, DestPortRangeMax);
				if(DestPort[0] != '\0')
					strcpy(Protocolmatch, "-p tcp");
			}
			else
			{
				if((DestPort[0] != '\0') && (DestPortRangeMax[0] == '\0'))
					sprintf(destportmatch, "--dport %s", DestPort);
				if((DestPort[0] != '\0') && (DestPortRangeMax[0] != '\0'))
					sprintf(destportmatch, "--dport %s:%s", DestPort, DestPortRangeMax);
				if(DestPort[0] != '\0')
					strcpy(Protocolmatch, "-p tcp");
			}
			
			char srcportmatch[64] = {0};
			if(atoi(SourcePortExclude) == 1)
			{
				if((SourcePort[0] != '\0') && (SourcePortRangeMax[0] == '\0'))
					sprintf(srcportmatch, "! --sport %s", SourcePort);
				if((SourcePort[0] != '\0') && (SourcePortRangeMax[0] != '\0'))
					sprintf(srcportmatch, "! --sport %s:%s", SourcePort, SourcePortRangeMax);
				if(SourcePort[0] != '\0')
					strcpy(Protocolmatch, "-p tcp");
			}
			else
			{
				if((SourcePort[0] != '\0') && (SourcePortRangeMax[0] == '\0'))
					sprintf(srcportmatch, "--sport %s", SourcePort);
				if((SourcePort[0] != '\0') && (SourcePortRangeMax[0] != '\0'))
					sprintf(srcportmatch, "--sport %s:%s", SourcePort, SourcePortRangeMax);
				if(SourcePort[0] != '\0')
					strcpy(Protocolmatch, "-p tcp");
			}
			
			char srcmacmatch[64] = {0};
			memset(srcmacmatch, 0, sizeof(srcmacmatch));
			if(atoi(SourceMACExclude) == 1)
			{
				if((SourceMACAddress[0] != '\0') && (SourceMACMask[0] == '\0'))
					sprintf(srcmacmatch, "-s ! %s", SourceMACAddress);
				if((SourceMACAddress[0] != '\0') && (SourceMACMask[0] != '\0'))
					sprintf(srcmacmatch, "-s ! %s/%s", SourceMACAddress, SourceMACMask);
			}
			else
			{
				if((SourceMACAddress[0] != '\0') && (SourceMACMask[0] == '\0'))
					sprintf(srcmacmatch, "-s %s", SourceMACAddress);
				if((SourceMACAddress[0] != '\0') && (SourceMACMask[0] != '\0'))
					sprintf(srcmacmatch, "-s %s/%s", SourceMACAddress, SourceMACMask);
			}

			char destmacmatch[64] = {0};
			memset(destmacmatch, 0, sizeof(destmacmatch));
			if(atoi(DestMACExclude) == 1)
			{
				if((DestMACAddress[0] != '\0') && (DestMACMask[0] == '\0'))
					sprintf(destmacmatch, "-d ! %s", DestMACAddress);
				if((DestMACAddress[0] != '\0') && (DestMACMask[0] != '\0'))
					sprintf(destmacmatch, "-d ! %s/%s", DestMACAddress, DestMACMask);
			}
			else
			{
				if((DestMACAddress[0] != '\0') && (DestMACMask[0] == '\0'))
					sprintf(destmacmatch, "-d %s", DestMACAddress);
				if((DestMACAddress[0] != '\0') && (DestMACMask[0] != '\0'))
					sprintf(destmacmatch, "-d %s/%s", DestMACAddress, DestMACMask);
			}

			char ethertypematch[64] = {0};
			memset(ethertypematch, 0, sizeof(ethertypematch));
			if(atoi(EthertypeExclude) == 1)
			{
				if((Ethertype[0] != '\0') && (atoi(Ethertype) != -1))
					sprintf(ethertypematch, "-p ! %s", Ethertype);
			}
			else
			{
				if((Ethertype[0] != '\0') && (atoi(Ethertype) != -1))
					sprintf(ethertypematch, "-p %s", Ethertype);
			}

			char sapmatch[64] = {0};
			memset(sapmatch, 0, sizeof(sapmatch));
			if(atoi(SSAPExclude) == 1)
			{
				if((SSAP[0] != '\0') && (atoi(SSAP) != -1))
				{
					strcpy(ethertypematch, "-p LENGTH");
					sprintf(sapmatch, "--802_3-sap ! %s", SSAP);
				}
			}
			else
			{
				if((SSAP[0] != '\0') && (atoi(SSAP) != -1))
				{
					strcpy(ethertypematch, "-p LENGTH");
					sprintf(sapmatch, "--802_3-sap %s", SSAP);
				}
			}
					
			char tcpackmatch[64] = {0};
			if(atoi(TCPACKExclude) == 1)
			{
				if(atoi(TCPACK) == 1)
				{
					sprintf(tcpackmatch, "! --tcp-flags ALL ACK");
					strcpy(Protocolmatch, "-p tcp");
				}
			}
			else
			{
				if(atoi(TCPACK) == 1)
				{
					sprintf(tcpackmatch, "--tcp-flags ALL ACK");
					strcpy(Protocolmatch, "-p tcp");
				}
			}

			char dscpmatch[64] = {0};
			if(atoi(DSCPExclude) == 1)
			{
				if((DSCPCheck[0] != '\0') && (atoi(DSCPCheck) != -1))
					sprintf(dscpmatch, "-m dscp ! --dscp %s", DSCPCheck);
			}
			else
			{
				if((DSCPCheck[0] != '\0') && (atoi(DSCPCheck) != -1))
					sprintf(dscpmatch, "-m dscp --dscp %s", DSCPCheck);
			}

			char etherpriomatch[64] = {0};
			memset(etherpriomatch, 0, sizeof(etherpriomatch));
			if(atoi(EthernetPriorityExclude) == 1)
			{
				if((EthernetPriorityCheck[0] != '\0') && (atoi(EthernetPriorityCheck) != -1))
				{
					strcpy(ethertypematch, "-p 0x8100");
					sprintf(etherpriomatch, "-vlan-prio ! %s", EthernetPriorityCheck);
					memset(sapmatch, 0, sizeof(sapmatch));
				}
			}
			else
			{
				if((EthernetPriorityCheck[0] != '\0') && (atoi(EthernetPriorityCheck) != -1))
				{
					strcpy(ethertypematch, "-p 0x8100");
					sprintf(etherpriomatch, "-vlan-prio %s", EthernetPriorityCheck);
					memset(sapmatch, 0, sizeof(sapmatch));
				}
			}

			char vlanidmatch[64] = {0};
			memset(vlanidmatch, 0, sizeof(vlanidmatch));
			if(atoi(VLANIDExclude) == 1)
			{
				if((VLANIDCheck[0] != '\0') && (atoi(VLANIDCheck) != -1))
				{
					strcpy(ethertypematch, "-p 0x8100");
					sprintf(vlanidmatch, "--vlan-id ! %s", VLANIDCheck);
					memset(sapmatch, 0, sizeof(sapmatch));
				}
			}
			else
			{
				if((VLANIDCheck[0] != '\0') && (atoi(VLANIDCheck) != -1))
				{
					strcpy(ethertypematch, "-p 0x8100");
					sprintf(vlanidmatch, "--vlan-id %s", VLANIDCheck);
					memset(sapmatch, 0, sizeof(sapmatch));
				}
			}

			char iplengthmatch[64] = {0};
			if(atoi(IPLengthExclude) == 1)
			{
				if((IPLengthMin[0] != '\0') && (IPLengthMax[0] == '\0'))
					sprintf(iplengthmatch, "-m length ! --length  %s", IPLengthMin);
				if((IPLengthMin[0] != '\0') && (IPLengthMax[0] != '\0'))
					sprintf(iplengthmatch, "-m length ! --length  %s:%s", IPLengthMin, IPLengthMax);
			}
			else
			{
				if((IPLengthMin[0] != '\0') && (IPLengthMax[0] == '\0'))
					sprintf(iplengthmatch, "-m length --length %s", IPLengthMin);
				if((IPLengthMin[0] != '\0') && (IPLengthMax[0] != '\0'))
					sprintf(iplengthmatch, "-m length --length %s:%s", IPLengthMin, IPLengthMax);
			}
			
			if(atoi(TrafficClass) < 0)
			{
				char *index = strrchr(App, '.');
				char Alias[256] = {0};
				char name[256] = {0};
				char ProtocolIdentifier[256] = {0};
				char DefaultTrafficClass[256] = {0};
				char DefaultDSCPMark[256] = {0};
				char *p = NULL;
				
				if (index != NULL)
				{
					tr_getvalue_mapfile_byinstance("/oneagent/conf/QoSAppMap.mapping", Alias, atoi(index+1));
					p = strchr(Alias, '_');
					if(p != NULL)
					{
						sprintf(name, "classification.Device_QoS_App_%d.ProtocolIdentifier", atoi(p+1));		
						printf("name[%s]\n", name);
						do_uci_get(name, ProtocolIdentifier);
						printf("value[%s]\n", ProtocolIdentifier);
						if(strcasestr(ProtocolIdentifier, "sip") != NULL)
						{
							strcpy(destportmatch, "--dport 6060");
							strcpy(Protocolmatch, "-p tcp");
						}
						else if(strcasestr(ProtocolIdentifier, "h.323") != NULL)
						{
							strcpy(destportmatch, "--dport 1720");
							strcpy(Protocolmatch, "-p tcp");
						}
						else if(strcasestr(ProtocolIdentifier, "h.248") != NULL)
						{
							strcpy(destportmatch, "--dport 2944");
							strcpy(Protocolmatch, "-p tcp");
						}
						else if(strcasestr(ProtocolIdentifier, "mgcp") != NULL)
						{
							strcpy(destportmatch, "--dport 2727");
							strcpy(Protocolmatch, "-p tcp");
						}
						sprintf(name, "classification.Device_QoS_App_%d.DefaultTrafficClass", atoi(p+1));		
						do_uci_get(name, DefaultTrafficClass);
						printf("name[%s]\n", name);
						printf("value[%s]\n", DefaultTrafficClass);
						sprintf(name, "classification.Device_QoS_App_%d.DefaultDSCPMark", atoi(p+1));		
						do_uci_get(name, DefaultDSCPMark);
						printf("name[%s]\n", name);
						printf("value[%s]\n", DefaultDSCPMark);
						strcpy(TrafficClass, DefaultTrafficClass);
						strcpy(DSCPMark, DefaultDSCPMark);
					}
				}
			}

			if(strcmp(classification_enable, "1") == 0)//Enable = 1
			{
				if((srcmacmatch[0] != '\0') || (destmacmatch[0] != '\0') || (ethertypematch[0] != '\0') || (sapmatch[0] != '\0') || (vlanidmatch[0] != '\0') || (etherpriomatch[0] != '\0'))
				{
					sprintf(ebtablescommand, "ebtables -A FORWARD %s %s %s %s %s %s -j mark --mark-set %s", ethertypematch, 
						srcmacmatch, destmacmatch, sapmatch, vlanidmatch, etherpriomatch, Order);	
					printf("ebtablescommand: %s\n", ebtablescommand);
					system(ebtablescommand);
					sprintf(iptablescommand, "iptables -t mangle -A FORWARD %s %s %s %s %s %s %s %s %s %s -j CLASSIFY --set-class %s:0", intefacematch, destipmatch, srcipmatch, Protocolmatch,
							destportmatch, srcportmatch, tcpackmatch, dscpmatch, markmatch, iplengthmatch, TrafficClass);
					printf("iptablescommand: %s\n", iptablescommand);
					system(iptablescommand);
					sprintf(iptablescommand, "iptables -t mangle -A FORWARD %s %s %s %s %s %s %s %s %s %s -j DSCP --set-dscp %s", intefacematch, destipmatch, srcipmatch, Protocolmatch,
							destportmatch, srcportmatch, tcpackmatch, dscpmatch, markmatch, iplengthmatch, DSCPMark);
					printf("iptablescommand: %s\n", iptablescommand);
					system(iptablescommand);
				}
				else if(strcmp(classification_enable, "0") == 0)//Disable = 0
				{
					sprintf(iptablescommand, "iptables -t mangle -A FORWARD %s %s %s %s %s %s %s %s %s -j CLASSIFY --set-class %s:0", intefacematch, destipmatch, srcipmatch, Protocolmatch,
							destportmatch, srcportmatch, tcpackmatch, dscpmatch, iplengthmatch, TrafficClass);
					printf("iptablescommand: %s\n", iptablescommand);
					system(iptablescommand);
					sprintf(iptablescommand, "iptables -t mangle -A FORWARD %s %s %s %s %s %s %s %s %s -j DSCP --set-dscp %s", intefacematch, destipmatch, srcipmatch, Protocolmatch,
							destportmatch, srcportmatch, tcpackmatch, dscpmatch, iplengthmatch, DSCPMark);
					printf("iptablescommand: %s\n", iptablescommand);
					system(iptablescommand);
				}
			}
			else
			{
				if((srcmacmatch[0] != '\0') || (destmacmatch[0] != '\0') || (ethertypematch[0] != '\0') || (sapmatch[0] != '\0') || (vlanidmatch[0] != '\0') || (etherpriomatch[0] != '\0'))
				{
					sprintf(ebtablescommand, "ebtables -D FORWARD %s %s %s %s %s %s -j mark --mark-set %s", ethertypematch, 
						srcmacmatch, destmacmatch, sapmatch, vlanidmatch, etherpriomatch, Order);	
					printf("ebtablescommand: %s\n", ebtablescommand);
					system(ebtablescommand);
					sprintf(iptablescommand, "iptables -t mangle -D FORWARD %s %s %s %s %s %s %s %s %s %s -j CLASSIFY --set-class %s:0", intefacematch, destipmatch, srcipmatch, Protocolmatch,
							destportmatch, srcportmatch, tcpackmatch, dscpmatch, markmatch, iplengthmatch, TrafficClass);
					printf("iptablescommand: %s\n", iptablescommand);
					system(iptablescommand);
					sprintf(iptablescommand, "iptables -t mangle -D FORWARD %s %s %s %s %s %s %s %s %s %s -j DSCP --set-dscp %s", intefacematch, destipmatch, srcipmatch, Protocolmatch,
							destportmatch, srcportmatch, tcpackmatch, dscpmatch, markmatch, iplengthmatch, DSCPMark);
					printf("iptablescommand: %s\n", iptablescommand);
					system(iptablescommand);
				}
				else
				{
					sprintf(iptablescommand, "iptables -t mangle -D FORWARD %s %s %s %s %s %s %s %s %s -j CLASSIFY --set-class %s:0", intefacematch, destipmatch, srcipmatch, Protocolmatch,
							destportmatch, srcportmatch, tcpackmatch, dscpmatch, iplengthmatch, TrafficClass);
					printf("iptablescommand: %s\n", iptablescommand);
					system(iptablescommand);
					sprintf(iptablescommand, "iptables -t mangle -D FORWARD %s %s %s %s %s %s %s %s %s -j DSCP --set-dscp %s", intefacematch, destipmatch, srcipmatch, Protocolmatch,
							destportmatch, srcportmatch, tcpackmatch, dscpmatch, iplengthmatch, DSCPMark);
					printf("iptablescommand: %s\n", iptablescommand);
					system(iptablescommand);
				}
			}
		}
		j++;
	}
	memset(classification_enable, 0, sizeof(classification_enable));
}


int get_D_RootDataModelVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(D_RootDataModelVersion, value);
	if(ret)
	{
		return -1;
	} */
	strcpy(value, "2.8"); //always 2.5
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_D_InterfaceStackNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(D_InterfaceStackNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	char keyvalue[MAXMAPITEMS][256];
	int num = get_Device_InterfaceStack_Entry(keyvalue);
	sprintf(value, "%d", num);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_DeviceCategory(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_DeviceCategory, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "COM_X_WirelessRouter");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_Manufacturer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_Manufacturer, value);
	if(ret)
	{
		return -1;
	}*/
	//getMfcInfo("Manufacturer", value);
	strcpy(value, "own");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_ManufacturerOUI(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_ManufacturerOUI, value);
	if(ret)
	{
		return -1;
	}*/
	//getMfcInfo("ManufacturerOUI", value);
	strcpy(value, "00AABB901016");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_ModelName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_ModelName, value);
	if(ret)
	{
		return -1;
	}*/
	getMfcInfo("ModelName", value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_ModelNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_ModelNumber, value);
	if(ret)
	{
		return -1;
	}*/
	getMfcInfo("ModelName", value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_Description(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_Description, value);
	if(ret)
	{
		return -1;
	}*/
	getMfcInfo("ModelName", value); //with the same as modelname
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_ProductClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_ProductClass, value);
	if(ret)
	{
		return -1;
	}*/
	//getMfcInfo("ModelName", value); //with the same as modelname
	strcpy(value, "HEATER_CWMP");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_SerialNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_SerialNumber, value);
	if(ret)
	{
		return -1;
	}*/
	#if 0
	char mac[32] = {0};
	char ACSIdentifier[32] = {0};

	ret = do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, ACSIdentifier);
	if(ret)
	{
		ret = 0;
	}

	if (strcmp(ACSIdentifier, "TWC") != 0)
		getMfcInfo("SerialNumber", value);
	else{
		getInfaceWanMac(mac); //Using WAN MAC address
		tr_log(LOG_DEBUG,"WAN Iinterface MAC [%s]",mac);
		if (mac[0] != '\0'){
			int i = 0, j = 0;
			for (i = 0; i < 17; i ++){
				if (mac[i] != ':'){
					value[j] = toupper(mac[i]);
					j ++;
				}
			}
			value[j] = '\0';
		}
		else
			strcpy(value, "");
	}
	#endif
	strcpy(value, "2015050422");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_X_CHARTER_COM_SerialNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_SerialNumber, value);
	if(ret)
	{
		return -1;
	}*/
	getMfcInfo("SerialNumber", value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_HardwareVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_HardwareVersion, value);
	if(ret)
	{
		return -1;
	}*/
	getMfcInfo("HardwareVersion", value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_SoftwareVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_SoftwareVersion, value);
	if(ret)
	{
		return -1;
	}*/
	getMfcInfo2("FW_VERSION", value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_AdditionalHardwareVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_AdditionalHardwareVersion, value);
	if(ret)
	{
		return -1;
	}*/
	getMfcInfo("DateOfManufacture", value); //Using the date of manufacture as the addition hardware version
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_AdditionalSoftwareVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_AdditionalSoftwareVersion, value);
	if(ret)
	{
		return -1;
	}*/
	getMfcInfo2("BUILD_DATE", value); //Using the date of building time as the addition software version
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_ProvisioningCode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DD_ProvisioningCode, value); //getting from trconf
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DD_ProvisioningCode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 64)
		return -2;
	
	ret = do_uci_set(DD_ProvisioningCode, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DD_UpTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_UpTime, value);
	if(ret)
	{
		return -1;
	}*/
	getDeviceUpTime("/proc/uptime", value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_FirstUseDate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_FirstUseDate, value);
	if(ret)
	{
		return -1;
	}*/
	FILE *fp = NULL;
	char line[128] = {0};
	char *ptr = NULL;
	if ((fp = fopen("/oneagent/conf/FirstUseDate", "r")) != NULL) {
		fgets(line,sizeof(line)-1,fp);
		if((ptr = strstr(line,"\n")) != NULL)
			*ptr = '\0';
		strcpy(value, line);
        fclose(fp);
    }
	else
		strcpy(value, "0001-01-01T00:00:00Z");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_VendorConfigFileNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_VendorConfigFileNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	int count = 0;
	FILE *fd = NULL;
	char line[256] = {0};

	if ((fd = popen("ls -l /overlay/etc/config", "r")) != NULL){
		while ( fgets(line, sizeof(line), fd) )
			count ++;
		pclose(fd);
	}
	sprintf(value, "%d", count);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_SupportedDataModelNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_SupportedDataModelNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always 1
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_ProcessorNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_ProcessorNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/

	int cpu_num = 0;
	cpu_num = sysconf(_SC_NPROCESSORS_CONF);
	sprintf(value, "%d", cpu_num);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_VendorLogFileNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_VendorLogFileNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/

	strcpy(value, "3"); //file /tmp/syslog/messages, /tmp/vendor/rebootreason.txt, /tmp/vendor/avail.txt
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_LocationNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_LocationNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_DeviceImageNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DD_DeviceImageNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0, platform doesn't support now
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDVt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDVt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDVt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDVt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDVt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDVt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDVt_Version(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDVt_Version, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDVt_Date(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDVt_Date, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDVt_Description(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDVt_Description, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDVt_UseForBackupRestore(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDVt_UseForBackupRestore, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDSt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDSt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDSt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DDSt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	// set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDSt_URL(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDSt_URL, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDSt_UUID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDSt_UUID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDSt_URN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDSt_URN, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDSt_Features(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDSt_Features, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDM_Total(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDM_Total, value);
	if(ret)
	{
		return -1;
	}*/
	struct sysinfo info;
	memset(&info, 0, sizeof(struct sysinfo));
	sysinfo(&info);
	info.totalram /= 1024; //changed to KByte
	sprintf(value, "%u", info.totalram);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDM_Free(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDM_Free, value);
	if(ret)
	{
		return -1;
	}*/
	struct sysinfo info;
	memset(&info, 0, sizeof(struct sysinfo));
	sysinfo(&info);
	info.freeram /= 1024; //changed to KByte
	sprintf(value, "%u", info.freeram);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDP_CPUUsage(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDP_CPUUsage, value);
	if(ret)
	{
		return -1;
	}*/
	getCpuUsage(value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDP_ProcessNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDP_ProcessNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/

	sprintf(value, "%d", get_all_process_num());
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDPPt_PID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDPPt_PID, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Process.");
	char pid[32] = {0};
	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ProcessMap, pid, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			strcpy(value, pid);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDPPt_Command(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDPPt_Command, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Process.");
	char pid[32] = {0};
	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ProcessMap, pid, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			getProcessStatus(pid, value, "Command");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDPPt_Size(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDPPt_Size, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Process.");
	char pid[32] = {0};
	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ProcessMap, pid, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			getProcessStatus(pid, value, "Size");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDPPt_Priority(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDPPt_Priority, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Process.");
	char pid[32] = {0};
	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ProcessMap, pid, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			getProcessStatus(pid, value, "Priority");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDPPt_CPUTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDPPt_CPUTime, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Process.");
	char pid[32] = {0};
	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ProcessMap, pid, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			getProcessStatus(pid, value, "CPUTime");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDPPt_State(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDPPt_State, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Process.");
	char pid[32] = {0};
	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ProcessMap, pid, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			getProcessStatus(pid, value, "State");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDT_ProcessNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDT_ProcessNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "13"); //0~10 -> core cpu sensors, 11->WLAN 2.4G chip sensor, 12->WLAN 5G chip sensor
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDTTt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DDTTt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDTTt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Enable);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Enable);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Enable);
		}
		else
			return -1;
	}

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDTTt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DDTTt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char cmd[128] = {0};
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	
	if (index != NULL){
		if (atoi(index) <= 11){ //core cpu
			if (atoi(value) == 0)
				sprintf(cmd, "echo disabled > /sys/devices/virtual/thermal/thermal_zone%d/mode", atoi(index)-1);
			else
				sprintf(cmd, "echo enabled > /sys/devices/virtual/thermal/thermal_zone%d/mode", atoi(index)-1);
			system(cmd);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			if (atoi(value) == 0)
				strcpy(cmd, "thermaltool -i wifi0 -set -e 0");
			else
				strcpy(cmd, "thermaltool -i wifi0 -set -e 1");
			system(cmd);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			if (atoi(value) == 0)
				strcpy(cmd, "thermaltool -i wifi1 -set -e 0");
			else
				strcpy(cmd, "thermaltool -i wifi1 -set -e 1");
			system(cmd);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDTTt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Status);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Status);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Status);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_Reset, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Reset);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Reset);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Reset);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDTTt_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DDTTt_Reset, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char cmd[128] = {0};
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			if (atoi(temperatureSensorInfo.Enable) == 0) //don't do anything
				return 0;
			if (atoi(value) == 0) //don't do anything
				return 0;
			else if (atoi(value) == 1){
				sprintf(cmd, "echo disabled > /sys/devices/virtual/thermal/thermal_zone%d/mode", atoi(index)-1);
				system(cmd);
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "echo enabled > /sys/devices/virtual/thermal/thermal_zone%d/mode", atoi(index)-1);
				system(cmd);
				//get reset time
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "date > /oneagent/conf/thermal_zone%dresttime", atoi(index)-1);
				system(cmd);
			}
			else
				return -1;
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			if (atoi(temperatureSensorInfo.Enable) == 0) //don't do anything
				return 0;
			if (atoi(value) == 0) //don't do anything
				return 0;
			else if (atoi(value) == 1){
				system("thermaltool -i wifi0 -set -e 0");
				system("thermaltool -i wifi0 -set -e 1");
				//get reset time
				system("date > /oneagent/conf/wifi0resttime");
			}
			else
				return -1;
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			if (atoi(temperatureSensorInfo.Enable) == 0) //don't do anything
				return 0;
			if (atoi(value) == 0) //don't do anything
				return 0;
			else if (atoi(value) == 1){
				system("thermaltool -i wifi1 -set -e 0");
				system("thermaltool -i wifi1 -set -e 1");
				//get reset time
				system("date > /oneagent/conf/wifi1resttime");
			}
			else
				return -1;
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDTTt_ResetTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_ResetTime, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.ResetTime);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.ResetTime);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.ResetTime);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_Name, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Name);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Name);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Name);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_Value(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_Value, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Value);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Value);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.Value);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_LastUpdate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_LastUpdate, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.LastUpdate);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.LastUpdate);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.LastUpdate);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_MinValue(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_MinValue, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MinValue);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MinValue);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MinValue);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_MinTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_MinTime, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MinTime);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MinTime);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MinTime);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_MaxValue(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_MaxValue, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MaxValue);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MaxValue);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MaxValue);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_MaxTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_MaxTime, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MaxTime);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MaxTime);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.MaxTime);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_LowAlarmValue(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_LowAlarmValue, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.LowAlarmValue);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.LowAlarmValue);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.LowAlarmValue);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDTTt_LowAlarmValue(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DDTTt_LowAlarmValue, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDTTt_LowAlarmTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_LowAlarmTime, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.ResetTime); //with the same as resettime
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.ResetTime); //with the same as resettime
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.ResetTime); //with the same as resettime
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDTTt_HighAlarmValue(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_HighAlarmValue, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.HighAlarmValue);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.HighAlarmValue);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.HighAlarmValue);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDTTt_HighAlarmValue(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DDTTt_HighAlarmValue, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDTTt_PollingInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_PollingInterval, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.PollingInterval);
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.PollingInterval);
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.PollingInterval);
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDTTt_PollingInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DDTTt_PollingInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDTTt_HighAlarmTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDTTt_HighAlarmTime, value);
	if(ret)
	{
		return -1;
	}*/
	a_TemperatureSensorInfo temperatureSensorInfo;
	char *index = parseTemplate(path_name, ".TemperatureSensor.");
	if (index != NULL){
		memset(&temperatureSensorInfo, 0, sizeof(a_TemperatureSensorInfo));
		if (atoi(index) <= 11){ //core cpu
			getCoreChipTemperatureStatus(atoi(index)-1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.ResetTime); //with the same as resettime
		}
		else if (atoi(index) == 12){ //wifi5g chip
			getWifiChipTemperatureStatus(0, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.ResetTime); //with the same as resettime
		}
		else if (atoi(index) == 13){ //wifi2.4g chip
			getWifiChipTemperatureStatus(1, &temperatureSensorInfo);
			strcpy(value, temperatureSensorInfo.ResetTime); //with the same as resettime
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDN_MaxTCPWindowSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDN_MaxTCPWindowSize, value);
	if(ret)
	{
		return -1;
	}*/
	FILE *fp = NULL;
	char buff[128] = {0};
	char * ptr = NULL;

	fp = fopen("/proc/sys/net/core/wmem_max", "r");
	if (fp != NULL) 
	{
		fgets(buff, sizeof(buff), fp);
		if ((ptr = strstr(buff, "\n")) != NULL)
			*ptr = '\0';
		strcpy(value, buff);
		fclose(fp);
	}
	else
	{
		return -1;
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDN_TCPImplementation(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDN_TCPImplementation, value);
	if(ret)
	{
		return -1;
	}*/
	FILE *fp = NULL;
	char buff[128] = {0};
    char *p = NULL;
    char *q = NULL;

	fp = fopen("/proc/sys/net/ipv4/tcp_available_congestion_control", "r");
	if(fp != NULL)
	{
		fgets(buff, sizeof(buff), fp);
		fclose(fp);
		
        q = buff;
        while((p = strstr(q, " ")) != NULL)
        {
            *p = ',';
            q = p;
        }
		if ((p = strstr(buff, "\n")) != NULL)
			*p = '\0';
		strcpy(value, buff);
	}
	else
	{
		return -1;
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDPt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Processor.");
	char newpath[128] = {0};
	if (index != NULL){
		sprintf(newpath, "trconf.Device_DeviceInfo_Processor_%s.Alias", index);
		ret = do_uci_get(newpath, value);
		if(ret)
		{
			return -1;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DDPt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *index = parseTemplate(path_name, ".Processor.");
	char newpath[128] = {0};

	if (!isalpha(value[0]))
		return -2;

	if (strlen(value) > 64)
		return -2;
	
	if (index != NULL){
		sprintf(newpath, "trconf.Device_DeviceInfo_Processor_%s.Alias", index);
		ret = do_uci_set(newpath, value);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit(MS);
			if(ret)
			{
				return (-1);
			}
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDPt_Architecture(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDPt_Architecture, value);
	if(ret)
	{
		return -1;
	}*/

	char arch[32] = {0};
	char line[512] = {0};
	FILE *fd = NULL;

	//All CPU with the same architecture
	fd = popen("cat /proc/cpuinfo | grep 'model name'", "r");
	if(fd != NULL){
		fgets(line, sizeof(line), fd);
		sscanf(line, "%*s %*s %*s %s %*s", arch);
		strcpy(value, arch);
		pclose(fd);
	}

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDVt_Alias_75(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDVt_Alias_75, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".VendorLogFile.");
	char newpath[128] = {0};
	if (index != NULL){
		sprintf(newpath, "trconf.Device_DeviceInfo_VendorLogFile_%s.Alias", index);
		ret = do_uci_get(newpath, value);
		if(ret)
		{
			strcpy(value, "");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDVt_Alias_75(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DDVt_Alias_75, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *index = parseTemplate(path_name, ".VendorLogFile.");
	char newpath[128] = {0};
	char tmp[128] = {0};

	if (!isalpha(value[0]))
		return -2;

	if (strlen(value) > 64)
		return -2;
		
	if (index != NULL){
		sprintf(newpath, "trconf.Device_DeviceInfo_VendorLogFile_%s.Alias", index);
		ret = do_uci_get(newpath, tmp);
		if(ret)
		{
			sprintf(newpath, "trconf.Device_DeviceInfo_VendorLogFile_%s", index);
			do_uci_set(newpath, "acs");
		}
		
		sprintf(newpath, "trconf.Device_DeviceInfo_VendorLogFile_%s.Alias", index);
		ret = do_uci_set(newpath, value);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit(MS);
			if(ret)
			{
				return (-1);
			}
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDVt_Name_77(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	char *index = parseTemplate(path_name, ".VendorLogFile.");
	if (index != NULL)
	{
		if(atoi(index) == 1)
		{
			ret = do_uci_get("system.system.log_file", value);
			if(ret)
			{
				return -1;
			}
		}
		else if(atoi(index) == 2)
		{
			strcpy(value, "/tmp/vendor/rebootreason.txt");
		}
		else if(atoi(index) == 3)
		{
			strcpy(value, "/tmp/vendor/avail.txt");
		}
		else
		{
			return -1;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDVt_MaximumSize(char * path_name, char *value)
{
	int ret = 0;
	struct stat theStat;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	char *index = parseTemplate(path_name, ".VendorLogFile.");
	if (index != NULL)
	{
		if(atoi(index) == 1)
		{
			ret = do_uci_get("system.system.log_size", value);
			if(ret)
			{
				return -1;
			}
		}
		else if(atoi(index) == 2)
		{
			if(stat( "/tmp/vendor/rebootreason.txt", &theStat ) != -1)
			{
				sprintf(value, "%lld", (long long)theStat.st_size);
			}
			else
			{
				strcpy(value, "0");
			}
		}
		else if(atoi(index) == 3)
		{
			if(stat( "/tmp/vendor/avail.txt", &theStat ) != -1)
			{
				sprintf(value, "%lld", (long long)theStat.st_size);
			}
			else
			{
				strcpy(value, "0");
			}
		}
		else
		{
			return -1;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDVt_Persistent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DDVt_Persistent, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDP_ManufacturerOUI(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDP_ManufacturerOUI, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDP_ProductClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDP_ProductClass, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDP_SerialNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDP_SerialNumber, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDP_ProxyProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDP_ProxyProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_Source(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_Source, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_AcquiredTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_AcquiredTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_ExternalSource(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_ExternalSource, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_ExternalProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_ExternalProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_DataObject(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_DataObject, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLt_DataObject(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLt_DataObject, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_EnableCWMP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_EnableCWMP, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_EnableCWMP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_EnableCWMP, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_URL(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_URL, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_URL(char * path_name, char *value)
{
	int ret = 0;
	char old_value[256] = {0};
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;

	do_uci_get(DM_URL, old_value);
	if(strcmp(old_value, value))
		tr_remove( FLAG_BOOTSTRAP );
	
	ret = do_uci_set(DM_URL, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_Username(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	char acs_identifier[32] = {0};
#if 0
	char defultFlag[32] = {0};

	do_uci_get(DM_UsernameFlag, defultFlag);
	if (atoi(defultFlag) == 0)
	{
		char OuiValue[64] = {0};
		char ProductClassValue[64] = {0};
		char SerialNumberValue[64] = {0};
		int res = 0;

		GET_NODE_VALUE( "Device.DeviceInfo.ManufacturerOUI", OuiValue );
		GET_NODE_VALUE( "Device.DeviceInfo.ProductClass", ProductClassValue );
		GET_NODE_VALUE( "Device.DeviceInfo.SerialNumber", SerialNumberValue );

		sprintf(value, "%s-%s-%s", OuiValue, ProductClassValue, SerialNumberValue);

		tr_log(LOG_DEBUG,"get value [%s]",value);
		return ret;
	}
#endif
	do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, acs_identifier);
	if(strcmp(acs_identifier, "TWC") == 0){
		ret = do_uci_get(DM_TWCUsername, value);
	}
	else{
		ret = do_uci_get(DM_Username, value);
	}
	
	if(ret)
	{
		strcpy(value, "");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_Username(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	char acs_identifier[32] = {0};
	
	if (strlen(value) > 256)
		return -2;

	do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, acs_identifier);
	if(strcmp(acs_identifier, "TWC") == 0){
		ret = do_uci_set(DM_TWCUsername, value);
	}
	else{
		ret = do_uci_set(DM_Username, value);
	}
	
	if(ret)
	{
		return (-1);
	}
	else
	{
		//do_uci_set(DM_UsernameFlag, "1");
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_Password(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char dmacs[32] = {0};

	if (managementServerPasswordFlag == 1){
		managementServerPasswordFlag = 0;

		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			ret = readFromNonvolatileFlashFile(DM_Password,value);
			if (ret)
			{
				return -1;
			}
		}
		else
		{
			ret = do_uci_get(DM_Password, value);
			if(ret)
			{
				return -1;
			}
		}
		tr_log(LOG_DEBUG,"password for auth[%s]",value);
	}
	else
		strcpy(value, ""); //When read, this parameter returns an empty string
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_Password(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char dmacs[32] = {0};

	if (strlen(value) > 256)
		return -2;
	
	do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
	if (strcmp(dmacs,"TWC") == 0)
		writeToNonvolatileFlashFile(DM_Password, value);
	else
		ret = do_uci_set(DM_Password, value);

	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_ScheduleReboot(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_ScheduleReboot, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_ScheduleReboot(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_ScheduleReboot, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		long int currenttimesec = 0;
		long int scheduletimesec = 0;
		currenttimesec = getLocalTimeWithSeconds();
		scheduletimesec = changedDateTimeToSeconds(value);
		if (currenttimesec < scheduletimesec){
			pthread_t id;
			schedulerebootsens = scheduletimesec - currenttimesec;
			pthread_create( &id, NULL, ( void * ) doScheduleReboot, NULL );
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_DelayReboot(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_DelayReboot, value);
	if(ret)
	{
		strcpy(value, "-1");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_DelayReboot(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (atoi(value) < 1) //don't allow to set less than 1s
		return -2;
	
	ret = do_uci_set(DM_DelayReboot, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	pthread_t id;
	dealayrebootsens = atoi(value);
    pthread_create( &id, NULL, ( void * ) doDelayReboot, NULL );
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_PeriodicInformEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_PeriodicInformEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_PeriodicInformEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_PeriodicInformEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_PeriodicInformInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_PeriodicInformInterval, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_PeriodicInformInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (atoi(value) < 1 || atoi(value) > 4294967294)
		return -2;
	
	ret = do_uci_set(DM_PeriodicInformInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_PeriodicInformTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_PeriodicInformTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_PeriodicInformTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_PeriodicInformTime, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_ParameterKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_ParameterKey, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_ParameterKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 32)
		return -2;
	
	ret = do_uci_set(DM_ParameterKey, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_ConnectionRequestURL(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_ConnectionRequestURL, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_ConnectionRequestUsername(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_ConnectionRequestUsername, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_ConnectionRequestUsername(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;

	value = skip_blanks(value);
	value = trim_blanks(value);
	if(!strcmp(value, ""))
		return -2;
	
	ret = do_uci_set(DM_ConnectionRequestUsername, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_ConnectionRequestPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	if (connectionRequestPasswordFlag == 1){
		connectionRequestPasswordFlag = 0;
		ret = do_uci_get(DM_ConnectionRequestPassword, value);
		if(ret)
		{
			return -1;
		}
	}
	else
		strcpy(value, ""); //When read, this parameter returns an empty string
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_ConnectionRequestPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;

	value = skip_blanks(value);
	value = trim_blanks(value);
	if(!strcmp(value, ""))
		return -2;

	ret = do_uci_set(DM_ConnectionRequestPassword, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_UpgradesManaged(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_UpgradesManaged, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_UpgradesManaged(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_UpgradesManaged, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_KickURL(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_KickURL, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_DownloadProgressURL(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_DownloadProgressURL, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_DefaultActiveNotificationThrottle(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_DefaultActiveNotificationThrottle, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_DefaultActiveNotificationThrottle(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_DefaultActiveNotificationThrottle, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_CWMPRetryMinimumWaitInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_CWMPRetryMinimumWaitInterval, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_CWMPRetryMinimumWaitInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (atoi(value) < 1 || atoi(value) > 65535)
		return -2;
	
	ret = do_uci_set(DM_CWMPRetryMinimumWaitInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_CWMPRetryIntervalMultiplier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_CWMPRetryIntervalMultiplier, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_CWMPRetryIntervalMultiplier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (atoi(value) < 1000 || atoi(value) > 65535)
		return -2;
		
	ret = do_uci_set(DM_CWMPRetryIntervalMultiplier, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_X_CHARTER_COM_ACSIdentifier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_X_CHARTER_COM_ACSIdentifier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 32)
		return -2;
		
	ret = do_uci_set(DM_X_CHARTER_COM_ACSIdentifier, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_HTTPConnectionRequestEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_HTTPConnectionRequestEnable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always 1
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_HTTPConnectionRequestEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DM_HTTPConnectionRequestEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_UDPConnectionRequestAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_UDPConnectionRequestAddress, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_STUNEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_STUNEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_STUNEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_STUNEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_STUNServerAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_STUNServerAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_STUNServerAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DM_STUNServerAddress, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_STUNServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_STUNServerPort, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_STUNServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (atoi(value) < 0 || atoi(value) > 65535)
		return -2;
	
	ret = do_uci_set(DM_STUNServerPort, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_STUNUsername(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_STUNUsername, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_STUNUsername(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DM_STUNUsername, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_STUNPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	if (managementServerSTUNPasswordFlag == 1) {
		managementServerSTUNPasswordFlag = 0;
		ret = do_uci_get(DM_STUNPassword, value);
		if(ret)
		{
			return -1;
		}
	}
	else
		strcpy(value, ""); //When read, this parameter returns an empty string
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_STUNPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DM_STUNPassword, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		//restartTR069CWMP();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_STUNMaximumKeepAlivePeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_STUNMaximumKeepAlivePeriod, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_STUNMaximumKeepAlivePeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (atoi(value) < -1)
		return -2;
	
	ret = do_uci_set(DM_STUNMaximumKeepAlivePeriod, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_STUNMinimumKeepAlivePeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_STUNMinimumKeepAlivePeriod, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_STUNMinimumKeepAlivePeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_STUNMinimumKeepAlivePeriod, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_NATDetected(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char stunenable[32] = {0};

	ret = do_uci_get(DM_NATDetected, value);
	if(ret)
	{
		return -1;
	}
	//When STUNEnable is false, this value MUST be false.
	ret = do_uci_get(DM_STUNEnable, stunenable);
	if(ret)
	{
		return -1;
	}
	if (atoi(stunenable) == 0)
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_AliasBasedAddressing(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_AliasBasedAddressing, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_InstanceMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_InstanceMode, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_InstanceMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strcmp(value, "InstanceNumber") != 0 &&  strcmp(value, "InstanceAlias") != 0)
		return -1;
	
	ret = do_uci_set(DM_InstanceMode, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_AutoCreateInstances(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_AutoCreateInstances, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_AutoCreateInstances(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_AutoCreateInstances, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_ManageableDeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_ManageableDeviceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	char keyvalue[MAXMAPITEMS][256];
	int  number = 0;
   
	number = get_Device_ManagementServer_ManageableDevice(keyvalue);
	sprintf(value, "%d", number);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_ManageableDeviceNotificationLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_ManageableDeviceNotificationLimit, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_ManageableDeviceNotificationLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DM_ManageableDeviceNotificationLimit, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_EmbeddedDeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/* = do_uci_get(DM_EmbeddedDeviceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //Not support, always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_VirtualDeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_VirtualDeviceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //Not support, always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_SupportedConnReqMethods(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_SupportedConnReqMethods, value);
	if(ret)
	{
		return -1;
	}*/
#ifdef XMPP
	strcpy(value, "HTTP,STUN,XMPP");
#else
	strcpy(value, "HTTP");
#endif
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_ConnReqXMPPConnection(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
#ifdef XMPP	
	ret = do_uci_get(DM_ConnReqXMPPConnection, value);
	if(ret)
	{
		return -1;
	}
	//strcpy(value, ""); //not support
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
#else
	strcpy(value, "");
#endif
}
int set_DM_ConnReqXMPPConnection(char * path_name, char *value)
{
	int ret = 0,runningInstance=0;
	char *index, curXmpp[256]={0};
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

#ifdef XMPP
	//stop the other xmpp agent if already running
	ret = do_uci_get(DM_ConnReqXMPPConnection, curXmpp);
	if(ret)
	{
		return -1;
	}

    index = parseTemplate(curXmpp,".Connection." );
    if(index)
    {
        runningInstance = atoi(index);
	}

	x_xmpp_agent_stop(runningInstance );

	ret = do_uci_set(DM_ConnReqXMPPConnection, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	
	//now start the new xmppagent
    index = parseTemplate(value,".Connection." );
    if(index)
    {
        runningInstance = atoi(index);
	}

	x_xmpp_agent_start(runningInstance );
	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
#else
	return -1;
#endif
}
int get_DM_ConnReqAllowedJabberIDs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
#ifdef XMPP
	ret = do_uci_get(DM_ConnReqAllowedJabberIDs, value);
	if(ret)
	{
		return -1;
	}
#else
	strcpy(value, "");
#endif
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_ConnReqAllowedJabberIDs(char * path_name, char *value)
{
	int ret = 0;
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
#ifdef XMPP
	ret = do_uci_set(DM_ConnReqAllowedJabberIDs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
#else
	return -1;
#endif
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_ConnReqJabberID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
#ifdef XMPP
	ret = do_uci_get(DM_ConnReqJabberID, value);
	if(ret)
	{
		return -1;
	}
#else
	strcpy(value, "");
#endif
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_HTTPCompressionSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_HTTPCompressionSupported, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "Compress");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_HTTPCompression(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_HTTPCompression, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "Compress");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_HTTPCompression(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	//make sure its one of the supported type or disabled
	/*ret = do_uci_set(DM_HTTPCompression, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_LightweightNotificationProtocolsSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_LightweightNotificationProtocolsSupported, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //not support
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_LightweightNotificationProtocolsUsed(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_LightweightNotificationProtocolsUsed, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //not support
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_LightweightNotificationProtocolsUsed(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DM_LightweightNotificationProtocolsUsed, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_UDPLightweightNotificationHost(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_UDPLightweightNotificationHost, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //not support
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_UDPLightweightNotificationHost(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DM_UDPLightweightNotificationHost, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_UDPLightweightNotificationPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_UDPLightweightNotificationPort, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //not support
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_UDPLightweightNotificationPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DM_UDPLightweightNotificationPort, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_InformParameterNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DM_InformParameterNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	sprintf(value, "%d", INFORMPARA_MAX_INSTANCE_NUM);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_X_TWC_COM_ValidateManagementServerCertificate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char dmacs[32] = {0};

	do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
	if (strcmp(dmacs,"TWC") == 0)
	{
		ret = readFromNonvolatileFlashFile(DM_X_TWC_COM_ValidateManagementServerCertificate,value);
		if (ret)
		{
			ret = do_uci_get(DM_X_TWC_COM_ValidateManagementServerCertificate, value);
			if(ret)
			{
				return -1;
			}
		}
	}
	else
	{
		ret = do_uci_get(DM_X_TWC_COM_ValidateManagementServerCertificate, value);
		if(ret)
		{
			return -1;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_X_TWC_COM_ValidateManagementServerCertificate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char dmacs[32] = {0};

	do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
	if (strcmp(dmacs,"TWC") == 0)
		writeToNonvolatileFlashFile(DM_X_TWC_COM_ValidateManagementServerCertificate, value);
	ret = do_uci_set(DM_X_TWC_COM_ValidateManagementServerCertificate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_X_TWC_COM_ValidateDownloadServerCertificate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char dmacs[32] = {0};

	do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
	if (strcmp(dmacs,"TWC") == 0)
	{
		ret = readFromNonvolatileFlashFile(DM_X_TWC_COM_ValidateDownloadServerCertificate,value);
		if (ret)
		{
			ret = do_uci_get(DM_X_TWC_COM_ValidateDownloadServerCertificate, value);
			if(ret)
			{
				return -1;
			}
		}
	}
	else
	{
		ret = do_uci_get(DM_X_TWC_COM_ValidateDownloadServerCertificate, value);
		if(ret)
		{
			return -1;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DM_X_TWC_COM_ValidateDownloadServerCertificate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char dmacs[32] = {0};

	do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
	if (strcmp(dmacs,"TWC") == 0)
		writeToNonvolatileFlashFile(DM_X_TWC_COM_ValidateDownloadServerCertificate, value);
	ret = do_uci_set(DM_X_TWC_COM_ValidateDownloadServerCertificate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DM_X_TWC_COM_RootCertificateNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_X_TWC_COM_RootCertificateNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMXt_Enabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");
	char name[256] = {0};
	char dmacs[32] = {0};

	if (index != NULL)
	{
		sprintf(name, "%s%d", DMXt_Enabled, atoi(index));
	}
	do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
	if (strcmp(dmacs,"TWC") == 0)
	{
		ret = readFromNonvolatileFlashFile(name,value);
		if(ret)
		{
			ret = do_uci_get(name, value);
			if(ret)
			{
				strcpy(value, "0");
				ret = 0;
			}
		}
	}
	else
	{
		ret = do_uci_get(name, value);
		if(ret)
		{
			strcpy(value, "0");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMXt_Enabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char name[256] = {0};
	char dmacs[32] = {0};
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL)
	{
		sprintf(name, "%s%d", DMXt_Enabled, atoi(index));
	}
	do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
	if (strcmp(dmacs,"TWC") == 0)
		writeToNonvolatileFlashFile(name,value);
	
	ret = do_uci_set(name, value);
	if(ret)
	{
		tr_log(LOG_DEBUG,"do_uci_set: set RootCertificate enable or disable value fail....");
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			tr_log(LOG_DEBUG,"do_uci_commit: commit RootCertificate enable or disable value fail....");
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMXt_Certificate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char name[256] = {0};
	char dmacs[32] = {0};
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL)
	{
		sprintf(name, "%s%d", DMXt_Certificate, atoi(index));
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			FILE *fp = NULL;
			char cert[4096] = {0};
			int Certlen = 0;
			char filename[64] = {0};
			sprintf(filename,"/tmp/vendor/cert%d",atoi(index));
        	fp = fopen(filename, "r");
        	if(fp != NULL)
        	{
				Certlen = fread(cert,1,4096,fp);
				strncpy(value, cert, Certlen+1);
				fclose(fp);
			}
			else
			{
				tr_log(LOG_DEBUG,"read certfile fail");
				ret = do_uci_get(name, value);
				if(ret)
				{
					ret = 0;
					strcpy(value, "");
				}
			}
		}
		else
		{
			ret = do_uci_get(name, value);
			if(ret)
			{
				ret = 0;
				strcpy(value, "");
			}
		}
	}
	
	/*char filename[256] = {0};
	FILE *fp = NULL;
	char cert[4096] = {0};
	int Certlen = 0;
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");
	if (index != NULL)
	{
		sprintf(filename, "/oneagent/conf/ca%d", atoi(index));
		fp = fopen(filename,"r");
		if(fp != NULL){
			Certlen = fread(cert,1,4096,fp);
			strncpy(value, cert, Certlen+1);
			fclose(fp);
		}
		else
			strcpy(value, "");
	}*/
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMXt_Certificate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DMXt_Certificate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char name[256] = {0};
	char acsidname[256] = {0};
	char filename[256] = {0};
	char cert[4096] = {0};
	FILE *fp = NULL;
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");
	char head[30] = "-----BEGIN CERTIFICATE-----";
	char end[30] = "-----END CERTIFICATE-----";
	char line[10] = "\r\n";
	char dmacs[32] = {0};

	if (index != NULL)
	{
		sprintf(name,"%s%d",DMXt_Certificate,atoi(index));
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		
		sprintf(acsidname,"%s%d",DMXt_X_CHARTER_COM_ACSIdentifier,atoi(index));
		do_uci_set(acsidname, dmacs);
		
		if (strcmp(dmacs,"TWC") == 0)
			ret = writeToNonvolatileCertFile(atoi(index), value);
		ret = do_uci_set(name, value);
		if(ret)
		{
			tr_log(LOG_DEBUG,"do_uci_set: set RootCertificate value fail....");
			return (-1);
		}
		else
		{
			ret = do_uci_commit(MS);
			if(ret)
			{
				tr_log(LOG_DEBUG,"do_uci_commit: commit RootCertificate value fail....");
				return (-1);
			}
		}

		sprintf(cert,"%s%s%s%s%s",head,line,value,line,end);
		sprintf(filename,"/oneagent/conf/ca%d",atoi(index));
		fp = fopen(filename,"w+");
		if(fp != NULL){
			char cmd[256] = {0};
			char filename1[256] = {0};
			fwrite(cert, 1, strlen(cert), fp);
			fclose(fp);
			sprintf(filename1,"/oneagent/conf/ca%d.pem",atoi(index));
			sprintf(cmd, "openssl x509 -outform PEM -in %s -out %s",filename,filename1);
			system(cmd);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMXt_LastModif(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMXt_LastModif, value);
	if(ret)
	{
		return -1;
	}*/
	char dmacs[32] = {0};
	struct stat st;
	char filename[256] = {0};
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL){
		sprintf(filename,"/oneagent/conf/ca%d",atoi(index));
		if (stat(filename, &st) == 0)
			changedSecondsToDateTime(st.st_mtime,value);
		else
		{
			do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
			if (strcmp(dmacs,"TWC") == 0)
			{
				sprintf(filename,"/tmp/vendor/cert%d",atoi(index));
				if (stat(filename, &st) == 0)
					changedSecondsToDateTime(st.st_mtime,value);
				else
					strcpy(value, "0001-01-01T00:00:00Z");
			}
			else
				strcpy(value, "0001-01-01T00:00:00Z");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMXt_SerialNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMXt_SerialNumber, value);
	if(ret)
	{
		return -1;
	}*/
	char dmacs[32] = {0};
	X509 *caCert = NULL;
	ASN1_INTEGER *bs = NULL;  
    BIGNUM  *bn = NULL;  
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL)
	{
		int i = atoi(index);
		
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			tr_log(LOG_DEBUG,"call write_vendor_file");
			write_vendor_file(i);	
		}
		ret = pem_to_x509(i, &caCert);
		if (ret == 0){
			bs = X509_get_serialNumber(caCert);
			if (bs != NULL){
				if (bs->length == 0) {
					strcpy(value, "");
				}
				else{
					bn = ASN1_INTEGER_to_BN(bs, NULL);
					if (bn != NULL){
						strcpy(value,BN_bn2hex(bn));
						BN_free(bn);
					}
					else
						strcpy(value, "");
				}
				free(bs);
			}
			else
				strcpy(value, "");
			if (caCert != NULL)
				free(caCert);
		}
		else{
			strcpy(value, "");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMXt_Issuer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMXt_Issuer, value);
	if(ret)
	{
		return -1;
	}*/
	char dmacs[32] = {0};
	X509 *caCert = NULL; 
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL)
	{
		int i = atoi(index);
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			tr_log(LOG_DEBUG,"call write_vendor_file");
			write_vendor_file(i);	
		}
		ret = pem_to_x509(i, &caCert);
		if (ret == 0){
			strcpy(value,X509_NAME_oneline( X509_get_issuer_name( caCert ), NULL, 0));
			if (caCert != NULL){
				free(caCert);
			}
		}
		else{
			strcpy(value, "");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMXt_NotBefore(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMXt_NotBefore, value);
	if(ret)
	{
		return -1;
	}*/
	char dmacs[32] = {0};
	X509 *caCert = NULL;
	ASN1_TIME *start = NULL;
	time_t ttStart = {0};
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL)
	{
		int i = atoi(index);
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			tr_log(LOG_DEBUG,"call write_vendor_file");
			write_vendor_file(i);	
		}
		ret = pem_to_x509(i, &caCert);
		if (ret == 0){
			start = X509_get_notBefore(caCert);
			if (start != NULL){
				/*ttStart = ASN1_UTCTIME_get(start);
				tr_log(LOG_DEBUG,"ttStart [%ld]",ttStart);
				changedSecondsToDateTime(ttStart,value);*/
				ASN1_UTCTIME_get1(start, value);
			}
			else
				strcpy(value, "0001-01-01T00:00:00Z");
			if (caCert != NULL)
				free(caCert);
		}
		else{
			strcpy(value, "0001-01-01T00:00:00Z");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMXt_NotAfter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMXt_NotAfter, value);
	if(ret)
	{
		return -1;
	}*/
	char dmacs[32] = {0};
	X509 *caCert = NULL;
	ASN1_TIME *end = NULL; 
	time_t ttEnd = {0};
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL)
	{
		int i = atoi(index);
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			tr_log(LOG_DEBUG,"call write_vendor_file");
			write_vendor_file(i);	
		}
		ret = pem_to_x509(i, &caCert);
		if (ret == 0){
			end = X509_get_notAfter(caCert);
			if (end != NULL){
				/*ttEnd = ASN1_UTCTIME_get(end);
				tr_log(LOG_DEBUG,"ttEnd [%ld]",ttEnd);
				if(ttEnd == -1)
				{
					ASN1_UTCTIME_get1(end, value);
				}
				else
				{
					changedSecondsToDateTime(ttEnd,value);
				}*/
				ASN1_UTCTIME_get1(end, value);
			}
			else
				strcpy(value, "0001-01-01T00:00:00Z");
			if (caCert != NULL)
				free(caCert);
		}
		else{
			strcpy(value, "0001-01-01T00:00:00Z");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMXt_Subject(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMXt_Subject, value);
	if(ret)
	{
		return -1;
	}*/
	char dmacs[32] = {0};
	X509 *caCert = NULL; 
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL)
	{
		int i = atoi(index);
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			write_vendor_file(i);	
			tr_log(LOG_DEBUG,"call write_vendor_file");
		}
		ret = pem_to_x509(i, &caCert);
		if (ret == 0){
			strcpy(value,X509_NAME_oneline( X509_get_subject_name( caCert ), NULL, 0));
			if (caCert != NULL)
				free(caCert);
		}
		else{
			strcpy(value, "");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMXt_SubjectAlt(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMXt_SubjectAlt, value);
	if(ret)
	{
		return -1;
	}*/
	char dmacs[32] = {0};
	X509 *caCert = NULL;
	char subalt[256] = {0};
	STACK_OF(GENERAL_NAME) *gens = NULL;
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL)
	{
		int i = atoi(index);
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			tr_log(LOG_DEBUG,"call write_vendor_file");
			write_vendor_file(i);	
		}
		ret = pem_to_x509(i, &caCert);
		if (ret == 0){
			gens = X509_get_ext_d2i(caCert, NID_subject_alt_name, NULL, NULL);
			if (gens != NULL){
				GENERAL_NAME *gen = NULL;
				ASN1_IA5STRING *cstr = NULL;
				tr_log(LOG_DEBUG,"can get gens");
				for (i = 0; i < sk_GENERAL_NAME_num(gens); i++) {
					gen = sk_GENERAL_NAME_value(gens, i);
					if (gen != NULL && gen->type == GEN_DNS){
						cstr = gen->d.dNSName;
						if(cstr != NULL){
							if (subalt[0] == '\0')
								sprintf(subalt, "%s", cstr->data);
							else
								sprintf(subalt, "%s,%s",subalt, cstr->data);
							free(cstr);
							tr_log(LOG_DEBUG,"subalt[%s]",subalt);
						}
						free(gen);
					}
				}
				strcpy(value, subalt);
				if (gens != NULL)
					free(gens);
			}
			else{
				tr_log(LOG_DEBUG,"can't get gens");
				strcpy(value, "");
			}
			if (caCert != NULL)
				free(caCert);
		}
		else{
			strcpy(value, "");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMXt_SignatureAlgorithm(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMXt_SignatureAlgorithm, value);
	if(ret)
	{
		return -1;
	}*/
	char dmacs[32] = {0};
	X509 *caCert = NULL;
	ASN1_OBJECT* salg  = NULL;
	char oid[128] = {0};
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");

	if (index != NULL)
	{
		int i = atoi(index);
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			tr_log(LOG_DEBUG,"call write_vendor_file");
			write_vendor_file(i);	
		}
		ret = pem_to_x509(i, &caCert);
		if (ret == 0){
			salg = caCert->sig_alg->algorithm;
			if(salg != NULL){
				OBJ_obj2txt(oid, 128, caCert->sig_alg->algorithm, 1);
				strcpy(value,oid);
				free(salg);
			}
			else{
				tr_log(LOG_DEBUG,"get sig_alg->algorithm error");
				strcpy(value, "");
			}
			if (caCert != NULL)
				free(caCert);
		}
		else{
			strcpy(value, "");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMXt_X_CHARTER_COM_ACSIdentifier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char acsidname[256] = {0};
	char *index = parseTemplate(path_name, ".X_TWC_COM_RootCertificate.");
	sprintf(acsidname,"%s%d",DMXt_X_CHARTER_COM_ACSIdentifier,atoi(index));
	ret = do_uci_get(acsidname, value);
	if(ret)
	{
		char dmacs[32] = {0};
		do_uci_get(DM_X_CHARTER_COM_ACSIdentifier, dmacs);
		if (strcmp(dmacs,"TWC") == 0)
		{
			strcpy(value,dmacs);
			tr_log(LOG_DEBUG,"value[%s]",value);
			ret = 0;
		}
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMMt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMMt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMMt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DMMt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMMt_ManufacturerOUI(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMMt_ManufacturerOUI, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".ManageableDevice.");
	char mac[32] = {0};

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ManagementServerManageableDeviceMapMap, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			char filename[128] = {0};
			char info[64] = {0};
			char *p = NULL;
			sprintf(filename, "/tmp/%s_option125", mac);
			getManagementServerManageableDeviceInfo(filename, "ManufacturerOUI", info);
			if ((p = strstr(info, "\n")) != NULL || (p = strstr(info, "\r")) != NULL)
				*p = '\0';
			strcpy(value, info);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMMt_SerialNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMMt_SerialNumber, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".ManageableDevice.");
	char mac[32] = {0};

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ManagementServerManageableDeviceMapMap, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			char filename[128] = {0};
			char info[64] = {0};
			char *p = NULL;
			sprintf(filename, "/tmp/%s_option125", mac);
			getManagementServerManageableDeviceInfo(filename, "SerialNumber", info);
			if ((p = strstr(info, "\n")) != NULL || (p = strstr(info, "\r")) != NULL)
				*p = '\0';
			strcpy(value, info);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMMt_ProductClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMMt_ProductClass, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".ManageableDevice.");
	char mac[32] = {0};

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ManagementServerManageableDeviceMapMap, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			char filename[128] = {0};
			char info[64] = {0};
			char *p = NULL;
			sprintf(filename, "/tmp/%s_option125", mac);
			getManagementServerManageableDeviceInfo(filename, "ProductClass", info);
			if ((p = strstr(info, "\n")) != NULL || (p = strstr(info, "\r")) != NULL)
				*p = '\0';
			strcpy(value, info);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMMt_Host(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMMt_Host, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".ManageableDevice.");
	char mac[32] = {0};
	char mac2[32] = {0};
	int i = 1;

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(ManagementServerManageableDeviceMapMap, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 1; ;i ++){
				ret = lib_getvalue_mapfile_byinstance(HostsHostMap, mac2, i);
				if(ret)
				{
					strcpy(value, ""); //no host entry, value an empty string
					break;
				}
				else
				{
					if (strcasecmp(mac, mac2) == 0){
						sprintf(value, "Device.Hosts.Host.%d", i);
						break;
					}
				}
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMA_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMA_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMA_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMA_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMA_TransferTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMA_TransferTypeFilter, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMA_TransferTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strcmp(value, "Upload") != 0 && strcmp(value, "Download") != 0 && strcmp(value, "Both") != 0)
		return -2;
	
	ret = do_uci_set(DMA_TransferTypeFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMA_ResultTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMA_ResultTypeFilter, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMA_ResultTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strcmp(value, "Success") != 0 && strcmp(value, "Failure") != 0 && strcmp(value, "Both") != 0)
		return -2;
	
	ret = do_uci_set(DMA_ResultTypeFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMA_FileTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMA_FileTypeFilter, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMA_FileTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 1024)
		return -2;
	
	ret = do_uci_set(DMA_FileTypeFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMA_Enable_177(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMA_Enable_177, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMA_Enable_177(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMA_Enable_177, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMA_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMA_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMA_GroupNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMA_GroupNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMAGt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMAGt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMAGt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMAGt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMAGt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMAGt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMAGt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMAGt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMAGt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMAGt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMAGt_URL(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMAGt_URL, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMAQ_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMAQ_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMAQ_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMAQ_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMAQ_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMAQ_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMAQ_URL(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMAQ_URL, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMAQ_URL(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMAQ_URL, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMD_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMD_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMD_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMD_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMD_OperationTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMD_OperationTypeFilter, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMD_OperationTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strcmp(value, "Install") != 0 && strcmp(value, "Update") != 0 && strcmp(value, "Uninstall") != 0)
		return -2;
	
	ret = do_uci_set(DMD_OperationTypeFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMD_ResultTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMD_ResultTypeFilter, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMD_ResultTypeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strcmp(value, "Success") != 0 && strcmp(value, "Failure") != 0 && strcmp(value, "Both") != 0)
		return -2;
	
	ret = do_uci_set(DMD_ResultTypeFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMD_FaultCodeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMD_FaultCodeFilter, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMD_FaultCodeFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strcmp(value, "9001") != 0 && strcmp(value, "9003") != 0 && strcmp(value, "9012") != 0
		&& strcmp(value, "9013") != 0 && strcmp(value, "9015") != 0 && strcmp(value, "9016") != 0
		&& strcmp(value, "9017") != 0 && strcmp(value, "9018") != 0 && strcmp(value, "9022") != 0
		&& strcmp(value, "9023") != 0 && strcmp(value, "9024") != 0 && strcmp(value, "9025") != 0
		&& strcmp(value, "9026") != 0 && strcmp(value, "9027") != 0 && strcmp(value, "9028") != 0
		&& strcmp(value, "9029") != 0 && strcmp(value, "9030") != 0 && strcmp(value, "9031") != 0
		&& strcmp(value, "9032") != 0)
		return -2;
	
	ret = do_uci_set(DMD_FaultCodeFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMEt_ControllerID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_ControllerID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_ProxiedDeviceID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_ProxiedDeviceID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_Reference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_Reference, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_SupportedDataModel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_SupportedDataModel, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_Host(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_Host, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_ProxyProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_ProxyProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_ProxyProtocolReference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_ProxyProtocolReference, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_DiscoveryProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_DiscoveryProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_DiscoveryProtocolReference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_DiscoveryProtocolReference, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_CommandProcessed(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_CommandProcessed, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_ommandProcessingErrMsg(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_ommandProcessingErrMsg, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMEt_LastSyncTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMEt_LastSyncTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMEt_LastSyncTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMEt_LastSyncTime, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMVt_ManufacturerOUI(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMVt_ManufacturerOUI, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMVt_ProductClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMVt_ProductClass, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMVt_SerialNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMVt_SerialNumber, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMVt_Host(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMVt_Host, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMVt_ProxyProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMVt_ProxyProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMVt_ProxyProtocolReference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMVt_ProxyProtocolReference, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMVt_DiscoveryProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMVt_DiscoveryProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMVt_DiscoveryProtocolReference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMVt_DiscoveryProtocolReference, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMS_CRUnawarenessMaxDuration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMS_CRUnawarenessMaxDuration, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMS_CRUnawarenessMaxDuration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMS_CRUnawarenessMaxDuration, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMS_MaxMissedPeriodic(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMS_MaxMissedPeriodic, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMS_MaxMissedPeriodic(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMS_MaxMissedPeriodic, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMS_NotifyMissedScheduled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMS_NotifyMissedScheduled, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMS_NotifyMissedScheduled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMS_NotifyMissedScheduled, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMS_NetworkAwarenessCapable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMS_NetworkAwarenessCapable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMS_SelfTimerCapable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMS_SelfTimerCapable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMS_CRAwarenessRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMS_CRAwarenessRequested, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMS_CRAwarenessRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMS_CRAwarenessRequested, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMS_PeriodicAwarenessRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMS_PeriodicAwarenessRequested, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMS_PeriodicAwarenessRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMS_PeriodicAwarenessRequested, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMS_ScheduledAwarenessRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMS_ScheduledAwarenessRequested, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMS_ScheduledAwarenessRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMS_ScheduledAwarenessRequested, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMIt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	int ret2 = 0;
	char buff[128] = {0};
	char name[256] = {0};
	char *p = parseTemplate(path_name, ".InformParameter.");

	if (p == NULL)
		return -1;
	else
	{
		sprintf(name, "%s%d", DMIt_Enable, atoi(p));
		ret = do_uci_get(name, value);
		if(ret)
		{
			if(atoi(p) > INFORMPARA_MAX_INSTANCE_NUM)
			{
				return -1;
			}
			ret2 = getinformparaEnbl(p, buff);
			if (ret2 == 0)
				strcpy(value, buff);
			else
				strcpy(value, "1");
			ret = 0;
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DMIt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char name[256] = {0};
	char *p = parseTemplate(path_name, ".InformParameter.");

	if (p == NULL)
		return -1;
	else
	{
		sprintf(name,"%s%d",DMIt_Enable,atoi(p));
		ret = do_uci_set(name, value);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit(MS);
			if(ret)
			{
				return (-1);
			}
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMIt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;

	/*ret = do_uci_set(DMIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_ParameterName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMIt_ParameterName, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".InformParameter.");

	if (p == NULL)
		return -1;
	else
	{
		if(atoi(p) > INFORMPARA_MAX_INSTANCE_NUM)
		{
			return -1;
		}
		getinformparaName(p, buff);
		strcpy(value, buff);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_ParameterName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DMIt_ParameterName, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_EventList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DMIt_EventList, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".InformParameter.");

	if (p == NULL)
		return -1;
	else
	{
		if(atoi(p) > INFORMPARA_MAX_INSTANCE_NUM)
		{
			return -1;
		}
		getinformparaEvent(p, buff);
		strcpy(value, buff);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_EventList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_EventList, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DG_ManufacturerOUI(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DG_ManufacturerOUI, value);
	if(ret)
	{
		return -1;
	}*/
	char mac[32] = {0};

	getNextHopGwMac(mac);
	if (mac[0] != '\0'){
		int i = 0, j = 0;
		for (i = 0; i < 8; i ++){
			if (mac[i] != ':'){
				value[j] = toupper(mac[i]);
				j ++;
			}
		}
		value[j] = '\0';
	}
	else
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DG_ProductClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DG_ProductClass, value);
	if(ret)
	{
		return -1;
	}*/
	char mac[32] = {0};

	getNextHopGwMac(mac);
	if (mac[0] != '\0'){
		int i = 0, j = 0;
		for (i = 0; i < 8; i ++){
			if (mac[i] != ':'){
				value[j] = toupper(mac[i]);
				j ++;
			}
		}
		value[j] = '\0';
	}
	else
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DG_SerialNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DG_SerialNumber, value);
	if(ret)
	{
		return -1;
	}*/
	char mac[32] = {0};

	getNextHopGwMac(mac);
	if (mac[0] != '\0'){
		int i = 0, j = 0;
		for (i = 0; i < 8; i ++){
			if (mac[i] != ':'){
				value[j] = toupper(mac[i]);
				j ++;
			}
		}
		value[j] = '\0';
	}
	else
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DT_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DT_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always 1, device doesn't support to disable
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DT_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DT_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DT_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DT_Status, value);
	if(ret)
	{
		return -1;
	}*/
	a_infinfo wanStatus;

	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo("wan", &wanStatus);
	if (wanStatus.status == 1)
	{
		strcpy(value, "Synchronized"); 
	}
	else
	{
		strcpy(value, "Unsynchronized"); 
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DT_NTPServer1(char * path_name, char *value)
{
	int ret = 0;
	char ntpserverlist[4096] = {0};
	char ntpserver1[256] = {0};
	char ntpserver2[256] = {0};
	char ntpserver3[256] = {0};
	char ntpserver4[256] = {0};
	char ntpserver5[256] = {0};
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get("system.ntp.server", ntpserverlist);
	if(ret)
	{
		return -1;
	}
	sscanf(ntpserverlist, "%s %s %s %s %s", ntpserver1, ntpserver2, ntpserver3, ntpserver4, ntpserver5);
	strcpy(value, ntpserver1);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DT_NTPServer1(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DT_NTPServer1, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	ret = updatentpserverlist(1, value);
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DT_NTPServer2(char * path_name, char *value)
{
	int ret = 0;
	char ntpserverlist[4096] = {0};
	char ntpserver1[256] = {0};
	char ntpserver2[256] = {0};
	char ntpserver3[256] = {0};
	char ntpserver4[256] = {0};
	char ntpserver5[256] = {0};
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get("system.ntp.server", ntpserverlist);
	if(ret)
	{
		return -1;
	}
	sscanf(ntpserverlist, "%s %s %s %s %s", ntpserver1, ntpserver2, ntpserver3, ntpserver4, ntpserver5);
	strcpy(value, ntpserver2);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DT_NTPServer2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DT_NTPServer2, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	ret = updatentpserverlist(2, value);
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DT_NTPServer3(char * path_name, char *value)
{
	int ret = 0;
	char ntpserverlist[4096] = {0};
	char ntpserver1[256] = {0};
	char ntpserver2[256] = {0};
	char ntpserver3[256] = {0};
	char ntpserver4[256] = {0};
	char ntpserver5[256] = {0};
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get("system.ntp.server", ntpserverlist);
	if(ret)
	{
		return -1;
	}
	sscanf(ntpserverlist, "%s %s %s %s %s", ntpserver1, ntpserver2, ntpserver3, ntpserver4, ntpserver5);
	strcpy(value, ntpserver3);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DT_NTPServer3(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DT_NTPServer3, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	ret = updatentpserverlist(3, value);
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DT_NTPServer4(char * path_name, char *value)
{
	int ret = 0;
	char ntpserverlist[4096] = {0};
	char ntpserver1[256] = {0};
	char ntpserver2[256] = {0};
	char ntpserver3[256] = {0};
	char ntpserver4[256] = {0};
	char ntpserver5[256] = {0};
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get("system.ntp.server", ntpserverlist);
	if(ret)
	{
		return -1;
	}
	sscanf(ntpserverlist, "%s %s %s %s %s", ntpserver1, ntpserver2, ntpserver3, ntpserver4, ntpserver5);
	strcpy(value, ntpserver4);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DT_NTPServer4(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DT_NTPServer4, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/

	if (strlen(value) > 64)
		return -2;

	ret = updatentpserverlist(4, value);
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DT_NTPServer5(char * path_name, char *value)
{
	int ret = 0;
	char ntpserverlist[4096] = {0};
	char ntpserver1[256] = {0};
	char ntpserver2[256] = {0};
	char ntpserver3[256] = {0};
	char ntpserver4[256] = {0};
	char ntpserver5[256] = {0};
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get("system.ntp.server", ntpserverlist);
	if(ret)
	{
		return -1;
	}
	sscanf(ntpserverlist, "%s %s %s %s %s", ntpserver1, ntpserver2, ntpserver3, ntpserver4, ntpserver5);
	strcpy(value, ntpserver5);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DT_NTPServer5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DT_NTPServer5, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/

	if (strlen(value) > 64)
		return -2;

	ret = updatentpserverlist(5, value);
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DT_CurrentLocalTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DT_CurrentLocalTime, value);
	if(ret)
	{
		return -1;
	}*/
	FILE *fp = NULL;
	char line[128] = {0};
	char *p = NULL;

    if ((fp = popen("date \"+%Y-%m-%dT%H:%M:%SZ\"", "r")) != NULL) {
		fgets(line, sizeof(line)-1, fp);
		if((p = strstr(line, "\n")) != NULL)
			*p = '\0';
        strcpy(value, line);
        pclose(fp);
    }
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DT_LocalTimeZone(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get("system.system.timezone", value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DT_LocalTimeZone(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char cmd[64] = "";

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set("system.system.timezone", value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("system");
		if(ret)
		{
			return (-1);
		}
		sprintf(cmd, "echo \"%s\" > /tmp/TZ;", value);
		system(cmd);
		system("/etc/init.d/sysntpd restart");
		system("date -k"); 
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DU_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
		if (atoi(value) == 0)
			system("/etc/init.d/lighttpd stop &");
		else
			system("/etc/init.d/lighttpd start &");
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_PasswordRequired(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DU_PasswordRequired, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always 1
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_PasswordRequired(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DU_PasswordRequired, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	// don't do anything, always 1
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_PasswordUserSelectable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DU_PasswordUserSelectable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DU_PasswordReset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DU_PasswordReset, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_PasswordReset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DU_PasswordReset, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	if (atoi(value) == 1){
		FILE *fp = NULL;
		char line[128] = {0};
		int len = 0;
		int i = 0;
		int dohttp = 0;

		if ((fp = popen("uci -c /rom/etc/config get pwd.super_user.passwd", "r")) != NULL) { //super user
			fgets(line, sizeof(line), fp);
			len = strlen(line);
			for (i = 0; i<= len; i++){
				if (line[i] == '\n')
					line[i] = '\0';
			}
			pclose(fp);
			ret = do_uci_set("pwd.super_user.passwd", line);
			if(ret)
			{
				return (-1);
			}
			else
			{
				dohttp = 1;
			}
    	}

		if ((fp = popen("uci -c /rom/etc/config get pwd.manager.passwd", "r")) != NULL) { //manager user
			fgets(line, sizeof(line), fp);
			len = strlen(line);
			for (i = 0; i<= len; i++){
				if (line[i] == '\n')
					line[i] = '\0';
			}
			pclose(fp);
			ret = do_uci_set("pwd.manager.passwd", line);
			if(ret)
			{
				return (-1);
			}
			else
			{
				dohttp = 1;
			}
    	}

		if (dohttp == 1){
			ret = do_uci_commit("pwd");
			if(ret)
			{
				return (-1);
			}
			system("/etc/init.d/lighttpd restart &");
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_UpgradeAvailable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DU_UpgradeAvailable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_UpgradeAvailable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DU_UpgradeAvailable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	// don't do anything, always 0
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_WarrantyDate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_WarrantyDate, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "0001-01-01T00:00:00Z");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_WarrantyDate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DU_WarrantyDate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ISPName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ISPName, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ISPName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 64)
		return -2;
	
	ret = do_uci_set(DU_ISPName, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ISPHelpDesk(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ISPHelpDesk, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ISPHelpDesk(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 32)
		return -2;
	
	ret = do_uci_set(DU_ISPHelpDesk, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ISPHomePage(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ISPHomePage, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ISPHomePage(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DU_ISPHomePage, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ISPHelpPage(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ISPHelpPage, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ISPHelpPage(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DU_ISPHelpPage, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ISPLogo(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ISPLogo, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ISPLogo(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DU_ISPLogo, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ISPLogoSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ISPLogoSize, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ISPLogoSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (atoi(value) < 0 || atoi(value) > 4095)
		return -2;
	
	ret = do_uci_set(DU_ISPLogoSize, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ISPMailServer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ISPMailServer, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ISPMailServer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DU_ISPMailServer, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ISPNewsServer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ISPNewsServer, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ISPNewsServer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	
	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DU_ISPNewsServer, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_TextColor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_TextColor, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_TextColor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 6)
		return -2;
	
	ret = do_uci_set(DU_TextColor, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_BackgroundColor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_BackgroundColor, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_BackgroundColor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 6)
		return -2;
	
	ret = do_uci_set(DU_BackgroundColor, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ButtonColor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ButtonColor, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ButtonColor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 6)
		return -2;
	
	ret = do_uci_set(DU_ButtonColor, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_ButtonTextColor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_ButtonTextColor, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_ButtonTextColor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 6)
		return -2;
	
	ret = do_uci_set(DU_ButtonTextColor, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_AutoUpdateServer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_AutoUpdateServer, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_AutoUpdateServer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DU_AutoUpdateServer, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_UserUpdateServer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_UserUpdateServer, value);
	if(ret)
	{
		return -1;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_UserUpdateServer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (strlen(value) > 256)
		return -2;
	
	ret = do_uci_set(DU_UserUpdateServer, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_AvailableLanguages(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get("system.system.languages", value);
	if(ret)
	{
		strcpy(value, "en");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_AvailableLanguages(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DU_AvailableLanguages, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DU_CurrentLanguage(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get("system.system.languages", value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DU_CurrentLanguage(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	//don't support to change the language
	/*ret = do_uci_set(DU_CurrentLanguage, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	if (strlen(value) > 16)
		return -2;

	if((strcmp(value, "en") != 0) && (strcmp(value, "zh-cn") != 0) && (strcmp(value, "zh-tw") != 0))
	{
		return (-1);
	}
	
	ret = do_uci_set("system.system.languages", value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("system");
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUR_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(ADMIN_SYS_MISC_HTTP, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUR_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	
	//ret = doRemoteAccess(atoi(value));
	//ret = setRemoteAccess(atoi(value));
	/*
	if(ret)
	{
		return (-1);
	}*/
	ret = do_uci_set(ADMIN_SYS_MISC_HTTP, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("lighttpd");
		if(ret)
		{
			return (-1);
		}
		setRemoteAccess2();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUR_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	int http = -1;
	int https = -1;
	char http_port[32] = {0};
	char https_port[32] = {0};
	
	http = do_uci_get(ADMIN_SYS_MISC_HTTP_PORT, http_port);
	https = do_uci_get(ADMIN_SYS_MISC_HTTPS_PORT, https_port);

	if (getRemoteAccessMode() == 0){ //http
		if (!http)
			sprintf(value, "%s", http_port);
		else
			sprintf(value, "%s", "8081");
	}
	else{ //https
		if (!https)
			sprintf(value, "%s", https_port);
		else
			sprintf(value, "%s", "8444");
	}

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUR_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (atoi(value) < 0 || atoi(value) > 65535)
		return -2;

	if (atoi(value) == 80 || atoi(value) == 443)
		return -2;

	if (checkPortUsing(atoi(value)) == 1)
		return -2;

	if (getRemoteAccessMode() == 0) //http
		ret = do_uci_set(ADMIN_SYS_MISC_HTTP_PORT, value);
	else
		ret = do_uci_set(ADMIN_SYS_MISC_HTTPS_PORT, value); //HTTP
	
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("lighttpd");
		if(ret)
		{
			return (-1);
		}
		char tmp[16] = {0};
		do_uci_get(ADMIN_SYS_MISC_HTTP, tmp);
		//ret = setRemoteAccess(atoi(tmp));
		setRemoteAccess2();
	}
	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUR_SupportedProtocols(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	
	strcpy(value, "HTTP,HTTPS");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUR_Protocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (getRemoteAccessMode() == 0) //http
		strcpy(value, "HTTP");
	else
		strcpy(value, "HTTPS"); //HTTP

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUR_Protocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	int http = -1;
	int https = -1;
	char http_port[32] = {0};
	char https_port[32] = {0};

	if(strcasestr("HTTP, HTTPS", value) == NULL)
	{
		return (-2);
	}
	
	http = do_uci_get(ADMIN_SYS_MISC_HTTP_PORT, http_port);
	https = do_uci_get(ADMIN_SYS_MISC_HTTPS_PORT, https_port);

	if (!strcasecmp(value, "HTTP")){ //http
		do_uci_set(ADMIN_SYS_MISC_HTTP_MODE, "0");
		if (http) //set default value
			do_uci_set(ADMIN_SYS_MISC_HTTP_PORT, "8081");
		do_uci_delete(ADMIN_SYS_MISC_HTTPS_PORT, NULL);
	}
	else{ //https
		do_uci_set(ADMIN_SYS_MISC_HTTP_MODE, "1");
		if (https) //set default value
			do_uci_set(ADMIN_SYS_MISC_HTTPS_PORT, "8444");
		do_uci_delete(ADMIN_SYS_MISC_HTTP_PORT, NULL);
	}

	ret = do_uci_set(DUR_Protocol, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("lighttpd");
		if(ret)
		{
			return (-1);
		}
		char tmp[16] = {0};
		do_uci_get(ADMIN_SYS_MISC_HTTP, tmp);
		//ret = setRemoteAccess(atoi(tmp));
		setRemoteAccess2();
	}
	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUR_X_Charter_AllowSpecifiedIP(char * path_name, char *value)
{
	int ret = 0;
	char allow_ip[32] = {0};
	
	tr_log(LOG_DEBUG, "path_name[%s]", path_name);
	ret = do_uci_get(LOGIN_ALLOW_SPECIFIED_IP, value);
	if(ret){
		strcpy(value, "0");
		ret = 0;
	}
	
	tr_log(LOG_DEBUG, "get value [%s]", value);
	return ret;
}
int set_DUR_X_Charter_AllowSpecifiedIP(char * path_name, char *value)
{
	int ret = 0;
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LOGIN_ALLOW_SPECIFIED_IP, value);
	if(ret)
		return -1;

	ret = do_uci_commit("system");
	if(ret)
		return -1;
	
	//char tmp[16] = {0};
	//do_uci_get(ADMIN_SYS_MISC_HTTP, tmp);
	//ret = setRemoteAccess(atoi(tmp));
	setRemoteAccess2();
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUR_X_Charter_AllowedIpRanges(char * path_name, char *value)
{
	int ret = 0;
	char *p = NULL;
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LOGIN_ALLOWED_IP, value);
	if(ret){
		strcpy(value, "");
		ret = 0;
		return ret;
	}
	p = value;
	while(*p != '\0'){
		if(*p == ' ')
			*p = ',';
		p++;
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUR_X_Charter_AllowedIpRanges(char * path_name, char *value)
{
	int ret = 0;
	char ipranges[128] = {0};
	int j;
	char *str;
	char *token;
	char *saveptr1;
	char *ptr = NULL;
	char ipstart[32] = {0};
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	do_uci_delete(LOGIN_ALLOWED_IP, NULL);

	strncpy(ipranges, value, sizeof(ipranges));
	ipranges[sizeof(ipranges)-1] = 0;
	
	for (j = 1, str = ipranges; ; j++, str = NULL) {
		token = strtok_r(str, ",", &saveptr1);
		tr_log(LOG_DEBUG,"token[%s]",token);
		if (token == NULL)
			break;

		if((ptr = strchr(token, '-')) != NULL)
		{
			strncpy(ipstart, token, ptr-token);
			if ((isValidIP(ipstart) == 0) || (isValidIP(ptr+1) == 0))
			{
				return -2;
			}
		}
		else
		{
			if (isValidIP(token) == 0)
				return -2;
		}
	}

	strcpy(ipranges, value);
	for (j = 1, str = ipranges; ; j++, str = NULL) {
		token = strtok_r(str, ",", &saveptr1);
		if (token == NULL)
			break;
		if(do_uci_add_list(LOGIN_ALLOWED_IP, token))
			return -1;
	}

	ret = do_uci_commit("system");
	if(ret)
		return (-1);
	char tmp[16] = {0};
	do_uci_get(ADMIN_SYS_MISC_HTTP, tmp);
	//ret = setRemoteAccess(atoi(tmp));
	setRemoteAccess2();
	do_uci_get(LOGIN_ALLOWED_IP, ipranges);
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUR_X_Charter_MaxAllowedIpRangeNumber(char * path_name, char *value)
{
	int ret = 0;
	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	strcpy(value, "4"); // web gui show max number 4.
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUL_Movable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUL_Movable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUL_Movable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUL_Movable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUL_Resizable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUL_Resizable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUL_Resizable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUL_Resizable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUL_PosX(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUL_PosX, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUL_PosX(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUL_PosX, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUL_PosY(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUL_PosY, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUL_PosY(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUL_PosY, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUL_Width(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUL_Width, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUL_Width(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUL_Width, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUL_Height(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUL_Height, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUL_Height(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUL_Height, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUL_DisplayWidth(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUL_DisplayWidth, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUL_DisplayHeight(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUL_DisplayHeight, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUX_X_Charter_ManagerPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUCP_X_Charter_ManagerPassword, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"coral################## get value [%s]",value);
	return ret;
}
int set_DUX_X_Charter_ManagerPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUCP_X_Charter_ManagerPassword, value);
	if(ret)
	{
		return -1;
	}
	else
	{
		ret = do_uci_commit(DUCP_X_Charter_ManagerPassword);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"coral##################get value [%s]",value);
	return ret;
}
int get_DUX_X_Charter_SuperPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUCP_X_Charter_ManagerPassword, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"coral################## get value [%s]",value);
	return ret;
}
int set_DUX_X_Charter_SuperPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUCP_X_Charter_SuperPassword, value);
	if(ret)
	{
		return -1;
	}
	else
	{
		ret = do_uci_commit(DUCP_X_Charter_SuperPassword);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"coral##################get value [%s]",value);
	return ret;
}
int get_DIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIt_HigherLayer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIt_HigherLayer, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".InterfaceStack.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(InterfaceStackMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%*s %s %*s", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIt_LowerLayer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIt_LowerLayer, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".InterfaceStack.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(InterfaceStackMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%*s %*s %s %*s", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIt_HigherAlias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIt_HigherAlias, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".InterfaceStack.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(InterfaceStackMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%*s %*s %*s %s %*s", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIt_LowerAlias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIt_LowerAlias, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char *p = NULL;
	int i = 0;
	char *index = parseTemplate(path_name, ".InterfaceStack.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(InterfaceStackMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%*s %*s %*s %*s %s", value);
			p = strchr(value, '\n');
			if(p != NULL)
			{
				*p = '\0';
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_LineNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DD_LineNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_ChannelNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DD_ChannelNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DD_BondingGroupNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DD_BondingGroupNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLt_EnableDataGathering(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_EnableDataGathering, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLt_EnableDataGathering(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLt_EnableDataGathering, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLt_Upstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_Upstream, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_FirmwareVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_FirmwareVersion, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_LinkStatus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_LinkStatus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_StandardsSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_StandardsSupported, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTSE(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTSE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_StandardUsed(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_StandardUsed, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTSUsed(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTSUsed, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_LineEncoding(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_LineEncoding, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_AllowedProfiles(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_AllowedProfiles, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_CurrentProfile(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_CurrentProfile, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_PowerManagementState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_PowerManagementState, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_SuccessFailureCause(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_SuccessFailureCause, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_UPBOKLER(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_UPBOKLER, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_UPBOKLEPb(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_UPBOKLEPb, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_UPBOKLERPb(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_UPBOKLERPb, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_RXTHRSHds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_RXTHRSHds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_ACTRAMODEds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_ACTRAMODEds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_ACTRAMODEus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_ACTRAMODEus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_ACTINPROCds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_ACTINPROCds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_ACTINPROCus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_ACTINPROCus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_SNRMROCds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_SNRMROCds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_SNRMROCus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_SNRMROCus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_LastStateTransmittedDownstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_LastStateTransmittedDownstream, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_LastStateTransmittedUpstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_LastStateTransmittedUpstream, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_UPBOKLE(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_UPBOKLE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_MREFPSDds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_MREFPSDds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_MREFPSDus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_MREFPSDus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_LIMITMASK(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_LIMITMASK, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_US0MASK(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_US0MASK, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_TRELLISds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_TRELLISds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_TRELLISus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_TRELLISus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_ACTSNRMODEds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_ACTSNRMODEds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_ACTSNRMODEus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_ACTSNRMODEus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_VirtualNoisePSDds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_VirtualNoisePSDds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_VirtualNoisePSDus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_VirtualNoisePSDus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_ACTUALCE(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_ACTUALCE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_LineNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_LineNumber, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_UpstreamMaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_UpstreamMaxBitRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_DownstreamMaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_DownstreamMaxBitRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_UpstreamNoiseMargin(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_UpstreamNoiseMargin, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_DownstreamNoiseMargin(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_DownstreamNoiseMargin, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_SNRMpbus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_SNRMpbus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_SNRMpbds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_SNRMpbds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_INMIATOds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_INMIATOds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_INMIATSds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_INMIATSds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_INMCCds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_INMCCds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_INMINPEQMODEds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_INMINPEQMODEds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_UpstreamAttenuation(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_UpstreamAttenuation, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_DownstreamAttenuation(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_DownstreamAttenuation, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_UpstreamPower(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_UpstreamPower, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_DownstreamPower(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_DownstreamPower, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTURVendor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTURVendor, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTURCountry(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTURCountry, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTURANSIStd(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTURANSIStd, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTURANSIRev(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTURANSIRev, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTUCVendor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTUCVendor, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTUCCountry(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTUCCountry, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTUCANSIStd(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTUCANSIStd, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLt_XTUCANSIRev(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLt_XTUCANSIRev, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDLtD_LoggingDepthR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtD_LoggingDepthR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtD_LoggingDepthR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtD_LoggingDepthR, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtD_ActLoggingDepthReportingR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtD_ActLoggingDepthReportingR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtD_ActLoggingDepthReportingR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtD_ActLoggingDepthReportingR, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtD_EventTraceBufferR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtD_EventTraceBufferR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtD_EventTraceBufferR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtD_EventTraceBufferR, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_BytesSent, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_BytesReceived, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_PacketsSent, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_PacketsReceived, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_ErrorsSent, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_ErrorsReceived, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_DiscardPacketsSent, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_TotalStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_TotalStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_TotalStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_TotalStart, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_ShowtimeStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_ShowtimeStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_ShowtimeStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_ShowtimeStart, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_LastShowtimeStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_LastShowtimeStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_LastShowtimeStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_LastShowtimeStart, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_CurrentDayStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_CurrentDayStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_CurrentDayStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_CurrentDayStart, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtS_QuarterHourStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtS_QuarterHourStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtS_QuarterHourStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtS_QuarterHourStart, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtST_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtST_ErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtST_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtST_ErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtST_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtST_SeverelyErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtST_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtST_SeverelyErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtSS_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtSS_ErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtSS_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtSS_ErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtSS_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtSS_SeverelyErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtSS_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtSS_SeverelyErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtSL_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtSL_ErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtSL_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtSL_ErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtSL_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtSL_SeverelyErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtSL_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtSL_SeverelyErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtSC_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtSC_ErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtSC_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtSC_ErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtSC_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtSC_SeverelyErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtSC_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtSC_SeverelyErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtSQ_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtSQ_ErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtSQ_ErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtSQ_ErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtSQ_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtSQ_SeverelyErroredSecs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtSQ_SeverelyErroredSecs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtSQ_SeverelyErroredSecs, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_HLOGGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_HLOGGds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_HLOGGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_HLOGGds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_HLOGGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_HLOGGus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_HLOGGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_HLOGGus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_HLOGpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_HLOGpsds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_HLOGpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_HLOGpsds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_HLOGpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_HLOGpsus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_HLOGpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_HLOGpsus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_HLOGMTds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_HLOGMTds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_HLOGMTds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_HLOGMTds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_HLOGMTus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_HLOGMTus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_HLOGMTus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_HLOGMTus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_QLNGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_QLNGds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_QLNGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_QLNGds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_QLNGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_QLNGus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_QLNGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_QLNGus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_QLNpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_QLNpsds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_QLNpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_QLNpsds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_QLNpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_QLNpsus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_QLNpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_QLNpsus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_QLNMTds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_QLNMTds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_QLNMTds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_QLNMTds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_QLNMTus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_QLNMTus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_QLNMTus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_QLNMTus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_SNRGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_SNRGds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_SNRGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_SNRGds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_SNRGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_SNRGus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_SNRGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_SNRGus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_SNRpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_SNRpsds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_SNRpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_SNRpsds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_SNRpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_SNRpsus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_SNRpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_SNRpsus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_SNRMTds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_SNRMTds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_SNRMTds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_SNRMTds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_SNRMTus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_SNRMTus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_SNRMTus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_SNRMTus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_LATNds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_LATNds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_LATNds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_LATNds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_LATNus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_LATNus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_LATNus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_LATNus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_SATNds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_SATNds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_SATNds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_SATNds, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDLtT_SATNus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDLtT_SATNus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDLtT_SATNus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDLtT_SATNus, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_LinkEncapsulationSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_LinkEncapsulationSupported, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_LinkEncapsulationUsed(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_LinkEncapsulationUsed, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_LPATH(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_LPATH, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_INTLVDEPTH(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_INTLVDEPTH, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_INTLVBLOCK(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_INTLVBLOCK, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_ActualInterleavingDelay(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_ActualInterleavingDelay, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_ACTINP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_ACTINP, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_INPREPORT(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_INPREPORT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_NFEC(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_NFEC, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_RFEC(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_RFEC, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_LSYMB(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_LSYMB, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_UpstreamCurrRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_UpstreamCurrRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_DownstreamCurrRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_DownstreamCurrRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_ACTNDR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_ACTNDR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCt_ACTINPREIN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCt_ACTINPREIN, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_TotalStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_TotalStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_ShowtimeStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_ShowtimeStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_LastShowtimeStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_LastShowtimeStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_CurrentDayStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_CurrentDayStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtS_QuarterHourStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtS_QuarterHourStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDCtST_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtST_XTURFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtST_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtST_XTURFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtST_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtST_XTUCFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtST_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtST_XTUCFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtST_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtST_XTURHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtST_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtST_XTURHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtST_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtST_XTUCHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtST_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtST_XTUCHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtST_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtST_XTURCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtST_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtST_XTURCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtST_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtST_XTUCCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtST_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtST_XTUCCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSS_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSS_XTURFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSS_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSS_XTURFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSS_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSS_XTUCFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSS_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSS_XTUCFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSS_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSS_XTURHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSS_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSS_XTURHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSS_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSS_XTUCHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSS_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSS_XTUCHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSS_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSS_XTURCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSS_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSS_XTURCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSS_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSS_XTUCCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSS_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSS_XTUCCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSL_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSL_XTURFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSL_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSL_XTURFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSL_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSL_XTUCFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSL_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSL_XTUCFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSL_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSL_XTURHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSL_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSL_XTURHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSL_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSL_XTUCHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSL_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSL_XTUCHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSL_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSL_XTURCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSL_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSL_XTURCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSL_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSL_XTUCCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSL_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSL_XTUCCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSC_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSC_XTURFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSC_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSC_XTURFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSC_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSC_XTUCFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSC_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSC_XTUCFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSC_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSC_XTURHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSC_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSC_XTURHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSC_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSC_XTUCHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSC_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSC_XTUCHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSC_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSC_XTURCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSC_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSC_XTURCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSC_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSC_XTUCCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSC_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSC_XTUCCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSQ_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSQ_XTURFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSQ_XTURFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSQ_XTURFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSQ_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSQ_XTUCFECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSQ_XTUCFECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSQ_XTUCFECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSQ_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSQ_XTURHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSQ_XTURHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSQ_XTURHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSQ_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSQ_XTUCHECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSQ_XTUCHECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSQ_XTUCHECErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSQ_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSQ_XTURCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSQ_XTURCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSQ_XTURCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDCtSQ_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDCtSQ_XTUCCRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDCtSQ_XTUCCRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDCtSQ_XTUCCRCErrors, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDBt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDBt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDBt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDBt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDBt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDBt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDBt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_GroupStatus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_GroupStatus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_GroupID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_GroupID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_BondSchemesSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_BondSchemesSupported, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_BondScheme(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_BondScheme, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_GroupCapacity(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_GroupCapacity, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_RunningTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_RunningTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_TargetUpRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_TargetUpRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_TargetDownRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_TargetDownRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_ThreshLowUpRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_ThreshLowUpRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_ThreshLowDownRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_ThreshLowDownRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_UpstreamDifferentialDelayTolerance(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_UpstreamDifferentialDelayTolerance, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_DownstreamDifferentialDelayTolerance(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_DownstreamDifferentialDelayTolerance, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBt_BondedChannelNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBt_BondedChannelNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtBt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDBtBt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDBtBt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDBtBt_Channel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBt_Channel, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtBtES_UnderflowErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBtES_UnderflowErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtBtES_CRCErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBtES_CRCErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtBtES_AlignmentErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBtES_AlignmentErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtBtES_ShortPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBtES_ShortPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtBtES_LongPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBtES_LongPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtBtES_OverflowErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBtES_OverflowErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtBtES_PauseFramesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBtES_PauseFramesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtBtES_FramesDropped(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtBtES_FramesDropped, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_TotalStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_TotalStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_CurrentDayStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_CurrentDayStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtS_QuarterHourStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtS_QuarterHourStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_FailureReasons(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_FailureReasons, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_UpstreamRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_UpstreamRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_DownstreamRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_DownstreamRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_UpstreamPacketLoss(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_UpstreamPacketLoss, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_DownstreamPacketLoss(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_DownstreamPacketLoss, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_UpstreamDifferentialDelay(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_UpstreamDifferentialDelay, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_DownstreamDifferentialDelay(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_DownstreamDifferentialDelay, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_FailureCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_FailureCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_ErroredSeconds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_ErroredSeconds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_SeverelyErroredSeconds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_SeverelyErroredSeconds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtST_UnavailableSeconds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtST_UnavailableSeconds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_FailureReasons(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_FailureReasons, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_UpstreamRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_UpstreamRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_DownstreamRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_DownstreamRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_UpstreamPacketLoss(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_UpstreamPacketLoss, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_DownstreamPacketLoss(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_DownstreamPacketLoss, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_UpstreamDifferentialDelay(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_UpstreamDifferentialDelay, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_DownstreamDifferentialDelay(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_DownstreamDifferentialDelay, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_FailureCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_FailureCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_ErroredSeconds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_ErroredSeconds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_SeverelyErroredSeconds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_SeverelyErroredSeconds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSC_UnavailableSeconds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSC_UnavailableSeconds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_FailureReasons(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_FailureReasons, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_UpstreamRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_UpstreamRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_DownstreamRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_DownstreamRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_UpstreamPacketLoss(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_UpstreamPacketLoss, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_DownstreamPacketLoss(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_DownstreamPacketLoss, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_UpstreamDifferentialDelay(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_UpstreamDifferentialDelay, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_DownstreamDifferentialDelay(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_DownstreamDifferentialDelay, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_FailureCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_FailureCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_ErroredSeconds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_ErroredSeconds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_SeverelyErroredSeconds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_SeverelyErroredSeconds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtSQ_UnavailableSeconds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtSQ_UnavailableSeconds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PAFErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PAFErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PAFSmallFragments(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PAFSmallFragments, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PAFLargeFragments(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PAFLargeFragments, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PAFBadFragments(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PAFBadFragments, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PAFLostFragments(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PAFLostFragments, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PAFLateFragments(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PAFLateFragments, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PAFLostStarts(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PAFLostStarts, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PAFLostEnds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PAFLostEnds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PAFOverflows(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PAFOverflows, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_PauseFramesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_PauseFramesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_CRCErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_CRCErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_AlignmentErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_AlignmentErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_ShortPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_ShortPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_LongPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_LongPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_OverflowErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_OverflowErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDBtES_FramesDropped(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDBtES_FramesDropped, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_DiagnosticsState, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDDA_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDDA_DiagnosticsState, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDDA_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_Interface, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DDDA_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DDDA_Interface, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DDDA_ACTPSDds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_ACTPSDds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_ACTPSDus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_ACTPSDus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_ACTATPds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_ACTATPds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_ACTATPus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_ACTATPus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLINSCds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLINSCds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLINSCus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLINSCus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLINGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLINGds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLINGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLINGus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLOGGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLOGGds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLOGGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLOGGus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLOGpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLOGpsds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLOGpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLOGpsus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLOGMTds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLOGMTds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLOGMTus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLOGMTus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_LATNpbds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_LATNpbds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_LATNpbus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_LATNpbus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_SATNds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_SATNds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_SATNus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_SATNus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLINpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLINpsds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_HLINpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_HLINpsus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_QLNGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_QLNGds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_QLNGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_QLNGus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_QLNpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_QLNpsds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_QLNpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_QLNpsus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_QLNMTds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_QLNMTds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_QLNMTus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_QLNMTus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_SNRGds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_SNRGds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_SNRGus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_SNRGus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_SNRpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_SNRpsds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_SNRpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_SNRpsus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_SNRMTds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_SNRMTds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_SNRMTus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_SNRMTus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_BITSpsds(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_BITSpsds, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DDDA_BITSpsus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DDDA_BITSpsus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DO_InterfaceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DO_InterfaceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DOIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DOIt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DOIt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DOIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DOIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DOIt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DOIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DOIt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DOIt_Upstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_Upstream, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_OpticalSignalLevel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_OpticalSignalLevel, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_LowerOpticalThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_LowerOpticalThreshold, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_UpperOpticalThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_UpperOpticalThreshold, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_TransmitOpticalLevel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_TransmitOpticalLevel, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_LowerTransmitPowerThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_LowerTransmitPowerThreshold, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOIt_UpperTransmitPowerThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOIt_UpperTransmitPowerThreshold, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOItS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOItS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOItS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOItS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOItS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOItS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOItS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOItS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOItS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOItS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOItS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOItS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOItS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOItS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DOItS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DOItS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DA_LinkNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DA_LinkNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALt_LinkType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_LinkType, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALt_LinkType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALt_LinkType, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALt_AutoConfig(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_AutoConfig, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALt_DestinationAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_DestinationAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALt_DestinationAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALt_DestinationAddress, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALt_Encapsulation(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_Encapsulation, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALt_Encapsulation(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALt_Encapsulation, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALt_FCSPreserved(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_FCSPreserved, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALt_FCSPreserved(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALt_FCSPreserved, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALt_VCSearchList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_VCSearchList, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALt_VCSearchList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALt_VCSearchList, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALt_AAL(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALt_AAL, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_TransmittedBlocks(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_TransmittedBlocks, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_ReceivedBlocks(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_ReceivedBlocks, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_CRCErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_CRCErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtS_HECErrors(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtS_HECErrors, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DALtQ_QoSClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtQ_QoSClass, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALtQ_QoSClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALtQ_QoSClass, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALtQ_PeakCellRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtQ_PeakCellRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALtQ_PeakCellRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALtQ_PeakCellRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALtQ_MaximumBurstSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtQ_MaximumBurstSize, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALtQ_MaximumBurstSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALtQ_MaximumBurstSize, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DALtQ_SustainableCellRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DALtQ_SustainableCellRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DALtQ_SustainableCellRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DALtQ_SustainableCellRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DADF_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DADF_DiagnosticsState, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DADF_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DADF_DiagnosticsState, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DADF_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DADF_Interface, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DADF_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DADF_Interface, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DADF_NumberOfRepetitions(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DADF_NumberOfRepetitions, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DADF_NumberOfRepetitions(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DADF_NumberOfRepetitions, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DADF_Timeout(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DADF_Timeout, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DADF_Timeout(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DADF_Timeout, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DADF_SuccessCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DADF_SuccessCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DADF_FailureCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DADF_FailureCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DADF_AverageResponseTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DADF_AverageResponseTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DADF_MinimumResponseTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DADF_MinimumResponseTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DADF_MaximumResponseTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DADF_MaximumResponseTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DP_LinkNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DP_LinkNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DP_LinkNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DP_LinkNumberOfEntries, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPLt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPLt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPLt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPLt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPLt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPLt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPLt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPLt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPLt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPLt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPLtS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPLtS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DE_InterfaceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DE_InterfaceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "2"); //Always 2, Ethernet Interfaces include eth0(WAN) and eth1(LAN)
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DE_LinkNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DE_LinkNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "2"); //Always 2, WAN and LAN
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DE_VLANTerminationNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DE_VLANTerminationNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //not support, always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DE_RMONStatsNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DE_RMONStatsNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //not support, always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DERt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DERt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DERt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DERt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DERt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DERt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Interface, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DERt_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DERt_Interface, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DERt_VLANID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_VLANID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DERt_VLANID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DERt_VLANID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DERt_Queue(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Queue, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DERt_Queue(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DERt_Queue, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DERt_AllQueues(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_AllQueues, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DERt_AllQueues(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DERt_AllQueues, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DERt_DropEvents(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_DropEvents, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Bytes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Bytes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Packets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Packets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_BroadcastPackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_BroadcastPackets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_MulticastPackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_MulticastPackets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_CRCErroredPackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_CRCErroredPackets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_UndersizePackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_UndersizePackets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_OversizePackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_OversizePackets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Packets64Bytes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Packets64Bytes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Packets65to127Bytes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Packets65to127Bytes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Packets128to255Bytes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Packets128to255Bytes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Packets512to1023Bytes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Packets512to1023Bytes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DERt_Packets1024to1518Bytes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DERt_Packets1024to1518Bytes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			strcpy(value, "1"); //lan
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			char wanup[32] = {0};
			ret = do_uci_get("network.wan.disabled", wanup);
			if(ret)
			{
				strcpy(value, "1"); //no this uci node with defalut settings
				ret = 0;
			}
			else{
				if (atoi(wanup) == 0)
					strcpy(value, "1");
				else
					strcpy(value, "0");
			}
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DEIt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *index = parseTemplate(path_name, ".Interface.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			//strcpy(value, "1"); //lan
			return -3; //don't allow to set
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			char wanup[32] = {0};
			if (atoi(value) == 1)
				strcpy(wanup, "0");
			else{
				//strcpy(wanup, "1");
				return -3; //CS attempt to disable the WAN interface, that parameter setting MUST be rejected and reported to ACS as an error, now the error code is 9001(Request denied)
			}
			ret = do_uci_set("network.wan.disabled", wanup);
			if(ret)
			{
				return -1;
			}
			else{
				ret = do_uci_commit("network");
				if(ret)
				{
					return (-1);
				}
				//ret = 1; //means need to reboot for taking effect
				doRestartNetwork();
			}
		}
		else{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEIt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			strcpy(value, "Up"); //lan
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			char wanup[32] = {0};
			ret = do_uci_get("network.wan.disabled", wanup);
			if(ret)
			{
				strcpy(value, "Up"); //no this uci node with defalut settings
				ret = 0;
			}
			else{
				if (atoi(wanup) == 1)
					strcpy(value, "Down");
				else
					strcpy(value, "Up");
			}
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
#if 0 //getting from tr.xml
	/*ret = do_uci_get(DEIt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char newpath[128] = {0};
	node_t target;
	if (index != NULL){
		sprintf(newpath, "Device.Ethernet.Interface.%s.Alias", index);
		ret = lib_resolve_node(newpath, &target); //get from tr.xml
		if(ret)
		{
			return -1;
		}
		strcpy(value, target->value);
	}
#endif
    ret = 1; //getting from tr.xml
    tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
#if 0 //set to tr.xml
	/*ret = do_uci_set(DEIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char newpath[128] = {0};
	node_t target;
	if (index != NULL){
		sprintf(newpath, "Device.Ethernet.Interface.%s.Alias", index);
		tr_log(LOG_DEBUG,"set_DEIt_Alias set,newpath [%s] value [%s]\n",newpath,value);
		ret = lib_resolve_node(newpath, &target);
		if(ret)
		{
			tr_log(LOG_DEBUG,"set_DEIt_Alias set value [%s]\n",value);
			return (-1);
		}
		else
		{
			strcpy(target->value, value);
			change = 1;
			lib_commit_transaction();
		}
	}
#endif	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEIt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_Name, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getEthInterfaceName("lan", value); //lan
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			getEthInterfaceName("wan", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEIt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_LastChange, value);
	if(ret)
	{
		return -1;
	}*/

	getInterfaceLastChangeTime(value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}*/
	/*char *index = parseTemplate(path_name, ".Interface.");
	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			strcpy(value, ETHERNET_LAN_INTERFACE_PATH); //lan
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			strcpy(value, ETHERNET_WAN_INTERFACE_PATH); //wan
		}
		else
		{
			return (-1);
		}
	}*/
	strcpy(value, ""); //Since Interface is a layer 1 interface, it is expected that LowerLayers will not be used
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DEIt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	 //don't allow to do set, always null
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEIt_Upstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_Upstream, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			strcpy(value, "0"); //lan, always false
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			strcpy(value, "1"); //wan, always true
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEIt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_MACAddress, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};
	char *ptr = NULL;

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			//getMfcInfo("LANPortMAC", value);
			getEthInterfaceName("lan", inf);
			getInfaceMac(inf, value);
			if((ptr= strstr(value, " ")) != NULL)
				*ptr = '\0';
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			//getMfcInfo("WANPortMAC", value);
			getEthInterfaceName("wan", inf);
			getInfaceWanMac(value);
			if((ptr= strstr(value, " ")) != NULL)
				*ptr = '\0';
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEIt_MaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_MaxBitRate, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "-1"); //always auto, means automatic selection of the maximum bit rate
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEIt_MaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DEIt_MaxBitRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEIt_CurrentBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_CurrentBitRate, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "speed", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "speed", value);
		}
		else
		{
			return (-1);
		}

		if (strcmp(value, "") != 0){
			char *p = NULL;
			if ((p = strstr(value, "F")) != NULL) //for full speed
				*p = '\0';
			if ((p = strstr(value, "H")) != NULL) //for half speed
				*p = '\0';
			if (strncmp(value, "-1", 2) == 0)
				strcpy(value, "0"); //no link with value 0
		}
		else
			strcpy(value, "0"); //no link with value 0
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEIt_DuplexMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_DuplexMode, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "Auto"); //always Auto
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEIt_DuplexMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DEIt_DuplexMode, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEIt_EEECapability(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_EEECapability, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always 1
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEIt_EEEEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEIt_EEEEnable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always 1
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEIt_EEEEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DEIt_EEEEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEItS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_BytesSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "tx_bytes", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "tx_bytes", value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "rx_bytes", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "rx_bytes", value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "tx_packets", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "tx_packets", value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "rx_packets", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "rx_packets", value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "tx_errors", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "tx_errors", value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "rx_errors", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "rx_errors", value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "TxUniCast");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			count = get_ssdk_mib_statistics(5, "TxUniCast");
			sum = sum + count;
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "RxUniCast");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			count = get_ssdk_mib_statistics(5, "RxUniCast");
			sum = sum + count;
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "tx_dropped", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "tx_dropped", value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "rx_dropped", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "rx_dropped", value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "TxMulti");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			count = get_ssdk_mib_statistics(5, "TxMulti");
			sum = sum + count;
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "RxMulti");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			count = get_ssdk_mib_statistics(5, "RxMulti");
			sum = sum + count;
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "TxBroad");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			count = get_ssdk_mib_statistics(5, "TxBroad");
			sum = sum + count;
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "RxBroad");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			count = get_ssdk_mib_statistics(5, "RxBroad");
			sum = sum + count;
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEItS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DEItS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Interface.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			getEthInterfaceName("lan", inf);
			getDevStatus(inf, "rx_frame_errors", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			getEthInterfaceName("wan", inf);
			getDevStatus(inf, "rx_frame_errors", value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			strcpy(value, "1"); //lan
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			char wanup[32] = {0};
			ret = do_uci_get("network.wan.disabled", wanup);
			if(ret)
			{
				strcpy(value, "1"); //no this uci node with defalut settings
				ret = 0;
			}
			else{
				if (atoi(wanup) == 1)
					strcpy(value, "0");
				else
					strcpy(value, "1");
			}
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DELt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DELt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if (atoi(value) == 0) //New Charter PRD don't allow to disable
			return -3;
		
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			strcpy(value, "1"); //lan
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			char wanup[32] = {0};
			if (atoi(value) == 1)
				strcpy(wanup, "0");
			else
				strcpy(wanup, "1");
			ret = do_uci_set("network.wan.disabled", wanup);
			if(ret)
			{
				return -1;
			}
			else{
				ret = do_uci_commit("network");
				if(ret)
				{
					return (-1);
				}
				//ret = 1; //means need to reboot for taking effect
				doRestartNetwork();
			}
		}
		else{
			return (-1);
		}
	}
	else
		return -1;
	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DELt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			strcpy(value, "Up"); //lan
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			char waninf[32] = {0};
			getWanHigherLayerInterface(waninf);
			if (waninf[0] != '\0')
				strcpy(value, "Up");
			else
				strcpy(value, "Down");
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DELt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DELt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);*/
	//set to tr.xml
	return ret;
}
int get_DELt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELt_Name, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if (atoi(index) == ETHERNET_LAN_INSTANCE_INDEX){
			strcpy(value, "LAN Group"); //textual name
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX){
			char wanmode[32] = {0};
			getWanMode(wanmode);
			sprintf(value, "WAN %s Mode", wanmode); //textual name
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELt_LastChange, value);
	if(ret)
	{
		return -1;
	}*/
	a_infinfo infStatus;
	char *index = parseTemplate(path_name, ".Link.");

	if (index == NULL)
		return -1;

	memset(&infStatus, 0, sizeof(infStatus));
	
	if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
	{
		getInterfaceInfo("lan", &infStatus);
	}
	else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
	{
		char wanmode[32] = {0};
		char wantype[32] = {0};
		getWanMode(wanmode);
		if (strcmp(wanmode, "pptp") == 0 || strcmp(wanmode, "l2tp") == 0)
			strcpy(wantype, "wan0");
		else
			strcpy(wantype, "wan");
		memset(&infStatus, 0, sizeof(infStatus));
		getInterfaceInfo(wantype, &infStatus);
	}
	else
	{
		return (-1);
	}
	
	if (strcmp(infStatus.uptime, "") != 0)
		strcpy(value, infStatus.uptime);
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if (atoi(index) == ETHERNET_LAN_INSTANCE_INDEX){
			strcpy(value, ETHERNET_LAN_INTERFACE_PATH);
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char wanup[32] = {0};
			ret = do_uci_get("network.wan.disabled", wanup);
			if(ret)
			{
				strcpy(wanup, "0"); //no this uci node with defalut settings
				ret = 0;
			}
			if (atoi(wanup) == 0)
				strcpy(value, ETHERNET_WAN_INTERFACE_PATH);
			else
				strcpy(value, "");
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DELt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DELt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to change
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DELt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELt_MACAddress, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");
	char inf[32] = {0};

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX) //lan
		{
			//getMfcInfo("LANPortMAC", value);
			getInfaceMac("br-lan", value);
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX) //wan
		{
			//getMfcInfo("WANPortMAC", value);
			getEthInterfaceName("wan", inf);
			getInfaceMac(inf, value);
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELt_PriorityTagging(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELt_PriorityTagging, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DELt_PriorityTagging(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DELt_PriorityTagging, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DELtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getDevStatus("br-lan", "tx_bytes", value); //lan
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char inf[32] = {0};
			getWanHigherLayerInterface(inf);
			getDevStatus(inf, "tx_bytes", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getDevStatus("br-lan", "rx_bytes", value); //lan
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char inf[32] = {0};
			getWanHigherLayerInterface(inf);
			getDevStatus(inf, "rx_bytes", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getDevStatus("br-lan", "tx_packets", value); //lan
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char inf[32] = {0};
			getWanHigherLayerInterface(inf);
			getDevStatus(inf, "tx_packets", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getDevStatus("br-lan", "rx_packets", value); //lan
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char inf[32] = {0};
			getWanHigherLayerInterface(inf);
			getDevStatus(inf, "rx_packets", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getDevStatus("br-lan", "tx_errors", value); //lan
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char inf[32] = {0};
			getWanHigherLayerInterface(inf);
			getDevStatus(inf, "tx_errors", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getDevStatus("br-lan", "rx_errors", value); //lan
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char inf[32] = {0};
			getWanHigherLayerInterface(inf);
			getDevStatus(inf, "rx_errors", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "TxUniCast");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			
			char inf[32] = {0};
			char inf2[32] = {0};
			getWanHigherLayerInterface(inf);
			getEthInterfaceName("wan", inf2);
			if(strcmp(inf, inf2) == 0)
			{
				count = get_ssdk_mib_statistics(5, "TxUniCast");
				sum = sum + count;
			}
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "RxUniCast");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			
			char inf[32] = {0};
			char inf2[32] = {0};
			getWanHigherLayerInterface(inf);
			getEthInterfaceName("wan", inf2);
			if(strcmp(inf, inf2) == 0)
			{
				count = get_ssdk_mib_statistics(5, "RxUniCast");
				sum = sum + count;
			}
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getDevStatus("br-lan", "tx_dropped", value); //lan
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char inf[32] = {0};
			getWanHigherLayerInterface(inf);
			getDevStatus(inf, "tx_dropped", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getDevStatus("br-lan", "rx_dropped", value); //lan
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char inf[32] = {0};
			getWanHigherLayerInterface(inf);
			getDevStatus(inf, "rx_dropped", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "TxMulti");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			
			char inf[32] = {0};
			char inf2[32] = {0};
			getWanHigherLayerInterface(inf);
			getEthInterfaceName("wan", inf2);
			if(strcmp(inf, inf2) == 0)
			{
				count = get_ssdk_mib_statistics(5, "TxMulti");
				sum = sum + count;
			}
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "RxMulti");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			
			char inf[32] = {0};
			char inf2[32] = {0};
			getWanHigherLayerInterface(inf);
			getEthInterfaceName("wan", inf2);
			if(strcmp(inf, inf2) == 0)
			{
				count = get_ssdk_mib_statistics(5, "RxMulti");
				sum = sum + count;
			}
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "TxBroad");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			
			char inf[32] = {0};
			char inf2[32] = {0};
			getWanHigherLayerInterface(inf);
			getEthInterfaceName("wan", inf2);
			if(strcmp(inf, inf2) == 0)
			{
				count = get_ssdk_mib_statistics(5, "TxBroad");
				sum = sum + count;
			}
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");
	int count = 0;
	int sum = 0;
	int i = 0;

	if (index != NULL)
	{
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			for(i=1; i<=4; i++)
			{
				count = get_ssdk_mib_statistics(i, "RxBroad");
				sum = sum + count;
				count = 0;
			}
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			
			char inf[32] = {0};
			char inf2[32] = {0};
			getWanHigherLayerInterface(inf);
			getEthInterfaceName("wan", inf2);
			if(strcmp(inf, inf2) == 0)
			{
				count = get_ssdk_mib_statistics(5, "RxBroad");
				sum = sum + count;
			}
		}
		else
		{
			return (-1);
		}
	}
	sprintf(value, "%d", sum);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DELtS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DELtS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Link.");

	if (index != NULL){
		if(atoi(index) == ETHERNET_LAN_INSTANCE_INDEX)
		{
			getDevStatus("br-lan", "rx_frame_errors", value); //lan
			
		}
		else if (atoi(index) == ETHERNET_WAN_INSTANCE_INDEX)
		{
			char inf[32] = {0};
			getWanHigherLayerInterface(inf);
			getDevStatus(inf, "rx_frame_errors", value); //wan
		}
		else
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEVt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DEVt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEVt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEVt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DEVt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEVt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEVt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DEVt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEVt_VLANID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVt_VLANID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEVt_VLANID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DEVt_VLANID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEVt_TPID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVt_TPID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DEVt_TPID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DEVt_TPID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DEVtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DEVtS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DEVtS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DU_InterfaceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_InterfaceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DU_PortNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DU_PortNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "2"); //support two usb ports
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_Upstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Upstream, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_MaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_MaxBitRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Port, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUPt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DUPt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUPt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUPt_Name, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUPt_Standard(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUPt_Standard, value);
	if(ret)
	{
		return -1;
	}*/
	char usbpath[128] = {0};
	FILE *fp = NULL;
	char line[128] = {0};
	char * ptr = NULL;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(usbpath, "/sys/bus/platform/devices/xhci-hcd.0/usb1/version");
		else if (atoi(index) == 2)
			strcpy(usbpath, "/sys/bus/platform/devices/xhci-hcd.1/usb3/version");
		else
			return -1;
		if((fp=fopen(usbpath, "r")) != NULL){
			fgets(line,sizeof(line)-1,fp);
			if ((ptr = strstr(line, "\n")) != NULL)
					*ptr = '\0';
			strcpy(value, line);
			fclose(fp);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUPt_Type(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUPt_Type, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "Host"); //always Host
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUPt_Receptacle(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUPt_Receptacle, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "Standard-A"); //always Standard-A
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUPt_Rate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUPt_Rate, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo2(busnum, value, "speed"); //get Rate
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUPt_Power(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUPt_Power, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "Bus"); //always Bus
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUU_HostNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUU_HostNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "2"); //has two usb ports
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DUUHt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always enable
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHt_Name, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHt_Type(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHt_Type, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "xHCI"); //always xHCI Host Controller
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHt_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHt_Reset, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHt_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUUHt_Reset, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHt_PowerManagementEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHt_PowerManagementEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHt_PowerManagementEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUUHt_PowerManagementEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHt_USBVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHt_USBVersion, value);
	if(ret)
	{
		return -1;
	}*/
	char usbpath[128] = {0};
	FILE *fp = NULL;
	char line[128] = {0};
	char * ptr = NULL;
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(usbpath, "/sys/bus/platform/devices/xhci-hcd.0/usb1/version");
		else if (atoi(index) == 2)
			strcpy(usbpath, "/sys/bus/platform/devices/xhci-hcd.1/usb3/version");
		else
			return -1;
		if((fp=fopen(usbpath, "r")) != NULL){
			fgets(line,sizeof(line)-1,fp);
			if ((ptr = strstr(line, "\n")) != NULL)
					*ptr = '\0';
			strcpy(value, line);
			fclose(fp);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHt_DeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHt_DeviceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	char keyvalue[MAXMAPITEMS][128];
	char usbubsnum[32] = {0};
	int num = 0;
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(usbubsnum, "1");
		else if (atoi(index) == 2)
			strcpy(usbubsnum, "3");
		else
			return -1;
		num = get_USBHostsDevice(keyvalue, usbubsnum);
		sprintf(value, "%d", num);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHt_DeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHt_DeviceNumberOfEntries, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHtDt_DeviceNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_DeviceNumber, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "devnum"); //get DeviceNumber
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_USBVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_USBVersion, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "version"); //get USBVersion
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_DeviceClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_DeviceClass, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "bDeviceClass"); //get DeviceClass
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_DeviceSubClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_DeviceSubClass, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "bDeviceSubClass"); //get DeviceSubClass
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHtDt_DeviceSubClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHtDt_DeviceSubClass, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHtDt_DeviceVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_DeviceVersion, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "bcdDevice"); //get DeviceVersion
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHtDt_DeviceVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHtDt_DeviceVersion, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHtDt_DeviceProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_DeviceProtocol, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "bDeviceProtocol"); //get DeviceProtocol
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHtDt_DeviceProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHtDt_DeviceProtocol, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHtDt_ProductID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_ProductID, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "idProduct"); //get ProductID
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHtDt_ProductID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHtDt_ProductID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHtDt_VendorID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_VendorID, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "idVendor"); //get VendorID
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHtDt_VendorID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHtDt_VendorID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHtDt_Manufacturer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_Manufacturer, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "manufacturer"); //get Manufacturer
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHtDt_Manufacturer(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHtDt_Manufacturer, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHtDt_ProductClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_ProductClass, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "product"); //get ProductClass
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHtDt_ProductClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHtDt_ProductClass, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHtDt_SerialNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_SerialNumber, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "serial"); //get SerialNumber
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUUHtDt_SerialNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DUUHtDt_SerialNumber, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUUHtDt_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_Port, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "busnum"); //get Port
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_USBPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_USBPort, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(value, "Device.USB.Port.1");
		else if (atoi(index) == 2)
			strcpy(value, "Device.USB.Port.2");
		else
			return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_Rate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_Rate, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "speed"); //get Rate
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_Parent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_Parent, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "devpath"); //get Parent
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_MaxChildren(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_MaxChildren, value);
	if(ret)
	{
		return -1;
	}*/
	char busnum[8] = {0};
	char *index = parseTemplate(path_name, ".Host.");

	if (index != NULL)
	{
		if (atoi(index) == 1)
			strcpy(busnum, "1");
		else if (atoi(index) == 2)
			strcpy(busnum, "3");
		else
			return -1;
		get_USBHostsDeviceInfo(busnum, value, "maxchild"); //get MaxChildren
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_IsSuspended(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHtDt_IsSuspended, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_IsSelfPowered(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHtDt_IsSelfPowered, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDt_ConfigurationNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DUUHtDt_ConfigurationNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0, not support
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDtCt_ConfigurationNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHtDtCt_ConfigurationNumber, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDtCt_InterfaceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHtDtCt_InterfaceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDtCtIt_InterfaceNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHtDtCtIt_InterfaceNumber, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDtCtIt_nterfaceClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHtDtCtIt_nterfaceClass, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDtCtIt_InterfaceSubClass(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHtDtCtIt_InterfaceSubClass, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUUHtDtCtIt_InterfaceProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUUHtDtCtIt_InterfaceProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DH_InterfaceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DH_InterfaceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_Upstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Upstream, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_FirmwareVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_FirmwareVersion, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_NodeID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_NodeID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_IsMaster(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_IsMaster, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_Synced(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Synced, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_TotalSyncTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_TotalSyncTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_MaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_MaxBitRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_NetworkUtilization(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_NetworkUtilization, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_PossibleConnectionTypes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_PossibleConnectionTypes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_ConnectionType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_ConnectionType, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_ConnectionType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_ConnectionType, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_PossibleSpectralModes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_PossibleSpectralModes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_SpectralMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_SpectralMode, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_SpectralMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_SpectralMode, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_MTU(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_MTU, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_MTU(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_MTU, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_NoiseMargin(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_NoiseMargin, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_DefaultNonLARQPER(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_DefaultNonLARQPER, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_LARQEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_LARQEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_MinMulticastRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_MinMulticastRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_NegMulticastRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_NegMulticastRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_MasterSelectionMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_MasterSelectionMode, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_MasterSelectionMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_MasterSelectionMode, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_AssociatedDeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_AssociatedDeviceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItQ_FlowSpecNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQ_FlowSpecNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItQFt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItQFt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_TrafficClasses(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_TrafficClasses, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_TrafficClasses(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_TrafficClasses, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_FlowType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_FlowType, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_FlowType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_FlowType, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_Priority(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_Priority, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItQFt_Latency(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_Latency, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_Latency(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_Latency, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_Jitter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_Jitter, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_Jitter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_Jitter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_PacketSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_PacketSize, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_PacketSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_PacketSize, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_MinRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_MinRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_MinRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_MinRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_AvgRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_AvgRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_AvgRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_AvgRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_MaxRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_MaxRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_MaxRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_MaxRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_PER(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_PER, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_PER(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_PER, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItQFt_Timeout(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItQFt_Timeout, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItQFt_Timeout(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItQFt_Timeout, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItAt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_NodeID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_NodeID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_IsMaster(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_IsMaster, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_Synced(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_Synced, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_TotalSyncTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_TotalSyncTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_MaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_MaxBitRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_PHYDiagnosticsEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_PHYDiagnosticsEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHItAt_PHYDiagnosticsEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHItAt_PHYDiagnosticsEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHItAt_Active(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_Active, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDP_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_DiagnosticsState, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_DiagnosticsState, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_Interface, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_Interface, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_NumPacketsInBurst(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_NumPacketsInBurst, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_NumPacketsInBurst(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_NumPacketsInBurst, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_BurstInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_BurstInterval, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_BurstInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_BurstInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_TestPacketPayloadLength(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_TestPacketPayloadLength, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_TestPacketPayloadLength(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_TestPacketPayloadLength, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_PayloadEncoding(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_PayloadEncoding, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_PayloadEncoding(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_PayloadEncoding, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_PayloadDataGen(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_PayloadDataGen, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_PayloadDataGen(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_PayloadDataGen, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_PayloadType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_PayloadType, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_PayloadType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_PayloadType, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_PriorityLevel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_PriorityLevel, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_PriorityLevel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_PriorityLevel, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_ResultNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_ResultNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPRt_SrcMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPRt_SrcMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPRt_DestMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPRt_DestMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPRt_PHYRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPRt_PHYRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPRt_BaudRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPRt_BaudRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPRt_SNR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPRt_SNR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPRt_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPRt_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPRt_Attenuation(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPRt_Attenuation, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDP_DiagnosticsState_1161(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_DiagnosticsState_1161, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_DiagnosticsState_1161(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_DiagnosticsState_1161, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_Interface_1163(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_Interface_1163, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_Interface_1163(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_Interface_1163, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDP_SampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDP_SampleInterval, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHDP_SampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHDP_SampleInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHDPN_CurrentStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPN_CurrentStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPN_CurrentEnd(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPN_CurrentEnd, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPN_NodeNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPN_NodeNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsCrcErrored(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsCrcErrored, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsCrcErroredHost(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsCrcErroredHost, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsShortErrored(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsShortErrored, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsShortErroredHost(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsShortErroredHost, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_RxPacketsDropped(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_RxPacketsDropped, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_TxPacketsDropped(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_TxPacketsDropped, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_ControlRequestLocal(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_ControlRequestLocal, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_ControlReplyLocal(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_ControlReplyLocal, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_ControlRequestRemote(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_ControlRequestRemote, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_ControlReplyRemote(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_ControlReplyRemote, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsSentWire(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsSentWire, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_BroadcastPacketsSentWire(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_BroadcastPacketsSentWire, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_MulticastPacketsSentWire(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_MulticastPacketsSentWire, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsInternalControl(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsInternalControl, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_BroadcastPacketsInternalControl(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_BroadcastPacketsInternalControl, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsReceivedQueued(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsReceivedQueued, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_PacketsReceivedForwardUnknown(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_PacketsReceivedForwardUnknown, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPNNt_NodeUtilization(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPNNt_NodeUtilization, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPC_TimeStamp(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPC_TimeStamp, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPC_ChannelNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPC_ChannelNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_HostSrcMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_HostSrcMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_HostDestMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_HostDestMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_HPNASrcMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_HPNASrcMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_HPNADestMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_HPNADestMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_PHYRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_PHYRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_BaudRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_BaudRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_SNR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_SNR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_LARQPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_LARQPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHDPCCt_FlowSpec(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHDPCCt_FlowSpec, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DM_InterfaceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DM_InterfaceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_Enable_1211(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_Enable_1211, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_Enable_1211(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_Enable_1211, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_Alias_1214(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_Alias_1214, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_Alias_1214(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_Alias_1214, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_Upstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_Upstream, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_FirmwareVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_FirmwareVersion, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_MaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_MaxBitRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_MaxIngressBW(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_MaxIngressBW, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_MaxEgressBW(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_MaxEgressBW, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_HighestVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_HighestVersion, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_CurrentVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_CurrentVersion, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_NetworkCoordinator(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_NetworkCoordinator, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_NodeID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_NodeID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_MaxNodes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_MaxNodes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_PreferredNC(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_PreferredNC, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_PreferredNC(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_PreferredNC, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_BackupNC(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_BackupNC, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_PrivacyEnabledSetting(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_PrivacyEnabledSetting, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_PrivacyEnabledSetting(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_PrivacyEnabledSetting, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_PrivacyEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_PrivacyEnabled, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_FreqCapabilityMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_FreqCapabilityMask, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_FreqCurrentMaskSetting(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_FreqCurrentMaskSetting, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_FreqCurrentMaskSetting(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_FreqCurrentMaskSetting, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_FreqCurrentMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_FreqCurrentMask, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_CurrentOperFreq(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_CurrentOperFreq, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_LastOperFreq(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_LastOperFreq, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMIt_KeyPassphrase(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_KeyPassphrase, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_KeyPassphrase(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_KeyPassphrase, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_TxPowerLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_TxPowerLimit, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_TxPowerLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_TxPowerLimit, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_PowerCntlPhyTarget(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_PowerCntlPhyTarget, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_PowerCntlPhyTarget(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_PowerCntlPhyTarget, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_BeaconPowerLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_BeaconPowerLimit, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_BeaconPowerLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_BeaconPowerLimit, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_NetworkTabooMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_NetworkTabooMask, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_NetworkTabooMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_NetworkTabooMask, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_TxBcastRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_TxBcastRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_TxBcastRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_TxBcastRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_TxBcastPowerReduction(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_TxBcastPowerReduction, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_TxBcastPowerReduction(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_TxBcastPowerReduction, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_QAM256Capable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_QAM256Capable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_QAM256Capable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_QAM256Capable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_PacketAggregationCapability(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_PacketAggregationCapability, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_PacketAggregationCapability(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_PacketAggregationCapability, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMIt_AssociatedDeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMIt_AssociatedDeviceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DMIt_AssociatedDeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DMIt_AssociatedDeviceNumberOfEntries, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DMItS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQ_EgressNumFlows(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQ_EgressNumFlows, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQ_IngressNumFlows(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQ_IngressNumFlows, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQ_FlowStatsNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQ_FlowStatsNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQFt_FlowID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQFt_FlowID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQFt_PacketDA(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQFt_PacketDA, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQFt_MaxRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQFt_MaxRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQFt_MaxBurstSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQFt_MaxBurstSize, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQFt_LeaseTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQFt_LeaseTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQFt_LeaseTimeLeft(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQFt_LeaseTimeLeft, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItQFt_FlowPackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItQFt_FlowPackets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_NodeID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_NodeID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_PreferredNC(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_PreferredNC, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_HighestVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_HighestVersion, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_PHYTxRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_PHYTxRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_PHYRxRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_PHYRxRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_TxPowerControlReduction(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_TxPowerControlReduction, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_RxPowerLevel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_RxPowerLevel, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_TxBcastRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_TxBcastRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_RxBcastPowerLevel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_RxBcastPowerLevel, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_TxPackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_TxPackets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_RxPackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_RxPackets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_RxErroredAndMissedPackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_RxErroredAndMissedPackets, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_QAM256Capable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_QAM256Capable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_PacketAggregationCapability(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_PacketAggregationCapability, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_RxSNR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_RxSNR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DMItAt_Active(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DMItAt_Active, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DG_InterfaceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DG_InterfaceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGIt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGIt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGIt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGIt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGIt_Upstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_Upstream, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_FirmwareVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_FirmwareVersion, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_ConnectionType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_ConnectionType, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_MaxTransmitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_MaxTransmitRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_TargetDomainNames(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_TargetDomainNames, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGIt_TargetDomainNames(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGIt_TargetDomainNames, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGIt_DomainName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_DomainName, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_DomainNameIdentifier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_DomainNameIdentifier, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_DomainId(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_DomainId, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_DeviceId(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_DeviceId, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_MaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_MaxBitRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_NodeTypeDMCapable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_NodeTypeDMCapable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_DMRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_DMRequested, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGIt_DMRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGIt_DMRequested, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGIt_IsDM(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_IsDM, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_NodeTypeSCCapable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_NodeTypeSCCapable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_SCRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_SCRequested, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGIt_SCRequested(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGIt_SCRequested, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGIt_IsSC(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_IsSC, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_StandardVersions(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_StandardVersions, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_MaxBandPlan(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_MaxBandPlan, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_MediumType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_MediumType, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_TAIFG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_TAIFG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_NotchedAmateurRadioBands(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_NotchedAmateurRadioBands, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_PHYThroughputDiagnosticsEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_PHYThroughputDiagnosticsEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_PerformanceMonitoringDiagnosticsEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_PerformanceMonitoringDiagnosticsEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_SMMaskedBandNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_SMMaskedBandNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_NodeTypeDMConfig(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_NodeTypeDMConfig, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_NodeTypeDMStatus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_NodeTypeDMStatus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_NodeTypeSCStatus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_NodeTypeSCStatus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGIt_AssociatedDeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGIt_AssociatedDeviceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItD_DomainName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_DomainName, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItD_DomainName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItD_DomainName, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItD_DomainNameIdentifier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_DomainNameIdentifier, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItD_DomainId(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_DomainId, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItD_MACCycleDuration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_MACCycleDuration, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItD_MACCycleDuration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItD_MACCycleDuration, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItD_SCDeviceId(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_SCDeviceId, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItD_SCDeviceId(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItD_SCDeviceId, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItD_SCMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_SCMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItD_SCMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItD_SCMACAddress, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItD_ReregistrationTimePeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_ReregistrationTimePeriod, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItD_ReregistrationTimePeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItD_ReregistrationTimePeriod, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItD_TopologyPeriodicInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_TopologyPeriodicInterval, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItD_TopologyPeriodicInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItD_TopologyPeriodicInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItD_MinSupportedBandplan(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_MinSupportedBandplan, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItD_MinSupportedBandplan(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItD_MinSupportedBandplan, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItD_MaxSupportedBandplan(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItD_MaxSupportedBandplan, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItD_MaxSupportedBandplan(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItD_MaxSupportedBandplan, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItS_ModesSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_ModesSupported, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_ModeEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_ModeEnabled, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItS_ModeEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItS_ModeEnabled, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItS_MICSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_MICSize, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItS_MICSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItS_MICSize, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItS_Location(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_Location, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItSt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItSt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItSt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItSt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItSt_BandNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItSt_BandNumber, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItSt_BandNumber(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItSt_BandNumber, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItSt_StartSubCarrier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItSt_StartSubCarrier, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItSt_StartSubCarrier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItSt_StartSubCarrier, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItSt_StopSubCarrier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItSt_StopSubCarrier, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItSt_StopSubCarrier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItSt_StopSubCarrier, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_MgmtBytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_MgmtBytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_MgmtBytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_MgmtBytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_MgmtPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_MgmtPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_MgmtPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_MgmtPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_BlocksSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_BlocksSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_BlocksReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_BlocksReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_BlocksResent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_BlocksResent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItS_BlocksErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItS_BlocksErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItAt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItAt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItAt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItAt_MACAddress, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItAt_DeviceId(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItAt_DeviceId, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGItAt_TxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItAt_TxPhyRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItAt_TxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItAt_TxPhyRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItAt_RxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItAt_RxPhyRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItAt_RxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItAt_RxPhyRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGItAt_Active(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGItAt_Active, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGItAt_Active(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGItAt_Active, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDP_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDP_DiagnosticsState, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDP_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDP_DiagnosticsState, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDP_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDP_Interface, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDP_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDP_Interface, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDP_DiagnoseMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDP_DiagnoseMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDP_DiagnoseMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDP_DiagnoseMACAddress, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDP_PHYThroughputResultNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDP_PHYThroughputResultNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPRt_DestinationMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPRt_DestinationMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDPRt_DestinationMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDPRt_DestinationMACAddress, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDPRt_LinkState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPRt_LinkState, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPRt_TxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPRt_TxPhyRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDPRt_TxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDPRt_TxPhyRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDPRt_RxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPRt_RxPhyRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDPRt_RxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDPRt_RxPhyRate, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDP_DiagnosticsState_1425(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDP_DiagnosticsState_1425, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDP_DiagnosticsState_1425(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDP_DiagnosticsState_1425, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDP_Interface_1427(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDP_Interface_1427, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDP_Interface_1427(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDP_Interface_1427, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDP_DiagnoseMACAddress_1429(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDP_DiagnoseMACAddress_1429, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDP_DiagnoseMACAddress_1429(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDP_DiagnoseMACAddress_1429, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDP_SampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDP_SampleInterval, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDP_SampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDP_SampleInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDP_SNRGroupLength(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDP_SNRGroupLength, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDP_SNRGroupLength(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDP_SNRGroupLength, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDPN_CurrentStart(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPN_CurrentStart, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPN_CurrentEnd(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPN_CurrentEnd, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDPN_CurrentEnd(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDPN_CurrentEnd, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDPN_NodeNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPN_NodeNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDPN_NodeNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDPN_NodeNumberOfEntries, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDPNNt_DestinationMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_DestinationMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_MgmtBytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_MgmtBytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_MgmtBytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_MgmtBytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_MgmtPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_MgmtPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_MgmtPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_MgmtPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_BlocksSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_BlocksSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_BlocksReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_BlocksReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_BlocksResent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_BlocksResent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPNNt_BlocksErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPNNt_BlocksErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPC_TimeStamp(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPC_TimeStamp, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPC_ChannelNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPC_ChannelNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DGDPC_ChannelNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DGDPC_ChannelNumberOfEntries, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DGDPCCt_DestinationMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPCCt_DestinationMACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DGDPCCt_SNR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DGDPCCt_SNR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DH_InterfaceNumberOfEntries_1469(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DH_InterfaceNumberOfEntries_1469, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_Enable_1470(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Enable_1470, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_Enable_1470(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_Enable_1470, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_Status_1472(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Status_1472, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_Alias_1473(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Alias_1473, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_Alias_1473(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_Alias_1473, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_Name_1475(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Name_1475, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_LastChange_1476(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_LastChange_1476, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_LowerLayers_1477(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_LowerLayers_1477, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_LowerLayers_1477(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_LowerLayers_1477, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_Upstream_1479(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Upstream_1479, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_MACAddress_1480(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_MACAddress_1480, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_LogicalNetwork(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_LogicalNetwork, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_LogicalNetwork(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_LogicalNetwork, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_Version(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_Version, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_FirmwareVersion_1484(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_FirmwareVersion_1484, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_ForceCCo(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_ForceCCo, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_ForceCCo(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_ForceCCo, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_NetworkPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_NetworkPassword, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DHIt_NetworkPassword(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DHIt_NetworkPassword, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DHIt_OtherNetworksPresent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_OtherNetworksPresent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_MaxBitRate_1490(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_MaxBitRate_1490, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHIt_AssociatedDeviceNumberOfEntries_1491(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHIt_AssociatedDeviceNumberOfEntries_1491, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_BytesSent_1492(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_BytesSent_1492, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_BytesReceived_1493(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_BytesReceived_1493, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_PacketsSent_1494(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_PacketsSent_1494, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_PacketsReceived_1495(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_PacketsReceived_1495, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_ErrorsSent_1496(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_ErrorsSent_1496, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_ErrorsReceived_1497(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_ErrorsReceived_1497, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_UnicastPacketsSent_1498(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_UnicastPacketsSent_1498, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_UnicastPacketsReceived_1499(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_UnicastPacketsReceived_1499, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_DiscardPacketsSent_1500(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_DiscardPacketsSent_1500, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_DiscardPacketsReceived_1501(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_DiscardPacketsReceived_1501, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_MulticastPacketsSent_1502(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_MulticastPacketsSent_1502, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_MulticastPacketsReceived_1503(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_MulticastPacketsReceived_1503, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_BroadcastPacketsSent_1504(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_BroadcastPacketsSent_1504, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_BroadcastPacketsReceived_1505(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_BroadcastPacketsReceived_1505, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_UnknownProtoPacketsReceived_1506(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_UnknownProtoPacketsReceived_1506, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_MPDUTxAck(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_MPDUTxAck, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_MPDUTxCol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_MPDUTxCol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_MPDUTxFailed(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_MPDUTxFailed, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_MPDURxAck(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_MPDURxAck, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItS_MPDURxFailed(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItS_MPDURxFailed, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_MACAddress_1512(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_MACAddress_1512, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_TxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_TxPhyRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_RxPhyRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_RxPhyRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_SNRPerTone(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_SNRPerTone, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_AvgAttenuation(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_AvgAttenuation, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_EndStationMACs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_EndStationMACs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DHItAt_Active_1518(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DHItAt_Active_1518, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DU_InterfaceNumberOfEntries_1519(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DU_InterfaceNumberOfEntries_1519, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_Enable_1520(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Enable_1520, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_Enable_1520(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_Enable_1520, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_Status_1522(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Status_1522, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_Alias_1523(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Alias_1523, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_Alias_1523(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_Alias_1523, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_Name_1525(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Name_1525, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_LastChange_1526(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_LastChange_1526, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_LowerLayers_1527(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_LowerLayers_1527, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_LowerLayers_1527(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_LowerLayers_1527, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_Upstream_1529(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_Upstream_1529, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_MACAddress_1530(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_MACAddress_1530, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_FirmwareVersion(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_FirmwareVersion, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_MaxBitRate_1532(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_MaxBitRate_1532, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_NodeType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_NodeType, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_NodeType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_NodeType, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_LogicalNetwork(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_LogicalNetwork, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_LogicalNetwork(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_LogicalNetwork, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_EncryptionMethod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_EncryptionMethod, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_EncryptionMethod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_EncryptionMethod, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_EncryptionKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_EncryptionKey, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_EncryptionKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_EncryptionKey, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_PowerBackoffEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_PowerBackoffEnabled, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_PowerBackoffEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_PowerBackoffEnabled, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_PowerBackoffMechanismActive(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_PowerBackoffMechanismActive, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_EstApplicationThroughput(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_EstApplicationThroughput, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_ActiveNotchEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_ActiveNotchEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUIt_ActiveNotchEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUIt_ActiveNotchEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUIt_ActiveNotchNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_ActiveNotchNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_AssociatedDeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_AssociatedDeviceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUIt_BridgeForNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUIt_BridgeForNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_BytesSent_1550(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_BytesSent_1550, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_BytesReceived_1551(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_BytesReceived_1551, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_PacketsSent_1552(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_PacketsSent_1552, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_PacketsReceived_1553(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_PacketsReceived_1553, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_ErrorsSent_1554(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_ErrorsSent_1554, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_ErrorsReceived_1555(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_ErrorsReceived_1555, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_UnicastPacketsSent_1556(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_UnicastPacketsSent_1556, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_UnicastPacketsReceived_1557(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_UnicastPacketsReceived_1557, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_DiscardPacketsSent_1558(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_DiscardPacketsSent_1558, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_DiscardPacketsReceived_1559(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_DiscardPacketsReceived_1559, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_MulticastPacketsSent_1560(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_MulticastPacketsSent_1560, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_MulticastPacketsReceived_1561(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_MulticastPacketsReceived_1561, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_BroadcastPacketsSent_1562(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_BroadcastPacketsSent_1562, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_BroadcastPacketsReceived_1563(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_BroadcastPacketsReceived_1563, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItS_UnknownProtoPacketsReceived_1564(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItS_UnknownProtoPacketsReceived_1564, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_Port, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_LogicalNetwork(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_LogicalNetwork, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_PhyTxThroughput(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_PhyTxThroughput, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_PhyRxThroughput(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_PhyRxThroughput, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_RealPhyRxThroughput(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_RealPhyRxThroughput, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_EstimatedPLR(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_EstimatedPLR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_MeanEstimatedAtt(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_MeanEstimatedAtt, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_SmartRouteIntermediatePLCMAC(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_SmartRouteIntermediatePLCMAC, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_DirectRoute(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_DirectRoute, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_Active(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_Active, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItAt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUItAt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUItAt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUItAt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUItAt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUItAt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUItAt_StartFreq(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_StartFreq, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUItAt_StartFreq(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUItAt_StartFreq, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUItAt_StopFreq(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_StopFreq, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUItAt_StopFreq(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUItAt_StopFreq, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUItAt_Depth(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItAt_Depth, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUItAt_Depth(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUItAt_Depth, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUItBt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItBt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUItBt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUItBt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUItBt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItBt_MACAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUItBt_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUItBt_Port, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUDI_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUDI_DiagnosticsState, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUDI_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUDI_DiagnosticsState, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUDI_Type(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUDI_Type, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUDI_Type(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUDI_Type, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUDI_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUDI_Interface, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUDI_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUDI_Interface, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUDI_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUDI_Port, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DUDI_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DUDI_Port, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DUDI_Measurements(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUDI_Measurements, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DUDI_RxGain(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DUDI_RxGain, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWX_Charter_BS_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_CONFIG_ENABLE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BS_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_CONFIG_ENABLE, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
/*int get_DWX_Charter_BS_MatchingSSID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_CONFIG_MATCHINGSSID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BS_MatchingSSID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_CONFIG_MATCHINGSSID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}*/
int get_DWX_Charter_BS_PHYBasedPrioritization(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_CONFIG_PHYBASEDPROIORITIZATION, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BS_PHYBasedPrioritization(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_CONFIG_PHYBASEDPROIORITIZATION, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BS_AgeLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_CONFIG_AGELIMIT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BS_AgeLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_CONFIG_AGELIMIT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_IncludeOutOfNetwork(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STADB_INCLUDEOUTOFNETWORK, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_IncludeOutOfNetwork(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STADB_INCLUDEOUTOFNETWORK, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_MarkAdvClientAsDualBand(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STADB_MARKADVCLIENTSDUALBAND, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_MarkAdvClientAsDualBand(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STADB_MARKADVCLIENTSDUALBAND, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_AgingSizeThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STADB_AGINGSIZETHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_AgingSizeThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STADB_AGINGSIZETHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_AgingFrequency(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STADB_AGINGFREQUENCY, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_AgingFrequency(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STADB_AGINGFREQUENCY, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}

int get_DWX_Charter_BSS_OutOfNetworkMaxAge(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STADB_OUTOFNETWORKMAXAGE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_OutOfNetworkMaxAge(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STADB_OUTOFNETWORKMAXAGE, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}

int get_DWX_Charter_BSS_InNetworkMaxAge(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STADB_INNETWORKMAXAGE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_InNetworkMaxAge(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STADB_INNETWORKMAXAGE, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}

int get_DWX_Charter_BSS_NumRemoteBSSes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STADB_NUMREMOTEBSSES, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_NumRemoteBSSes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STADB_NUMREMOTEBSSES, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}

int get_DWX_Charter_BSS_PopulateNonServingPHYInfo(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STADB_POPULATENONSERVINGPHYINFO, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_PopulateNonServingPHYInfo(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STADB_POPULATENONSERVINGPHYINFO, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_RSSISteeringPoint_DG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IDLESTEER_RSSISTEERINGPOINT_DG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_RSSISteeringPoint_DG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IDLESTEER_RSSISTEERINGPOINT_DG, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_RSSISteeringPoint_UG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IDLESTEER_RSSISTEERINGPOINT_UG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_RSSISteeringPoint_UG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IDLESTEER_RSSISTEERINGPOINT_UG, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_NormalInactTimeout(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IDLESTEER_NORMALINACTTIMEOUT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_NormalInactTimeout(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IDLESTEER_NORMALINACTTIMEOUT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_OverloadInactTimeout(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IDLESTEER_OVERLOADINACTTIMEOUT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_OverloadInactTimeout(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IDLESTEER_OVERLOADINACTTIMEOUT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_InactCheckInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IDLESTEER_INACTCHECKINTERVAL, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_InactCheckInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IDLESTEER_INACTCHECKINTERVAL, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_AuthAllow(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IDLESTEER_AUTHALLOW, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_AuthAllow(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IDLESTEER_AUTHALLOW, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_TxRateXingThreshold_UG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ACTIVESTEER_TXRATEXINGTHRESHOLD_UG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_TxRateXingThreshold_UG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ACTIVESTEER_TXRATEXINGTHRESHOLD_UG, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_RateRSSIXingThreshold_UG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ACTIVESTEER_RATERSSIXINGTHRESHOLD_UG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_RateRSSIXingThreshold_UG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ACTIVESTEER_RATERSSIXINGTHRESHOLD_UG, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_TxRateXingThreshold_DG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ACTIVESTEER_TXRATEXINGTHRESHOLD_DG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_TxRateXingThreshold_DG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ACTIVESTEER_TXRATEXINGTHRESHOLD_DG, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_RateRSSIXingThreshold_DG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ACTIVESTEER_RATERSSIXINGTHRESHOLD_DG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_RateRSSIXingThreshold_DG(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ACTIVESTEER_RATERSSIXINGTHRESHOLD_DG, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSO_MUAvgPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_OFFLOAD_MUAVGPERIOD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSO_MUAvgPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_OFFLOAD_MUAVGPERIOD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSO_MUOverloadThreshold_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_OFFLOAD_MUOVERLOADTHRESHOLD_W2, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSO_MUOverloadThreshold_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_OFFLOAD_MUOVERLOADTHRESHOLD_W2, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSO_MUOverloadThreshold_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_OFFLOAD_MUOVERLOADTHRESHOLD_W5, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSO_MUOverloadThreshold_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_OFFLOAD_MUOVERLOADTHRESHOLD_W5, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSO_MUSafetyThreshold_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_OFFLOAD_MUSAFETYTHRESHOLD_W2, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSO_MUSafetyThreshold_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_OFFLOAD_MUSAFETYTHRESHOLD_W2, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSO_MUSafetyThreshold_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_OFFLOAD_MUSAFETYTHRESHOLD_W5, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSO_MUSafetyThreshold_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_OFFLOAD_MUSAFETYTHRESHOLD_W5, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSO_OffloadingMinRSSI(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_OFFLOAD_OFFLOADINGMINRSSI, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSO_OffloadingMinRSSI(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_OFFLOAD_OFFLOADINGMINRSSI, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_ENABLEW2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IAS_ENABLEW2, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_ENABLEW2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IAS_ENABLEW2, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_ENABLEW5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IAS_ENABLEW5, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_ENABLEW5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IAS_ENABLEW5, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_MAXPOLLUTIONTIME(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IAS_MAXPOLLUTIONTIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_MAXPOLLUTIONTIM(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IAS_MAXPOLLUTIONTIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSI_USEBESTEFFORT(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_IAS_USEBESTEFFORT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSI_USEBESTEFFORT(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_IAS_USEBESTEFFORT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_SteeringProhibitTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_STEERINGPROHIBITTIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_SteeringProhibitTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_STEERINGPROHIBITTIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_BTMSteeringProhibitShortTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_BTMSTEERINGPROHIBITSHORTTIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_BTMSteeringProhibitShortTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_BTMSTEERINGPROHIBITSHORTTIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_TSteering(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_TSTEERING, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_TSteering(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_TSTEERING, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_InitialAuthRejCoalesceTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_INITIALAUTHREJCOALESCETIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_InitialAuthRejCoalesceTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_INITIALAUTHREJCOALESCETIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}

int get_DWX_Charter_BSS_AuthRejMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_AUTHREJMAX, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_AuthRejMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_AUTHREJMAX, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_SteeringUnfriendlyTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_STEERINGUNFRIENDLYTIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_SteeringUnfriendlyTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_STEERINGUNFRIENDLYTIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_MaxSteeringUnfriendly(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_MAXSTEERINGUNFRIENDLY, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_MaxSteeringUnfriendly(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_MAXSTEERINGUNFRIENDLY, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_TargetLowRSSIThreshold_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_TARGETLOWRSSITHRESHOLDW2, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_TargetLowRSSIThreshold_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_TARGETLOWRSSITHRESHOLDW2, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_TargetLowRSSIThreshold_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_TARGETLOWRSSITHRESHOLDW5, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_TargetLowRSSIThreshold_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_TARGETLOWRSSITHRESHOLDW5, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_BlacklistTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_BLACKLISTTIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_BlacklistTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_BLACKLISTTIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_BTMResponseTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_BTMRESPONSETIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_BTMResponseTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_BTMRESPONSETIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_BTMAssociationTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_BTMASSOCIATIONTIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_BTMAssociationTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_BTMASSOCIATIONTIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_BTMAlsoBlacklist(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_BTMALSOBLACKLIST, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_BTMAlsoBlacklist(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_BTMALSOBLACKLIST, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_BTMUnfriendlyTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_BTMUNFRIENDLYTIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_BTMUnfriendlyTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_BTMUNFRIENDLYTIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_MaxBTMUnfriendly(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_MAXBTMUNFRIENDLY, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_MaxBTMUnfriendly(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_MAXBTMUNFRIENDLY, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_MaxBTMActiveUnfriendly(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_MAXBTMACTIVEUNFRIENDLY, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_MaxBTMActiveUnfriendly(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_MAXBTMACTIVEUNFRIENDLY, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_MinRSSIBestEffort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_MINRSSIBESTEFFORT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_MinRSSIBestEffort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_MINRSSIBESTEFFORT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_StartInBTMActiveState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_STARTINBTMACTIVESTATE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_StartInBTMActiveState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_STARTINBTMACTIVESTATE, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_LowRSSIXingThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_LOWRSSIXINGTHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_LowRSSIXingThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_LOWRSSIXINGTHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_Delay24GProbeRSSIThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_DELAY24GPROBERSSITHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_Delay24GProbeRSSIThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_DELAY24GPROBERSSITHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_Delay24GProbeTimeWindow(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_DELAY24GPROBETIMEWINDOW, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_Delay24GProbeTimeWindowd(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_DELAY24GPROBETIMEWINDOW, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_Delay24GProbeMinReqCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEEREXEC_DELAY24GPROBEMINREQCOUNT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_Delay24GProbeMinReqCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEEREXEC_DELAY24GPROBEMINREQCOUNT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_RSSIDiff_EstW5FromW2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_RSSIDIFF_ESTW5FROMW2, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_RSSIDiff_EstW5FromW2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_RSSIDIFF_ESTW5FROMW2, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_RSSIDiff_EstW2FromW5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_RSSIDIFF_ESTW2FROMW5, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_RSSIDiff_EstW2FromW5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_RSSIDIFF_ESTW2FROMW5, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_ProbeCountThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_PROBECOUNTTHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_ProbeCountThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_PROBECOUNTTHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_StatsSampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_STATSSAMPLEINTERVAL, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_StatsSampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_STATSSAMPLEINTERVAL, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_11kProhibitTimeShort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_11KPROHIBITTIMESHORT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_11kProhibitTimeShort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_11KPROHIBITTIMESHORT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_11kProhibitTimeLong(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_11KPROHIBITTIMELONG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_11kProhibitTimeLong(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_11KPROHIBITTIMELONG, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}

int get_DWX_Charter_BSS_PhyRateScalingForAirtime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_PHYRATESCALINGFORAIRTIME, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_PhyRateScalingForAirtime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_PHYRATESCALINGFORAIRTIME, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_EnableContinuousThroughput(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_ENABLECONTINUOUSTHROUGHPUT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_EnableContinuousThroughput(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_ENABLECONTINUOUSTHROUGHPUT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_BcnrptActiveDuration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_BCNRPTACTIVEDURATION, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_BcnrptActiveDuration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_BCNRPTACTIVEDURATION, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_BcnrptPassiveDuration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_BCNRPTPASSIVEDURATION, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_BcnrptPassiveDuration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_BCNRPTPASSIVEDURATION, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_FastPollutionDetectBufSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_FASTPOLLUTIONDETECTBUFSIZE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_FastPollutionDetectBufSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_FASTPOLLUTIONDETECTBUFSIZE, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_NormalPollutionDetectBufSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_NORMALPOLLUTIONDETECTBUFSIZE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_NormalPollutionDetectBufSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_NORMALPOLLUTIONDETECTBUFSIZE, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_PollutionDetectThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_POLLUTIONDETECTTHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_PollutionDetectThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_POLLUTIONDETECTTHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_PollutionClearThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_POLLUTIONCLEARTHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_PollutionClearThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_POLLUTIONCLEARTHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_InterferenceAgeLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_INTERFERENCEAGELIMIT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_InterferenceAgeLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_INTERFERENCEAGELIMIT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_IASLowRSSIThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_IASLOWRSSITHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_IASLowRSSIThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_IASLOWRSSITHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_IASMaxRateFactor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_IASMAXRATEFACTOR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_IASMaxRateFactor(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_IASMAXRATEFACTOR, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_IASMinDeltaBytes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_IASMINDELTABYTES, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_IASMinDeltaBytes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_IASMINDELTABYTES, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_IASMinDeltaPackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_ESTIMATOR_ADV_IASMINDELTAPACKETS, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_IASMinDeltaPackets(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_ESTIMATOR_ADV_IASMINDELTAPACKETS, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_EnableLog(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_ENABLELOG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_EnableLog(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_DIAGLOG_ENABLELOG, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_LogServerIP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_LOGSERVERIP, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_LogServerIP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (isValidIP(value) == 0)
		return -2;

	ret = do_uci_set(LBD_DIAGLOG_LOGSERVERIP, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_LogServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_LOGSERVERPORT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_LogServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (atoi(value) < 0 || atoi(value) > 65535)
		return -2;

	ret = do_uci_set(LBD_DIAGLOG_LOGSERVERPORT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_LogLevelWlanIF(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_LOGLEVELWLANIF, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_LogLevelWlanIF(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_DIAGLOG_LOGLEVELWLANIF, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_LogLevelBandMon(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_LOGLEVELBANDMON, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_LogLevelBandMon(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_DIAGLOG_LOGLEVELBANDMON, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_LogLevelStaDB(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_LOGLEVELSTADB, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_LogLevelStaDB(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_DIAGLOG_LOGLEVELSTADB, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_LogLevelSteerExec(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_LOGLEVELSTEEREXEC, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_LogLevelSteerExec(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_DIAGLOG_LOGLEVELSTEEREXEC, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_LogLevelStaMon(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_LOGLEVELSTAMON, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_LogLevelStaMon(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_DIAGLOG_LOGLEVELSTAMON, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_LogLevelEstimator(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_LOGLEVELESTIMATOR, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_LogLevelEstimator(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_DIAGLOG_LOGLEVELESTIMATOR, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSD_LogLevelDiagLog(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_DIAGLOG_LOGLEVELDIAGLOG, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSD_LogLevelDiagLog(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_DIAGLOG_LOGLEVELDIAGLOG, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_LowRSSIAPSteerThreshold_CAP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_APSTEER_LOWRSSIAPSTEERTHRESHOLD_CAP, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_LowRSSIAPSteerThreshold_CAP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_APSTEER_LOWRSSIAPSTEERTHRESHOLD_CAP, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_LowRSSIAPSteerThreshold_RE(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_APSTEER_LOWRSSIAPSTEERTHRESHOLD_RE, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_LowRSSIAPSteerThreshold_RE(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_APSTEER_LOWRSSIAPSTEERTHRESHOLD_RE, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_APSteerToRootMinRSSIIncThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_APSTEER_APSTEERTOROOTMINRSSIINCTHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_APSteerToRootMinRSSIIncThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_APSTEER_APSTEERTOROOTMINRSSIINCTHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_APSteerToLeafMinRSSIIncThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_APSTEER_APSTEERTOLEAFMINRSSIINCTHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_APSteerToLeafMinRSSIIncThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_APSTEER_APSTEERTOLEAFMINRSSIINCTHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_APSteerToPeerMinRSSIIncThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_APSTEER_APSTEERTOPEERMINRSSIINCTHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_APSteerToPeerMinRSSIIncThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_APSTEER_APSTEERTOPEERMINRSSIINCTHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSA_DownlinkRSSIThreshold_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_APSTEER_DOWNLINKRSSITHRESHOLD_W5, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSA_DownlinkRSSIThreshold_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_APSTEER_DOWNLINKRSSITHRESHOLD_W5, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_RSSIMeasureSamples_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STAMONITOR_ADV_RSSIMEASURESAMPLES_W2, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_RSSIMeasureSamples_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STAMONITOR_ADV_RSSIMEASURESAMPLES_W2, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_RSSIMeasureSamples_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STAMONITOR_ADV_RSSIMEASURESAMPLES_W5, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_RSSIMeasureSamples_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STAMONITOR_ADV_RSSIMEASURESAMPLES_W5, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_MinTxRateIncreaseThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEERALG_ADV_MINTXRATEINCREASETHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_MinTxRateIncreaseThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEERALG_ADV_MINTXRATEINCREASETHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSS_MaxSteeringTargetCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_STEERALG_ADV_MAXSTEERINGTARGETCOUNT, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSS_MaxSteeringTargetCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_STEERALG_ADV_MAXSTEERINGTARGETCOUNT, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSB_ProbeCountThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_BANDMONITOR_ADV_PROBECOUNTTHRESHOLD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSB_ProbeCountThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_BANDMONITOR_ADV_PROBECOUNTTHRESHOLD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSB_MUCheckInterval_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_BANDMONITOR_ADV_MUCHECKINTERVAL_W2, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSB_MUCheckInterval_W2(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_BANDMONITOR_ADV_MUCHECKINTERVAL_W2, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSB_MUCheckInterval_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_BANDMONITOR_ADV_MUCHECKINTERVAL_W5, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSB_MUCheckInterval_W5(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_BANDMONITOR_ADV_MUCHECKINTERVAL_W5, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSB_MUReportPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_BANDMONITOR_ADV_MUREPORTPERIOD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSB_MUReportPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_BANDMONITOR_ADV_MUREPORTPERIOD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSB_LoadBalancingAllowedMaxPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_BANDMONITOR_ADV_LOADBALANCINGALLOWEDMAXPERIOD, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSB_LoadBalancingAllowedMaxPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_BANDMONITOR_ADV_LOADBALANCINGALLOWEDMAXPERIOD, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWX_Charter_BSB_NumRemoteChannels(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(LBD_BANDMONITOR_ADV_NUMREMOTECHANNELS, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWX_Charter_BSB_NumRemoteChannels(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(LBD_BANDMONITOR_ADV_NUMREMOTECHANNELS, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(LBD);
		if(ret)
		{
			return (-1);
		}
	}
	ret = doBandSteeringFuncs();
	if(ret)
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DW_RadioNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DW_RadioNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/;

	/*int number = 0;
	int find = 0;
	
	find = do_uci_get(WIFI0_DISABLED, value);

	if(!find)
	{
		number++;
	}
	
	find = do_uci_get(WIFI1_DISABLED, value);
	if(!find)
	{
		number++;
	}

	sprintf(value, "%d", number);*/
	strcpy(value, "2"); //2.4G and 5G
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DW_SSIDNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DW_SSIDNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/

	sprintf(value, "%d", WIFI_MAX_INSTANCE_NUM);
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DW_AccessPointNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DW_AccessPointNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/

	sprintf(value, "%d", WIFI_MAX_INSTANCE_NUM);
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DW_EndPointNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DW_EndPointNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always 1, menas one repeater
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_Enable(char * path_name, char *value)
{
	int ret = 0;
	/*ret = do_uci_get(DWRt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.radio%d.disabled", getWiFiRadioUciNum(p));

	tr_log(LOG_DEBUG,"buff [%s]",buff);

	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		//return -1;
		strcpy(value, "1"); // disabled VALUE only 1 and NULL
	}

	if(atoi(tmp) == 1)
	{
		strcpy(value, "0");
	}
	#if 0
	else
	{
		strcpy(value, "1");
	}
	#endif
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(buff, "wireless.wifi%d.disabled", getWiFiRadioUciNum(p));
	
	if(atoi(value) == 1)
	{
		ret = do_uci_set(buff, "0");
	}
	else if(atoi(value) == 0)
	{
		ret = do_uci_set(buff, "1");
	}
	else
	{
		return (-1);
	}	

	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.wifi%d.disabled", getWiFiRadioUciNum(p));
	
	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}
	
	if(atoi(tmp) == 1)
	{
		strcpy(value, "Down");
	}
	else
	{
		strcpy(value, "Up");
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DWRt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_Name, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	type = getWiFiRadioType(p);

	if(type == 5)
	{
		ret = do_uci_get("wireless.wla.device", value);
	}
	else if(type == 24)
	{
		ret = do_uci_get("wireless.wlg.device", value);
	}

	if(ret)
	{
		return -1;
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_LastChange, value);
	if(ret)
	{
		return -1;
	}*/
	getInterfaceLastChangeTime(value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //Since Radio is a layer 1 interface, it is expected that?LowerLayers?will not be used.
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't do anything
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_Upstream(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_Upstream, value);
	if(ret)
	{
		return -1;
	}*/

	strcpy(value, "0");//Upstream will be false for all LAN interface.
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_MaxBitRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_MaxBitRate, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");
	
	if (p == NULL)
		return -1;
		
	type = getWiFiRadioType(p);


	if(type == 5)
	{
		strcpy(value, "1733");
	}
	else if(type == 24)
	{
		strcpy(value, "800");
	}

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_SupportedFrequencyBands(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_SupportedFrequencyBands, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5)
	{
		strcpy(value, "5GHz");
	}
	else if(type == 24)
	{
		strcpy(value, "2.4GHz");
	}

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_OperatingFrequencyBand(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_OperatingFrequencyBand, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(buff, "wireless.wifi%d.mode", getWiFiRadioUciNum(p));
	
	ret = do_uci_get(buff, value);

	p = strstr(value, "_");
	if(p != NULL)
	{
		*p = '.';
	}

	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_OperatingFrequencyBand(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_OperatingFrequencyBand, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_SupportedStandards(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_SupportedStandards, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	type = getWiFiRadioType(p);

	if(type == 5)	
	{
		strcpy(value, "a,n,ac");
	}
	else if(type == 24)
	{
		strcpy(value, "b,g,n");
	}	
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_OperatingStandards(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_OperatingStandards, value);
	if(ret)
	{
		return -1;
	}*/
	char path[128] = {0};
	char buff[32] = {0};
 	char pure_n[32] = {0};
	char pure11ac[32] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	strcpy(pure_n, "0");
	sprintf(path, "wireless.radio%d.hwmode", getWiFiRadioUciNum(p));
	ret = do_uci_get(path, buff);
	//ret = do_uci_get("wireless.wlg.puren", pure_n);
	//ret = do_uci_get("wireless.wla.pure11ac", pure11ac);

	if(!strcmp(buff, "11b"))
		strcpy(value, "b");
	else if(!strcmp(buff, "11bg"))
		strcpy(value, "b,g");
	else if(!strcmp(buff, "11g"))
		strcpy(value, "g");
	else if(!strcmp(buff, "11a"))
		strcpy(value, "a");
	//else if(!strcmp(buff, "11ng") && !strcmp(pure_n, "0"))
	else if(!strcmp(buff, "11ng"))
		strcpy(value, "b,g,n");
	else if(!strcmp(buff, "11na"))
		strcpy(value, "n,a");
	//else if(!strcmp(buff, "11ac")  && !strcmp(pure11ac, "1"))
	else if(!strcmp(buff, "11ac"))
		strcpy(value, "ac");
	//else if(!strcmp(buff, "11ac")  && !strcmp(pure11ac, "0"))
		//strcpy(value, "ac,n,a");
	//else if(!strcmp(buff, "11ng") && !strcmp(pure_n, "1"))
		//strcpy(value, "n");
	  
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_OperatingStandards(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_OperatingStandards, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *pure_n = "0";
	char *pure11ac = "0";
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	sprintf(buff, "wireless.wifi%d.hwmode", getWiFiRadioUciNum(p));

	if(type == 24)
	{
		if(!strcasecmp(value, "b"))
		{
			strcpy(tmp, "11b");
		}
		else if(!strcasecmp(value, "g"))
		{
			strcpy(tmp, "11g");
		}
		else if(!strcasecmp(value, "n"))
		{
			strcpy(tmp, "11ng");
			pure_n = "1";
		}
		else if(!strcasecmp(value, "b,g") || !strcasecmp(value, "g,b"))
		{
			strcpy(tmp, "11bg");
		}
		else if(!strcasecmp(value, "n,g") || !strcasecmp(value, "g,n"))
		{
			strcpy(tmp, "11ng");
		}
		else
			return -2;
	}
	else if(type == 5)
	{
		if(!strcasecmp(value, "a"))
		{
			strcpy(tmp, "11a");
		}
		else if(!strcasecmp(value, "n,a") || !strcasecmp(value, "a,n"))
		{
			strcpy(tmp, "11na");
		}		
		else if(!strcasecmp(value, "ac"))
		{
			strcpy(tmp, "11ac");
			pure11ac = "1";
		}
		else if(!strcasecmp(value, "ac,n,a") || !strcasecmp(value, "ac,a,n") 
			 || !strcasecmp(value, "n,ac,a") || !strcasecmp(value, "n,a,ac") 
			 || !strcasecmp(value, "a,n,ac") || !strcasecmp(value, "a,ac,n"))
		{
			strcpy(tmp, "11ac");
			pure11ac = "0";
		}
		else
			return -2;
	}

	ret = do_uci_set(buff, tmp);
	
	if(type == 24) //don't care guest_zone puren value
	{
		ret = do_uci_set("wireless.wlg.puren", pure_n);
	}
	else if(type == 5)
	{
		ret = do_uci_set("wireless.wla.pure11ac", pure11ac);
	}
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_PossibleChannels(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_PossibleChannels, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5)
	{
		sprintf(value, "36-165");
	}
	else if(type == 24)
	{
		sprintf(value, "1-11");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_ChannelsInUse(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_ChannelsInUse, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5)
	{
		sprintf(value, "36-165");
	}
	else if(type == 24)
	{
		sprintf(value, "1-11");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_Channel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_Channel, value);
	if(ret)
	{
		return -1;
	}*/
	char path[128] = {0};
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(path, "wireless.radio%d.channel", getWiFiRadioUciNum(p));
	ret = do_uci_get(path, buff);

	strcpy(value,buff);

	#if 0
	if(getWiFiRadioUciNum(p)==0)
	{
		/*5G*/ 
		ret = do_uci_get("detection.@curchan[0].5G", value);
		if(ret)
		{
			return -1;
		}
		tr_log(LOG_DEBUG,"get value 5G= [%s]",value);
	}
	if(getWiFiRadioUciNum(p)==1)
	{
		/*2.4G*/
		ret = do_uci_get("detection.@curchan[0].2_4G", value);
		if(ret)
		{
			return -1;
		}		
		tr_log(LOG_DEBUG,"get value 2.4G=[%s]",value);
	}
	#if 0
	sprintf(buff, "wireless.wifi%d.channel", getWiFiRadioUciNum(p));
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}

	//for auto channel, to get the current using channel
	if (strcasecmp(value, "auto") == 0){
		FILE *fd = NULL;
		char cmd[128] = {0};
		char line[128] = {0};
		char *ptr = NULL, *ptr2 = NULL;
			
		sprintf(cmd, "iwlist ath%d channel | grep 'Current Frequency' | sed 's/(//g' | sed 's/)//g'", getWiFiRadioUciNum(p));
		if ((fd = popen(cmd, "r")) != NULL){
			if (fgets(line,sizeof(line)-1,fd)){
				if ((ptr = strstr(line, "Channel")) != NULL){
					sscanf(ptr, "%*s %s", value);
					if ((ptr2 = strstr(value, "\n")) != NULL)
						*ptr2 = '\0';
				}
			}
			pclose(fd);
		}
	}
	#endif
	#endif
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_Channel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_Channel, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5)
	{
		if(!(atoi(value) >= 36 && atoi(value) <= 165) && strcasecmp(value, "auto"))
		{
			return -2;
		}
	}
	else if(type == 24)
	{
		if(!(atoi(value) >= 1 && atoi(value) <= 11) && strcasecmp(value, "auto"))
		{
			return -2;
		}
	}
	
	sprintf(buff, "wireless.wifi%d.channel", getWiFiRadioUciNum(p));

	if(!strcasecmp(value, "auto"))
	{
		strcpy(value, "auto");
	}
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_AutoChannelSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_AutoChannelSupported, value);
	if(ret)
	{
		return -1;
	}*/

	strcpy(value, "1");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_AutoChannelEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_AutoChannelEnable, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.wifi%d.channel", getWiFiRadioUciNum(p));
	
	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}

	if(strcmp(tmp, "auto") != 0)
	{
		strcpy(value, "0");
	}
	else
	{
		strcpy(value, "1");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_AutoChannelEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_AutoChannelEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	const struct iwinfo_ops *iw;
	int ch;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(buff, "wireless.wifi%d.channel", getWiFiRadioUciNum(p));

	if(atoi(value) == 1)
	{
		ret = do_uci_set(buff, "auto");
	}
	else if(atoi(value) == 0) //to set current channel value
	{
		sprintf(tmp, "ath%d", getWiFiRadioUciNum(p));
		iw = iwinfo_backend(tmp);
		if (!iw)
		{
			tr_log(LOG_DEBUG,"No such wireless device: %s\n",tmp);
		}
		else
		{
			if (iw->channel(tmp, &ch))
			{
				ch = -1;
			}
			if (ch <= 0)
			{
				ret = do_uci_set(buff, "auto");
			}
			else
			{
				snprintf(tmp, sizeof(tmp), "%d", ch);
				ret = do_uci_set(buff, tmp);
			}
		}
		iwinfo_finish();	
	}
	else
	{
		return (-1);
	}
		

	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_AutoChannelRefreshPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_AutoChannelRefreshPeriod, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_AutoChannelRefreshPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_AutoChannelRefreshPeriod, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_OperatingChannelBandwidth(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_OperatingChannelBandwidth, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char buff1[128] = {0};
	char tmp[128] = {0};
	char tmp1[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.wifi%d.htmode", getWiFiRadioUciNum(p));
	sprintf(buff1, "wireless.wifi%d.chbandwh", getWiFiRadioUciNum(p));
	ret = do_uci_get(buff, tmp);
	ret = do_uci_get(buff1, tmp1);
	if(ret)
	{
		return -1;
	}

	if(strcmp(tmp, "HT80") == 0)
	{
		if (atoi(tmp1) == 4)
			strcpy(value, "Auto");
		else //buff1 value is 3
			strcpy(value, "80MHz");
	}
	else if(strstr(tmp, "HT40") != NULL)
	{
		if (atoi(p) == 1) //5G
			strcpy(value, "40MHz");
		else{ //2.4G
			if (atoi(tmp1) == 0)
				strcpy(value, "Auto");
			else //buff1 value is 2
				strcpy(value, "40MHz");
		}
	}
	else if(strcmp(tmp, "HT20") == 0)
	{
		strcpy(value, "20MHz");
	}
	else if(strcmp(tmp, "HT160") == 0)
	{
		strcpy(value, "160MHz"); //tmp1 is 5
	}
	else if(strcmp(tmp, "HT80_80") == 0)
	{
		strcpy(value, "80+80MHz"); //tmp1 is 6
	}
		
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_OperatingChannelBandwidth(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_OperatingChannelBandwidth, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char buff1[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.wifi%d.htmode", getWiFiRadioUciNum(p));
	sprintf(buff1, "wireless.wifi%d.chbandwh", getWiFiRadioUciNum(p));

	if(strcasecmp(value, "auto") == 0)
	{
		if(atoi(p) == 1)
		{
			ret = do_uci_set(buff, "HT80");
			ret = do_uci_set(buff1, "4");
		}
		else if(atoi(p) == 2)
		{
			ret = do_uci_set(buff, "HT40");
			ret = do_uci_set(buff1, "0");
		}
	}
	else if(strcasecmp(value, "80MHz") == 0)
	{
		if(atoi(p) == 1){
			ret = do_uci_set(buff, "HT80");
			ret = do_uci_set(buff1, "3");
		}
		else //2.4G not support 80MHz
			return -1;
	}
	else if(strcasecmp(value, "40MHz") == 0)
	{
		ret = do_uci_set(buff, "HT40");
		ret = do_uci_set(buff1, "2");
	}
	else if(strcasecmp(value, "20MHz") == 0)
	{
		ret = do_uci_set(buff, "HT20");
		ret = do_uci_set(buff1, "1");
	}
	else if(strcasecmp(value, "160MHz") == 0)
	{
		ret = do_uci_set(buff, "HT160");
		ret = do_uci_set(buff1, "5");
	}
	else if(strcasecmp(value, "80+80MHz") == 0)
	{
		ret = do_uci_set(buff, "HT80_80");
		ret = do_uci_set(buff1, "6");
	}
	else
	{
		return -2;
	}

	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}		
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_CurrentOperatingChannelBandwidth(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_CurrentOperatingChannelBandwidth, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char buff1[128] = {0};
	char tmp[128] = {0};
	char tmp1[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.wifi%d.htmode", getWiFiRadioUciNum(p));
	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}

	if(strcmp(tmp, "HT80") == 0)
	{
		strcpy(value, "80MHz");
	}
	else if(strstr(tmp, "HT40") != NULL)
	{
		strcpy(value, "40MHz");
	}
	else if(strcmp(tmp, "HT20") == 0)
	{
		strcpy(value, "20MHz");
	}
	else if(strcmp(tmp, "HT160") == 0)
	{
		strcpy(value, "160MHz");
	}
	else if(strcmp(tmp, "HT80_80") == 0)
	{
		strcpy(value, "80MHz");
	}
	else
		strcpy(value, "40MHz");
		
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_ExtensionChannel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_ExtensionChannel, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.wifi%d.htmode", getWiFiRadioUciNum(p));

	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}	

	if(!strcmp(tmp, "HT40+"))
	{
		strcpy(value, "AboveControlChannel");
	}
	else if(!strcmp(tmp, "HT40-")) 
	{
		strcpy(value, "BelowControlChannel");
	}
	else
	{
		strcpy(value, "Auto");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_ExtensionChannel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_ExtensionChannel, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.wifi%d.htmode", getWiFiRadioUciNum(p));

	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}	

	if(strstr(tmp, "HT40") != NULL)
	{
		if(strcasecmp(value, "AboveControlChannel") == 0)
		{
			ret = do_uci_set(buff, "HT40+");	
		}
		else if(strcasecmp(value, "BelowControlChannel") == 0)
		{
			ret = do_uci_set(buff, "HT40-");	
		}
		else if(strcasecmp(value, "auto") == 0)
		{
			ret = do_uci_set(buff, "HT40");	
		}
		else
		{
			return -2;
		}				
	}
	else
	{
		return -1;
	}	

	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_GuardInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_GuardInterval, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5)
	{
		strcpy(buff, "wireless.wla.shortgi");
	}
	else if(type == 24)
	{
		strcpy(buff, "wireless.wlg.shortgi");
	}
	else
	{
		return -1;
	}

	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}

	if(atoi(tmp) == 1)
	{
		strcpy(value, "400nsec");
	}
	else
	{
		strcpy(value, "800nsec");
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_GuardInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_GuardInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char buff1[128] = {0};
	char buff2[128] = {0};
	char tmp[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5) //don't care guest_zone ssid value
	{
		strcpy(buff, "wireless.wla.shortgi");
		strcpy(buff1, "wireless.wla1.shortgi");
		strcpy(buff2, "wireless.wla2.shortgi");
	}
	else if(type == 24)
	{
		strcpy(buff, "wireless.wlg.shortgi");
		strcpy(buff1, "wireless.wlg1.shortgi");
		strcpy(buff2, "wireless.wlg2.shortgi");
	}
	else
	{
		return -1;
	}

	if(strcasecmp(value, "400nsec") == 0)
	{
		ret = do_uci_set(buff, "1");
		ret = do_uci_set(buff1, "1");
		ret = do_uci_set(buff2, "1");
	}
	else if(strcasecmp(value, "800nsec") == 0)
	{
		ret = do_uci_set(buff, "0");
		ret = do_uci_set(buff1, "0");
		ret = do_uci_set(buff2, "0");
	}
	else if(strcasecmp(value, "auto") == 0)
	{
		ret = do_uci_set(buff, "1");
		ret = do_uci_set(buff1, "1");
		ret = do_uci_set(buff2, "1");
	}
	else
	{
		return -2;
	}

	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_MCS(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_MCS, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "-1"); //means auto mode
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_MCS(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_MCS, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_TransmitPowerSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_TransmitPowerSupported, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "25,50,75,100");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_TransmitPower(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_TransmitPower, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.wifi%d.tpscale", getWiFiRadioUciNum(p));

	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}

	if(!strcmp(tmp, "0"))
	{
		strcpy(value, "100");
	}
	else if(!strcmp(tmp, "1"))
	{
		strcpy(value, "75");
	}
	else if(!strcmp(tmp, "2"))
	{
		strcpy(value, "50");
	}
	else if(!strcmp(tmp, "3"))
	{
		strcpy(value, "25");
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_TransmitPower(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_TransmitPower, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	sprintf(buff, "wireless.wifi%d.tpscale", getWiFiRadioUciNum(p));

	if(!strcmp(value, "100"))	
	{
		ret = do_uci_set(buff, "0");
	}
	else if(!strcmp(value, "75"))	
	{
		ret = do_uci_set(buff, "1");
	}
	else if(!strcmp(value, "50"))	
	{
		ret = do_uci_set(buff, "2");
	}
	else if(!strcmp(value, "25"))	
	{
		ret = do_uci_set(buff, "3");
	}
	else
	{
		return -2;
	}
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_IEEE80211hSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_IEEE80211hSupported, value);
	if(ret)
	{
		return -1;
	}*/
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5)
	{
		strcpy(value, "1");
	}
	else if(type == 24)
	{
		strcpy(value, "0");
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_IEEE80211hEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_IEEE80211hEnabled, value);
	if(ret)
	{
		return -1;
	}*/
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5)
	{
		strcpy(value, "1");
	}
	else if(type == 24)
	{
		strcpy(value, "0");
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_IEEE80211hEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_IEEE80211hEnabled, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don''t do anything
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_RegulatoryDomain(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_RegulatoryDomain, value);
	if(ret)
	{
		return -1;
	}*/

	strcpy(value, "US");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_RegulatoryDomain(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_RegulatoryDomain, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don''t do anything
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_RetryLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRt_RetryLimit, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_RetryLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWRt_RetryLimit, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_CCARequest(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRt_CCARequest, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_CCARequest(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWRt_CCARequest, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_CCAReport(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRt_CCAReport, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_RPIHistogramRequest(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRt_RPIHistogramRequest, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_RPIHistogramRequest(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWRt_RPIHistogramRequest, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_RPIHistogramReport(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRt_RPIHistogramReport, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRt_FragmentationThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_FragmentationThreshold, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if (type == 5)
		strcpy(buff, "wireless.wla.frag");
	else
		strcpy(buff, "wireless.wlg.frag");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_FragmentationThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_FragmentationThreshold, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	type = getWiFiRadioType(p);

	if (atoi(value) < 256 || atoi(value) > 2346)
		return -2;

	if (type == 5)
		strcpy(buff, "wireless.wla.frag");
	else
		strcpy(buff, "wireless.wlg.frag");
		
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_RTSThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_RTSThreshold, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	type = getWiFiRadioType(p);

	if (type == 5)
		strcpy(buff, "wireless.wla.rts");
	else
		strcpy(buff, "wireless.wlg.rts");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_RTSThreshold(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_RTSThreshold, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	type = getWiFiRadioType(p);

	if (atoi(value) < 0 || atoi(value) > 2347)
		return -2;
	
	if (type == 5)
		strcpy(buff, "wireless.wla.rts");
	else
		strcpy(buff, "wireless.wlg.rts");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}			
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_LongRetryLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRt_LongRetryLimit, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_LongRetryLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWRt_LongRetryLimit, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_BeaconPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_BeaconPeriod, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	type = getWiFiRadioType(p);

	if (type == 5)
		strcpy(buff, "wireless.wla.bintval");
	else
		strcpy(buff, "wireless.wlg.bintval");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_BeaconPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_BeaconPeriod, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	type = getWiFiRadioType(p);

	if (atoi(value) < 100 || atoi(value) > 1000)
		return -2;

	if (type == 5)
		strcpy(buff, "wireless.wla.bintval");
	else
		strcpy(buff, "wireless.wlg.bintval");
		
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}			
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_DTIMPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_DTIMPeriod, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	type = getWiFiRadioType(p);

	if (type == 5)
		strcpy(buff, "wireless.wla.dtim_period");
	else
		strcpy(buff, "wireless.wlg.dtim_period");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_DTIMPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_DTIMPeriod, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;
	
	type = getWiFiRadioType(p);

	if (atoi(value) < 1 || atoi(value) > 255)
		return -2;

	if (type == 5)
		strcpy(buff, "wireless.wla.dtim_period");
	else
		strcpy(buff, "wireless.wlg.dtim_period");
		
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}			
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_PacketAggregationEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRt_PacketAggregationEnable, value);
	/*if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1");//always 1
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_PacketAggregationEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_PacketAggregationEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_PreambleType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_PreambleType, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5)
	{
		strcpy(buff, "wireless.wla.shortgi");
	}
	else if(type == 24)
	{
		strcpy(buff, "wireless.wlg.shortgi");
	}
	else
	{
		return -1;
	}

	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}

	if(atoi(tmp) == 1)
	{
		strcpy(value, "short");
	}
	else
	{
		strcpy(value, "long");
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_PreambleType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWRt_PreambleType, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char buff1[128] = {0};
	char buff2[128] = {0};
	char buff3[128] = {0};
	char tmp[128] = {0};
	int type;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	type = getWiFiRadioType(p);

	if(type == 5)
	{
		strcpy(buff, "wireless.wla.shortgi");
		strcpy(buff1, "wireless.spectrumWiFi5g.shortgi");
		strcpy(buff2, "wireless.wla_hotspot.shortgi");
		strcpy(buff3, "wireless.wla_mesh.shortgi");
	}
	else if(type == 24)
	{
		strcpy(buff, "wireless.wlg.shortgi");
		strcpy(buff1, "wireless.spectrumWiFi.shortgi");
		strcpy(buff2, "wireless.wlg_hotspot.shortgi");
		strcpy(buff3, "wireless.wlg_mesh.shortgi");
	}
	else
	{
		return -1;
	}

	if(strcasecmp(value, "short") == 0)
	{
		ret = do_uci_set(buff, "1");
		ret = do_uci_set(buff1, "1");
		ret = do_uci_set(buff2, "1");
		ret = do_uci_set(buff3, "1");
	}
	else if(strcasecmp(value, "long") == 0)
	{
		ret = do_uci_set(buff, "0");
		ret = do_uci_set(buff1, "0");
		ret = do_uci_set(buff2, "0");
		ret = do_uci_set(buff3, "0");
	}
	else if(strcasecmp(value, "auto") == 0)
	{
		ret = do_uci_set(buff, "1");
		ret = do_uci_set(buff1, "1");
		ret = do_uci_set(buff2, "1");
		ret = do_uci_set(buff3, "1");
	}
	else
	{
		return -2;
	}

	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_BasicDataTransmitRates(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRt_BasicDataTransmitRates, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_BasicDataTransmitRates(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWRt_BasicDataTransmitRates, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_OperationalDataTransmitRates(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRt_OperationalDataTransmitRates, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWRt_OperationalDataTransmitRates(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWRt_OperationalDataTransmitRates, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWRt_SupportedDataTransmitRates(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRt_SupportedDataTransmitRates, value);
	if(ret)
	{
		return -1;
	}*/
	FILE *fd = NULL;
	char cmd[128] = {0};
	char line[128] = {0};
	char rate[32] = {0};
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(cmd, "iwlist ath%d rate", getWiFiRadioUciNum(p));
	if ((fd = popen(cmd, "r")) != NULL){
		fgets(line,sizeof(line)-1,fd); //ingor first line
		memset(line, 0, sizeof(line));
		while (fgets(line,sizeof(line)-1,fd)){
			if (strstr(line, "Current") != NULL)
				break;
			memset(rate, 0, sizeof(rate));
			sscanf(line, "%s %*s", rate);
			if (strcmp(value, "") == 0){
				strcpy(value, rate);
			}
			else
				sprintf(value, "%s,%s", value, rate);
			memset(line, 0, sizeof(line));
		}
		pclose(fd);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] ={0};
	char tmp[128] ={0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(tmp, "apstats -r -i wifi%d | grep 'Tx Data Bytes'", getWiFiRadioUciNum(p));
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
			if((q = strstr(value, "\n")) != NULL)
				*q = '\0';
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] ={0};
	char tmp[128] ={0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(tmp, "apstats -r -i wifi%d | grep 'Rx Data Bytes'", getWiFiRadioUciNum(p));
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
			if((q = strstr(value, "\n")) != NULL)
				*q = '\0';
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] ={0};
	char tmp[128] ={0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(tmp, "apstats -r -i wifi%d | grep 'Tx Data Packets'", getWiFiRadioUciNum(p));
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
			if((q = strstr(value, "\n")) != NULL)
				*q = '\0';
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] ={0};
	char tmp[128] ={0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(tmp, "apstats -r -i wifi%d | grep 'Rx Data Packets'", getWiFiRadioUciNum(p));
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
			if((q = strstr(value, "\n")) != NULL)
				*q = '\0';
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] ={0};
	char tmp[128] ={0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(tmp, "ifconfig wifi%d | grep 'TX packets'", getWiFiRadioUciNum(p));
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "dropped")) != NULL)
			{
				*q = '\0';
				if((q = strstr(buff, "errors")) != NULL)
				{
					strcpy(value, q+strlen("errors:"));
					if ((q = strstr(value, " ")) != NULL)
						*q = '\0';
				}
			}
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] ={0};
	char tmp[128] ={0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(tmp, "apstats -r -i wifi%d | grep 'Rx errors'", getWiFiRadioUciNum(p));
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
			if((q = strstr(value, "\n")) != NULL)
				*q = '\0';
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] ={0};
	char tmp[128] ={0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(tmp, "apstats -r -i wifi%d | grep 'Tx Dropped'", getWiFiRadioUciNum(p));
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
			if((q = strstr(value, "\n")) != NULL)
				*q = '\0';
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] ={0};
	char tmp[128] ={0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(tmp, "ifconfig wifi%d | grep 'RX packets'", getWiFiRadioUciNum(p));
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "overruns")) != NULL)
			{
				*q = '\0';
				if((q = strstr(buff, "dropped")) != NULL)
				{
					strcpy(value, q+strlen("dropped:"));
					if ((q = strstr(value, " ")) != NULL)
						*q = '\0';
				}
			}
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_PLCPErrorCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRtS_PLCPErrorCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_FCSErrorCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRtS_FCSErrorCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_InvalidMACCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_InvalidMACCount, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] ={0};
	char tmp[128] ={0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(tmp, "apstats -r -i wifi%d | grep 'Rx PHY errors'", getWiFiRadioUciNum(p));
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
			if((q = strstr(value, "\n")) != NULL)
				*q = '\0';
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_PacketsOtherReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWRtS_PacketsOtherReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWRtS_Noise(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWRtS_Noise, value);
	if(ret)
	{
		return -1;
	}*/
	FILE *fd = NULL;
	char cmd[128] = {0};
	char line[128] = {0};
	char *ptr = NULL;
	char *p = parseTemplate(path_name, ".Radio.");

	if (p == NULL)
		return -1;

	sprintf(cmd, "iwconfig ath%d | grep Noise", getWiFiRadioUciNum(p));
	if ((fd = popen(cmd, "r")) != NULL){
		if (fgets(line,sizeof(line)-1,fd)){
			if ((ptr = strrchr(line, '=')) != NULL){
				sscanf(ptr+1, "%s %*s", value);
			}
		}
		pclose(fd);
	}

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWN_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWN_DiagnosticsState, value);
	if(ret)
	{
		strcpy(value, "None");
		ret = 0;
	}
	if (strcmp(value, " ") == 0)
		strcpy(value, "None");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWN_DiagnosticsState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWN_DiagnosticsState, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/

		
	if(strcasecmp(value, "requested") == 0)
	{
		system("iwlist ath0 scanning > /tmp/ath0_scan_result");
		system("iwlist ath1 scanning > /tmp/ath1_scan_result");
	}
	else
	{
		return (-1);
	}

	ret = do_uci_set(DWN_DiagnosticsState, "Complete");
	if(ret)
	{
		return (-1);
	}
	else
	{
		sentEventforDiagnostic();
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWN_ResultNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWN_ResultNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	int i = 0;
	FILE *fp = NULL;
	char buff[256] = {0};
	
	fp = fopen("/tmp/ath0_scan_result", "r");

	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp))
		{
			if(strstr(buff, "Cell") != NULL && strstr(buff, "- Address:") != NULL)
			{
				i++;
			}
		}
		fclose(fp);
	}

	fp = fopen("/tmp/ath1_scan_result", "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp))
		{
			if(strstr(buff, "Cell") != NULL && strstr(buff, "- Address:") != NULL)
			{
				i++;
			}
		}
		fclose(fp);
	}
	sprintf(value, "%d", i);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_Radio(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_Radio, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "Radio", value);
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_SSID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_SSID, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "SSID", value);
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_BSSID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_BSSID, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		strcpy(value, BSSID);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_Mode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_Mode, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "Mode", value);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_Channel(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_Channel, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "Channel", value);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_SignalStrength(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_SignalStrength, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "SignalStrength", value);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_SecurityModeEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_SecurityModeEnabled, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "SecurityModeEnabled", value);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_EncryptionMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_EncryptionMode, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};
	char *p = NULL;

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "EncryptionMode", value);		
		p = strchr(value, '\n');
		if(p != NULL)
		{
			*p = '\0';
		}
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_OperatingFrequencyBand(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_OperatingFrequencyBand, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "OperatingFrequencyBand", value);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_SupportedStandards(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_SupportedStandards, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "SupportedStandards", value);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_OperatingStandards(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_OperatingStandards, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "OperatingStandards", value);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_OperatingChannelBandwidth(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_OperatingChannelBandwidth, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "OperatingChannelBandwidth", value);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_BeaconPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_BeaconPeriod, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};
	char *p = NULL;

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "BeaconPeriod", value);	
		p = strchr(value, '\n');
		if(p != NULL)
		{
			*p = '\0';
		}
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_Noise(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_Noise, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};

	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "Noise", value);		
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_BasicDataTransferRates(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_BasicDataTransferRates, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};
	char BitRates[256] = {0};
	char temp[256] = {0};
	char *p = NULL;
	char *q = NULL;
	int i = 0;
	int j = 0;
	int n = 0;
	
	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "BasicDataTransferRates", BitRates);
		while((p = strchr(BitRates, '\n')) != NULL)
		{
			*p = ';';
		}
		n = strlen(BitRates);
		for(i = 0; i < n && BitRates[i] != '\0'; i++)
		{
			if(!isblank(BitRates[i]))
			{
				temp[j++] = BitRates[i];
			}
		}
		p = strchr(temp, ':');
		if(p != NULL)
		{
			strcpy(BitRates, p+1);
		}

		p = BitRates;
		memset(temp, 0, sizeof(temp));
		while((q = strstr(p, "Mb")) != NULL)
		{
			*q = '\0';
			strcat(temp, p);
			strcat(temp, ",");
			p = q + strlen("Mb/s;");
		}
		q = strrchr(temp, ',');
		if(q != NULL)
		{
			*q = '\0';
		}
		strcpy(value, temp);
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_SupportedDataTransferRates(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWNRt_SupportedDataTransferRates, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".Result.");
	char BSSID[256] = {0};
	char BitRates[256] = {0};
	char temp[256] = {0};
	char *p = NULL;
	char *q = NULL;
	int i = 0;
	int j = 0;
	int n = 0;
	
	if (index != NULL)
	{
		ret = lib_getvalue_mapfile_byinstance(WiFiNeighboringWiFiDiagnosticMap, BSSID, atoi(index));
		if(ret)
		{
			return (-1);
		}
		tr_log(LOG_DEBUG,"BSSID[%s]", BSSID);
		get_NeighboringWiFi_info(BSSID, "SupportedDataTransferRates", BitRates);
		while((p = strchr(BitRates, '\n')) != NULL)
		{
			*p = ';';
		}
		n = strlen(BitRates);
		for(i = 0; i < n && BitRates[i] != '\0'; i++)
		{
			if(!isblank(BitRates[i]))
			{
				temp[j++] = BitRates[i];
			}
		}
		p = strchr(temp, ':');
		if(p != NULL)
		{
			strcpy(BitRates, p+1);
		}

		p = BitRates;
		memset(temp, 0, sizeof(temp));
		while((q = strstr(p, "Mb")) != NULL)
		{
			*q = '\0';
			strcat(temp, p);
			strcat(temp, ",");
			p = q + strlen("Mb/s;");
		}
		q = strrchr(temp, ',');
		if(q != NULL)
		{
			*q = '\0';
		}
		strcpy(value, temp);
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWNRt_DTIMPeriod(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWNRt_DTIMPeriod, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWSt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWSt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "ath_enable");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWSt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWSt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "ath_enable");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWSt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWSt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "ath_enable");
	
	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		strcpy(tmp, "0");
		ret = 0;
	}

	if(atoi(tmp) == 1)
	{
		strcpy(value, "Up");
	}
	else
	{
		strcpy(value, "Down");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWSt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWSt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWSt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DWSt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWSt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWSt_Name, value);
	if(ret)
	{
		return -1;
	}*/

	/*char buff[128] = {0};
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "ssid");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWSt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWSt_LastChange, value);
	if(ret)
	{
		return -1;
	}*/
	getInterfaceLastChangeTime(value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWSt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWSt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;
	
	ret = getWiFiLowerLayersPath(p, value);
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWSt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWSt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't do anything
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWSt_BSSID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWSt_BSSID, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(buff, "/sys/class/net/%s/address", inf);

	fp = fopen(buff, "r");	
	if(fp != NULL)
	{
		fgets(tmp, sizeof(tmp), fp);
		p = strchr(tmp, '\n');
		if(p != NULL)
		{
			*p = '\0';
		}
		strcpy(value, tmp);
		fclose(fp);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWSt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWSt_MACAddress, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char inf[32] = {0};
	char tmp[128] = {0};
	FILE *fp = NULL;
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;
	
	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(buff, "/sys/class/net/%s/address", inf);
		
	fp = fopen(buff, "r");	
	if(fp != NULL)
	{
		fgets(tmp, sizeof(tmp), fp);
		p = strchr(tmp, '\n');
		if(p != NULL)
		{
			*p = '\0';
		}
		strcpy(value, tmp);
		fclose(fp);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWSt_SSID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWSt_SSID, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "ssid");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "");
		ret = 0;
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWSt_SSID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWSt_SSID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	if (strlen(value) > 32)
		return -2;

	getSSIDuciConfig(p, buff, "ssid");
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWStS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_BytesSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Tx Data Bytes", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Rx Data Bytes", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Tx Data Packets", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Rx Data Packets", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "ifconfig %s | grep 'TX packets'", inf);

	tr_log(LOG_DEBUG,"################################################tmp[%s]",tmp);
	
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		if(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if((q = strstr(buff, "dropped")) != NULL)
			{
				*q = '\0';
				if((q = strstr(buff, "errors")) != NULL)
				{
					strcpy(value, q+strlen("errors:"));
					if ((q = strstr(value, " ")) != NULL)
						*q = '\0';
				}
			}
		}
		else
			strcpy(value, "0");
		pclose(fp);
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_RetransCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWStS_RetransCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_FailedRetransCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWStS_FailedRetransCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_RetryCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWStS_RetryCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_MultipleRetryCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWStS_MultipleRetryCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_ACKFailureCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWStS_ACKFailureCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_AggregatedPacketCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWStS_AggregatedPacketCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Rx errors", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Tx Unicast Data Packets", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Rx Unicast Data Packets", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Tx Dropped", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Rx Dropped", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Tx multicast Data Packets", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Rx multicast Data Packets", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Tx Broadcast Data Packets", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".SSID.");

	if (p == NULL)
		return -1;

	getSSIStats(p, "Rx Broadcast Data Packets", value);

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWStS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWStS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "ath_enable");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "ath_enable");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}		
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "ath_enable");
	
	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		strcpy(tmp, "0");
		ret = 0;
	}

	if(atoi(tmp) == 1)
	{
		strcpy(value, "Enabled");
	}
	else
	{
		strcpy(value, "Disabled");
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //get from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DWAt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_SSIDReference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_SSIDReference, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	sprintf(value, "Device.WiFi.SSID.%s", p);
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_SSIDReference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAt_SSIDReference, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_SSIDAdvertisementEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_SSIDAdvertisementEnabled, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "hidden");
	
	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}

	if(atoi(tmp) == 1)
	{
		strcpy(value, "0");
	}
	else if(atoi(tmp) == 0)
	{
		strcpy(value, "1");
	}
		
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_SSIDAdvertisementEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAt_SSIDAdvertisementEnabled, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "hidden");

	if(atoi(value) == 1)
	{
		ret = do_uci_set(buff, "0");
	}
	else if(atoi(value) == 0)
	{
		ret = do_uci_set(buff, "1");
	}
	
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_RetryLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_RetryLimit, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_RetryLimit(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAt_RetryLimit, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_WMMCapability(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_WMMCapability, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wmm");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAt_WMMCapability_1751(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAt_WMMCapability_1751, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAt_UAPSDCapability(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_UAPSDCapability, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "uapsd");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAt_WMMEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_WMMEnable, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wmm");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_WMMEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAt_WMMEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wmm");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_UAPSDEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_UAPSDEnable, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "uapsd");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_UAPSDEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAt_UAPSDEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "uapsd");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_AssociatedDeviceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_AssociatedDeviceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[1024] = {0};
	char inf[32] = {0};
	char tmp[128] = {0};
	FILE *fp = NULL;
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "wlanconfig %s list sta", inf);
	
	tr_log(LOG_DEBUG,"######################################tmp [%s]",tmp);

	int i = 0;
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		//root@puma:/sys/class/net# wlanconfig ath1 list sta
		//ADDR               AID CHAN TXRATE RXRATE RSSI IDLE  TXSEQ  RXSEQ  CAPS        ACAPS     ERP    STATE MAXRATE(DOT11) HTCAPS ASSOCTIME    IEs   MODE PSMODE
		//7c:01:91:2f:11:e7    1    1 129M     29M   36    0      0   65535  EPSs         0          f              0             AP 00:00:52 RSN WME IEEE80211_MODE_11NG_HT20  0 
		//44:6d:57:fe:97:87    2    1  64M     26M   35 4320      0   65535  EPSs         0          f              0            WPS 00:00:10 RSN WME IEEE80211_MODE_11NG_HT20  0 

		//root@puma:/sys/class/net# wlanconfig ath13 list sta
		//wlanconfig: unable to get station information
		while(fgets(buff, sizeof(buff), fp))
		{
			i++;
		}
		pclose(fp);
		tr_log(LOG_DEBUG,"######################################associated device number [%d]",i);
		if(i != 0)
		{
			sprintf(value, "%d", i-1); //MUST i-1, to ingor first line
		}
		else
		{
			strcpy(value, "0");
		}
	}

	//update node
	node_t node;
	char path_name_change[128] = {0};
	sprintf(path_name_change,"%s.AssociatedDevice.",path_name);
	ret = lib_resolve_node( path_name_change, &node );
	tr_log(LOG_DEBUG,"ret[%d]",ret);
	if (ret == 0){
		lib_dynamic_init_children(node);
	}
	ret = lib_resolve_node( "Device.Hosts.Host.", &node );
	tr_log(LOG_DEBUG,"ret[%d]",ret);
	if (ret == 0){
		lib_dynamic_init_children(node);
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAt_MaxAssociatedDevices(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_MaxAssociatedDevices, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "maxsta");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_MaxAssociatedDevices(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAt_MaxAssociatedDevices, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};	
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	if(!(atoi(value) >=1 && atoi(value) <= 100))
	{
		return -2;
	}
	
	getSSIDuciConfig(p, buff, "maxsta");
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_IsolationEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAt_IsolationEnable, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "isolate");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_IsolationEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAt_IsolationEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "isolate");
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_MACAddressControlEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "aclenable");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_MACAddressControlEnabled(char * path_name, char *value)
{
	int ret = 0;	
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "macfilter"); 
	if(do_uci_set(buff, "deny")) // default is allow white list, set to deny, or may won't work as expect
	{
		return (-1);
	}
	
	getSSIDuciConfig(p, buff, "aclenable");
	if(do_uci_set(buff, value))
	{
		return (-1);
	}

	if(do_uci_commit("wireless"))
	{
		return (-1);
	}
	//ret = 1; //means need to reboot for taking effect
	doWifiReload(atoi(p));

	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAt_AllowedMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char buff[128] = {0};
	char *q = NULL;
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "maclist");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "");
		ret = 0; //MUST
	}

	while((q = strchr(value, ' ')) != NULL)
	{
		*q = ',';
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAt_AllowedMACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char buff[128] = {0};
	char *q = NULL;
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	if (strlen(value) > 1024)
		return -2;
	
	while((q = strchr(value, ',')) != NULL)
	{
		*q = ' ';
	}
	getSSIDuciConfig(p, buff, "maclist");
	if(do_uci_set(buff, value))
	{
		return (-1);
	}

	getSSIDuciConfig(p, buff, "macfilter");
	if(do_uci_set(buff, "deny"))
	{
		return (-1);
	}
	
	if(do_uci_commit("wireless"))
	{
		return (-1);
	}
	//ret = 1; //means need to reboot for taking effect
	doWifiReload(atoi(p));

	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_Reset, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char str[128] = {0};
	char buff[128] = {0};

	if (p == NULL)
		return -1;

	if (atoi(value) != 1)
		return -1;

	if(atoi(p) == WIFI5G_START_INSTANCE_NUM){
		memset(str, 0, sizeof(str));
		getMfcInfo("WLAN5GPassword", str);
		if (str[0] == '\0')
			return (-1);
		
		ret = do_uci_set("wireless.wla.encryption", "psk2+ccmp"); //default value
		if(ret)
		{
			return (-1);
		}
		
		ret = do_uci_set("wireless.wla.key", str); //default value
		if(ret)
		{
			return (-1);
		}

		ret = do_uci_set("wireless.wla.wpapsk", str); //default value
		if(ret)
		{
			return (-1);
		}
	}
	else if(WIFI5G_START_INSTANCE_NUM < atoi(p) <= WIFI5G_END_INSTANCE_NUM)
	{
		memset(buff, 0, sizeof(buff));
		getSSIDuciConfig(p, buff, "ath_enable");
		ret = do_uci_set(buff, "0"); //default value
		if(ret)
		{
			return (-1);
		}
	}
	else if(atoi(p) == WIFI24G_START_INSTANCE_NUM)
	{
		memset(str, 0, sizeof(str));
		getMfcInfo("WLAN2GPassword", str);
		if (str[0] == '\0')
			return (-1);
		
		ret = do_uci_set("wireless.wlg.encryption", "psk2+ccmp"); //default value
		if(ret)
		{
			return (-1);
		}
		
		ret = do_uci_set("wireless.wlg.key", str); //default value
		if(ret)
		{
			return (-1);
		}

		ret = do_uci_set("wireless.wlg.wpapsk", str); //default value
		if(ret)
		{
			return (-1);
		}
	}
	else if(WIFI24G_START_INSTANCE_NUM < atoi(p) <= WIFI24G_END_INSTANCE_NUM)
	{
		memset(buff, 0, sizeof(buff));
		getSSIDuciConfig(p, buff, "ath_enable");
		ret = do_uci_set(buff, "0"); //default value
		if(ret)
		{
			return (-1);
		}
	}

	ret = do_uci_commit("wireless");
	if(ret)
	{
		return (-1);
	}

	//ret = 1; //means need to reboot for taking effect
	doWifiReload(atoi(p));

	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_ModesSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_ModesSupported, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char tmp[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	sprintf(value, "None,WEP-64,WEP-128,WPA-Personal,WPA2-Personal,WPA-WPA2-Personal,WPA-Enterprise,WPA2-Enterprise,WPA-WPA2-Enterprise,SAE");	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtS_ModeEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_ModeEnabled, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char buff1[128] = {0};
	char tmp[128] = {0};
	char tmp1[128] = {0};
	int index = -1;		
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "encryption");
	getSSIDuciConfig(p, buff1, "wepkeyid");
	
	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}	

	if(strstr(tmp, "wep") != NULL)
	{
		ret = do_uci_get(buff1, tmp1);
		if(ret)
		{
			return -1;
		}
		index = atoi(tmp1);
		switch(index)	
		{
			case 1:
				getSSIDuciConfig(p, buff, "key1");
				ret = do_uci_get(buff, tmp1);
				break;
			case 2:
				getSSIDuciConfig(p, buff, "key2");
				ret = do_uci_get(buff, tmp1);
				break;
			case 3:
				getSSIDuciConfig(p, buff, "key3");
				ret = do_uci_get(buff, tmp1);
				break;
			case 4:
				getSSIDuciConfig(p, buff, "key4");
				ret = do_uci_get(buff, tmp1);
				break;					
		}
		if(ret)
		{
			return -1;
		}

		if(strstr(tmp1, "s:") != NULL)
		{
			char tmp2[128] = {0};

			strcpy(tmp2, tmp1+2);
			strcpy(tmp1, tmp2);
			tr_log(LOG_DEBUG,"tmp2[%s]",tmp2);
		}

		if(strlen(tmp1) == 5 || strlen(tmp1) == 10)
		{
			strcpy(value, "WEP-64");
		}
		else if(strlen(tmp1) == 13 || strlen(tmp1) == 26)
		{
			strcpy(value, "WEP-128");			
		}
	}
	else if(!strcmp(tmp, "psk+tkip"))
	{
		strcpy(value, "WPA-Personal");			
	}
	else if(!strcmp(tmp, "psk2+ccmp"))
	{
		strcpy(value, "WPA2-Personal");			
	}
	else if(!strcmp(tmp, "psk-mixed+tkip+ccmp"))
	{
		strcpy(value, "WPA-WPA2-Personal");			
	}	
	else if(!strcmp(tmp, "wpa+tkip"))
	{
		strcpy(value, "WPA-Enterprise");			
	}
	else if(!strcmp(tmp, "wpa2+ccmp"))
	{
		strcpy(value, "WPA2-Enterprise");			
	}
	else if(!strcmp(tmp, "wpa-mixed+tkip+ccmp"))
	{
		strcpy(value, "WPA-WPA2-Enterprise");			
	}
	else if(!strcmp(tmp, "none"))
	{
		strcpy(value, "None");
	}
	else if(!strcmp(tmp, "sae"))
	{
		strcpy(value, "SAE");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_ModeEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_ModeEnabled, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char buff1[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	getSSIDuciConfig(p, buff, "encryption");
	getSSIDuciConfig(p, buff1, "wps_enable");

	if(!strcasecmp(value, "wep-64") || !strcasecmp(value, "wep-128"))
	{
		ret = do_uci_set(buff, "wep+mixed");
		ret = do_uci_set(buff1, "0");
	}
	else if(!strcasecmp(value, "wpa-personal"))
	{
		ret = do_uci_set(buff, "psk+tkip");
	}
	else if(!strcasecmp(value, "wpa2-personal"))
	{
		ret = do_uci_set(buff, "psk2+ccmp");
	}
	else if(!strcasecmp(value, "wpa-wpa2-personal"))
	{
		ret = do_uci_set(buff, "psk-mixed+tkip+ccmp");
	}
	else if(!strcasecmp(value, "wpa-enterprise"))
	{
		ret = do_uci_set(buff, "wpa+tkip");
	}
	else if(!strcasecmp(value, "wpa2-enterprise"))
	{
		ret = do_uci_set(buff, "wpa2+ccmp");
	}
	else if(!strcasecmp(value, "wpa-wpa2-enterprise"))
	{
		ret = do_uci_set(buff, "wpa-mixed+tkip+ccmp");
	}
	else if(!strcasecmp(value, "none"))
	{
		ret = do_uci_set(buff, "none");
	}
	else if(!strcasecmp(value, "sae"))
	{
		ret = do_uci_set(buff, "sae");
	}
	else
		return (-2);
	
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_WEPKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_WEPKey, value);
	if(ret)
	{
		return -1;
	}*/
#if 0
	char *p = NULL;
	char buff[128] = {0};
	char buff1[128] = {0};
	char tmp[128] = {0};
	char tmp1[128] = {0};
	int index = -1;		
	p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wepkeyid");
	
	ret = do_uci_get(buff, tmp);
	if(ret)
	{
		return -1;
	}
	index = atoi(tmp);
	switch(index)
	{
		case 1:
			getSSIDuciConfig(p, buff1, "key1");
			ret = do_uci_get(buff1, tmp1);
			break;
		case 2:
			getSSIDuciConfig(p, buff1, "key2");
			ret = do_uci_get(buff1, tmp1);
			break;
		case 3:
			getSSIDuciConfig(p, buff1, "key3");
			ret = do_uci_get(buff1, tmp1);
			break;
		case 4:
			getSSIDuciConfig(p, buff1, "key4");
			ret = do_uci_get(buff1, tmp1);
			break;
	}
	if(ret)
	{
		return -1;
	}
	strcpy(value, tmp1);
#else
	strcpy(value, ""); //When read, this parameter returns an empty string
#endif
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_WEPKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_WEPKey, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char buff1[128] = {0};
	char buff2[128] = {0};
	char svalue[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	if (strlen(value) != 5 && strlen(value) != 13 && strlen(value) != 10 && strlen(value) != 26)
	{
		return -2;
	}

	if (strlen(value) == 10 || strlen(value) == 26)
	{
		int i;
		char *ptr = value;
		
		for (i=0; i<strlen(value); i++) 
		{
			if (((*ptr>='0') && (*ptr<='9')) || ((*ptr>='a') && (*ptr<='f')) || ((*ptr>='A') && (*ptr<='F')))
			{
				ptr++;
			}
			else
			{
				return -2;
			}
		}		
	}
	
	getSSIDuciConfig(p, buff, "wepkeyid");
	getSSIDuciConfig(p, buff1, "key");
	getSSIDuciConfig(p, buff2, "key1");

	if (strlen(value) == 5 || strlen(value) == 13)
	{
		sprintf(svalue, "s:%s", value);
	}
	else
	{
		sprintf(svalue, "%s", value);
	}

	ret = do_uci_set(buff, "1");
	ret = do_uci_set(buff1, "1");
	ret = do_uci_set(buff2, svalue);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_PreSharedKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_PreSharedKey, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //When read, this parameter returns an empty string
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_PreSharedKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_PreSharedKey, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_KeyPassphrase(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_KeyPassphrase, value);
	if(ret)
	{
		return -1;
	}*/
#if 0
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wpapsk");
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}
#else
	strcpy(value, ""); //When read, this parameter returns an empty string
#endif
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_KeyPassphrase(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_KeyPassphrase, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char buff1[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	if(strlen(value) < 8 || strlen(value) > 63)
	{
		return -2;
	}
	
	getSSIDuciConfig(p, buff, "wpapsk");
	getSSIDuciConfig(p, buff1, "key");
	ret = do_uci_set(buff, value);
	ret = do_uci_set(buff1, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_RekeyingInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_RekeyingInterval, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wpa_group_rekey");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_RekeyingInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_RekeyingInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	if (atoi(value) < 0 || atoi(value) > 86400)
		return -2;
	
	getSSIDuciConfig(p, buff, "wpa_group_rekey");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}			
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_RadiusServerIPAddr(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_RadiusServerIPAddr, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "auth_server");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "");
		ret = 0;
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_RadiusServerIPAddr(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_RadiusServerIPAddr, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	if (strlen(value) > 45)
		return -2;

	if (isValidIP(value) == 0)
		return -2;
	
	getSSIDuciConfig(p, buff, "auth_server");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}			
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_SecondaryRadiusServerIPAddr(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_SecondaryRadiusServerIPAddr, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //always null, only support one radius server
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_SecondaryRadiusServerIPAddr(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_SecondaryRadiusServerIPAddr, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't support
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_RadiusServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_RadiusServerPort, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "auth_port");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_RadiusServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_RadiusServerPort, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	if (atoi(value) < 1 || atoi(value) > 65535)
		return -2;
	
	getSSIDuciConfig(p, buff, "auth_port");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}			
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_SecondaryRadiusServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_SecondaryRadiusServerPort, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //always null, only support one radius server
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_SecondaryRadiusServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_SecondaryRadiusServerPort, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't support
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_RadiusSecret(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_RadiusSecret, value);
	if(ret)
	{
		return -1;
	}*/

#if 0
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "auth_secret");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "");
		ret = 0; //MUST
	}
#else
	strcpy(value, ""); //When read, this parameter returns an empty string
#endif
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_RadiusSecret(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_RadiusSecret, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "auth_secret");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}			
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_SecondaryRadiusSecret(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_SecondaryRadiusSecret, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //always null, only support one radius server, When read, this parameter returns an empty string
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_SecondaryRadiusSecret(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_SecondaryRadiusSecret, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't support
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_MFPConfig(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0}, buff2[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "ieee80211w");
	
	ret = do_uci_get(buff, buff2);
	if(ret)
	{
		return -1;
	}
	if(!strcmp(buff2, "1")){
		strcpy(value, "Optional");
	}
	else if(!strcmp(buff2, "2")){
		strcpy(value, "Required");
	}
	else{
		strcpy(value, "Disabled");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_MFPConfig(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_MFPConfig, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0}, buff2[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "ieee80211w");

	if(!strcmp(value, "Optional")){
		strcpy(buff2, "1");
	}
	else if(!strcmp(value, "Required")){
		strcpy(buff2, "2");
	}
	else if(!strcmp(value, "Disabled")){
		strcpy(buff2, "0");
	}
	else
		return -2;
	
	ret = do_uci_set(buff, buff2);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_X_TWC_COM_PreSharedKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char value2[256] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wpapsk");
	
	ret = do_uci_get(buff, value2);
	if(ret)
	{
		strcpy(value, "");
		ret = 0;
	}
	strToHex(value2, value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_X_TWC_COM_PreSharedKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_MFPConfig, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0}, buff2[128] = {0};
	char value_hex_to_ch[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	if(strlen(value) > 32)
	{
		return -2;
	}
	
	getSSIDuciConfig(p, buff, "key");
	getSSIDuciConfig(p, buff2, "wpapsk");
	hexToStr(value, value_hex_to_ch);
	ret = do_uci_set(buff, value_hex_to_ch);
	ret = do_uci_set(buff2, value_hex_to_ch);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_X_TWC_COM_PreSharedKeySHA1(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char value2[256] = {0}, value_ready_to_SHA1[256] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "key");
	
	ret = do_uci_get(buff, value2);
	if(ret)
	{
		strcpy(value, "");
		ret = 0;
	}
	strToHex(value2, value_ready_to_SHA1);
	sha1_encode(value_ready_to_SHA1, value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtS_X_TWC_COM_KeyPassPhrase(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wpapsk");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtS_X_TWC_COM_KeyPassPhrase(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_MFPConfig, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0}, buff2[128] = {0};	
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;

	if(strlen(value) < 8 || strlen(value) > 63)
	{
		return -2;
	}
	
	getSSIDuciConfig(p, buff, "key");
	getSSIDuciConfig(p, buff2, "wpapsk");
	ret = do_uci_set(buff, value);
	ret = do_uci_set(buff2, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doWifiReload(atoi(p));
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtS_X_TWC_COM_KeyPassPhraseSHA1(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char value2[256] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "key");
	
	ret = do_uci_get(buff, value2);
	if(ret)
	{
		strcpy(value, "");
		ret = 0;
	}
	sha1_encode(value2, value);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtA_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtA_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtA_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtA_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtA_ServerIPAddr(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtA_ServerIPAddr, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtA_ServerIPAddr(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtA_ServerIPAddr, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtA_SecondaryServerIPAddr(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtA_SecondaryServerIPAddr, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtA_SecondaryServerIPAddr(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtA_SecondaryServerIPAddr, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtA_ServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtA_ServerPort, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtA_ServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtA_ServerPort, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtA_SecondaryServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtA_SecondaryServerPort, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtA_SecondaryServerPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtA_SecondaryServerPort, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtA_Secret(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtA_Secret, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtA_Secret(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtA_Secret, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtA_SecondarySecret(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtA_SecondarySecret, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtA_SecondarySecret(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtA_SecondarySecret, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtA_InterimInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtA_InterimInterval, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtA_InterimInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtA_InterimInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtW_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtW_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wps_enable");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtW_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtW_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wps_enable");
	
	ret = do_uci_set(buff, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		if(atoi(p) < WIFI5G_RADIO_INSTANCE_NUM)
		{
			system("hostapd_cli -i ath1 -p /var/run/hostapd-wifi1 wps_pbc");
		}
		else
		{
			system("hostapd_cli -i ath0 -p /var/run/hostapd-wifi0 wps_pbc");
		}
	}			
	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtW_ConfigMethodsSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtW_ConfigMethodsSupported, value);
	if(ret)
	{
		return -1;
	}*/

	strcpy(value, "PushButton,PIN");	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtW_ConfigMethodsSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtW_ConfigMethodsSupported, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtW_ConfigMethodsEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtW_ConfigMethodsEnabled, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "PushButton");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtW_ConfigMethodsEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtW_ConfigMethodsEnabled, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtW_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0}, buff2[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wps_state");
	
	ret = do_uci_get(buff, buff2);
	if(ret)
	{
		return -1;
	}
	if(!strcmp(buff2, "1")){
		strcpy(value, "Unconfigured");
	}
	else if(!strcmp(buff2, "2")){
		strcpy(value, "Configured");
	}
	else{
		strcpy(value, "Disabled");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtW_Version(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "2.0"); 
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtW_PIN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_PIN, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AccessPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig(p, buff, "wps_pin");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_MACAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_MACAddress, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char *q = NULL;
	
	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
			strcpy(value, mac);
		}
	}
	else
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_OperatingStandard(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_MACAddress, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};

	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			if (strstr(mac, "IEEE80211_MODE_11B") != NULL)
				strcpy(value, "b");
			else if (strstr(mac, "IEEE80211_MODE_11G") != NULL)
				strcpy(value, "g");
			else if (strstr(mac, "IEEE80211_MODE_11NA_HT20") != NULL)
				strcpy(value, "n");
			else if (strstr(mac, "IEEE80211_MODE_11NA_HT40PLUS") != NULL)
				strcpy(value, "n");
			else if (strstr(mac, "IEEE80211_MODE_11NA_HT40MINUS") != NULL)
				strcpy(value, "n");
			else if (strstr(mac, "IEEE80211_MODE_11NA_HT40") != NULL)
				strcpy(value, "n");
			else if (strstr(mac, "IEEE80211_MODE_11NG_HT20") != NULL)
				strcpy(value, "n");
			else if (strstr(mac, "IEEE80211_MODE_11NG_HT40PLUS") != NULL)
				strcpy(value, "n");
			else if (strstr(mac, "IEEE80211_MODE_11NG_HT40MINUS") != NULL)
				strcpy(value, "n");
			else if (strstr(mac, "IEEE80211_MODE_11NG_HT40") != NULL)
				strcpy(value, "n");
			else if (strstr(mac, "IEEE80211_MODE_11AC_VHT20") != NULL)
				strcpy(value, "ac");
			else if (strstr(mac, "IEEE80211_MODE_11AC_VHT40PLUS") != NULL)
				strcpy(value, "ac");
			else if (strstr(mac, "IEEE80211_MODE_11AC_VHT40MINUS") != NULL)
				strcpy(value, "ac");
			else if (strstr(mac, "IEEE80211_MODE_11AC_VHT40") != NULL)
				strcpy(value, "ac");
			else if (strstr(mac, "IEEE80211_MODE_11AC_VHT80") != NULL)
				strcpy(value, "ac");
			else if (strstr(mac, "IEEE80211_MODE_11AC_VHT160") != NULL)
				strcpy(value, "ac");
			else if (strstr(mac, "IEEE80211_MODE_11AC_VHT80_80") != NULL)
				strcpy(value, "ac");
			else if (strstr(mac, "IEEE80211_MODE_11A") != NULL)
				strcpy(value, "a");
			else
				return -1;
				
		}
	}
	else
	{
		return (-1);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_AuthenticationState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_AuthenticationState, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char buff[128] ={0};
	char tmp[128] ={0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;
	int found = 0;
	
	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
		}
	}
	else
	{
		return (-1);
	}
	
	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "apstats -s -i %s -m %s", inf, mac);
	
	tr_log(LOG_DEBUG,"################################################tmp[%s]",tmp);
	
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp))
		{
			if(strstr(buff, mac) != NULL)
			{
				found = 1;
				break;
			}
		}
		pclose(fp);
	}

	if (found == 1)
	{
		char encry[32] = {0};
		memset(buff, 0, sizeof(buff));
		getSSIDuciConfig(p, buff, "encryption");

		ret = do_uci_get(buff, encry);
		if(ret)
		{
			strcpy(value, "0");
			ret = 0;
		}
		else
		{
			if (strcasecmp(encry, "none") != 0)
				strcpy(value, "1");
			else
				strcpy(value, "0");
		}
	}
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_LastDataDownlinkRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_LastDataDownlinkRate, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char buff[128] ={0};
	char tmp[128] ={0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;
	
	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
		}
	}
	else
	{
		return (-1);
	}

	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "apstats -s -i %s -m %s", inf, mac);

	tr_log(LOG_DEBUG,"################################################tmp[%s]",tmp);

	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if(strstr(buff, "Last tx rate") != NULL)
			{
				if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
				if ((q = strstr(value, "\n")) != NULL || (q = strstr(value, "\r")) != NULL)
					*q = '\0';
				break;
			}
		}
		pclose(fp);
	}

	if (strcmp(value, "") == 0)
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_LastDataUplinkRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_LastDataUplinkRate, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char buff[128] ={0};
	char tmp[128] ={0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
		}
	}
	else
	{
		return (-1);
	}
	
	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "apstats -s -i %s -m %s", inf, mac);
	
	tr_log(LOG_DEBUG,"################################################tmp[%s]",tmp);
	
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if(strstr(buff, "Last rx rate") != NULL)
			{
				if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
				if ((q = strstr(value, "\n")) != NULL || (q = strstr(value, "\r")) != NULL)
					*q = '\0';
				break;
			}
		}
		pclose(fp);
	}

	if (strcmp(value, "") == 0)
		strcpy(value, "0");

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_SignalStrength(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_SignalStrength, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char buff[128] ={0};
	char tmp[128] ={0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
		}
	}
	else
	{
		return (-1);
	}
	
	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "apstats -s -i %s -m %s", inf, mac);
	
	tr_log(LOG_DEBUG,"################################################tmp[%s]",tmp);
	
	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if(strstr(buff, "Rx RSSI") != NULL)
			{
				if((q = strstr(buff, "=")) != NULL)
					sprintf(value, "-%s", q+2);
				if ((q = strstr(value, "\n")) != NULL || (q = strstr(value, "\r")) != NULL)
					*q = '\0';
				break;
			}
		}
		pclose(fp);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_Retransmissions(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAt_Retransmissions, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_Active(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_Active, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char buff[1024] = {0};
	char tmp[128] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char idle[16] = {0};
	int found = 0;
	char *q = NULL;

	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
		}
	}
	else
	{
		return (-1);
	}
	
	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "wlanconfig %s list sta", inf);
	
	tr_log(LOG_DEBUG,"######################################tmp [%s]",tmp);

	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp))
		{
			if(strstr(buff, mac) != NULL)
			{
				/*sscanf(buff, "%*s %*s %*s %*s %*s %*s %s %*s", idle);
				tr_log(LOG_DEBUG,"######################################idle [%s]",idle);
				if(atoi(idle) == 0)
				{
					strcpy(value, "1");
				}
				else if(atoi(idle) == 1)
				{
					strcpy(value, "0");
				}*/
				found = 1;
				break;
			}
		}
		pclose(fp);
	}	

	if (found == 1)
		strcpy(value, "1");
	else
		strcpy(value, "0");
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char buff[128] ={0};
	char tmp[128] ={0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
		}
	}
	else
	{
		return (-1);
	}

	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "apstats -s -i %s -m %s", inf, mac);

	tr_log(LOG_DEBUG,"################################################tmp[%s]",tmp);

	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if(strstr(buff, "Tx Data Bytes") != NULL)
			{
				if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
				if ((q = strstr(value, "\n")) != NULL || (q = strstr(value, "\r")) != NULL)
					*q = '\0';
				break;
			}
		}
		pclose(fp);
	}

	if (strcmp(value, "") == 0)
		strcpy(value, "0");
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char buff[128] ={0};
	char tmp[128] ={0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
		}
	}
	else
	{
		return (-1);
	}

	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "apstats -s -i %s -m %s", inf, mac);

	tr_log(LOG_DEBUG,"################################################tmp[%s]",tmp);

	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if(strstr(buff, "Rx Data Bytes") != NULL)
			{
				if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
				if ((q = strstr(value, "\n")) != NULL || (q = strstr(value, "\r")) != NULL)
					*q = '\0';
				break;
			}
		}
		pclose(fp);
	}

	if (strcmp(value, "") == 0)
		strcpy(value, "0");
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char buff[128] ={0};
	char tmp[128] ={0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
		}
	}
	else
	{
		return (-1);
	}

	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "apstats -s -i %s -m %s", inf, mac);

	tr_log(LOG_DEBUG,"################################################tmp[%s]",tmp);

	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if(strstr(buff, "Tx Data Packets") != NULL)
			{
				if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
				if ((q = strstr(value, "\n")) != NULL || (q = strstr(value, "\r")) != NULL)
					*q = '\0';
				break;
			}
		}
		pclose(fp);
	}

	if (strcmp(value, "") == 0)
		strcpy(value, "0");
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char *index = parseTemplate(path_name, ".AssociatedDevice.");
	char *p = parseTemplate(path_name, ".AccessPoint.");
	char mac[128] = {0};
	char buff[128] ={0};
	char tmp[128] ={0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (p != NULL && index != NULL)
	{
		char filename[64] = {0};
		sprintf(filename, "AssociatedDevice%s.mapping", p);
		ret = lib_getvalue_mapfile_byinstance(filename, mac, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else{
			if ((q = strstr(mac, "|")) != NULL)
				*q = '\0';
		}
	}
	else
	{
		return (-1);
	}

	getWiFiInterfaceNameWithInstanceNum(p, inf);
	sprintf(tmp, "apstats -s -i %s -m %s", inf, mac);

	tr_log(LOG_DEBUG,"################################################tmp[%s]",tmp);

	fp = popen(tmp, "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp) != NULL)
		{
			if(strstr(buff, "Rx Data Packets") != NULL)
			{
				if((q = strstr(buff, "=")) != NULL)
					strcpy(value, q+2);
				if ((q = strstr(value, "\n")) != NULL || (q = strstr(value, "\r")) != NULL)
					*q = '\0';
				break;
			}
		}
		pclose(fp);
	}

	if (strcmp(value, "") == 0)
		strcpy(value, "0");

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_RetransCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_RetransCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_FailedRetransCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_FailedRetransCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_RetryCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_RetryCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_MultipleRetryCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_MultipleRetryCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_AccessCategory(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_AccessCategory, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".AC.");
	
	if (p == NULL)
		return -1;
	
	if(atoi(p) > 4)
	{
		return -1;
	}

	if(atoi(p) == 1)
	{
		strcpy(value, "BE");
	}
	else if(atoi(p) == 2)
	{
		strcpy(value, "BK");
	}
	else if(atoi(p) == 3)
	{
		strcpy(value, "VI");
	}
	else if(atoi(p) == 4)
	{
		strcpy(value, "VO");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtAt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DWAtAt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtAt_AIFSN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_AIFSN, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if ((p != NULL) && (index != NULL))
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s get_aifs %d 0", inf, atoi(p)-1);
		
		if((fp=popen(command,"r")) != NULL)
		{
			while(fgets(line,sizeof(line)-1,fp))
			{
				q = strchr(line, ':');
				if(q != NULL)
				{
					strcpy(value, q+1);
					if ((q = strstr(value, " ")) != NULL)
						*q = '\0';
					if ((q = strstr(value, "\n")) != NULL)
						*q = '\0';
				}
			}
	    	pclose(fp);
		}
	}	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtAt_AIFSN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtAt_AIFSN, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (atoi(value) < 2 || atoi(value) > 15)
		return -2;
	
	if (p != NULL && index != NULL)
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s aifs %d 0 %s", inf, atoi(p)-1, value);
		system(command);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtAt_ECWMin(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_ECWMin, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if ((p != NULL) && (index != NULL))
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s get_cwmin %d 0", inf, atoi(p)-1);
		
		if((fp=popen(command,"r")) != NULL)
		{
			while(fgets(line,sizeof(line)-1,fp))
			{
				q = strchr(line, ':');
				if(q != NULL)
				{
					strcpy(value, q+1);
					if ((q = strstr(value, " ")) != NULL)
						*q = '\0';
					if ((q = strstr(value, "\n")) != NULL)
						*q = '\0';
				}
			}
	    	pclose(fp);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtAt_ECWMin(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtAt_ECWMin, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (atoi(value) < 0 || atoi(value) > 15)
		return -2;
	
	if (p != NULL && index != NULL)
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s cwmin %d 0 %s", inf, atoi(p)-1, value);
		system(command);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtAt_ECWMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_ECWMax, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;
	
	if (p != NULL && index != NULL)
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s get_cwmax %d 0", inf, atoi(p)-1);

		if((fp=popen(command,"r")) != NULL)
		{
			while(fgets(line,sizeof(line)-1,fp))
			{
				q = strchr(line, ':');
				if(q != NULL)
				{
					strcpy(value, q+1);
					if ((q = strstr(value, " ")) != NULL)
						*q = '\0';
					if ((q = strstr(value, "\n")) != NULL)
						*q = '\0';
				}
			}
	    	pclose(fp);
		}		
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtAt_ECWMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtAt_ECWMax, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (atoi(value) < 0 || atoi(value) > 15)
		return -2;
	
	if (p != NULL && index != NULL)
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s cwmax %d 0 %s", inf, atoi(p)-1, value);
		system(command);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtAt_TxOpMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_TxOpMax, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;
	
	if (p != NULL && index != NULL)
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s get_txoplimit %d 0", inf, atoi(p)-1);

		if((fp=popen(command,"r")) != NULL)
		{
			while(fgets(line,sizeof(line)-1,fp))
			{
				q = strchr(line, ':');
				if(q != NULL)
				{
					strcpy(value, q+1);
					if ((q = strstr(value, " ")) != NULL)
						*q = '\0';
					if ((q = strstr(value, "\n")) != NULL)
						*q = '\0';
				}
			}
	    	pclose(fp);
		}		
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtAt_TxOpMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtAt_TxOpMax, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;

	if (atoi(value) < 0 || atoi(value) > 255)
		return -2;
	
	if (p != NULL && index != NULL)
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s txoplimit %d 0 %s", inf, atoi(p)-1, value);
		system(command);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtAt_AckPolicy(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtAt_AckPolicy, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;
	
	if (p != NULL && index != NULL)
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s get_noackpolicy %d 0", inf, atoi(p)-1);

		if((fp=popen(command,"r")) != NULL)
		{
			while(fgets(line,sizeof(line)-1,fp))
			{
				q = strchr(line, ':');
				if(q != NULL)
				{
					strcpy(value, q+1);
					if ((q = strstr(value, " ")) != NULL)
						*q = '\0';
					if ((q = strstr(value, "\n")) != NULL)
						*q = '\0';
				}
			}
	    	pclose(fp);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtAt_AckPolicy(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtAt_AckPolicy, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *p = parseTemplate(path_name, ".AC.");
	char *index = parseTemplate(path_name, ".AccessPoint.");
	char command[256] = {0};
	char line[256] = {0};
	char inf[32] = {0};
	FILE *fp = NULL;
	char *q = NULL;
	
	if (p != NULL && index != NULL)
	{
		getWiFiInterfaceNameWithInstanceNum(index, inf);
		sprintf(command, "iwpriv %s noackpolicy %d 0 %d", inf, atoi(p)-1, 1-atoi(value));
		system(command);
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtAt_OutQLenHistogramIntervals(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAt_OutQLenHistogramIntervals, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtAt_OutQLenHistogramIntervals(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtAt_OutQLenHistogramIntervals, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtAt_OutQLenHistogramSampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAt_OutQLenHistogramSampleInterval, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWAtAt_OutQLenHistogramSampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWAtAt_OutQLenHistogramSampleInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWAtAtS_BytesSent_1846(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_BytesSent_1846, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_BytesReceived_1847(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_BytesReceived_1847, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_PacketsSent_1848(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_PacketsSent_1848, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_PacketsReceived_1849(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_PacketsReceived_1849, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_ErrorsSent_1850(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_ErrorsSent_1850, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_RetransCount_1854(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_RetransCount_1854, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWAtAtS_OutQLenHistogram(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWAtAtS_OutQLenHistogram, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	int en1 = 0, en2 = 0;

	en1 = _get_endporint_5g_enable();
	en2 = _get_endporint_24g_enable();
	if (en1 == -1 && en2 == -1)
		return -1;
	else{
		if (en1 == 1 || en2 == 1)
			strcpy(value, "1");
		else
			strcpy(value, "0");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/

	if (atoi(value) == 1){
		//for 5G
		ret = do_uci_set("wireless.wla.mode", "sta");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("wireless.wla.extap", "1");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("wireless.wla.bridge_enable", "1");
		if(ret)
		{
			return (-1);
		}
		//for 2.4G
		ret = do_uci_set("wireless.wlg.mode", "sta");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("wireless.wlg.extap", "1");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("wireless.wlg.bridge_enable", "1");
		if(ret)
		{
			return (-1);
		}
		//for lan, default with dhcp mode
		ret = do_uci_set("network.lan.proto", "dhcp");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("network.lan.peerdns", "1");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_commit("network");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doSbinWifi();
	}
	else
	{
		//for 5G
		ret = do_uci_set("wireless.wla.mode", "ap");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("wireless.wla.extap", "");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("wireless.wla.bridge_enable", "");
		if(ret)
		{
			return (-1);
		}
		//for 2.4G
		ret = do_uci_set("wireless.wlg.mode", "ap");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("wireless.wlg.extap", "");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("wireless.wlg.bridge_enable", "");
		if(ret)
		{
			return (-1);
		}
		//for lan, default with dhcp mode
		ret = do_uci_set("network.lan.proto", "static");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_set("network.lan.peerdns", "");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		ret = do_uci_commit("network");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doSbinWifi();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	int en1 = 0, en2 = 0;

	en1 = _get_endporint_5g_enable();
	en2 = _get_endporint_24g_enable();

	if (en1 == -1 && en2 == -1)
		return -1;
	else{
		if (en1 == 1 || en2 == 1)
			strcpy(value, "Enabled");
		else
			strcpy(value, "Disabled");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DWEt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEt_ProfileReference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEt_ProfileReference, value);
	if(ret)
	{
		return -1;
	}*/
	char profile5GStatus[32] = {0};
	char profile24GStatus[32] = {0};

	_get_endporint_5g_profile_status(profile5GStatus);
	_get_endporint_24g_profile_status(profile24GStatus);
	if (strcmp(profile5GStatus, "Active") == 0)
		strcpy(value, "Device.WiFi.EndPoint.1.Profile.1");
	else if (strcmp(profile24GStatus, "Active") == 0)
		strcpy(value, "Device.WiFi.EndPoint.1.Profile.2");
	else
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEt_ProfileReference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEt_ProfileReference, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEt_SSIDReference(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEt_SSIDReference, value);
	if(ret)
	{
		return -1;
	}*/
	char profile5GStatus[32] = {0};
	char profile24GStatus[32] = {0};

	_get_endporint_5g_profile_status(profile5GStatus);
	_get_endporint_24g_profile_status(profile24GStatus);
	if (strcmp(profile5GStatus, "Active") == 0)
		sprintf(value, "Device.WiFi.SSID.%d", WIFI5G_START_INSTANCE_NUM);
	else if (strcmp(profile24GStatus, "Active") == 0)
		sprintf(value, "Device.WiFi.SSID.%d", WIFI24G_START_INSTANCE_NUM);
	else
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEt_ProfileNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEt_ProfileNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "2"); //only support 2.4G and 5G
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtS_LastDataDownlinkRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtS_LastDataDownlinkRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtS_LastDataUplinkRate(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtS_LastDataUplinkRate, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtS_SignalStrength(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtS_SignalStrength, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtS_Retransmissions(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtS_Retransmissions, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtS_ModesSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtS_ModesSupported, value);
	if(ret)
	{
		return -1;
	}*/
	sprintf(value, "None,WEP-64,WEP-128,WPA-Personal,WPA2-Personal,WPA-WPA2-Personal");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtS_ModesSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEtS_ModesSupported, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtS_MFPConfig(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWAtS_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0}, buff2[128] = {0};
	char *p = parseTemplate(path_name, ".EndPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig2(p, buff, "ieee80211w");
	
	ret = do_uci_get(buff, buff2);
	if(ret)
	{
		return -1;
	}
	if(!strcmp(buff2, "1")){
		strcpy(value, "Optional");
	}
	else if(!strcmp(buff2, "2")){
		strcpy(value, "Required");
	}
	else{
		strcpy(value, "Disabled");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtS_MFPConfig(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWAtS_MFPConfig, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[128] = {0}, buff2[128] = {0};
	char *p = parseTemplate(path_name, ".EndPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig2(p, buff, "ieee80211w");

	if(!strcmp(value, "Optional")){
		strcpy(buff2, "1");
	}
	else if(!strcmp(value, "Required")){
		strcpy(buff2, "2");
	}
	else{
		strcpy(buff2, "0");
	}
	
	ret = do_uci_set(buff, buff2);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("wireless");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doSbinWifi();
	}	
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtPt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtPt_Enable, value);
	if(ret)
	{
		return -1;
	}*/

	int en = 0;
	char buff[128] = {0};
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			en = _get_endporint_5g_enable();
			if (en == -1)
				return -1;
			else
				sprintf(value, "%d", en);
		}
		else if (atoi(index) == 2){
			en = _get_endporint_24g_enable();
			if (en == -1)
				return -1;
			else
				sprintf(value, "%d", en);
		}
		else
			return -1;
	}
	else
		return -1;
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtPt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEtPt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtPt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtPt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char profile5GStatus[32] = {0};
	char profile24GStatus[32] = {0};
	char buff[128] = {0};
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			ret = _get_endporint_5g_profile_status(profile5GStatus);
			if (ret == -1)
				return -1;
			strcpy(value, profile5GStatus);
		}
		else if (atoi(index) == 2){
			ret = _get_endporint_24g_profile_status(profile24GStatus);
			if (ret == -1)
				return -1;
			strcpy(value, profile24GStatus);
		}
		else
			return -1;
	}
	else
		return -1;

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtPt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DWEtPt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtPt_SSID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	int en = 0;
	char buff[128] = {0};
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			en = _get_endporint_5g_enable();
			if (en == -1)
				return -1;
			else{
				if (en == 1)
					strcpy(buff, "wireless.wla.ssid");
			}
		}
		else if (atoi(index) == 2){
			en = _get_endporint_24g_enable();
			if (en == -1)
				return -1;
			else{
				if (en == 1)
					strcpy(buff, "wireless.wlg.ssid");
			}
		}
		else
			return -1;
	}
	else
		return -1;
	
	if (en == 1){
		ret = do_uci_get(buff, value);
		if(ret)
		{
			return -1;
		}
	}
	else
		strcpy(value, "");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtPt_SSID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	int en = 0;
	char buff[128] = {0};
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			en = _get_endporint_5g_enable();
			if (en == -1)
				return -1;
			else{
				if (en == 1)
					strcpy(buff, "wireless.wla.ssid");
			}
		}
		else if (atoi(index) == 2){
			en = _get_endporint_24g_enable();
			if (en == -1)
				return -1;
			else{
				if (en == 1)
					strcpy(buff, "wireless.wlg.ssid");
			}
		}
		else
			return -1;
	}
	else
		return -1;
	
	if (en == 1){
		if (strlen(value) > 32) //SSID range: 0~32
			return -2;
		ret = do_uci_set(buff, value);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit("wireless");
			if(ret)
			{
				return (-1);
			}
			//ret = 1; //means need to reboot for taking effect
			doSbinWifi();
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtPt_Location(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtPt_Location, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtPt_Location(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEtPt_Location, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtPt_Priority(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtPt_Priority, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtPt_Priority(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEtPt_Priority, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtPtS_ModeEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtPtS_ModeEnabled, value);
	if(ret)
	{
		return -1;
	}*/
	int en = 0;
	char buff[128] = {0};
	char buff1[128] = {0};
	char tmp[128] = {0};
	char tmp1[128] = {0};
	int num = 0;
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			en = _get_endporint_5g_enable();
			if (en == -1)
				return -1;
		}
		else if (atoi(index) == 2){
			en = _get_endporint_24g_enable();
			if (en == -1)
				return -1;
		}
		else
			return -1;
	}
	else
		return -1;

	if (en == 1){
		getSSIDuciConfig2(index, buff, "encryption");
		getSSIDuciConfig2(index, buff1, "wepkeyid");
		ret = do_uci_get(buff, tmp);
		if(ret)
		{
			return -1;
		}
		
		if(strstr(tmp, "wep") != NULL)
		{
			ret = do_uci_get(buff1, tmp1);
			if(ret)
			{
				return -1;
			}
			num = atoi(tmp1);
			switch(num)	
			{
				case 1:
					getSSIDuciConfig2(index, buff, "key1");
					ret = do_uci_get(buff, tmp1);
					break;
				case 2:
					getSSIDuciConfig2(index, buff, "key2");
					ret = do_uci_get(buff, tmp1);
					break;
				case 3:
					getSSIDuciConfig2(index, buff, "key3");
					ret = do_uci_get(buff, tmp1);
					break;
				case 4:
					getSSIDuciConfig2(index, buff, "key4");
					ret = do_uci_get(buff, tmp1);
					break;					
			}
			if(ret)
			{
				return -1;
			}

			if(strlen(tmp1) == 5 || strlen(tmp1) == 10)
			{
				strcpy(value, "WEP-64");
			}
			else if(strlen(tmp1) == 13 || strlen(tmp1) == 26)
			{
				strcpy(value, "WEP-128");			
			}
		}
		else if(!strcmp(tmp, "psk+tkip"))
		{
			strcpy(value, "WPA-Personal");			
		}
		else if(!strcmp(tmp, "psk2+ccmp"))
		{
			strcpy(value, "WPA2-Personal");			
		}
		else if(!strcmp(tmp, "psk-mixed+tkip+ccmp"))
		{
			strcpy(value, "WPA-WPA2-Personal");			
		}	
		/*else if(!strcmp(tmp, "wpa+tkip"))
		{
			strcpy(value, "WPA-Enterprise");			
		}
		else if(!strcmp(tmp, "wpa2+ccmp"))
		{
			strcpy(value, "WPA2-Enterprise");			
		}
		else if(!strcmp(tmp, "wpa-mixed+tkip+ccmp"))
		{
			strcpy(value, "WPA-WPA2-Enterprise");			
		}*/
		else if(!strcmp(tmp, "none"))
		{
			strcpy(value, "None");			
		}
	}
	else
		strcpy(value, "");
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtPtS_ModeEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEtPtS_ModeEnabled, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	int en = 0;
	char buff[128] = {0};
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			en = _get_endporint_5g_enable();
			if (en == -1)
				return -1;
		}
		else if (atoi(index) == 2){
			en = _get_endporint_24g_enable();
			if (en == -1)
				return -1;
		}
		else
			return -1;
	}
	else
		return -1;

	if (en == 1){
		getSSIDuciConfig2(index, buff, "encryption");

		if(!strcasecmp(value, "wep-64") || !strcasecmp(value, "wep-128"))
		{
			ret = do_uci_set(buff, "wep+mixed");
		}
		else if(!strcasecmp(value, "wpa-personal"))
		{
			ret = do_uci_set(buff, "psk+tkip");
		}
		else if(!strcasecmp(value, "wpa2-personal"))
		{
			ret = do_uci_set(buff, "psk2+ccmp");
		}
		else if(!strcasecmp(value, "wpa-wpa2-personal"))
		{
			ret = do_uci_set(buff, "psk-mixed+tkip+ccmp");
		}
		/*else if(!strcasecmp(value, "wpa-enterprise"))
		{
			ret = do_uci_set(buff, "wpa+tkip");
		}
		else if(!strcasecmp(value, "wpa2-enterprise"))
		{
			ret = do_uci_set(buff, "wpa2+ccmp");
		}
		else if(!strcasecmp(value, "wpa-wpa2-enterprise"))
		{
			ret = do_uci_set(buff, "wpa-mixed+tkip+ccmp");
		}*/
		else if(!strcasecmp(value, "none"))
		{
			ret = do_uci_set(buff, "none");
		}
		else
			return -2;
		
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit("wireless");
			if(ret)
			{
				return (-1);
			}
			//ret = 1; //means need to reboot for taking effect
			doSbinWifi();
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtPtS_WEPKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtPtS_WEPKey, value);
	if(ret)
	{
		return -1;
	}*/

#if 0
	int en = 0;
	char buff[128] = {0};
	char buff1[128] = {0};
	char tmp[128] = {0};
	char tmp1[128] = {0};
	int num = 0;
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			en = _get_endporint_5g_enable();
			if (en == -1)
				return -1;
		}
		else if (atoi(index) == 2){
			en = _get_endporint_24g_enable();
			if (en == -1)
				return -1;
		}
		else
			return -1;
	}
	else
		return -1;

	if (en == 1){
		getSSIDuciConfig2(index, buff, "wepkeyid");
		
		ret = do_uci_get(buff, tmp);
		if(ret)
		{
			return -1;
		}
		num = atoi(tmp);
		switch(num)
		{
			case 1:
				getSSIDuciConfig2(index, buff1, "key1");
				ret = do_uci_get(buff1, tmp1);
				break;
			case 2:
				getSSIDuciConfig2(index, buff1, "key2");
				ret = do_uci_get(buff1, tmp1);
				break;
			case 3:
				getSSIDuciConfig2(index, buff1, "key3");
				ret = do_uci_get(buff1, tmp1);
				break;
			case 4:
				getSSIDuciConfig2(index, buff1, "key4");
				ret = do_uci_get(buff1, tmp1);
				break;
		}
		if(ret)
		{
			return -1;
		}
		strcpy(value, tmp1);
	}
	else
		strcpy(value, "");
#else
	strcpy(value, ""); //When read, this parameter returns an empty string
#endif
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtPtS_WEPKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEtPtS_WEPKey, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	int en = 0;
	char buff[128] = {0};
	char buff1[128] = {0};
	char buff2[128] = {0};
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			en = _get_endporint_5g_enable();
			if (en == -1)
				return -1;
		}
		else if (atoi(index) == 2){
			en = _get_endporint_24g_enable();
			if (en == -1)
				return -1;
		}
		else
			return -1;
	}
	else
		return -1;

	if (en == 1){
		if(strlen(value) != 5 && strlen(value) != 13 && strlen(value) != 10 && strlen(value) != 26)
		{
			return -2;
		}
		
		getSSIDuciConfig2(index, buff, "wepkeyid");
		getSSIDuciConfig2(index, buff1, "key");
		getSSIDuciConfig2(index, buff2, "key1");

		ret = do_uci_set(buff, "1");
		ret = do_uci_set(buff1, "1");
		ret = do_uci_set(buff2, value);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit("wireless");
			if(ret)
			{
				return (-1);
			}
			//ret = 1; //means need to reboot for taking effect
			doSbinWifi();
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtPtS_PreSharedKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtPtS_PreSharedKey, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //this parameter returns an empty string, regardless of the actual value
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtPtS_PreSharedKey(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEtPtS_PreSharedKey, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtPtS_KeyPassphrase(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtPtS_KeyPassphrase, value);
	if(ret)
	{
		return -1;
	}*/
#if 0
	int en = 0;
	char buff[128] = {0};
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			en = _get_endporint_5g_enable();
			if (en == -1)
				return -1;
		}
		else if (atoi(index) == 2){
			en = _get_endporint_24g_enable();
			if (en == -1)
				return -1;
		}
		else
			return -1;
	}
	else
		return -1;

	if (en == 1){
		getSSIDuciConfig2(index, buff, "wpapsk");
		ret = do_uci_get(buff, value);
		if(ret)
		{
			return -1;
		}
	}
	else
		strcpy(value, "");
#else
	strcpy(value, ""); //When read, this parameter returns an empty string
#endif
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtPtS_KeyPassphrase(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEtPtS_KeyPassphrase, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	int en = 0;
	char buff[128] = {0};
	char buff1[128] = {0};
	char *index = parseTemplate(path_name, ".Profile.");

	if (index != NULL){
		if (atoi(index) == 1){
			en = _get_endporint_5g_enable();
			if (en == -1)
				return -1;
		}
		else if (atoi(index) == 2){
			en = _get_endporint_24g_enable();
			if (en == -1)
				return -1;
		}
		else
			return -1;
	}
	else
		return -1;

	if (en == 1){
		if(strlen(value) < 8 || strlen(value) > 63)
		{
			return -2;
		}
		
		getSSIDuciConfig2(index, buff, "wpapsk");
		getSSIDuciConfig2(index, buff1, "key");
		ret = do_uci_set(buff, value);
		ret = do_uci_set(buff1, value);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit("wireless");
			if(ret)
			{
				return (-1);
			}
			//ret = 1; //means need to reboot for taking effect
			doSbinWifi();
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtW_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtW_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	int en = 0;
	char buff[128] = {0};
	char wps1[32] = {0};
	char wps2[32] = {0};

	en = _get_endporint_5g_enable();
	if (en == 1){
		strcpy(buff, "wireless.wla.wps_enable");
		ret = do_uci_get(buff, wps1);
		if(ret)
		{
			return (-1);
		}
	}

	en = _get_endporint_24g_enable();
	if (en == 1){
		strcpy(buff, "wireless.wlg.wps_enable");
		ret = do_uci_get(buff, wps2);
		if(ret)
		{
			return (-1);
		}
	}

	if (atoi(wps1) == 1 || atoi(wps2) == 1)
		strcpy(value, "1");
	else
		strcpy(value, "0");

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtW_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DWEtW_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	int en = 0;
	char buff[128] = {0};

	en = _get_endporint_5g_enable();
	if (en == 1){
		strcpy(buff, "wireless.wla.wps_enable");
		ret = do_uci_set(buff, value);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit("wireless");
			if(ret)
			{
				return (-1);
			}
			//ret = 1; //means need to reboot for taking effect
			system("hostapd_cli -i ath0 -p /var/run/hostapd-wifi0 wps_pbc");
		}
	}

	en = _get_endporint_24g_enable();
	if (en == 1){
		strcpy(buff, "wireless.wlg.wps_enable");
		ret = do_uci_set(buff, value);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit("wireless");
			if(ret)
			{
				return (-1);
			}
			//ret = 1; //means need to reboot for taking effect
			system("hostapd_cli -i ath1 -p /var/run/hostapd-wifi1 wps_pbc");
		}
	}

	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtW_ConfigMethodsSupported(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtW_ConfigMethodsSupported, value);
	if(ret)
	{
		return -1;
	}*/
	int en1 = 0, en2 = 0;

	en1 = _get_endporint_5g_enable();
	en2 = _get_endporint_24g_enable();
	if (en1 == -1 && en2 == -1)
		return -1;
	else{
		if (en1 == 1 || en2 == 1)
			strcpy(value, "PushButton,PIN");
		else
			strcpy(value, "");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtW_ConfigMethodsEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtW_ConfigMethodsEnabled, value);
	if(ret)
	{
		return -1;
	}*/
	int en1 = 0, en2 = 0;

	en1 = _get_endporint_5g_enable();
	en2 = _get_endporint_24g_enable();
	if (en1 == -1 && en2 == -1)
		return -1;
	else{
		if (en1 == 1 || en2 == 1)
			strcpy(value, "PushButton");
		else
			strcpy(value, "");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtW_ConfigMethodsEnabled(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtW_ConfigMethodsEnabled, value);
	/*if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtW_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtW_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0}, buff2[128] = {0};
	char *p = parseTemplate(path_name, ".EndPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig2(p, buff, "wps_state");
	
	ret = do_uci_get(buff, buff2);
	if(ret)
	{
		return -1;
	}
	if(!strcmp(buff2, "1")){
		strcpy(value, "Unconfigured");
	}
	else if(!strcmp(buff2, "2")){
		strcpy(value, "Configured");
	}
	else{
		strcpy(value, "Disabled");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtW_Version(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtW_MFPConfig, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "2.0"); 
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtW_PIN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DWEtW_PIN, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[128] = {0};
	char *p = parseTemplate(path_name, ".EndPoint.");

	if (p == NULL)
		return -1;
	
	getSSIDuciConfig2(p, buff, "wps_pin");
	
	ret = do_uci_get(buff, value);
	if(ret)
	{
		return -1;
	}

	if (strlen(value) != 4 && strlen(value) !=8)
		return -2;
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAt_AccessCategory(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAt_AccessCategory, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtAt_AccessCategory(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtAt_AccessCategory, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtAt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtAt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtAt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtAt_AIFSN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAt_AIFSN, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtAt_AIFSN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtAt_AIFSN, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtAt_ECWMin(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAt_ECWMin, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtAt_ECWMin(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtAt_ECWMin, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtAt_ECWMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAt_ECWMax, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtAt_ECWMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtAt_ECWMax, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtAt_TxOpMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAt_TxOpMax, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtAt_TxOpMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtAt_TxOpMax, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtAt_AckPolicy(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAt_AckPolicy, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtAt_AckPolicy(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtAt_AckPolicy, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtAt_OutQLenHistogramIntervals(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAt_OutQLenHistogramIntervals, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtAt_OutQLenHistogramIntervals(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtAt_OutQLenHistogramIntervals, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtAt_OutQLenHistogramSampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAt_OutQLenHistogramSampleInterval, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DWEtAt_OutQLenHistogramSampleInterval(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DWEtAt_OutQLenHistogramSampleInterval, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DWEtAtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAtS_RetransCount(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_RetransCount, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DWEtAtS_OutQLenHistogram(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DWEtAtS_OutQLenHistogram, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DB_MaxBridgeEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DB_MaxBridgeEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DB_MaxDBridgeEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DB_MaxDBridgeEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DB_MaxQBridgeEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DB_MaxQBridgeEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DB_MaxVLANEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DB_MaxVLANEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DB_MaxProviderBridgeEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DB_MaxProviderBridgeEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DB_ProviderBridgeNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DB_ProviderBridgeNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DB_MaxFilterEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DB_MaxFilterEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DB_BridgeNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DB_BridgeNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DB_FilterNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DB_FilterNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char en[32] = {0};

	getDevStatus("br-lan", "up", en);
	if (strcmp(en, "true") == 0)
		strcpy(value, "1");
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DBBt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char en[32] = {0};

	getDevStatus("br-lan", "up", en);
	if (strcmp(en, "true") == 0)
		strcpy(value, "Enabled");
	else
		strcpy(value, "Disabled");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	//set to tr.xml
	/*ret = do_uci_set(DBBt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBt_Standard(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBt_Standard, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "802.1D-2004");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBt_Standard(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DBBt_Standard, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBt_PortNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBt_PortNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	char keyvalue[MAXMAPITEMS][256];
	int num = get_Device_Bridging_Bridge_1_Port_Entry(keyvalue);

	sprintf(value, "%d", num);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBt_VLANNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBt_VLANNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBt_VLANPortNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBt_VLANPortNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	char status[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "up", status);
			if (strcmp(status, "true") == 0)
				strcpy(value, "1");
			else
				strcpy(value, "0");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DBBtPt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	char status[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "up", status);
			if (strcmp(status, "true") == 0)
				strcpy(value, "Up");
			else
				strcpy(value, "Down");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	//set to tr.xml
	/*ret = do_uci_set(DBBtPt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPt_Name, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPt_LastChange, value);
	if(ret)
	{
		return -1;
	}*/
	getInterfaceLastChangeTime(value); //all interface with the same value
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	int i = 0;
	char *p = NULL;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%*s %*s %s", value);
			if ((p = strstr(value, "\n")) != NULL)
				*p = '\0';
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DBBtPt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_ManagementPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPt_ManagementPort, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%*s %s %*s", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_ManagementPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DBBtPt_ManagementPort, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_Type(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_Type, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_Type(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_Type, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_DefaultUserPriority(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_DefaultUserPriority, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_DefaultUserPriority(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_DefaultUserPriority, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_PriorityRegeneration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_PriorityRegeneration, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_PriorityRegeneration(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_PriorityRegeneration, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_PortState(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_PortState, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPt_PVID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_PVID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_PVID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_PVID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_TPID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_TPID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_TPID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_TPID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_AcceptableFrameTypes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_AcceptableFrameTypes, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_AcceptableFrameTypes(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_AcceptableFrameTypes, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_IngressFiltering(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_IngressFiltering, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_IngressFiltering(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_IngressFiltering, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_ServiceAccessPrioritySelection(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_ServiceAccessPrioritySelection, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_ServiceAccessPrioritySelection(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_ServiceAccessPrioritySelection, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_ServiceAccessPriorityTranslation(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_ServiceAccessPriorityTranslation, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_ServiceAccessPriorityTranslation(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_ServiceAccessPriorityTranslation, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPt_PriorityTagging(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPt_PriorityTagging, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPt_PriorityTagging(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPt_PriorityTagging, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPtP_PCPSelection(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPtP_PCPSelection, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPtP_PCPSelection(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPtP_PCPSelection, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPtP_UseDEI(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPtP_UseDEI, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPtP_UseDEI(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPtP_UseDEI, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPtP_RequireDropEncoding(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPtP_RequireDropEncoding, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPtP_RequireDropEncoding(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPtP_RequireDropEncoding, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPtP_PCPEncoding(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPtP_PCPEncoding, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPtP_PCPEncoding(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPtP_PCPEncoding, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPtP_PCPDecoding(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtPtP_PCPDecoding, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtPtP_PCPDecoding(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtPtP_PCPDecoding, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtPtS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_BytesSent, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "tx_bytes", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "rx_bytes", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "tx_packets", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "rx_packets", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "tx_errors", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "rx_errors", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	char laninf[32] = {0};
	char waninf[32] = {0};
	int i = 0;
	int num = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			if (strstr(inf, "ath") != NULL || strstr(inf, "mesh") != NULL){
				char p[32] = {32};
				num = getWiFiInstanceNumWithInterfaceName(inf);
				if (num == 0)
					return -1;

				sprintf(p, "%d", num);

				if (p)
					getSSIStats(p, "Tx Unicast Data Packets", value);
			}
			else {
				int count = 0;
				int sum = 0;
				int j = 0;
				getEthInterfaceName("lan", laninf);
				getEthInterfaceName("wan", waninf);
				if (strcmp(inf, laninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(j, "TxUniCast");
						sum = sum + count;
						count = 0;
					}
				}
				else if (strcmp(inf, waninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(5, "TxUniCast");
						sum = sum + count;
					}
				}
				else
					sum = 0;

				sprintf(value, "%d", sum);
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	char laninf[32] = {0};
	char waninf[32] = {0};
	int i = 0;
	int num = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			if (strstr(inf, "ath") != NULL || strstr(inf, "mesh") != NULL){
				char p[32] = {32};
				num = getWiFiInstanceNumWithInterfaceName(inf);
				if (num == 0)
					return -1;

				sprintf(p, "%d", num);

				if (p)
					getSSIStats(p, "Rx Unicast Data Packets", value);
			}
			else {
				int count = 0;
				int sum = 0;
				int j = 0;
				getEthInterfaceName("lan", laninf);
				getEthInterfaceName("wan", waninf);
				if (strcmp(inf, laninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(j, "RxUniCast");
						sum = sum + count;
						count = 0;
					}
				}
				else if (strcmp(inf, waninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(5, "RxUniCast");
						sum = sum + count;
					}
				}
				else
					sum = 0;

				sprintf(value, "%d", sum);
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "tx_dropped", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "rx_dropped", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	char laninf[32] = {0};
	char waninf[32] = {0};
	int i = 0;
	int num = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			if (strstr(inf, "ath") != NULL || strstr(inf, "mesh") != NULL){
				char p[32] = {32};
				num = getWiFiInstanceNumWithInterfaceName(inf);
				if (num == 0)
					return -1;

				sprintf(p, "%d", num);

				if (p)
					getSSIStats(p, "Tx multicast Data Packets", value);
			}
			else {
				int count = 0;
				int sum = 0;
				int j = 0;
				getEthInterfaceName("lan", laninf);
				getEthInterfaceName("wan", waninf);
				if (strcmp(inf, laninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(j, "TxMulti");
						sum = sum + count;
						count = 0;
					}
				}
				else if (strcmp(inf, waninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(5, "TxMulti");
						sum = sum + count;
					}
				}
				else
					sum = 0;

				sprintf(value, "%d", sum);
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	char laninf[32] = {0};
	char waninf[32] = {0};
	int i = 0;
	int num = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			if (strstr(inf, "ath") != NULL || strstr(inf, "mesh") != NULL){
				char p[32] = {32};
				num = getWiFiInstanceNumWithInterfaceName(inf);
				if (num == 0)
					return -1;

				sprintf(p, "%d", num);

				if (p)
					getSSIStats(p, "Rx multicast Data Packets", value);
			}
			else {
				int count = 0;
				int sum = 0;
				int j = 0;
				getEthInterfaceName("lan", laninf);
				getEthInterfaceName("wan", waninf);
				if (strcmp(inf, laninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(j, "RxMulti");
						sum = sum + count;
						count = 0;
					}
				}
				else if (strcmp(inf, waninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(5, "RxMulti");
						sum = sum + count;
					}
				}
				else
					sum = 0;

				sprintf(value, "%d", sum);
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	char laninf[32] = {0};
	char waninf[32] = {0};
	int i = 0;
	int num = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			if (strstr(inf, "ath") != NULL || strstr(inf, "mesh") != NULL){
				char p[32] = {32};
				num = getWiFiInstanceNumWithInterfaceName(inf);
				if (num == 0)
					return -1;

				sprintf(p, "%d", num);

				if (p)
					getSSIStats(p, "Tx Broadcast Data Packets", value);
			}
			else {
				int count = 0;
				int sum = 0;
				int j = 0;
				getEthInterfaceName("lan", laninf);
				getEthInterfaceName("wan", waninf);
				if (strcmp(inf, laninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(j, "TxBroad");
						sum = sum + count;
						count = 0;
					}
				}
				else if (strcmp(inf, waninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(5, "TxBroad");
						sum = sum + count;
					}
				}
				else
					sum = 0;

				sprintf(value, "%d", sum);
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	char laninf[32] = {0};
	char waninf[32] = {0};
	int i = 0;
	int num = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			if (strstr(inf, "ath") != NULL || strstr(inf, "mesh") != NULL){
				char p[32] = {32};
				num = getWiFiInstanceNumWithInterfaceName(inf);
				if (num == 0)
					return -1;

				sprintf(p, "%d", num);

				if (p)
					getSSIStats(p, "Rx Broadcast Data Packets", value);
			}
			else {
				int count = 0;
				int sum = 0;
				int j = 0;
				getEthInterfaceName("lan", laninf);
				getEthInterfaceName("wan", waninf);
				if (strcmp(inf, laninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(j, "RxBroad");
						sum = sum + count;
						count = 0;
					}
				}
				else if (strcmp(inf, waninf) == 0){
					for(j=1; j<=4; j++)
					{
						count = get_ssdk_mib_statistics(5, "RxBroad");
						sum = sum + count;
					}
				}
				else
					sum = 0;

				sprintf(value, "%d", sum);
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtPtS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DBBtPtS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}*/
	char str[256] = {0};
	char inf[32] = {0};
	int i = 0;
	char *index = parseTemplate(path_name, ".Port.");

	if (index != NULL){
		ret = lib_getvalue_mapfile_byinstance(BridgingBridge1PortMap, str, atoi(index));
		if(ret)
		{
			return (-1);
		}
		else
		{
			for (i = 0; i < strlen(str); i ++){
				if (str[i] == '|')
					str[i] = ' ';
			}
			sscanf(str, "%s %*s", inf);
			getDevStatus(inf, "rx_frame_errors", value);
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBBtVt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtVt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtVt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtVt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtVt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtVt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtVt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtVt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtVt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtVt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtVt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtVt_Name, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtVt_VLANID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtVt_VLANID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtVt_VLANID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtVt_VLANID, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtVt_Enable_2007(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtVt_Enable_2007, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtVt_Enable_2007(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtVt_Enable_2007, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtVt_Alias_2009(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtVt_Alias_2009, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtVt_Alias_2009(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtVt_Alias_2009, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtVt_VLAN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtVt_VLAN, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtVt_VLAN(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtVt_VLAN, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtVt_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtVt_Port, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtVt_Port(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtVt_Port, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBBtVt_Untagged(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBBtVt_Untagged, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBBtVt_Untagged(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBBtVt_Untagged, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DBFt_Bridge(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_Bridge, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_Bridge(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_Bridge, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_Order(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_Order, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_Order(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_Order, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_Interface, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_Interface(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_Interface, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DHCPType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DHCPType, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DHCPType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DHCPType, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_VLANIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_VLANIDFilter, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_VLANIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_VLANIDFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_EthertypeFilterList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_EthertypeFilterList, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_EthertypeFilterList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_EthertypeFilterList, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_EthertypeFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_EthertypeFilterExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_EthertypeFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_EthertypeFilterExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACAddressFilterList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACAddressFilterList, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACAddressFilterList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACAddressFilterList, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACAddressFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACAddressFilterExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACAddressFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACAddressFilterExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMACAddressFilterList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMACAddressFilterList, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMACAddressFilterList(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMACAddressFilterList, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMACAddressFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMACAddressFilterExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMACAddressFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMACAddressFilterExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACFromVendorClassIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACFromVendorClassIDFilter, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACFromVendorClassIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACFromVendorClassIDFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACFromVendorClassIDFilterv6(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACFromVendorClassIDFilterv6, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACFromVendorClassIDFilterv6(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACFromVendorClassIDFilterv6, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACFromVendorClassIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACFromVendorClassIDFilterExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACFromVendorClassIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACFromVendorClassIDFilterExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACFromVendorClassIDMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACFromVendorClassIDMode, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACFromVendorClassIDMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACFromVendorClassIDMode, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_estMACFromVendorClassIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_estMACFromVendorClassIDFilter, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_estMACFromVendorClassIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_estMACFromVendorClassIDFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMACFromVendorClassIDFilterv6(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMACFromVendorClassIDFilterv6, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMACFromVendorClassIDFilterv6(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMACFromVendorClassIDFilterv6, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMACFromVendorClassIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMACFromVendorClassIDFilterExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMACFromVendorClassIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMACFromVendorClassIDFilterExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMACFromVendorClassIDMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMACFromVendorClassIDMode, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMACFromVendorClassIDMode(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMACFromVendorClassIDMode, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACFromClientIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACFromClientIDFilter, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACFromClientIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACFromClientIDFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACFromClientIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACFromClientIDFilterExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACFromClientIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACFromClientIDFilterExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMACFromClientIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMACFromClientIDFilter, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMACFromClientIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMACFromClientIDFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMACFromClientIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMACFromClientIDFilterExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMACFromClientIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMACFromClientIDFilterExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACFromUserClassIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACFromUserClassIDFilter, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACFromUserClassIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACFromUserClassIDFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMACFromUserClassIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMACFromUserClassIDFilterExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMACFromUserClassIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMACFromUserClassIDFilterExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMACFromUserClassIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMACFromUserClassIDFilter, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMACFromUserClassIDFilter(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMACFromUserClassIDFilter, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMACFromUserClassIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMACFromUserClassIDFilterExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMACFromUserClassIDFilterExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMACFromUserClassIDFilterExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestIP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestIP, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestIP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestIP, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestMask, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestMask, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestIPExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestIPExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestIPExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestIPExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceIP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceIP, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceIP(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceIP, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceMask, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceMask, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourceIPExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourceIPExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourceIPExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourceIPExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_Protocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_Protocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_Protocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_Protocol, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_ProtocolExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_ProtocolExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_ProtocolExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_ProtocolExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestPort, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestPort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestPort, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestPortRangeMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestPortRangeMax, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestPortRangeMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestPortRangeMax, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_DestPortExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_DestPortExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_DestPortExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_DestPortExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourcePort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourcePort, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourcePort(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourcePort, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourcePortRangeMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourcePortRangeMax, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourcePortRangeMax(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourcePortRangeMax, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBFt_SourcePortExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBFt_SourcePortExclude, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBFt_SourcePortExclude(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBFt_SourcePortExclude, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBPt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBPt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBPt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBPt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBPt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBPt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBPt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBPt_Status, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBPt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBPt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBPt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBPt_Type(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBPt_Type, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBPt_Type(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBPt_Type, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBPt_SVLANComponent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBPt_SVLANComponent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBPt_SVLANComponent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBPt_SVLANComponent, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DBPt_CVLANComponents(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DBPt_CVLANComponents, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DBPt_CVLANComponents(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DBPt_CVLANComponents, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DP_InterfaceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DP_InterfaceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DP_SupportedNCPs(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DP_SupportedNCPs, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_Enable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_Status, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_Alias, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_Name, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPIt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_LastChange, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_Reset, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_Reset, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_ConnectionStatus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_ConnectionStatus, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPIt_LastConnectionError(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_LastConnectionError, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPIt_AutoDisconnectTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_AutoDisconnectTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_AutoDisconnectTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_AutoDisconnectTime, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_IdleDisconnectTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_IdleDisconnectTime, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_IdleDisconnectTime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_IdleDisconnectTime, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_WarnDisconnectDelay(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_WarnDisconnectDelay, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_WarnDisconnectDelay(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_WarnDisconnectDelay, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_Username(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_Username, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_Username(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_Username, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_Password(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_Password, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_Password(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_Password, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_EncryptionProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_EncryptionProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_EncryptionProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_EncryptionProtocol, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_CompressionProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_CompressionProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_CompressionProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_CompressionProtocol, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_AuthenticationProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_AuthenticationProtocol, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_AuthenticationProtocol(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_AuthenticationProtocol, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_MaxMRUSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_MaxMRUSize, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_MaxMRUSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_MaxMRUSize, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_CurrentMRUSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_CurrentMRUSize, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPIt_ConnectionTrigger(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_ConnectionTrigger, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_ConnectionTrigger(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_ConnectionTrigger, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_LCPEcho(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_LCPEcho, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPIt_LCPEchoRetry(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_LCPEchoRetry, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPIt_IPCPEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_IPCPEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_IPCPEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_IPCPEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPIt_IPv6CPEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPIt_IPv6CPEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPIt_IPv6CPEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPIt_IPv6CPEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPItP_SessionID(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItP_SessionID, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItP_ACName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItP_ACName, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPItP_ACName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPItP_ACName, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPItP_ServiceName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItP_ServiceName, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPItP_ServiceName(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPItP_ServiceName, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPItI_LocalIPAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItI_LocalIPAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItI_RemoteIPAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItI_RemoteIPAddress, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItI_DNSServers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItI_DNSServers, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItI_PassthroughEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItI_PassthroughEnable, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPItI_PassthroughEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPItI_PassthroughEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPItI_PassthroughDHCPPool(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItI_PassthroughDHCPPool, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DPItI_PassthroughDHCPPool(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_set(DPItI_PassthroughDHCPPool, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DPItI_LocalInterfaceIdentifier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItI_LocalInterfaceIdentifier, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItI_RemoteInterfaceIdentifier(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItI_RemoteInterfaceIdentifier, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_BytesSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_BytesSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_BytesReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_BytesReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_PacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_PacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_PacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_PacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_ErrorsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_ErrorsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_ErrorsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_ErrorsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_UnicastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_UnicastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_UnicastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_UnicastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_DiscardPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_DiscardPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_DiscardPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_DiscardPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_MulticastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_MulticastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_MulticastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_MulticastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_BroadcastPacketsSent(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_BroadcastPacketsSent, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_BroadcastPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_BroadcastPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DPItS_UnknownProtoPacketsReceived(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get(DPItS_UnknownProtoPacketsReceived, value);
	if(ret)
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DI_IPv4Capable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DI_IPv4Capable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always true
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DI_IPv4Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DI_IPv4Enable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always enable
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DI_IPv4Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DI_IPv4Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DI_IPv4Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DI_IPv4Status, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "Enabled"); //don't allow to set
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DI_IPv6Capable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DI_IPv6Capable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always 1
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DI_IPv6Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	ret = do_uci_get("ipv6.@global[0].enable", value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	if (atoi(value) == 0)
		strcpy(value, "0");
	else
		strcpy(value, "1");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DI_IPv6Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char ipv6en[32] = {0};
	
	if (atoi(value) == 0)
		strcpy(ipv6en, "0");
	else
		strcpy(ipv6en, "1"); //always make the ipv6 with Native mode
		
	ret = do_uci_set("ipv6.@global[0].enable", ipv6en);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("ipv6");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doIPv6Restart();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DI_IPv6Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get("ipv6.@global[0].enable", value);
	if(ret)
	{
		strcpy(value, "0");
		ret = 0;
	}
	if (atoi(value) == 0)
		strcpy(value, "Disabled");
	else
		strcpy(value, "Enabled"); */
	ret = do_uci_get("ipv6.@global[0].status", value);
	if(ret)
	{
		strcpy(value, "Disabled");
		ret = 0;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DI_ULAPrefix(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DI_ULAPrefix, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //always an empty string
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DI_ULAPrefix(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DI_ULAPrefix, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DI_InterfaceNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DI_InterfaceNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	sprintf(value, "%d", IP_LAN_INSTANCE_NUM+IP_WAN_INSTANCE_NUM);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DI_ActivePortNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DI_ActivePortNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	FILE *fp = NULL;
	char buff[1024] = {0};
	char localIP[256] = {0};
	char remoteIP[256] = {0};
	char state[64] = {0};
	int i = 0;
	
	system("netstat -n -t > /tmp/netstatresult");
	system("netstat -n -t -l >> /tmp/netstatresult");

	fp = fopen("/tmp/netstatresult", "r");
	if(fp != NULL)
	{
		while(fgets(buff, sizeof(buff), fp))
		{
			if(strstr(buff, "tcp") != NULL)
			{
				sscanf(buff, "%*s %*s %*s %s %s %s", localIP, remoteIP, state);
				if(strcmp(state, "LISTEN") == 0 || strcmp(state, "ESTABLISHED") == 0)
				{
					i++;
				}
			}
		}
		fclose(fp);
	}
	unlink("/tmp/netstatresult");
	sprintf(value, "%d", i);
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char inf[32] = {0};
		a_infinfo infStatus;

		memset(&infStatus, 0, sizeof(infStatus));
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
		getInterfaceInfo(inf, &infStatus);

		if (infStatus.status == 1)
			strcpy(value, "1");
		else
			strcpy(value, "0");
	}
	else if(atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		char wanup[32] = {0};
		ret = do_uci_get("network.wan.disabled", wanup);
		if(ret)
		{
			strcpy(value, "1"); //no this uci node with default settings
			ret = 0;
		}
		else
		{
			if (atoi(wanup) == 1)
				strcpy(value, "0");
			else
				strcpy(value, "1");
		}
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIIt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if(atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		char wanup[32] = {0};
		if (atoi(value) == 1)
			strcpy(wanup, "0");
		else{
			//strcpy(wanup, "1");
			return -3; //CS attempt to disable the WAN interface, that parameter setting MUST be rejected and reported to ACS as an error, now the error code is 9001(Request denied)
		}
		ret = do_uci_set("network.wan.disabled", wanup);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit("network");
			if(ret)
			{
				return (-1);
			}
			/*if(atoi(buff) != atoi(value))
			{
				system("/etc/init.d/network restart &");
			}*/
			//ret = 1; //means need to reboot for taking effect
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIIt_IPv4Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_IPv4Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char inf[32] = {0};
		a_infinfo infStatus;

		memset(&infStatus, 0, sizeof(infStatus));
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
		getInterfaceInfo(inf, &infStatus);

		if (infStatus.status == 1)
			strcpy(value, "1");
		else
			strcpy(value, "0");
	}
	else if(atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		char ipv4up[32] = {0};
		ret = do_uci_get("network.wan.ipv4_disabled", ipv4up);
		if(ret)
		{
			strcpy(value, "1"); //no this uci node with default settings
			ret = 0;
		}
		else
		{
			if (atoi(ipv4up) == 1)
				strcpy(value, "0");
			else
				strcpy(value, "1");
		}
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_IPv4Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIIt_IPv4Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if(atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		char ipv4up[32] = {0};
		if (atoi(value) == 1)
			strcpy(ipv4up, "0");
		else
			//strcpy(ipv4up, "1");
			return -3; //CS attempt to disable the WAN interface, that parameter setting MUST be rejected and reported to ACS as an error, now the error code is 9001(Request denied)
		ret = do_uci_set("network.wan.ipv4_disabled", ipv4up);
		if(ret)
		{
			return (-1);
		}
		else
		{
			ret = do_uci_commit("network");
			if(ret)
			{
				return (-1);
			}
			//ret = 1; //means need to reboot for taking effect
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIIt_IPv6Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan and network.wan support ipv6, network.lan1 ~ network.lan3 don't support
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
	}
	else
		ipv6capable = 1;

	if (ipv6capable == 0)
		strcpy(value, "0");
	else{
		//LAN and WAN with the same node for ipv6
		ret = do_uci_get("ipv6.@global[0].connection_type", value);
		if(ret)
		{
			strcpy(value, "0");
			ret = 0;
		}
		if (atoi(value) == 0)
			strcpy(value, "0");
		else
			strcpy(value, "1");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_IPv6Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char ipv6en[32] = {0};
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan and network.wan support ipv6, network.lan1 ~ network.lan3 don't support
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		if (ipv6capable != 1)
			return 0;
	}
	
	if (atoi(value) == 0)
		strcpy(ipv6en, "0");
	else
		strcpy(ipv6en, "1"); //always make the ipv6 with Native mode

	ret = do_uci_set("ipv6.@global[0].enable", ipv6en);
	if(ret)
	{
		return (-1);
	}

	ret = do_uci_set("ipv6.@global[0].connection_type", ipv6en);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("ipv6");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doIPv6Restart();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIIt_ULAEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_ULAEnable, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_ULAEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIIt_ULAEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIIt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char inf[32] = {0};
		a_infinfo infStatus;

		memset(&infStatus, 0, sizeof(infStatus));
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
		getInterfaceInfo(inf, &infStatus);

		if (infStatus.status == 1)
			strcpy(value, "Up");
		else
			strcpy(value, "Down");
	}
	else if(atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		char waninf[32] = {0};
		getWanHigherLayerInterface(waninf);
		if (waninf[0] != '\0')
			strcpy(value, "Up");
		else
			strcpy(value, "Down");
	}
	else
	{
		return -1;
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DIIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);*/
	//set to tr.xml
	return ret;
}
int get_DIIt_Name(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_Name, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char inf[32] = {0};
		getLanInterfaceNameWithInstanceNum(p, inf);
		sprintf(value, "LAN Group %s", inf);
	}
	else
	{
		//getWanHigherLayerInterface(value);
		char wanmode[32] = {0};
		getWanMode(wanmode);
		sprintf(value, "WAN %s Mode", wanmode); //textual name
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIIt_LastChange(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_LastChange, value);
	if(ret)
	{
		return -1;
	}*/
	a_infinfo infStatus;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	memset(&infStatus, 0, sizeof(infStatus));
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char inf[32] = {0};
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
		getInterfaceInfo(inf, &infStatus);
	}
	else
	{
		char wanmode[32] = {0};
		char wantype[32] = {0};
		getWanMode(wanmode);
		if (strcmp(wanmode, "pptp") == 0 || strcmp(wanmode, "l2tp") == 0)
			strcpy(wantype, "wan0");
		else
			strcpy(wantype, "wan");
		memset(&infStatus, 0, sizeof(infStatus));
		getInterfaceInfo(wantype, &infStatus);
	}
	if (strcmp(infStatus.uptime, "") != 0)
		strcpy(value, infStatus.uptime);
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_LowerLayers, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char inf[32] = {0};
		char inf2[128] = {0};
		int has24g = 0;
		int has5g = 0;
		int i = 0;

		getLanInterfaceNameWithInstanceNum(p, inf);
		getLanLowerLayerInterface(inf2, inf);
		if (strstr(inf2, "eth") != NULL){
			strcpy(value, ETHERNET_LAN_INTERFACE_PATH);
		}

		for (i = 0; i < WIFI_MAX_INSTANCE_NUM; i ++){
			if (strstr(inf2, wifi_map[i].wlaninf) != NULL){
				if (wifi_map[i].num >= WIFI5G_START_INSTANCE_NUM && wifi_map[i].num <= WIFI5G_END_INSTANCE_NUM)
					has5g ++;
				else
					has24g ++;
			}
		}

		if (has5g != 0){
			if (strcmp(value, "") == 0)
				strcpy(value, WIFI_RADIO_5G_PATH);
			else
				sprintf(value, "%s,%s", value, WIFI_RADIO_5G_PATH);
		}

		if (has24g != 0){
			if (strcmp(value, "") == 0)
				strcpy(value, WIFI_RADIO_24G_PATH);
			else
				sprintf(value, "%s,%s", value, WIFI_RADIO_24G_PATH);
		}
	}
	else
	{
		char wanup[32] = {0};
		ret = do_uci_get("network.wan.disabled", wanup);
		if(ret)
		{
			strcpy(wanup, "0"); //no this uci node with defalut settings
			ret = 0;
		}
		if (atoi(wanup) == 0) //0 means up, 1 means down
			strcpy(value, ETHERNET_WAN_INTERFACE_PATH);
		else
			strcpy(value, "");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_LowerLayers(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIIt_LowerLayers, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIIt_Router(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_Router, value);
	if(ret)
	{
		return -1;
	}*/
	char routerinfo[128] = {0};
	char inf[32] = {0};
	int i =0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		getLanInterfaceNameWithInstanceNum(p, inf);
	}
	else
	{
		getEthInterfaceName("wan", inf);
	}

	for (i = 1; ;i ++){
		ret = lib_getvalue_mapfile_byinstance(RouterIPv4Map, routerinfo, i);
		if(ret)
		{
			strcpy(value, "");
			ret = 0; //must
			break;
		}
		else
		{
			char inf2[32] = {0};
			getRouterIPv4Option(routerinfo, inf2, "Iface");
			if (strcmp(inf, inf2) == 0){
				if (strcmp(value, "") == 0)
					sprintf(value, "Device.Routing.Router.1.IPv4Forwarding.%d", i);
				else
					sprintf(value, "%s,Device.Routing.Router.1.IPv4Forwarding.%d", value, i);
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_Router(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIIt_Router, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIIt_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_Reset, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0");//always false
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_Reset(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIIt_Reset, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		doIPInterfaceReset();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIIt_MaxMTUSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_MaxMTUSize, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char ucipath[64] = {0};
		getLanInterfaceNameWithInstanceNum(p, inf);
		getLanUciPathWithInstanceNum(p, ucipath);
		sprintf(ucipath, "%s.mtu", ucipath);
		ret = do_uci_get(ucipath, value);
		if(ret)
		{
			getNetMtu(inf, value);
			ret = 0;
		}
	}
	else
	{
		char wanmode[32] = {0};
		char str[64] = {0};
		char mtu[64] = {0};
		getWanMode(wanmode);
		if (strcmp(wanmode, "pptp") == 0 || strcmp(wanmode, "l2tp") == 0)
			strcpy(str, "network.wan0.mtu");
		else
			strcpy(str, "network.wan.mtu");

		ret = do_uci_get(str, mtu);
		if(ret)
		{ //for network.wan.mtu and network.wan0.mtu value are null
			getWanHigherLayerInterface(inf);
			getNetMtu(inf, mtu);
			ret = 0; //MUST
		}

		strcpy(value, mtu);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_MaxMTUSize(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char *p = parseTemplate(path_name, ".Interface.");

	if (atoi(value) < 64 || atoi(value) > 1500)
		return -2;

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char ucipath[64] = {0};
		char name[64] = {0};
		char tmpvalue[64] = {0};

		getLanUbsInterfaceNameWithInstanceNum(p, name);
		getLanUciPathWithInstanceNum(p, ucipath);

		//ucipath==>network.lan%d, name==>lan%d
		ret = do_uci_get(ucipath, tmpvalue);
		if(ret)
		{
			addUciTopNode("network", name);
			ret = 0;
		}

		sprintf(ucipath, "%s.mtu", ucipath);
		ret = do_uci_set(ucipath, value);
		if(ret)
	    {
		    return (-1);
	    }
	    else
	    {
		    ret = do_uci_commit("network");
		    if(ret)
		    {
			    return (-1);
		    }
			//ret = 1; //means need to reboot for taking effect
	    }
	}
	else if(atoi(p) == IP_WAN_INSTANCE_INDEX){
		char wanmode[32] = {0};
		char str[64] = {0};
		char str2[64] = {0};
		getWanMode(wanmode);
		if (strcmp(wanmode, "pptp") == 0 || strcmp(wanmode, "l2tp") == 0)
			strcpy(str, "network.wan0.mtu");
		else{
			strcpy(str, "network.wan.mtu");
			strcpy(str2, "network.commwan.mtu"); //new SDK need
		}

		if (str2[0] != '\0'){
			ret = do_uci_set(str2, value);
			if(ret)
	    	{
		    	return (-1);
	    	}
		}
			
	    ret = do_uci_set(str, value);
	    if(ret)
	    {
		    return (-1);
	    }
	    else
	    {
		    ret = do_uci_commit("network");
		    if(ret)
		    {
			    return (-1);
		    }
			//ret = 1; //means need to reboot for taking effect
	    }
	}
	else
		return -1;
	
	doRestartNetwork();
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIIt_Type(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_Type, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "Normal"); //always normal
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIIt_Loopback(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_Loopback, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_Loopback(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIIt_Loopback, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIIt_IPv4AddressNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_IPv4AddressNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "1"); //always 1
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIIt_IPv6AddressNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_IPv6AddressNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		if (ipv6capable == 1)
			strcpy(value, "1");
		else
			strcpy(value, "0");
	}
	else
		strcpy(value, "1");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIIt_IPv6PrefixNumberOfEntries(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_IPv6PrefixNumberOfEntries, value);
	if(ret)
	{
		return -1;
	}*/
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		if (ipv6capable == 1)
			strcpy(value, "1");
		else
			strcpy(value, "0");
	}
	else
		strcpy(value, "1");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIIt_AutoIPEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIIt_AutoIPEnable, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[32] = {0};
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char ucipath[64] = {0};
		getLanUciPathWithInstanceNum(p, ucipath);
		sprintf(ucipath, "%s.proto", ucipath);

		ret = do_uci_get(ucipath, buff);
		if(ret)
		{
			strcpy(value, "0");
			ret = 0;
		}
		else{
			if(strcmp(buff, "static") == 0)
			{
				strcpy(value, "0");		
			}
			else
			{
				strcpy(value, "1");
			}
		}
	}
	else
	{
		ret = do_uci_get("network.wan.proto", buff);
		if(ret)
		{
			strcpy(value, "1");
			ret = 0;
		}
		if(strcmp(buff, "static") == 0)
		{
			strcpy(value, "0");		
		}
		else
		{
			strcpy(value, "1");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIIt_AutoIPEnable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIIt_AutoIPEnable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		return 0; //don't support to set
	}
	else
	{
		if (atoi(value) == 1){
			ret = do_uci_set("network.wan.proto", "dhcp");
			if(ret)
			{
				return (-1);
			}
			else
			{
				ret = do_uci_commit("network");
				if(ret)
				{
					return (-1);
				}
				//ret = 1; //means need to reboot for taking effect
				doRestartNetwork();
			}
		}
	}

	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Enable, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
	a_infinfo infStatus;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		memset(&infStatus, 0, sizeof(infStatus));
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
		getInterfaceInfo(inf, &infStatus);
	}
	else
	{
		char wanmode[32] = {0};
		char wantype[32] = {0};
		getWanMode(wanmode);
		if (strcmp(wanmode, "pptp") == 0 || strcmp(wanmode, "l2tp") == 0)
			strcpy(wantype, "wan0");
		else
			strcpy(wantype, "wan");
		memset(&infStatus, 0, sizeof(infStatus));
		getInterfaceInfo(wantype, &infStatus);
	}

	if (infStatus.status == 1)
		strcpy(value, "1");
	else
		strcpy(value, "0");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_Enable(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIItIt_Enable, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_Status(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Status, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
 	a_infinfo infStatus;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		memset(&infStatus, 0, sizeof(infStatus));
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
		getInterfaceInfo(inf, &infStatus);
	}
	else{
		char wanmode[32] = {0};
		char wantype[32] = {0};
		getWanMode(wanmode);
		if (strcmp(wanmode, "pptp") == 0 || strcmp(wanmode, "l2tp") == 0)
			strcpy(wantype, "wan0");
		else
			strcpy(wantype, "wan");
		memset(&infStatus, 0, sizeof(infStatus));
		getInterfaceInfo(wantype, &infStatus);
	}

	if (infStatus.status == 1)
 		strcpy(value, "Enabled");
 	else
		strcpy(value, "Disabled");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Alias, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_Alias(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DIItIt_Alias, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_IPAddress(char * path_name, char *value)
{
	int ret = 0;
	char is_send_notify[8]={0};
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	executeCMD("cat /var/is_send_notify", is_send_notify);
	tr_log(LOG_DEBUG,"is_send_notify=[%s]",is_send_notify);

	/*ret = do_uci_get(DIItIt_IPAddress, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
	a_infinfo infStatus;
	char *p = parseTemplate(path_name, ".Interface.");
	if( strcmp(is_send_notify,"1") != 0){//for runtime
		if (p == NULL)
			return -1;

		if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
		{
			memset(&infStatus, 0, sizeof(infStatus));
			getLanUbsInterfaceNameWithInstanceNum(p, inf);
			getInterfaceInfo(inf, &infStatus);
		}
		else{
			char wanmode[32] = {0};
			char wantype[32] = {0};
			getWanMode(wanmode);
			if (strcmp(wanmode, "pptp") == 0 || strcmp(wanmode, "l2tp") == 0)
				strcpy(wantype, "wan0");
			else
				strcpy(wantype, "wan");
			memset(&infStatus, 0, sizeof(infStatus));
			getInterfaceInfo(wantype, &infStatus);
		}

		if (infStatus.status == 1)
		{
			strcpy(value, infStatus.ipv4_address);
		}
	}
	else if (strcmp(is_send_notify,"1") == 0){//for send_notify
		char buff[32] = {0};
		if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
		{

			char ucipath[64] = {0};
			char ucipath1[64] = {0};
			char ucipath2[64] = {0};
			char name[64] = {0};
			char tmpvalue[64] = {0};

			getLanUbsInterfaceNameWithInstanceNum(p, name);
			getLanUciPathWithInstanceNum(p, ucipath);
			tr_log(LOG_DEBUG,"###############ucipath===[%s]",ucipath);

			//ucipath==>network.lan%d, name==>lan%d
			do_uci_get(ucipath, tmpvalue);
			//	if(ret)
			//	{
			//		addUciTopNode("network", name);
			//		ret = 0;
			//	}
			tr_log(LOG_DEBUG,"###############tmpvalue===[%s]",tmpvalue);
			sprintf(ucipath1, "%s.proto", ucipath);
			sprintf(ucipath2, "%s.ipaddr", ucipath);

			ret = do_uci_get(ucipath1, buff);
			if(ret)
			{
				strcpy(buff, "static"); //set to static anyway
				ret = 0;
			}
			if(strcmp(buff, "static") == 0)
			{
				ret = do_uci_get(ucipath2, value);
				if(ret)
				{
					return (-1);
				}
			}
		}
		else{
			ret = do_uci_get("network.wan.proto", buff);
			if(ret)
			{
				return -1;
			}
			if(strcmp(buff, "static") == 0)
			{
				ret = do_uci_get("network.wan.ipaddr", value);
				if(ret)
				{
					return (-1);
				}
			}
			else
			{
				char wanmode[32] = {0};
				char wantype[32] = {0};
				getWanMode(wanmode);
				if (strcmp(wanmode, "pptp") == 0 || strcmp(wanmode, "l2tp") == 0)
					strcpy(wantype, "wan0");
				else
					strcpy(wantype, "wan");
				memset(&infStatus, 0, sizeof(infStatus));
				getInterfaceInfo(wantype, &infStatus);
	
				if (infStatus.status == 1)
				{
					strcpy(value, infStatus.ipv4_address);
				}
			}
		}
		system("echo 0 >/var/is_send_notify");
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_IPAddress(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	char buff[32] = {0};
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (strlen(value) > 15)
		return -2;

	if (isValidIP(value) == 0)
		return -2;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
 		char ucipath[64] = {0};
		char ucipath1[64] = {0};
		char ucipath2[64] = {0};
		char name[64] = {0};
		char tmpvalue[64] = {0};

		getLanUbsInterfaceNameWithInstanceNum(p, name);
		getLanUciPathWithInstanceNum(p, ucipath);

		//ucipath==>network.lan%d, name==>lan%d
		ret = do_uci_get(ucipath, tmpvalue);
		if(ret)
		{
			addUciTopNode("network", name);
			ret = 0;
		}

		sprintf(ucipath1, "%s.proto", ucipath);
		sprintf(ucipath2, "%s.ipaddr", ucipath);

		ret = do_uci_get(ucipath1, buff);
		if(ret)
		{
			strcpy(buff, "static"); //set to static anyway
			ret = 0;
		}
		if(strcmp(buff, "static") == 0)
		{
			ret = do_uci_set(ucipath2, value);
			if(ret)
			{
				return (-1);
			}
			else
			{
				ret = do_uci_commit("network");
				if(ret)
				{
					return (-1);
				}
				//ret = 1; //means need to reboot for taking effect
				doRestartLanNetwork();
			}	
		}
	}
	else{
		ret = do_uci_get("network.wan.proto", buff);
		if(ret)
		{
			return -1;
		}
		if(strcmp(buff, "static") == 0)
		{
			ret = do_uci_set("network.wan.ipaddr", value);
			if(ret)
			{
				return (-1);
			}
			else
			{
				ret = do_uci_commit("network");
				if(ret)
				{
					return (-1);
				}
				//ret = 1; //means need to reboot for taking effect
				doRestartNetwork();
			}	
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_SubnetMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_SubnetMask, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char ucipath[64] = {0};
		getLanUciPathWithInstanceNum(p, ucipath);

		sprintf(ucipath, "%s.netmask", ucipath);

		ret = do_uci_get(ucipath, value);
		if(ret)
		{
			strcpy(value, "");
			ret = 0;
		}
	}
	else if(atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		char inf[32] = {0};
		getWanHigherLayerInterface(inf);
		getNetmask(inf, value);
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_SubnetMask(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIItIt_Enable_2237, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	char buff[32] = {0};
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (strlen(value) > 15)
		return -2;

	if (isValidNetmask(value) == 0)
		return -2;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char ucipath[64] = {0};
		char ucipath1[64] = {0};
		char ucipath2[64] = {0};
		char name[64] = {0};
		char tmpvalue[64] = {0};

		getLanUbsInterfaceNameWithInstanceNum(p, name);
		getLanUciPathWithInstanceNum(p, ucipath);

		//ucipath==>network.lan%d, name==>lan%d
		ret = do_uci_get(ucipath, tmpvalue);
		if(ret)
		{
			addUciTopNode("network", name);
			ret = 0;
		}

		sprintf(ucipath1, "%s.proto", ucipath);
		sprintf(ucipath2, "%s.netmask", ucipath);

		ret = do_uci_get(ucipath1, buff);
		if(ret)
		{
			strcpy(buff, "static"); //set to static anyway
			ret = 0;
		}
		if(strcmp(buff, "static") == 0)
		{
			ret = do_uci_set(ucipath2, value);
			if(ret)
			{
				return (-1);
			}
			else
			{
				ret = do_uci_commit("network");
				if(ret)
				{
					return (-1);
				}
				//ret = 1; //means need to reboot for taking effect
				doRestartLanNetwork();
			}	
		}
	}
	else{
		ret = do_uci_get("network.wan.proto", buff);
		if(ret)
		{
			return -1;
		}
		if(strcmp(buff, "static") == 0)
		{
			ret = do_uci_set("network.wan.netmask", value);
			if(ret)
			{
				return (-1);
			}
			else
			{
				ret = do_uci_commit("network");
				if(ret)
				{
					return (-1);
				}
				//ret = 1; //means need to reboot for taking effect
				doRestartNetwork();
			}	
		}
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}

int get_DIItIt_AddressingType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_AddressingType, value);
	if(ret)
	{
		return -1;
	}*/
	char buff[32] = {0};
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		char ucipath[64] = {0};
		getLanUciPathWithInstanceNum(p, ucipath);

		sprintf(ucipath, "%s.proto", ucipath);

		ret = do_uci_get(ucipath, buff);
		if(ret)
		{
			strcpy(buff, "static"); //set to static anyway
			ret = 0;
		}
		if(strcmp(buff, "static") == 0)
			strcpy(value, "Static");
		else
			strcpy(value, "DHCP");
	}
	else{
		a_infinfo wanStatus;
		memset(&wanStatus, 0, sizeof(wanStatus));
		getInterfaceInfo("wan0", &wanStatus);
		if(wanStatus.status == 1){ //for pptp and l2tp
			strcpy(value, "IPCP");
		}
		else{
			memset(&wanStatus, 0, sizeof(wanStatus));
			getInterfaceInfo("wan", &wanStatus);
			if(wanStatus.status == 1){
				if(strcmp(wanStatus.proto, "dhcp") == 0){
					strcpy(value, "DHCP");
				}else if(strcmp(wanStatus.proto, "pppoe") == 0){
					strcpy(value, "IPCP");
				}else
					strcpy(value, "Static");
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_Enable_2237(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Enable_2237, value);
	if(ret)
	{
		return -1;
	}*/
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
	}
	else
		ipv6capable = 1;

	if (ipv6capable == 0)
		strcpy(value, "0");
	else
		sprintf(value, "%d", getIPv6Enable());
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_Enable_2237(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIItIt_Enable_2237, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_Status_2239(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Status_2239, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
	a_infinfo wanStatus;
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
	}
	else
	{
		strcpy(inf, "wan6");
		ipv6capable = 1;
	}

	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo(inf, &wanStatus);
	if (strcmp(wanStatus.ipv6_address, "") == 0)
		strcpy(value, "Disabled");
	else
		strcpy(value, "Enabled");

	if ((ipv6capable == 1) && (strcmp(wanStatus.ipv6_address, "") == 0))
	{
		char active[32] = {0};
		char ipv6addr[128] = {0};
		ret = do_uci_get("ipv6.@global[0].connection_type", active);
		if (ret){
			strcpy(active, "0");
			ret = 0;
		}
		if (atoi(active) != 0){ //for LAN with active and static ipv6 mode, 'ubus call' can't get ipv6 info
			if (atoi(active) == 1) //active mode
				ret = do_uci_get("ipv6.@native[0].ipv6_addr", ipv6addr);
			else //static
				ret = do_uci_get("ipv6.@static[0].lanipv6_addr", ipv6addr);
			if (ret){
				strcpy(ipv6addr, "");
				ret = 0;
			}
			if (strcmp(ipv6addr, "") == 0)
				strcpy(value, "Disabled");
			else
				strcpy(value, "Enabled");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_IPAddressStatus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_IPAddressStatus, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
	a_infinfo wanStatus;
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
	}
	else
	{
		strcpy(inf, "wan6");
		ipv6capable = 1;
	}

	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo(inf, &wanStatus);
	if (strcmp(wanStatus.ipv6_address, "") == 0)
		strcpy(value, "Invalid");
	else
		strcpy(value, "Preferred");

	if ((ipv6capable == 1) && (strcmp(wanStatus.ipv6_address, "") == 0))
	{
		char active[32] = {0};
		char ipv6addr[128] = {0};
		ret = do_uci_get("ipv6.@global[0].connection_type", active);
		if (ret){
			strcpy(active, "0");
			ret = 0;
		}
		if (atoi(active) != 0){ //for LAN with active and static ipv6 mode, 'ubus call' can't get ipv6 info
			if (atoi(active) == 1) //active mode
				ret = do_uci_get("ipv6.@native[0].ipv6_addr", ipv6addr);
			else //static
				ret = do_uci_get("ipv6.@static[0].lanipv6_addr", ipv6addr);
			if (ret){
				strcpy(ipv6addr, "");
				ret = 0;
			}
			if (strcmp(ipv6addr, "") == 0)
				strcpy(value, "Invalid");
			else
				strcpy(value, "Preferred");
		}
	}

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_Alias_2241(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Alias_2241, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_Alias_2241(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DIItIt_Alias_2241, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_IPAddress_2242(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_IPAddress_2242, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
	a_infinfo wanStatus;
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
	}
	else
	{
		strcpy(inf, "wan6");
		ipv6capable = 1;
	}

	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo(inf, &wanStatus);
	strcpy(value, wanStatus.ipv6_address);

	if ((ipv6capable == 1) && (strcmp(wanStatus.ipv6_address, "") == 0))
	{
		char active[32] = {0};
		char ipv6addr[128] = {0};
		ret = do_uci_get("ipv6.@global[0].connection_type", active);
		if (ret){
			strcpy(active, "0");
			ret = 0;
		}
		if (atoi(active) != 0){ //for LAN with active and static ipv6 mode, 'ubus call' can't get ipv6 info
			if (atoi(active) == 1) //active mode
				ret = do_uci_get("ipv6.@native[0].ipv6_addr", value);
			else //static
				ret = do_uci_get("ipv6.@static[0].lanipv6_addr", value);
			if (ret){
				strcpy(value, "");
				ret = 0;
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_IPAddress_2242(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char inf[32] = {0};
	a_infinfo wanStatus;
	char inf_name_path[64] = {0};
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (strlen(value) > 39)
		return -2;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		if (ipv6capable == 1)
			strcpy(inf_name_path, "ipv6.@static[0].lanipv6_addr");
		else
			return (-1);
	}else{
		strcpy(inf_name_path, "ipv6.@static[0].wanipv6_addr");
	}

	strcpy(inf, "wan6"); //all of the lan and wan MUST check the wan6 status
	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo(inf, &wanStatus);
	if (strcmp(wanStatus.proto, "static") != 0)
		return -1;
	
	ret = do_uci_set(inf_name_path, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("ipv6");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doRestartNetwork();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_Origin(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char connection_type[32] = {0};
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	ret = do_uci_get("ipv6.@global[0].connection_type", connection_type);
	if(ret)
	{
		return -1;
	}

	if (atoi(connection_type) == 5){ //static
		strcpy(value, "Static");
	}
	else if (atoi(connection_type) == 1){ //Native
		if (atoi(p) == 1)
			strcpy(value, "AutoConfigured"); //for lan
		else
			strcpy(value, "DHCPv6"); //for wan
	}
	else
		strcpy(value, "WellKnown");
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_Prefix(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Prefix, value);
	if(ret)
	{
		return -1;
	}*/
	char native[32] = {0};
	char dhcp_pd[32] = {0};
	char inf[32] = {0};
	int ipv6capable = 0;
	a_infinfo wanStatus;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
	}
	else
	{
		strcpy(inf, "wan6");
		ipv6capable = 1;
	}

	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo(inf, &wanStatus);
	if (strcmp(wanStatus.ipv6_prefix_address, "") != 0)
		sprintf(value, "Device.IP.Interface.%s.IPv6Prefix.1.Prefix", p);
	else
		strcpy(value, "");

	if (strcmp(value, "") == 0 && ipv6capable == 1){ //double check
		ret = do_uci_get("ipv6.@global[0].connection_type", native);
		if(ret)
		{
			return -1;
		}

		ret = do_uci_get("ipv6.@native[0].dhcp_pd", dhcp_pd);
		if(ret)
		{
			return -1;
		}

		if (atoi(native) == 1 && atoi(dhcp_pd) == 1) //native mode(dhcpv6 client and enable IA_PD)
			sprintf(value, "Device.IP.Interface.%s.IPv6Prefix.1.Prefix", p);
		else if (atoi(native) == 5 && atoi(p) == 1) //static mode
			sprintf(value, "Device.IP.Interface.%s.IPv6Prefix.1.Prefix", p);
		else
			strcpy(value, "");
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_PreferredLifetime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_PreferredLifetime, value);
	if(ret)
	{
		return -1;
	}*/
	char native[32] = {0};
	char lifetime[128] = {0};
	long int lifetime2 = 0;
	char *ptr;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan and network.wan support ipv6, network.lan1 ~ network.lan3 don't support
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM){
		int ipv6capable = 0;
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		if (ipv6capable != 1)
			return 0;
	}

	ret = do_uci_get("ipv6.@global[0].connection_type", native);
	if(ret)
	{
		strcpy(native, "0");
		ret = 0;
	}
	if (atoi(native) != 0)
	{
		if (atoi(native) == 1)
		{
			if(atoi(p) == 5)
			{
				ret = do_uci_get("ipv6.@wan_addr[0].wan_ipv6addres_tr_preferlifetime", value);
			}
			else if(atoi(p) == 1)
			{
				ret = do_uci_get("ipv6.@native[0].pd_preferred_tr_lifetime", value);
			}
		}
		else
		{
			if(atoi(p) == 5)
			{
				ret = do_uci_get("ipv6.@static[0].wan_tr_prefer_lifetime", value);
			}
			else if(atoi(p) == 1)
			{
				ret = do_uci_get("ipv6.@static[0].lan_tr_prefer_lifetime", value);
			}
		}

		if(ret)
		{
			strcpy(value, "0001-01-01T00:00:00Z");
			ret = 0;
		}
	}
	else
		strcpy(value, "0001-01-01T00:00:00Z");

	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_PreferredLifetime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan and network.wan support ipv6, network.lan1 ~ network.lan3 don't support
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM){
		int ipv6capable = 0;
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		if (ipv6capable != 1)
			return 0;
	}

	char native[32] = {0};
	ret = do_uci_get("ipv6.@global[0].connection_type", native);
	if(ret)
	{
		strcpy(native, "0");
		ret = 0;
	}

	if (atoi(native) != 5)// only for static mode
		return -1;
	
	long int currenttimesec = 0;
	long int preferlifetime = 0;
	char tmp[128] = {0};
	currenttimesec = getLocalTimeWithSeconds();
	preferlifetime = changedDateTimeToSeconds(value);
	sprintf(tmp, "%ld", preferlifetime-currenttimesec);

	if(atoi(p) == 5)
	{
		ret = do_uci_set("ipv6.@static[0].wan_tr_prefer_lifetime", value);
		ret = do_uci_set("ipv6.@static[0].wan_preferred_lifetime", tmp);
	}
	else if(atoi(p) == 1)
	{
		ret = do_uci_set("ipv6.@static[0].lan_tr_prefer_lifetime", value);
		ret = do_uci_set("ipv6.@static[0].pd_preferred_lifetime", tmp);
	}

	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("ipv6");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doRestartNetwork();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_ValidLifetime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_ValidLifetime, value);
	if(ret)
	{
		return -1;
	}*/
	char native[32] = {0};
	char lifetime[128] = {0};
	long int lifetime2 = 0;
	char *ptr;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan and network.wan support ipv6, network.lan1 ~ network.lan3 don't support
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM){
		int ipv6capable = 0;
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		if (ipv6capable != 1)
			return 0;
	}

	ret = do_uci_get("ipv6.@global[0].connection_type", native);
	if(ret)
	{
		strcpy(native, "0");
		ret = 0;
	}
	if (atoi(native) != 0)
	{
		if (atoi(native) == 1)
		{
			if(atoi(p) == 5)
			{
				ret = do_uci_get("ipv6.@wan_addr[0].wan_ipv6addres_tr_vaillifetime", value);
			}
			else if(atoi(p) == 1)
			{
				ret = do_uci_get("ipv6.@native[0].pd_valid_tr_lifetime", value);
			}
		}
		else
		{
			if(atoi(p) == 5)
			{
				ret = do_uci_get("ipv6.@static[0].wan_tr_valid_lifetime", value);
			}
			else if(atoi(p) == 1)
			{
				ret = do_uci_get("ipv6.@static[0].lan_tr_valid_lifetime", value);
			}
		}

		if(ret)
		{
			strcpy(value, "0001-01-01T00:00:00Z");
			ret = 0;
		}
	}
	else
		strcpy(value, "0001-01-01T00:00:00Z");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_ValidLifetime(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan and network.wan support ipv6, network.lan1 ~ network.lan3 don't support
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM){
		int ipv6capable = 0;
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		if (ipv6capable != 1)
			return 0;
	}

	char native[32] = {0};

	ret = do_uci_get("ipv6.@global[0].connection_type", native);
	if(ret)
	{
		strcpy(native, "0");
		ret = 0;
	}

	if (atoi(native) != 5)// only for static mode
		return -1;

	long int currenttimesec = 0;
	long int preferlifetime = 0;
	char tmp[128] = {0};
	currenttimesec = getLocalTimeWithSeconds();
	preferlifetime = changedDateTimeToSeconds(value);
	sprintf(tmp, "%ld", preferlifetime-currenttimesec);

	if(atoi(p) == 5)
	{
		ret = do_uci_set("ipv6.@static[0].wan_tr_valid_lifetime", value);
		ret = do_uci_set("ipv6.@static[0].wan_valid_lifetime", tmp);
	}
	else if(atoi(p) == 1)
	{
		ret = do_uci_set("ipv6.@static[0].lan_tr_valid_lifetime", value);
		ret = do_uci_set("ipv6.@static[0].pd_valid_lifetime", tmp);
	}

	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("ipv6");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doRestartNetwork();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_Anycast(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Anycast, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, "0"); //always 0
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_Enable_2248(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Enable_2248, value);
	if(ret)
	{
		return -1;
	}*/
	int ipv6capable = 0;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
	}
	else
		ipv6capable = 1;

	if (ipv6capable == 0)
		strcpy(value, "0");
	else
		sprintf(value, "%d", getIPv6Enable());
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_Enable_2248(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIItIt_Enable_2248, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_Status_2250(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Status_2250, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
	char native[32] = {0};
	int ipv6capable = 0;
	a_infinfo wanStatus;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
	}
	else
	{
		strcpy(inf, "wan6");
		ipv6capable = 1;
	}

	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo(inf, &wanStatus);
	if (strcmp(wanStatus.ipv6_prefix_address, "") == 0)
		strcpy(value, "Disabled");
	else
		strcpy(value, "Enabled");

	if (strcmp(value, "Disabled") == 0 && ipv6capable == 1){ //double check
		ret = do_uci_get("ipv6.@global[0].connection_type", native);
		if(ret)
		{
			strcpy(native, "0");
			ret = 0;
		}
		if (atoi(native) != 0){
			char prefix[128] = {0};
			if (atoi(native) == 1)
				ret = do_uci_get("ipv6.@native[0].ipv6_prefix", prefix);
			else
				ret = do_uci_get("ipv6.@static[0].lanipv6_prefix", prefix);
			if(ret)
			{
				ret = 0;
			}
			if (strcmp(prefix, "") != 0)
				strcpy(value, "Enabled");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_PrefixStatus(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_PrefixStatus, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
	char native[32] = {0};
	int ipv6capable = 0;
	a_infinfo wanStatus;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
	}
	else
	{
		strcpy(inf, "wan6");
		ipv6capable = 1;
	}

	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo(inf, &wanStatus);
	if (strcmp(wanStatus.ipv6_prefix_address, "") == 0)
		strcpy(value, "Invalid");
	else
		strcpy(value, "Preferred");

	if (strcmp(value, "Invalid") == 0 && ipv6capable == 1){ //double check
		ret = do_uci_get("ipv6.@global[0].connection_type", native);
		if(ret)
		{
			strcpy(native, "0");
			ret = 0;
		}
		if (atoi(native) != 0){
			char prefix[128] = {0};
			if (atoi(native) == 1)
				ret = do_uci_get("ipv6.@native[0].ipv6_prefix", prefix);
			else
				ret = do_uci_get("ipv6.@static[0].lanipv6_prefix", prefix);
			if(ret)
			{
				ret = 0;
			}
			if (strcmp(prefix, "") != 0)
				strcpy(value, "Preferred");
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_Alias_2252(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Alias_2252, value);
	if(ret)
	{
		return -1;
	}*/
	ret = 1; //getting from tr.xml
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_Alias_2252(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	if (!isalpha(value[0]))
		return -2;
	
	if (strlen(value) > 64)
		return -2;
	
	/*ret = do_uci_set(DIItIt_Alias_2252, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//set to tr.xml
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_Prefix_2254(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Prefix_2254, value);
	if(ret)
	{
		return -1;
	}*/
	char inf[32] = {0};
	char native[32] = {0};
	int ipv6capable = 0;
	a_infinfo wanStatus;
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;
	
	if (atoi(p) >= IP_LAN_START_INSTANCE_NUM && atoi(p) <= IP_LAN_END_INSTANCE_NUM)
	{
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		getLanUbsInterfaceNameWithInstanceNum(p, inf);
	}
	else
	{
		strcpy(inf, "wan6");
		ipv6capable = 1;
	}

	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo(inf, &wanStatus);
	sprintf(value, "%s/%s", wanStatus.ipv6_prefix_address, wanStatus.ipv6_prefix_mask);

	if (strcmp(wanStatus.ipv6_prefix_address, "") == 0 && ipv6capable == 1){ //double check
		ret = do_uci_get("ipv6.@global[0].connection_type", native);
		if(ret)
		{
			strcpy(native, "0");
			ret = 0;
		}
		if (atoi(native) != 0){
			char prefix[128] = {0};
			char prefix_len[32] = {0};
			if (atoi(native) == 1){
				ret = do_uci_get("ipv6.@native[0].ipv6_prefix", prefix);
				ret = do_uci_get("ipv6.@native[0].prefix_len", prefix_len);
			}
			else{
				ret = do_uci_get("ipv6.@static[0].lanipv6_prefix", prefix);
				ret = do_uci_get("ipv6.@static[0].prefix_len", prefix_len);
			}

			
			if(ret)
			{
				ret = 0;
			}
			if (strcmp(prefix, "") != 0){
				if (strcmp(prefix_len, "") != 0)
					sprintf(value, "%s/%s", prefix, prefix_len);
				else
					sprintf(value, "%s/64", prefix);
			}
		}
	}
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_Prefix_2254(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char inf[32] = {0};
	a_infinfo wanStatus;
	int ipv6capable = 0;
	char inf_name_path[64] = {0};
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (strlen(value) > 39)
		return -2;
	
	if(atoi(p) == 1){
		ipv6capable = getLanIPv6CapableWithInstanceNum(p);
		if (ipv6capable == 1)
			strcpy(inf_name_path, "ipv6.@static[0].lanipv6_prefix");
		else{
			strcpy(value, "");
			return 0;
		}
	}else{
		strcpy(value, ""); //wan no prefix
		return 0;
	}

	strcpy(inf, "wan6"); //all of the lan and wan MUST check the wan6 status

	memset(&wanStatus, 0, sizeof(wanStatus));
	getInterfaceInfo(inf, &wanStatus);
	if (strcmp(wanStatus.proto, "static") != 0)
		return -1;
	
	ret = do_uci_set(inf_name_path, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("ipv6");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doRestartNetwork();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_Origin_2256(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Origin_2256, value);
	if(ret)
	{
		return -1;
	}*/
	char connection_type[32] = {0};
	char dhcp_pd[32] = {0};
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	ret = do_uci_get("ipv6.@global[0].connection_type", connection_type);
	if(ret)
	{
		return -1;
	}

	ret = do_uci_get("ipv6.@native[0].dhcp_pd", dhcp_pd);
	if(ret)
	{
		return -1;
	}

	if (atoi(connection_type) == 5){ //static
		strcpy(value, "Static");
	}
	else if (atoi(connection_type) == 1){ //Native
		if (atoi(dhcp_pd) == 1)
			strcpy(value, "PrefixDelegation");
		else
			strcpy(value, "AutoConfigured"); //for lan
	}
	else
		strcpy(value, "WellKnown");
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int get_DIItIt_StaticType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_StaticType, value);
	if(ret)
	{
		return -1;
	}*/
	char connection_type[32] = {0};
	char dhcp_pd[32] = {0};
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	ret = do_uci_get("ipv6.@global[0].connection_type", connection_type);
	if(ret)
	{
		return -1;
	}

	ret = do_uci_get("ipv6.@native[0].dhcp_pd", dhcp_pd);
	if(ret)
	{
		return -1;
	}

	if (atoi(connection_type) == 5){ //static
		strcpy(value, "Static");
	}
	else if (atoi(connection_type) == 1 && atoi(dhcp_pd) == 1)
		strcpy(value, "PrefixDelegation");
	else
		strcpy(value, "Inapplicable");
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_StaticType(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIItIt_StaticType, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_ParentPrefix(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_ParentPrefix, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //always null
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_ParentPrefix(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIItIt_ParentPrefix, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_ChildPrefixBits(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_ChildPrefixBits, value);
	if(ret)
	{
		return -1;
	}*/
	strcpy(value, ""); //always null
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_ChildPrefixBits(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_set(DIItIt_ChildPrefixBits, value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit(MS);
		if(ret)
		{
			return (-1);
		}
	}*/
	//don't allow to set
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_OnLink(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_OnLink, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) == IP_WAN_INSTANCE_INDEX){ //only check lan
		return 0; 
	}

	ret = do_uci_get("radvd.@prefix[0].AdvOnLink", value);
	if(ret)
	{
		strcpy(value, "1");
		ret = 0;
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_OnLink(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) == IP_WAN_INSTANCE_INDEX){ //only check lan
		return 0; 
	}
	
	ret = do_uci_set("radvd.@prefix[0].AdvOnLink", value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("radvd");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doRadvdRestart();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_Autonomous(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_Autonomous, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) == IP_WAN_INSTANCE_INDEX){ //only check lan
		return 0; 
	}

	ret = do_uci_get("radvd.@prefix[0].AdvAutonomous", value);
	if(ret)
	{
		strcpy(value, "1");
		ret = 0;
	}
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_Autonomous(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	if (atoi(p) == IP_WAN_INSTANCE_INDEX){ //only check lan
		return 0; 
	}
	
	ret = do_uci_set("radvd.@prefix[0].AdvAutonomous", value);
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("radvd");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doRadvdRestart();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_PreferredLifetime_2267(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_PreferredLifetime_2267, value);
	if(ret)
	{
		return -1;
	}*/

	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan support IPv6Prefix, network.lan1 ~ network.lan3 and network.wan don't support
	if (atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		return 0;
	}

	char native[32] = {0};
	ret = do_uci_get("ipv6.@global[0].connection_type", native);
	if(ret)
	{
		strcpy(native, "0");
		ret = 0;
	}
	if (atoi(native) != 0)
	{
		if (atoi(native) == 1)
			ret = do_uci_get("ipv6.@native[0].pd_preferred_tr_lifetime", value);
		else
			ret = do_uci_get("ipv6.@static[0].lan_tr_prefer_lifetime", value);

		if(ret)
		{
			strcpy(value, "0001-01-01T00:00:00Z");
			ret = 0;
		}
	}
	else
		strcpy(value, "0001-01-01T00:00:00Z");
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_PreferredLifetime_2267(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);

	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan support IPv6Prefix, network.lan1 ~ network.lan3 and network.wan don't support
	if (atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		return 0;
	}

	char native[32] = {0};
	ret = do_uci_get("ipv6.@global[0].connection_type", native);
	if(ret)
	{
		strcpy(native, "0");
		ret = 0;
	}

	if (atoi(native) != 5)// only for static mode
		return -1;

	long int currenttimesec = 0;
	long int preferlifetime = 0;
	char tmp[128] = {0};
	currenttimesec = getLocalTimeWithSeconds();
	preferlifetime = changedDateTimeToSeconds(value);
	sprintf(tmp, "%ld", preferlifetime-currenttimesec);

	ret = do_uci_set("ipv6.@static[0].lan_tr_prefer_lifetime", value);
	ret = do_uci_set("ipv6.@static[0].pd_preferred_lifetime", tmp);
	
	if(ret)
	{
		return (-1);
	}
	else
	{
		ret = do_uci_commit("ipv6");
		if(ret)
		{
			return (-1);
		}
		//ret = 1; //means need to reboot for taking effect
		doRestartNetwork();
	}
	tr_log(LOG_DEBUG,"set value [%s]",value);
	return ret;
}
int get_DIItIt_ValidLifetime_2269(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	/*ret = do_uci_get(DIItIt_ValidLifetime_2269, value);
	if(ret)
	{
		return -1;
	}*/
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan support IPv6Prefix, network.lan1 ~ network.lan3 and network.wan don't support
	if (atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		return 0;
	}

	char native[32] = {0};

	ret = do_uci_get("ipv6.@global[0].connection_type", native);
	if(ret)
	{
		strcpy(native, "0");
		ret = 0;
	}
	if (atoi(native) != 0)
	{
		if (atoi(native) == 1)
			ret = do_uci_get("ipv6.@native[0].pd_valid_tr_lifetime", value);
		else
			ret = do_uci_get("ipv6.@static[0].lan_tr_valid_lifetime", value);

		if(ret)
		{
			strcpy(value, "0001-01-01T00:00:00Z");
			ret = 0;
		}
	}
	else
		strcpy(value, "0001-01-01T00:00:00Z");
	
	tr_log(LOG_DEBUG,"get value [%s]",value);
	return ret;
}
int set_DIItIt_ValidLifetime_2269(char * path_name, char *value)
{
	int ret = 0;
	tr_log(LOG_DEBUG,"path_name[%s]",path_name);
	char *p = parseTemplate(path_name, ".Interface.");

	if (p == NULL)
		return -1;

	//only network.lan support IPv6Prefix, network.lan1 ~ network.lan3 and network.wan don't support
	if (atoi(p) == IP_WAN_INSTANCE_INDEX)
	{
		return 0;
	}

	char native[32] = {0};

	ret = do_uci_get("ipv6.@global[0].connection_type", native);
	if(ret)
	{
		strcpy(native, "0");
		ret = 0;
	}

	if (atoi(native) != 5)// only for static mode
		return -1;
	
	long int currenttimesec = 0;
	long int preferlifetime = 0;
	char tmp[128] = {0};
	currenttimesec = getLocalTimeWithSeconds();


	if(ret)
	{