#pragma once
#include<opencv2\opencv.hpp>
#include <opencv2\imgproc\imgproc_c.h>
#include <vector>
using namespace cv;
using namespace std;

typedef struct INPUT_POINT
{
	double x;
	double y;
	INPUT_POINT()
	{
		x = 0;
		y = 0;
	}
};

class LeastSquaresCurve
{
public:
	LeastSquaresCurve();
	~LeastSquaresCurve();
private:
	/*
	计算多项式中的 K 的值	
	*/
	Mat polyfit(vector<Point2d> in_point, int n);
	/*
	double 转换成 string
	*/
	std::string LeastSquaresCurve::dtos(double t);

	/*
	int 转换成 string
	*/
	std::string LeastSquaresCurve::itos(int t);


public:
	/*************************
	inputData     数据要计算的原数据。
	out_point     经过计算数据数据。
	bflag	      输出数据类型的标志；
			      true 表示输出公式；
			      false 表示输出计算后的数据。
	strFormula    输出计算后的阶层公式。
	n		      多项式的阶数 一般使用 3，5，7，9。 下面公式和 n 相关。
	              公式： y = k0 + k1 * X^1 + k2 * X^2 + k3 * X^3 + k4 * X^4 + k5 * X^5

	return        -1 执行失败， 0 执行成功。
	**************************/
	int ReturnPoint(std::list<INPUT_POINT> inputData, std::list<INPUT_POINT> &out_point,std::string &strFormula,bool bflag = true, int n = 2);
};

