/*
cuda tutorial 5 ray tracing 예제의
c++ version
*/

#include <stdio.h>
#include <cmath>
#include <cpu_bitmap.h>
#include <ctime>
#include <chrono>

#define rnd(x) (x*rand()/RAND_MAX)
#define SPHERES 40
#define INF 2e10f
#define DIM 1024

using namespace std::chrono;

struct Sphere
{
	/*
	구의 속성값
	*/
	float r, g, b;
	float radius;
	float x, y, z;

	/*
	projective view
	카메라의 위치는 +INF
	(ox, oy)에 위치한 픽셀에서 발사한 광선이 구와 충돌하는지 판별. 충돌시 그 깊이값 반환
	*/
	float hit(float ox, float oy, float *n)
	{
		float dx = ox - x;
		float dy = oy - y;
		if (radius * radius > dx*dx + dy * dy)
		{
			float dz = sqrtf(radius*radius - dx * dx - dy * dy);
			*n = dz / sqrtf(radius*radius); //음영을 부여하기 위한 scale 값. 구의 중심에서 멀어질수록 작아짐
			return dz + z; //깊이값을 반환
		}
		return -INF;
	}
};

void kernel(unsigned char *ptr, Sphere *s, int x, int y)
{
	int offset = x + y * 1024;

	/*
	z축이 화면의 원점에 오도록 함
	*/
	float ox = (x - DIM / 2);
	float oy = (y - DIM / 2);
	
	/*
	각 스레드(픽셀)마다 모든 구와의 충돌 검사
	더 가까운 구로 픽셀 데이터 갱신
	*/
	float r = 0, g = 0, b = 0;
	float maxz = -INF;
	for (int i = 0; i < SPHERES; i++)
	{
		float n = 0;
		float t = s[i].hit(ox, oy, &n);
		if (t > maxz)
		{
			float scale = n;
			r = s[i].r*scale;
			g = s[i].g*scale;
			b = s[i].b*scale;
			maxz = t;
		}
	}
	ptr[offset * 4 + 0] = (int)(r * 255);
	ptr[offset * 4 + 1] = (int)(g * 255);
	ptr[offset * 4 + 2] = (int)(b * 255);
	ptr[offset * 4 + 3] = 255;
}

int main()
{
	auto start = high_resolution_clock::now();

	srand(time(NULL));

	CPUBitmap bitmap(DIM, DIM);
	Sphere *s;

	s = (Sphere*)malloc(sizeof(Sphere)*SPHERES);
	for (int i = 0; i < SPHERES; i++)
	{
		s[i].r = rnd(1.0f);
		s[i].g = rnd(1.0f);
		s[i].b = rnd(1.0f);
		s[i].x = rnd(1000.0f) - 500;
		s[i].y = rnd(1000.0f) - 500;
		s[i].z = rnd(1000.0f) - 500;
		s[i].radius = rnd(100.0f) + 20;
	}

	for (int x = 0; x < DIM; x++)
		for (int y = 0; y < DIM; y++)
		{
			kernel(bitmap.get_ptr(), s, x, y);
		}

	free(s);

	auto stop = high_resolution_clock::now();
	duration<double> elapsedTime = stop - start;

	printf("Time to generate : %3.3f ms\n", elapsedTime.count()*1000);
	bitmap.display_and_exit();

    return 0;
}

