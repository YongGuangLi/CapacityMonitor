#include "LeastSquareSurface.h"


LeastSquareSurface::LeastSquareSurface()
{
}


LeastSquareSurface::~LeastSquareSurface()
{
}

std::string LeastSquareSurface::dtos(double t)
{
	char szBuffer[32] = { 0 };
	sprintf(szBuffer, "%0.4f", t);
	return string(szBuffer);
}

int LeastSquareSurface::Calculation(std::list<INPUT_POINT3D> inputData,Mat& A)
{
	double sigma_x = 0;
	double sigma_y = 0;
	double sigma_z = 0;
	double sigma_x2 = 0;
	double sigma_y2 = 0;
	double sigma_x3 = 0;
	double sigma_y3 = 0;
	double sigma_x4 = 0;
	double sigma_y4 = 0;
	double sigma_x_y = 0;
	double sigma_x_y2 = 0;
	double sigma_x_y3 = 0;
	double sigma_x2_y = 0;
	double sigma_x2_y2 = 0;
	double sigma_x3_y = 0;
	double sigma_z_x2 = 0;
	double sigma_z_y2 = 0;
	double sigma_z_x_y = 0;
	double sigma_z_x = 0;
	double sigma_z_y = 0;
	int n = inputData.size();
	std::list<INPUT_POINT3D>::iterator it = inputData.begin();
	for (; it != inputData.end(); it++)
	{
		sigma_x += it->x;
		sigma_y += it->y;
		sigma_z += it->z;

		sigma_x2 += it->x * it->x;
		sigma_y2 += it->y * it->y;

		sigma_x3 += it->x * it->x * it->x;
		sigma_y3 += it->y * it->y * it->y;

		sigma_x4 += it->x * it->x * it->x * it->x;
		sigma_y4 += it->y * it->y * it->y * it->y;

		sigma_x_y += it->x * it->y;

		sigma_x_y2 += it->x * it->y * it->y;

		sigma_x_y3 += it->x * it->y * it->y * it->y;

		sigma_x2_y += it->x * it->x * it->y;

		sigma_x2_y2 += it->x * it->x * it->y * it->y;

		sigma_x3_y += it->x * it->x * it->x * it->y;

		sigma_z_x2 += it->z * it->x * it->x;

		sigma_z_y2 += it->z * it->y * it->y;

		sigma_z_x_y += it->z * it->x * it->y;

		sigma_z_x += it->z * it->x;

		sigma_z_y += it->z * it->y;
	}
	double Edata[36] = { sigma_x4,    sigma_x2_y2,   sigma_x3_y,     sigma_x3 ,    sigma_x2_y,      sigma_x2,
						 sigma_x2_y2, sigma_y4,      sigma_x_y3,     sigma_x_y2 ,  sigma_y3 ,       sigma_y2 ,
						 sigma_x3_y , sigma_x_y3 ,   sigma_x2_y2 ,   sigma_x2_y,   sigma_x_y2,      sigma_x_y ,
						 sigma_x3 ,   sigma_x_y2 ,   sigma_x2_y,     sigma_x2,     sigma_x_y,       sigma_x,
						 sigma_x2_y , sigma_y3 ,     sigma_x_y2,     sigma_x_y,    sigma_y2 ,       sigma_y ,
						 sigma_x2 ,   sigma_y2,      sigma_x_y,      sigma_x,      sigma_y,         n };
	double Ndata[6] = {  sigma_z_x2 , sigma_z_y2 ,   sigma_z_x_y,    sigma_z_x,    sigma_z_y ,      sigma_z };
	Mat E(6, 6, CV_64F, Edata);
	Mat N(6, 1, CV_64F, Ndata);
	//Mat inv;
	//invert(E, inv);//求矩阵的逆
	//Mat A = inv*N;//计算输出
	A = E.inv()*N;//计算输出
	return 0;		 
}



int LeastSquareSurface::ReturnPoint(std::list<INPUT_POINT3D> inputData, std::list<INPUT_POINT3D> &out_point, std::string &strFormula, bool bflag)
{
	char szBuffer[512] = { 0 };
	double y = 0;
	Mat A;
	Calculation(inputData, A);

	double a_ = A.at<double>(0, 0);
	double b_ = A.at<double>(1, 0);
	double c_ = A.at<double>(2, 0);
	double d_ = A.at<double>(3, 0);
	double e_ = A.at<double>(4, 0);
	double f_ = A.at<double>(5, 0);
	if (bflag == true)
	{
		memset(szBuffer, 0, 512);
		//sprintf(szBuffer, "z=%.6f*x^2 + %.6f*y^2 + %.6f*xy + %.6f*x + %.6f*y + %.6f", a_, b_, c_, d_, e_, f_);
		sprintf(szBuffer, "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f", a_, b_, c_, d_, e_, f_);
		strFormula = szBuffer;
		return 0;
	}

	if (bflag == false)
	{
		INPUT_POINT3D info;
		std::list<INPUT_POINT3D>::iterator it = inputData.begin();
		for (; it != inputData.end(); it++)
		{
			info.x = it->x;
			info.y = it->y;
			info.z  =	a_ * it->x * it->x + b_ * it->y * it->y + c_ * it->x * it->y + d_ * it->x + e_ * it->y + f_;
			//info.z = a_ * it->x  + b_ * it->y  + c_ ;
			printf("data.push([%f, %f, %f]);\n", info.x, info.y, info.z);
			out_point.push_back(info);
		}
	}

	double max_xpod = (2 * b_*d_ - c_*e_) / (c_*c_ - 4 * a_*b_);
	double max_ypod = (2 * a_*e_ - d_*c_) / (c_*c_ - 4 * a_*b_);

	Point result;
	result.x = max_xpod;
	result.y = max_ypod;
	
	return 0;
}
