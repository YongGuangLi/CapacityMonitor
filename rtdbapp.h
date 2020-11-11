#include <string.h>
#include <map>
#include "VarDef.h"
#include "RtdbFun.h"
using namespace std;

struct CDPiontInfo 
{
	std::string			m_strSetCode;			//机组编号
	std::string			m_strPointCode;			//测点编号
	std::string			m_strSourceID;			//实时数据库测点ID
	
	double				m_dCurrenVale;			//测点值
	long				m_lStatus;			   //点状态
	long				lTimeStamp;			  //时间戳
};


struct CMonmentDatas
{
	long				m_lTimeStamp;				  //时间片标记
	int										m_iNum;						 //时间顺序编号
	std::map<std::string,CDPiontInfo>		m_mapPointInfo;				//该时间片所有点值
};

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
//void SaveToMap(Tag* pTagBuf,int iNum,std::string szPoint,std::string szSouceId);

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
//bool GetHisValue(long startTime,long endTime,std::map<std::string,std::string> m_mapDPiontName,int iStep);
