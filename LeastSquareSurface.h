#pragma once
#include<opencv2\opencv.hpp>
#include <opencv2\imgproc\imgproc_c.h>
#include <vector>
using namespace cv;
using namespace std;


typedef struct INPUT_POINT3D
{
	double x;
	double y;
	double z;
	INPUT_POINT3D()
	{
		x = 0;
		y = 0;
		z = 0;
	}
};
class LeastSquareSurface
{
public:
	LeastSquareSurface();
	~LeastSquareSurface();
	/*
	double 转换成 string
	*/
	std::string dtos(double t);
	int Calculation(std::list<INPUT_POINT3D> inputData, Mat& A);
public:
	/*************************
	inputData     数据要计算的原数据。
	out_point     经过计算数据数据。
	bflag	      输出数据类型的标志；
				  true 表示输出公式；
				  false 表示输出计算后的数据。
	strFormula    输出计算后的阶层公式。
	return        -1 执行失败， 0 执行成功。
				  返回参数顺序 a, b, c, d, e, f
	**************************/
	int ReturnPoint(std::list<INPUT_POINT3D> inputData, std::list<INPUT_POINT3D> &out_point, std::string &strFormula, bool bflag = true);
};

