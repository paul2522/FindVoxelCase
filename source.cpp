//마칭 큐브 케이스 분류 프로그램
//Marching Cube Case Classification Program
//경북대학교 컴퓨터 학부 김구진, 김용훈
//2021.09.15

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <windows.h>


typedef struct point {
	double x;
	double y;
	double z;
}Point;

typedef struct voxel {
	int edgePassNum;		//복셀에서 edge가 mesh와 만나는 개수
	int edgePassSum;		//복셀에서 edge가 mesh와 만나는 위치를 0~11까지 하여 2진수로 만든 뒤 10진수로 표현
	int b;					//복셀의 행 값 = row
	int c;					//복셀의 열 값 = col
	int h;					//복셀의 높이 값 = height
	int vertexInNum;		//복셀에서 mesh에 포함되는 vertex의 개수
	int vertexInSum;		//복셀에서 mesh에 포함되는 vertex의 위치를 2진수로 만든 뒤 10진수로 표현
	int voxelCase = 0;		//해당이 복셀이 무슨 Case인지 나타내는 값
	int voxelIndex;			//해당 복셀의 index(번호)
	int vError = 0;			//해당 복셀의 Skinny Triangle 유무
	int vErrorFaceNum = 0;	//해당 복셀의 Skinny Triangle 개수
	int vFaceNum = 0;		//해당 복셀의 face 개수
	int startIndex = -1;	//faceList에서 n번째 index부터 faceNum 개수만큼까지가 해당 복셀의 영역
}Voxel;

typedef struct vertexset {
	Point P;
	int index;
	int use = 1;
}vertexSet;

typedef struct faceset {
	int v1;
	int v2;
	int v3;
	int voxelIndex;
	int isErrorFace;
	int use = 1;
}faceSet;


//FILE* fpEdgePassCheckFile1 = fopen("1step_emd_1003_EdgePassCheck1.txt", "r");//Data 파일
FILE* fpEdgePassCheckFile1;//Data 파일
//파일 형식 7개의 data
//edgePassNum, edgePassSum, b, c, h, vertexInNum, vertexInSum
//edgePassNum	:	복셀에서 edge가 mesh와 만나는 개수
//edgePassSum	:	복셀에서 edge가 mesh와 만나는 위치를 0~11까지 하여 2진수로 만든 뒤 10진수로 표현
//b,c,h			:	복셀의 행, 열, 위치 정보 = row, col, height
//vertexInNum	:	복셀에서 mesh에 포함되는 vertex의 개수
//vertexInSum	:	복셀에서 mesh에 포함되는 vertex의 위치를 2진수로 만든 뒤 10진수로 표현
//voxelIndex	:	해당 복셀의 index(번호)

//FILE* fpVOFF = fopen("V_1step_emd_1003.OFF", "r");
FILE* fpVOFF;
//vertexNum FaceNum 0
//x, y, z...
//3 v1 v2 v3 voxelNum isErrorFace
//voxelIndex: 위치하는 복셀의 index(0~)

FILE* fpVoxelCase = fopen("VoxelCase.txt", "w");

double cl = 82.8;
double half_cl = cl / 2;

int main(void)
{
	int edgePassNum = 0, edgePassSum = 0, valVoxelNum = 0;
	int b = 0, c = 0, h = 0;
	int vertexNum, faceNum, zero, three;
	int errorIndex = 0;								//추가되는 vertex의 개수
	int errorNum = 0;
	int A[12];
	int n1 = 1;
	int n2 = 3;
	double vertexX, vertexY, vertexZ;
	char OFFName[40] = "";
	char EdgePassName[40] = "";

	printf("마칭큐브를 활용하여 만든 OFF파일과 복셀의 정보가 있는 파일 2개가 필요합니다.\n");
	printf("OFF파일의 이름을 입력하세요\n");
	scanf("%s", OFFName);

	printf("EdgePass 정보가 든 파일의 이름을 입력하세요\n");
	scanf("%s", EdgePassName);
	strcat(OFFName, ".OFF");
	strcat(EdgePassName, ".txt");
	fpVOFF = fopen(OFFName, "r");
	if (fpVOFF == NULL)
	{
		printf("오류 : 해당 OFF파일이 존재하지않습니다.\n");
		system("pause");
		return 0;
	}
	fpEdgePassCheckFile1 = fopen(EdgePassName, "r");
	if (fpEdgePassCheckFile1 == NULL)
	{
		printf("오류 : 해당 복셀 정보 파일이 존재하지않습니다.\n");
		system("pause");
		return 0;
	}

	fscanf(fpVOFF, "%d %d %d %d", &vertexNum, &faceNum, &zero, &valVoxelNum);

	Voxel* Vo = new Voxel[valVoxelNum];
	vertexSet* Vs = new vertexSet[vertexNum];
	faceSet* Fs = new faceSet[faceNum];
	vertexSet* newVs = new vertexSet[vertexNum];
	faceSet* newFs = new faceSet[faceNum];

	//EdgePassCheckFile1 : edgePassNum, edgePassSum, b, c, h, vertexInNum, vertexInSum
	for (int i = 0; i < valVoxelNum; i++)
	{
		fscanf(fpEdgePassCheckFile1, "%d %d %d %d %d %d %d", &Vo[i].edgePassNum, &Vo[i].edgePassSum, &Vo[i].b, &Vo[i].c, &Vo[i].h, &Vo[i].vertexInNum, &Vo[i].vertexInSum);
		Vo[i].voxelIndex = i;
	}

	for (int i = 0; i < vertexNum; i++)
	{
		fscanf(fpVOFF, "%lf %lf %lf", &Vs[i].P.x, &Vs[i].P.y, &Vs[i].P.z);
		Vs[i].index = i;

		newVs[i].P.x = Vs[i].P.x;
		newVs[i].P.y = Vs[i].P.y;
		newVs[i].P.z = Vs[i].P.z;
		newVs[i].index = i;
	}

	for (int i = 0; i < faceNum; i++)
	{
		fscanf(fpVOFF, "%d %d %d %d %d %d", &three, &Fs[i].v1, &Fs[i].v2, &Fs[i].v3, &Fs[i].voxelIndex, &Fs[i].isErrorFace);
		if (Fs[i].isErrorFace == 1)
		{
			Vo[Fs[i].voxelIndex].vErrorFaceNum++;
			Vo[Fs[i].voxelIndex].vError = 1;
		}
		if (Vo[Fs[i].voxelIndex].startIndex == -1)
		{
			Vo[Fs[i].voxelIndex].startIndex = i;
		}
		Vo[Fs[i].voxelIndex].vFaceNum++;

		newFs[i].v1 = Fs[i].v1;
		newFs[i].v2 = Fs[i].v2;
		newFs[i].v3 = Fs[i].v3;
		newFs[i].voxelIndex = Fs[i].voxelIndex;
		newFs[i].isErrorFace = Fs[i].isErrorFace;
	}

	//분류 과정
	for (int i = 0; i < valVoxelNum; i++)
	{
		if (Vo[i].edgePassNum == 3)
		{
			Vo[i].voxelCase = 1;
		}
		else if (Vo[i].edgePassNum == 5)
		{
			Vo[i].voxelCase = 4;
		}
		else if (Vo[i].edgePassNum == 7)
		{
			Vo[i].voxelCase = 11;
		}
		else if (Vo[i].edgePassNum == 9)
		{
			Vo[i].voxelCase = 12;
		}
		else if (Vo[i].edgePassNum == 12)
		{
			Vo[i].voxelCase = 7;
		}
		else if (Vo[i].edgePassNum == 4)
		{
			if (Vo[i].edgePassSum == 85 || Vo[i].edgePassSum == 170 || Vo[i].edgePassSum == 3840)
			{
				Vo[i].voxelCase = 5;
			}
			else
			{
				Vo[i].voxelCase = 2;
			}
		}
		else if (Vo[i].edgePassNum == 8)//case
		{
			if (Vo[i].edgePassSum == 255 || Vo[i].edgePassSum == 3925 || Vo[i].edgePassSum == 4010)
			{
				Vo[i].voxelCase = 13;
			}
			else//추가되면 확실히 가능
			{
				Vo[i].voxelCase = 6;
			}
		}
		else if (Vo[i].edgePassNum == 6)//case 3, 10 / 8, 9, 14
		{
			if (Vo[i].vertexInNum == 2 || Vo[i].vertexInNum == 6)//case 3, 10  (index 2개 또는 reverse로 6개)
			{
				if (Vo[i].vertexInSum == 20 || Vo[i].vertexInSum == 40 || Vo[i].vertexInSum == 65 || Vo[i].vertexInSum == 125\
					|| Vo[i].vertexInSum == 130 || Vo[i].vertexInSum == 190 || Vo[i].vertexInSum == 215 || Vo[i].vertexInSum == 235)
				{
					Vo[i].voxelCase = 3;
				}
				else//추가되면 확실히 가능
				{
					Vo[i].voxelCase = 10;
				}
			}
			else if (Vo[i].vertexInNum == 4)
			{
				if (Vo[i].vertexInSum == 27 || Vo[i].vertexInSum == 39 || Vo[i].vertexInSum == 78 || Vo[i].vertexInSum == 114\
					|| Vo[i].vertexInSum == 141 || Vo[i].vertexInSum == 177 || Vo[i].vertexInSum == 216 || Vo[i].vertexInSum == 228)
				{
					Vo[i].voxelCase = 8;
				}
				else if (Vo[i].vertexInSum == 23 || Vo[i].vertexInSum == 46 || Vo[i].vertexInSum == 57 || Vo[i].vertexInSum == 77\
					|| Vo[i].vertexInSum == 99 || Vo[i].vertexInSum == 116 || Vo[i].vertexInSum == 139 || Vo[i].vertexInSum == 156\
					|| Vo[i].vertexInSum == 178 || Vo[i].vertexInSum == 198 || Vo[i].vertexInSum == 209 || Vo[i].vertexInSum == 232)
				{
					Vo[i].voxelCase = 9;
				}
				else//추가되면 확실히 가능
				{
					Vo[i].voxelCase = 14;
				}
			}
		}
		else
		{
			printf("edgePassNum 오류 확인됨\n");
		}

		//오류 확인 및 출력
		if (Vo[i].voxelCase == 0)
		{
			printf("voxelCase 미분류 확인됨\n");
		}
		if (Vo[i].edgePassNum < 3 || Vo[i].edgePassNum > 9 && Vo[i].edgePassNum != 12)
		{
			printf("edgePassNum 오류 확인됨 : edgePassNum : %d, edgePassSum : %d, Case : %d\n", Vo[i].edgePassNum, Vo[i].edgePassSum, Vo[i].voxelCase);
		}
		if (Vo[i].edgePassSum < 7 || Vo[i].edgePassSum > 4095)//000000000111 ~ 111111111111 정상
		{
			printf("edgePassSum 오류 확인됨 : edgePassNum : %d, edgePassSum : %d, Case : %d\n", Vo[i].edgePassNum, Vo[i].edgePassSum, Vo[i].voxelCase);
		}
		if (Vo[i].vertexInNum < 1 || Vo[i].vertexInNum > 7)//1~7 정상
		{
			printf("vertexInNum 오류 확인됨 : vertexInNum : %d, vertexInSum : %d, Case : %d\n", Vo[i].vertexInNum, Vo[i].vertexInSum, Vo[i].voxelCase);
		}
		if (Vo[i].vertexInSum < 1 || Vo[i].vertexInSum > 254)//00000001 ~ 11111110 정상
		{
			printf("vertexInNum 오류 확인됨 : vertexInNum : %d, vertexInSum : %d, Case : %d\n", Vo[i].vertexInNum, Vo[i].vertexInSum, Vo[i].voxelCase);
		}
		if (Vo[i].vError == 1)
		{
			//fprintf(fpErrorFace, "ErrorFace를 가지는 %6d번째 Case %2d 복셀의 위치 %3d %3d %3d, face %2d개인 Case %2d \n", Vo[i].voxelIndex, Vo[i].voxelCase, Vo[i].b, Vo[i].c, Vo[i].h, Vo[i].voxelCase, Vo[i].vFaceNum);
		}
		fprintf(fpVoxelCase, "Index %3d : Case %2d\n", Vo[i].voxelIndex, Vo[i].voxelCase);
	}

	//입력시 출력용
	//예시 index sample 복셀과 해당 복셀 내부 Face 출력
	int indexSample = 0;
	int sample = 0;

	printf("몇번째 복셀의 정보를 알고 싶습니까?\n");
	scanf("%d", &sample);

	printf("index%d 복셀은 Case%2d입니다.\n", sample, Vo[sample].voxelCase);
	printf("세부정보\n위치 : b = %d, c = %d, h = %d\nedgePassNum = %d, edgePassSum = %d, vertexInNum = %d, vertexInSum = %d\n", Vo[sample].b, Vo[sample].c, Vo[sample].h, Vo[sample].edgePassNum, Vo[sample].edgePassSum, Vo[sample].vertexInNum, Vo[sample].vertexInSum);
	//printf("vError = %d, vfaceNum = %d\n", Vo[sample].vError, Vo[sample].vFaceNum);
	printf("\n해당 복셀에 포함된 face의 정보\n");
	printf("시작 face index = %d\n", Vo[sample].startIndex);
	indexSample = Vo[sample].startIndex;
	for (int i = 0; i < Vo[sample].vFaceNum; i++)
	{
		printf("%d번째 face index %d %d %d\n", indexSample, Fs[indexSample].v1, Fs[indexSample].v2, Fs[indexSample].v3);
		printf("좌표1 : %lf %lf %lf\n", Vs[Fs[indexSample].v1].P.x, Vs[Fs[indexSample].v1].P.y, Vs[Fs[indexSample].v1].P.z);
		printf("좌표2 : %lf %lf %lf\n", Vs[Fs[indexSample].v2].P.x, Vs[Fs[indexSample].v2].P.y, Vs[Fs[indexSample].v2].P.z);
		printf("좌표3 : %lf %lf %lf\n", Vs[Fs[indexSample].v3].P.x, Vs[Fs[indexSample].v3].P.y, Vs[Fs[indexSample].v3].P.z);
		indexSample++;
	}
	printf("\n");

	delete[] Vo;
	delete[] Vs;
	delete[] Fs;

	fclose(fpVOFF);
	fclose(fpEdgePassCheckFile1);
	fclose(fpVoxelCase);

	system("pause");

	return 0;
}
