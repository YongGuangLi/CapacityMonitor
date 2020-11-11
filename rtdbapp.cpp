#include <string.h>
#include <map>
#include "rtdbapp.h"



std::map<long,CMonmentDatas> m_mapMonmentDatas;

/*----------------------------------------------------------------------------*/
//	函数名称:	SaveToMap
//	功能描述:	从实时数据库中取的历史值存入内存
//	输入参数:	
//				Tag* pTagBuf			历史点值结构
//				int iNum				点值个数
//				std::string szPoint			测点关系数据库编号
//				std::string szSouceId		测点实时数据库编号
//	返回值:		
//				void
/*----------------------------------------------------------------------------*/
//void SaveToMap(Tag* pTagBuf,int iNum,std::string szPoint,std::string szSouceId)
//{
//	/////////////////////////装载前清空
//	//ClearMonmentDatas();
//	/////////////////////////装载前清空

//	int i = 0;
//	long lTime = 0;
//	std::string strPointID = "";
//	std::string strInfo = "";
//	std::map<long,CMonmentDatas>::iterator itr;
//    CMonmentDatas monmentDatas;
//    CDPiontInfo pointInfo;
//	for (i=0; i<iNum; i++)
//	{
//		lTime = pTagBuf[i].lTimeStamp;
//		//内存中没找到
//		itr  = m_mapMonmentDatas.find(lTime);
//		if (itr!=m_mapMonmentDatas.end())
//        {
//                m_mapMonmentDatas.insert(make_pair(lTime,monmentDatas));
//                monmentDatas.m_iNum = i;

//		}
//		else
//		{
//            monmentDatas  = itr->second;
//		}

//		//新加时间
//        monmentDatas.m_lTimeStamp = lTime;
//		//新加时间

//		strPointID = szPoint;
//		std::map<std::string,CDPiontInfo>::iterator iter;
//        iter  = monmentDatas.m_mapPointInfo.find(szPoint);
//        if (iter!=monmentDatas.m_mapPointInfo.end())
//		{
//            pointInfo = iter->second;
//		}

//        pointInfo.m_dCurrenVale=pTagBuf[i].fValue;
//        pointInfo.m_lStatus=pTagBuf[i].lState;
//        pointInfo.lTimeStamp=pTagBuf[i].lTimeStamp;

//        monmentDatas.m_mapPointInfo[strPointID] = pointInfo;
//    }
//}

/*----------------------------------------------------------------------------*/
//	函数名称:	GetDValue
//	功能描述:	获取一段时间段内实时数据库D点的值保存在内存中
//	输入参数:	
//				long startTime         开始时间
//				long endTime           结束时间
//				std::map<std::string,std::string> m_mapDPiontName 测点关系数据库和实时数据库点名集合
//				int iStep  取数步长
//	返回值:		
//				bool
/*----------------------------------------------------------------------------*/
//bool GetHisValue(long startTime,long endTime,std::map<std::string,std::string> m_mapDPiontName,int iStep)
//{
//	bool bReturn;
//	std::map<std::string,std::string>::iterator itr;
//	std::string strPoint,strSouceId;
//	int m_pTagsNum = (endTime - startTime)/60+1;
//	Tag* m_paTags;
//	m_paTags = (Tag *)malloc(sizeof(Tag)*m_pTagsNum);
//    int iCount  = 0;

//	itr = m_mapDPiontName.begin();
//	for (;itr!=m_mapDPiontName.end();itr++)
//	{
//		strPoint = itr->first;
//		strSouceId = itr->second;

//		rtdb_GetHisValue(strSouceId.c_str(),startTime,endTime,iStep,0,0,0,NULL,&iCount);//统计时间段内点个数
//		memset(m_paTags,0,sizeof(Tag)*m_pTagsNum);
//		bReturn=rtdb_GetHisValue(strSouceId.c_str(),startTime,endTime,iStep,0,0,0,m_paTags,&iCount);//取出时间段内所有点值存入pTag
//		if (!bReturn)
//		{
//			continue;
//		}
//	}
//	if (m_paTags != NULL)
//	{
//		free(m_paTags);
//		m_paTags = NULL;
//	}
//	return true;
//}
