
#define GL_GLEXT_PROTOTYPES

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <GL/glut.h>
#include <cuda_gl_interop.h>
#include <book.h>
#include <cpu_bitmap.h>


#define DIM 512

/*
공유 데이터 핸들
*/
GLuint bufferObj;
cudaGraphicsResource *resource;

__global__ void kernal(uchar4 * ptr)
{

}

void draw()
{


	glutSwapBuffers();
}

void key(unsigned char key, int x, int y)
{

}


int main(int argc, char **argv)
{
	cudaDeviceProp prop;	//cuda device
	int dev;	//cuda device 식별값
	
	/*
	cuda device 멤버 값을 0으로 초기화한 후 주 버전을 1, 부 버전을 0으로 설정
	*/
	memset(&prop, 0, sizeof(cudaDeviceProp));
	prop.major = 1;
	prop.minor = 0;
	

	HANDLE_ERROR(cudaChooseDevice(&dev, &prop));
	HANDLE_ERROR(cudaGLSetGLDevice(dev));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(DIM, DIM);
	glutCreateWindow("bitmap");
	
	glGenBuffers(1, &bufferObj);	//버퍼 핸들 생성
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, bufferObj);	//핸들을 픽셀 버퍼에 바인딩

	/*
	OpenGL 드라이버에게 버퍼를 할당하도록 요청.
	DIM * DIM 크기의 32 비트 데이터 버퍼를 생성함.
	버퍼는 런타임 중 여러차례 수정되므로 GL_DYNAMIC_DRAW_ARB 의 패턴을 따른다.
	*/
	glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, DIM * DIM * 4, NULL, GL_DYNAMIC_DRAW_ARB);
	
	/*
	OpenGL과 CUDA 양족에서 PBO를 사용할 것임을 CUDA 런타임에 명시.
	resource에 버퍼를 가리키는 CUDA 전용 핸들이 기록됨.
	버퍼의 접근 패턴은 오직 읽기 또는 오직 쓰기 전용이 아닌 None.
	*/
	HANDLE_ERROR(cudaGraphicsGLRegisterBuffer(&resource, bufferObj, cudaGraphicsMapFlagsNone));

	/*
	커널에 전달될 디바이스 메모리의 실제 주소 생성
	*/
	uchar4* devPtr;		//x y z w
	size_t size;
	HANDLE_ERROR(cudaGraphicsMapResources(1, &resource, NULL));
	HANDLE_ERROR(cudaGraphicsResourceGetMappedPointer((void**)devPtr, &size, resource));

	dim3 grids(DIM / 16, DIM / 16);
	dim3 threads(16, 16);
	kernal << <grids, threads >> > (devPtr);

	HANDLE_ERROR(cudaGraphicsUnmapResources(1, &resource, NULL));

	glutKeyboardFunc(key);
	glutDisplayFunc(draw);
	glutMainLoop();
    return 0;
}
