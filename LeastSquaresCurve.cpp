#include "LeastSquaresCurve.h"


LeastSquaresCurve::LeastSquaresCurve()
{
}


LeastSquaresCurve::~LeastSquaresCurve()
{
}

std::string LeastSquaresCurve::dtos(double t)
{
	char szBuffer[32] = { 0 };
	sprintf(szBuffer, "%0.4f", t);
	return string(szBuffer);
}
std::string LeastSquaresCurve::itos(int t)
{
	char szBuffer[2] = { 0 };
	sprintf(szBuffer, "%d", t);
	return string(szBuffer);
}
Mat LeastSquaresCurve::polyfit(vector<Point2d> in_point, int n)
{
	int size = (int)in_point.size();
	//所求未知数个数
	int x_num = n + 1;
	//构造矩阵U和Y
	Mat mat_u(size, x_num, CV_64F);
	Mat mat_y(size, 1, CV_64F);
	for (int i = 0; i < mat_u.rows; ++i)
	{ 
		for (int j = 0; j < mat_u.cols; ++j)
		{
			mat_u.at<double>(i, j) = pow(in_point[i].x, j);
			//printf("i = %d== j = %d ===:%f\n", i, j, pow(in_point[i].x, j));
		}
	}
	
	for (int i = 0; i < mat_y.rows; ++i)
	{
		mat_y.at<double>(i, 0) = in_point[i].y;
	}
	
	//矩阵运算，获得系数矩阵K
	Mat mat_k(x_num, 1, CV_64F);
	mat_k = (mat_u.t()*mat_u).inv()*mat_u.t()*mat_y;
	//cout << mat_k << endl;
	return mat_k;
}

std::string outPowString(int j)
{
	std::string strRet = " ";
	for (int i = 0; i < j; i++)
	{
		strRet += "*X";
	}
	return strRet;
}

int  LeastSquaresCurve::ReturnPoint(std::list<INPUT_POINT> input_Data, std::list<INPUT_POINT> &out_point, std::string &strFormula, bool bflag , int n)
{
	if ((int)input_Data.size() <= 0)
	{
		return -1;
	}
	int i = 0;
	vector<Point2d> inPoint;
	//cv::Point *in = new cv::Point[input_Data.size()]();
	std::list<INPUT_POINT>::iterator it = input_Data.begin();
	for (; it  != input_Data.end(); it++)
	{
		Point2d oneInfo;
		oneInfo.x = it->x;
		oneInfo.y = it->y;
		inPoint.push_back(oneInfo);
	}
	Mat mat_k = polyfit(inPoint,n);


	if (bflag == true)
	{
		//strFormula += "y = ";
		//for (int j = 0; j < n + 1; ++j)
		//{
		//	strFormula += dtos(mat_k.at<double>(j, 0)) + "x^" + itos(j); //ipt.y += mat_k.at<double>(j, 0)*pow(it->x, j);
		//	if (j < n)
		//	{
		//		strFormula += " + ";
		//	}
		//}
		strFormula =  "";
		for (int j = 0; j < n + 1; ++j)
		{
			if (strFormula.length() > 0)
				strFormula += ",";
			strFormula += dtos(mat_k.at<double>(j, 0));	
		}
		return 0;
	}

	if (bflag == false)
	{
		//画出拟合曲线
		it = input_Data.begin();
		for (; it != input_Data.end(); it++)
		{
			Point2d ipt;
			ipt.x = it->x;
			ipt.y = 0;
			for (int j = 0; j < n + 1; ++j)
			{
				ipt.y += mat_k.at<double>(j, 0)*pow(it->x, j);
			}
			INPUT_POINT info;
			info.x = ipt.x;
			info.y = ipt.y;
			out_point.push_back(info);
		}
		return 0;
	}
	return -1;
}