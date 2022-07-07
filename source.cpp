#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

typedef struct point {
	double x = 0;
	double y = 0;
	double z = 0;
}Point;

typedef struct voxel {
	int edgePassNum = 0;		//복셀에서 edge가 mesh와 만나는 개수
	int edgePassSum = 0;		//복셀에서 edge가 mesh와 만나는 위치를 0~11까지 하여 2진수로 만든 뒤 10진수로 표현
	int b = 0;					//복셀의 행 값 = row
	int c = 0;					//복셀의 열 값 = col
	int h = 0;					//복셀의 높이 값 = height
	int vertexInNum = 0;		//복셀에서 mesh에 포함되는 vertex의 개수
	int vertexInSum = 0;		//복셀에서 mesh에 포함되는 vertex의 위치를 2진수로 만든 뒤 10진수로 표현
	int voxelCase = 0;			//해당이 복셀이 무슨 Case인지 나타내는 값
	int voxelIndex = 0;			//해당 복셀의 index(번호)
	int vError = 0;				//해당 복셀의 Skinny Triangle 유무
	int vErrorFaceNum = 0;		//해당 복셀의 Skinny Triangle 개수
	int vFaceNum = 0;			//해당 복셀의 face 개수
	int startIndex = -1;		//faceList에서 n번째 index부터 faceNum 개수만큼까지가 해당 복셀의 영역
}Voxel;

typedef struct vertexset {
	Point P;
	int index = 0;
	int use = 1;
}vertexSet;

typedef struct faceset {
	int v1 = 0;
	int v2 = 0;
	int v3 = 0;
	int voxelIndex = 0;
	int isErrorFace = 0;
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

//FILE* fpErrorFace = fopen("ErrorFace.txt", "w");
FILE* fpVoxelCase = fopen("VoxelCase.txt", "w");
FILE* fpDebug = fopen("Debug.txt", "w");
//FILE* fpErrorCase = fopen("ErrorCase.txt", "w");
FILE* fpErrorCase;

//FILE* fpNewVOFF = fopen("New.OFF", "w");
FILE* fpNewVOFF;

Point findNearIndex(faceSet Fs[], vertexSet Vs[], int n);
void setCloseCorner(Point p1, Point* newp1, Voxel Vo[], int i);
Point getCloseCorner(Point p1, Voxel Vo[], int i);
//void setCenterNotCorner(Point p1, Point* newp1, Voxel* Vo, int i);

int IsSamePoint(Point p1, Point p2);
int IsNearFace(int A, int B, faceSet Fs[]);


//double cl = 82.8
//double half_cl = cl / 2;
//double cl;
//ouble half_cl;
//conotur_level 필요 없어 보임

double stepSize = 1;
double half_stepSize = stepSize / 2;


int main(void)
{
	int edgePassNum = 0, edgePassSum = 0, valVoxelNum = 0;
	int b = 0, c = 0, h = 0;
	int vertexNum, faceNum, zero, three;
	int newVertexNum, newFaceNum;
	int errorIndex = 0;								//추가되는 vertex의 개수
	int errorNum = 0;
	int n1 = 1;
	int n2 = 3;
	int i = 0;

	int errorCase[15] = { 0 };
	//int AllErrorFace = 0;
	int AllnowErrorFace = 0;
	int AllErrorCase = 0;
	int AllnowErrorCase = 0;
	int nowErrorCase[15] = { 0 };
	int nowErrorCaseTemp[15] = { 0 };

	char OFFName[40] = "";
	char EdgePassName[40] = "";
	char ErrorCaseName[40] = "";
	char EMDNum[40] = "";
	char newOFFName[40] = "";

	/*
	printf("마칭큐브를 활용하여 만든 OFF파일과 복셀의 정보가 있는 파일 2개가 필요합니다.\n");
	printf("OFF파일의 이름을 입력하세요\n");

	scanf("%s", OFFName);

	//ErrorCase 구분 파일 생성
	strcat(ErrorCaseName, OFFName);
	strcat(ErrorCaseName, "_ErrorCase.txt");
	fpErrorCase = fopen(ErrorCaseName, "w");

	printf("EdgePass 정보가 든 파일의 이름을 입력하세요\n");
	scanf("%s", EdgePassName);
	strcat(OFFName, ".OFF");
	strcat(EdgePassName, ".txt");
	*/

	printf("활용할 EMD 넘버를 입력하세요.\n");
	scanf("%s", EMDNum);

	strcat(OFFName, "V_1step_emd_");	//OFFName = V_1step_emd_
	strcat(OFFName, EMDNum);			//OFFName = V_1step_emd_****
	strcat(OFFName, ".OFF");			//OFFName = V_1step_emd_****.OFF

	strcat(EdgePassName, "1step_emd_");				//EdgePassName = 1step_emd_
	strcat(EdgePassName, EMDNum);					//EdgePassName = 1step_emd_****
	strcat(EdgePassName, "_EdgePassCheck1.txt");	//EdgePassName = 1step_emd_****_EdgePassCheck1.txt

	strcat(ErrorCaseName, EMDNum);
	strcat(ErrorCaseName, "_ErrorCase.txt");
	fpErrorCase = fopen(ErrorCaseName, "w");

	strcat(newOFFName, "new");
	strcat(newOFFName, EMDNum);
	strcat(newOFFName, ".OFF");

	fpNewVOFF = fopen(newOFFName, "w");

	fpVOFF = fopen(OFFName, "r");
	if (fpVOFF == NULL)
	{
		printf("오류 : 해당 OFF파일이 존재하지않습니다.\n");
		return 0;
	}
	fpEdgePassCheckFile1 = fopen(EdgePassName, "r");
	if (fpEdgePassCheckFile1 == NULL)
	{
		printf("오류 : 해당 복셀 정보 파일이 존재하지않습니다.\n");
		return 0;
	}

	/*
	printf("Contour Level을 입력하세요.\n");
	scanf("%lf", &cl);

	half_cl = cl / 2;
	*/

	fscanf(fpVOFF, "%d %d %d %d", &vertexNum, &faceNum, &zero, &valVoxelNum);

	Voxel* Vo = new Voxel[valVoxelNum];
	vertexSet* Vs = new vertexSet[vertexNum];
	faceSet* Fs = new faceSet[faceNum];
	vertexSet* newVs = new vertexSet[vertexNum];
	faceSet* newFs = new faceSet[faceNum];

	newVertexNum = vertexNum;
	newFaceNum = faceNum;

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
			//printf("%d\n", Vo[i].voxelIndex);
			if (Vo[i].vError == 1)
				errorCase[1]++;
		}
		else if (Vo[i].edgePassNum == 5)
		{
			Vo[i].voxelCase = 4;
			if (Vo[i].vError == 1)
				errorCase[4]++;
		}
		else if (Vo[i].edgePassNum == 7)
		{
			Vo[i].voxelCase = 11;
			if (Vo[i].vError == 1)
			{
				errorCase[11]++;
			}

		}
		else if (Vo[i].edgePassNum == 9)
		{
			Vo[i].voxelCase = 12;
			if (Vo[i].vError == 1)
				errorCase[12]++;
		}
		else if (Vo[i].edgePassNum == 12)
		{
			Vo[i].voxelCase = 7;
			if (Vo[i].vError == 1)
				errorCase[7]++;
		}
		else if (Vo[i].edgePassNum == 4)
		{
			if (Vo[i].edgePassSum == 85 || Vo[i].edgePassSum == 170 || Vo[i].edgePassSum == 3840)
			{
				Vo[i].voxelCase = 5;
				if (Vo[i].vError == 1)
					errorCase[5]++;
			}
			else
			{
				Vo[i].voxelCase = 2;
				if (Vo[i].vError == 1)
					errorCase[2]++;
			}
		}
		else if (Vo[i].edgePassNum == 8)//case
		{
			if (Vo[i].edgePassSum == 255 || Vo[i].edgePassSum == 3925 || Vo[i].edgePassSum == 4010)
			{
				Vo[i].voxelCase = 13;
				if (Vo[i].vError == 1)
				{
					errorCase[13]++;
				}

			}
			else//추가되면 확실히 가능
			{
				Vo[i].voxelCase = 6;
				if (Vo[i].vError == 1)
					errorCase[6]++;
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
					if (Vo[i].vError == 1)
						errorCase[3]++;
				}
				else//추가되면 확실히 가능
				{
					Vo[i].voxelCase = 10;
					if (Vo[i].vError == 1)
						errorCase[10]++;
				}
			}
			else if (Vo[i].vertexInNum == 4)
			{
				if (Vo[i].vertexInSum == 27 || Vo[i].vertexInSum == 39 || Vo[i].vertexInSum == 78 || Vo[i].vertexInSum == 114\
					|| Vo[i].vertexInSum == 141 || Vo[i].vertexInSum == 177 || Vo[i].vertexInSum == 216 || Vo[i].vertexInSum == 228)
				{
					Vo[i].voxelCase = 8;
					if (Vo[i].vError == 1)
						errorCase[8]++;
				}
				else if (Vo[i].vertexInSum == 23 || Vo[i].vertexInSum == 46 || Vo[i].vertexInSum == 57 || Vo[i].vertexInSum == 77\
					|| Vo[i].vertexInSum == 99 || Vo[i].vertexInSum == 116 || Vo[i].vertexInSum == 139 || Vo[i].vertexInSum == 156\
					|| Vo[i].vertexInSum == 178 || Vo[i].vertexInSum == 198 || Vo[i].vertexInSum == 209 || Vo[i].vertexInSum == 232)
				{
					Vo[i].voxelCase = 9;
					if (Vo[i].vError == 1)
						errorCase[9]++;
				}
				else//추가되면 확실히 가능
				{
					Vo[i].voxelCase = 14;
					if (Vo[i].vError == 1)
						errorCase[14]++;
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
	/*
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
	*/

	/*
	//findNearIndex 실험
	int n5 = 0;
	Point sample;
	sample = findNearIndex(Fs, Vs, n5);

	printf("nearPoint : %lf %lf %lf\n\n", sample.x, sample.y, sample.z);
	*/

	printf("복셀 Case 분류 완료, 문제 해결 시작\n");
	//문제 해결
	int tempStart = 0;

	printf("참조선1\n");
	//모든 복셀을 탐색
	for (i = 0; i < valVoxelNum; i++)
	{

		if (Vo[i].vError == 1)				//복셀에 문제가 있다. 복셀에 문제가 있는 face가 있다.
		{
			//Case1
			
			if (Vo[i].voxelCase == 1)		//복셀의 Case가 1이다. face는 1개
			{
				Point nearPoint0, nearPoint1, nearPoint2, nearPoint3;
				int fIndex01 = 0, fIndex02 = 0, fIndex03 = 0;
				int fIndex11 = 0, fIndex12 = 0, fIndex13 = 0;
				int fIndex21 = 0, fIndex22 = 0, fIndex23 = 0;
				int fIndex31 = 0, fIndex32 = 0, fIndex33 = 0;

				//printf("%d index 복셀 Case1\n", Vo[i].voxelIndex);
				tempStart = Vo[i].startIndex;	//복셀의 face 시작 index 값 넣기
				//1번째 face (tempStart, fIndex01,02,03 , nearPoint0)
				if (Fs[tempStart].isErrorFace == 1) // 1번 Face가 오류 Face
				{
					//printf("Case1 Error Face 발생\n");
					fIndex01 = Fs[tempStart].v1;	//face index 값 넣기 1
					fIndex02 = Fs[tempStart].v2;	//face index 값 넣기 2
					fIndex03 = Fs[tempStart].v3;	//face index 값 넣기 3
					nearPoint0 = findNearIndex(Fs, Vs, tempStart);   //해당 face의 가까운 복셀 꼭짓점 찾기

					newVs[fIndex01].P = nearPoint0; // 가까운 꼭짓점을 넣기 1
					newVs[fIndex02].P = nearPoint0; // 가까운 꼭짓점을 넣기 2
					newVs[fIndex03].P = nearPoint0; // 가까운 꼭짓점을 넣기 3

					Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
					newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
					newFaceNum--;							// face 개수 한개 제거
					nowErrorCase[1]++;
					for (int j = 0; j < faceNum; j++)		// face 1번 값으로 2,3번 값 바꾸기
					{
						if (newFs[j].v1 == fIndex02 || newFs[j].v1 == fIndex03)
						{
							newFs[j].v1 = fIndex01;
						}
						if (newFs[j].v2 == fIndex02 || newFs[j].v2 == fIndex03)
						{
							newFs[j].v2 = fIndex01;
						}
						if (newFs[j].v3 == fIndex02 || newFs[j].v3 == fIndex03)
						{
							newFs[j].v3 = fIndex01;
						}
					}
				}
			}
			//Case1 제거 완료

			//Case3,10
			
			if (Vo[i].voxelCase == 3 || Vo[i].voxelCase == 10)
			{
				Point nearPoint0, nearPoint1, nearPoint2, nearPoint3;
				int fIndex01 = 0, fIndex02 = 0, fIndex03 = 0;
				int fIndex11 = 0, fIndex12 = 0, fIndex13 = 0;
				int fIndex21 = 0, fIndex22 = 0, fIndex23 = 0;
				int fIndex31 = 0, fIndex32 = 0, fIndex33 = 0;

				//printf("%d index 복셀 Case3, 10\n", Vo[i].voxelIndex);
				tempStart = Vo[i].startIndex;	//복셀의 face 파일 시작 index 값 넣기
				//printf("페이스 개수 %d\n", Vo[i].vFaceNum);
				nowErrorCaseTemp[3] = 0;
				nowErrorCaseTemp[10] = 0;

				if (Vo[i].vFaceNum == 2)// face가 2개가 아닌 경우는 형태가 기본과 다르므로 다른 방법을 적용해야 한다.
				{
					//1번째 face (tempStart, fIndex01,02,03 , nearPoint0)
					if (Fs[tempStart].isErrorFace == 1) // 1번 Face가 오류 Face
					{
						//printf("Case3, 10 Error Face 발생\n");
						fIndex01 = Fs[tempStart].v1;	//face index 값 넣기 1
						fIndex02 = Fs[tempStart].v2;	//face index 값 넣기 2
						fIndex03 = Fs[tempStart].v3;	//face index 값 넣기 3
						nearPoint0 = findNearIndex(Fs, Vs, tempStart);   //해당 face의 가까운 복셀 꼭짓점 찾기

						//printf("%lf, %lf, %lf\n", nearPoint0.x, nearPoint0.y, nearPoint0.z);

						newVs[fIndex01].P = nearPoint0; // 가까운 꼭짓점을 넣기 1
						newVs[fIndex02].P = nearPoint0; // 가까운 꼭짓점을 넣기 2
						newVs[fIndex03].P = nearPoint0; // 가까운 꼭짓점을 넣기 3

						Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
						newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
						newFaceNum--;							// face 개수 한개 제거	

						if (Vo[i].voxelCase == 3)
						{
							nowErrorCaseTemp[3] = 1;
							nowErrorCase[3]++;
						}
						if (Vo[i].voxelCase == 10)
						{
							nowErrorCaseTemp[10] = 1;
							nowErrorCase[10]++;
						}

						for (int j = 0; j < faceNum; j++)		// face 1번 값으로 2,3번 값 바꾸기
						{
							if (newFs[j].v1 == fIndex02 || newFs[j].v1 == fIndex03)
							{
								newFs[j].v1 = fIndex01;
							}
							if (newFs[j].v2 == fIndex02 || newFs[j].v2 == fIndex03)
							{
								newFs[j].v2 = fIndex01;
							}
							if (newFs[j].v3 == fIndex02 || newFs[j].v3 == fIndex03)
							{
								newFs[j].v3 = fIndex01;
							}
						}
					}

					//2번째 face (tempStart + 1 , fIndex11,12,13 , nearPoint1)
					if (Fs[tempStart + 1].isErrorFace == 1) // 2번 Face가 오류 Face
					{
						//printf("Case3, 10 Error Face 발생\n");
						fIndex11 = Fs[tempStart + 1].v1;	//face index 값 넣기 1
						fIndex12 = Fs[tempStart + 1].v2;	//face index 값 넣기 2
						fIndex13 = Fs[tempStart + 1].v3;	//face index 값 넣기 3
						nearPoint1 = findNearIndex(Fs, Vs, tempStart + 1);   //해당 face의 가까운 복셀 꼭짓점 찾기

						//printf("%lf, %lf, %lf\n", nearPoint1.x, nearPoint1.y, nearPoint1.z);

						newVs[fIndex11].P = nearPoint1; // 가까운 꼭짓점을 넣기 1
						newVs[fIndex12].P = nearPoint1; // 가까운 꼭짓점을 넣기 2
						newVs[fIndex13].P = nearPoint1; // 가까운 꼭짓점을 넣기 3

						if (Vo[i].voxelCase == 3 && nowErrorCaseTemp[3] == 0)
						{
							nowErrorCase[3]++;
						}
						if (Vo[i].voxelCase == 10 && nowErrorCaseTemp[10] == 0)
						{
							nowErrorCase[10]++;							
						}

						Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
						newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
						newFaceNum--;							// face 개수 한개 제거

						for (int j = 0; j < faceNum; j++)		// face 1번 값으로 2,3번 값 바꾸기
						{
							if (newFs[j].v1 == fIndex12 || newFs[j].v1 == fIndex13)
							{
								newFs[j].v1 = fIndex11;
							}
							if (newFs[j].v2 == fIndex12 || newFs[j].v2 == fIndex13)
							{
								newFs[j].v2 = fIndex11;
							}
							if (newFs[j].v3 == fIndex12 || newFs[j].v3 == fIndex13)
							{
								newFs[j].v3 = fIndex11;
							}
						}
					}
				}
				else
				{
					if (Vo[i].voxelCase == 3)
					{
						printf("!!Case3 face %d개 수정필요\n", Vo[i].vFaceNum);
					}
					if (Vo[i].voxelCase == 10)
					{
						//if (Vo[i].vFaceNum != 4)
							printf("!!Case10 face %d개 수정필요\n", Vo[i].vFaceNum);
						//Case10일 경우 face 4개가 나올 수 있다.
					}
				}

			}
			//Case3,10 제거 완료

			//Case12
			
			if (Vo[i].voxelCase == 12)
			{
				Point nearPoint0, nearPoint1, nearPoint2, nearPoint3;
				int fIndex01 = 0, fIndex02 = 0, fIndex03 = 0;
				int fIndex11 = 0, fIndex12 = 0, fIndex13 = 0;
				int fIndex21 = 0, fIndex22 = 0, fIndex23 = 0;
				int fIndex31 = 0, fIndex32 = 0, fIndex33 = 0;
				nowErrorCaseTemp[12] = 0;


				//printf("%d index 복셀 Case12\n", Vo[i].voxelIndex);
				tempStart = Vo[i].startIndex;	//복셀의 face 파일 시작 index 값 넣기


				//printf("페이스 개수 %d\n", Vo[i].vFaceNum);

				if (Vo[i].vFaceNum == 3)
				{

					//1번째 face (tempStart, fIndex01,02,03 , nearPoint0)
					if (Fs[tempStart].isErrorFace == 1) // 1번 Face가 오류 Face
					{
						//printf("Case12 Error Face 발생\n");
						fIndex01 = Fs[tempStart].v1;	//face index 값 넣기 1
						fIndex02 = Fs[tempStart].v2;	//face index 값 넣기 2
						fIndex03 = Fs[tempStart].v3;	//face index 값 넣기 3
						nearPoint0 = findNearIndex(Fs, Vs, tempStart);   //해당 face의 가까운 복셀 꼭짓점 찾기

						newVs[fIndex01].P = nearPoint0; // 가까운 꼭짓점을 넣기 1
						newVs[fIndex02].P = nearPoint0; // 가까운 꼭짓점을 넣기 2
						newVs[fIndex03].P = nearPoint0; // 가까운 꼭짓점을 넣기 3

						Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
						newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
						newFaceNum--;							// face 개수 한개 제거
						nowErrorCase[12]++;
						nowErrorCaseTemp[12] = 1;

						for (int j = 0; j < faceNum; j++)		// face 1번 값으로 2,3번 값 바꾸기
						{
							if (newFs[j].v1 == fIndex02 || newFs[j].v1 == fIndex03)
							{
								newFs[j].v1 = fIndex01;
							}
							if (newFs[j].v2 == fIndex02 || newFs[j].v2 == fIndex03)
							{
								newFs[j].v2 = fIndex01;
							}
							if (newFs[j].v3 == fIndex02 || newFs[j].v3 == fIndex03)
							{
								newFs[j].v3 = fIndex01;
							}
						}
					}


					//2번째 face (tempStart + 1 , fIndex11,12,13 , nearPoint1)
					if (Fs[tempStart + 1].isErrorFace == 1) // 2번 Face가 오류 Face
					{
						//printf("Case12 Error Face 발생\n");
						fIndex11 = Fs[tempStart + 1].v1;	//face index 값 넣기 1
						fIndex12 = Fs[tempStart + 1].v2;	//face index 값 넣기 2
						fIndex13 = Fs[tempStart + 1].v3;	//face index 값 넣기 3
						nearPoint1 = findNearIndex(Fs, Vs, tempStart + 1);   //해당 face의 가까운 복셀 꼭짓점 찾기

						newVs[fIndex11].P = nearPoint1; // 가까운 꼭짓점을 넣기 1
						newVs[fIndex12].P = nearPoint1; // 가까운 꼭짓점을 넣기 2
						newVs[fIndex13].P = nearPoint1; // 가까운 꼭짓점을 넣기 3

						Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
						newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
						newFaceNum--;							// face 개수 한개 제거

						if (nowErrorCaseTemp[12] == 0)
						{
							nowErrorCase[12]++;
							nowErrorCaseTemp[12] = 1;
						}
							

						for (int j = 0; j < faceNum; j++)		// face 1번 값으로 2,3번 값 바꾸기
						{
							if (newFs[j].v1 == fIndex12 || newFs[j].v1 == fIndex13)
							{
								newFs[j].v1 = fIndex11;
							}
							if (newFs[j].v2 == fIndex12 || newFs[j].v2 == fIndex13)
							{
								newFs[j].v2 = fIndex11;
							}
							if (newFs[j].v3 == fIndex12 || newFs[j].v3 == fIndex13)
							{
								newFs[j].v3 = fIndex11;
							}
						}
					}

					//3번째 face (tempStart + 2 , findex21,22,23 , nearPoint2)
					if (Fs[tempStart + 2].isErrorFace == 1) // 3번 Face가 오류 Face
					{
						//printf("Case12 Error Face 발생\n");
						fIndex21 = Fs[tempStart + 2].v1;	//face index 값 넣기 1
						fIndex22 = Fs[tempStart + 2].v2;	//face index 값 넣기 2
						fIndex23 = Fs[tempStart + 2].v3;	//face index 값 넣기 3
						nearPoint2 = findNearIndex(Fs, Vs, tempStart + 2);   //해당 face의 가까운 복셀 꼭짓점 찾기

						newVs[fIndex21].P = nearPoint2; // 가까운 꼭짓점을 넣기 1
						newVs[fIndex22].P = nearPoint2; // 가까운 꼭짓점을 넣기 2
						newVs[fIndex23].P = nearPoint2; // 가까운 꼭짓점을 넣기 3

						Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
						newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
						newFaceNum--;							// face 개수 한개 제거

						if (nowErrorCaseTemp[12] == 0)
						{
							nowErrorCase[12]++;
							nowErrorCaseTemp[12] = 1;
						}

						for (int j = 0; j < faceNum; j++)		// face 1번 값으로 2,3번 값 바꾸기
						{
							if (newFs[j].v1 == fIndex22 || newFs[j].v1 == fIndex23)
							{
								newFs[j].v1 = fIndex21;
							}
							if (newFs[j].v2 == fIndex22 || newFs[j].v2 == fIndex23)
							{
								newFs[j].v2 = fIndex21;
							}
							if (newFs[j].v3 == fIndex22 || newFs[j].v3 == fIndex23)
							{
								newFs[j].v3 = fIndex21;
							}
						}
					}

				}
				else
				{
					//if (Vo[i].vFaceNum != 5)
						printf("!!Case12 face %d개 수정필요\n", Vo[i].vFaceNum);
					//Case12일 경우 face 5개가 나올 수 있다.
				}
			}
			//Case12 제거 완료

			//Case7
			
			if (Vo[i].voxelCase == 7)	// 삼각형 형태 4개
			{
				Point nearPoint0, nearPoint1, nearPoint2, nearPoint3;
				int fIndex01 = 0, fIndex02 = 0, fIndex03 = 0;
				int fIndex11 = 0, fIndex12 = 0, fIndex13 = 0;
				int fIndex21 = 0, fIndex22 = 0, fIndex23 = 0;
				int fIndex31 = 0, fIndex32 = 0, fIndex33 = 0;
				nowErrorCaseTemp[7] = 0;

				//printf("%d index 복셀 Case7\n", Vo[i].voxelIndex);
				tempStart = Vo[i].startIndex;	//복셀의 face 파일 시작 index 값 넣기

				if (Vo[i].vFaceNum == 4)
				{
					//1번째 face (tempStart, fIndex01,02,03 , nearPoint0)
					if (Fs[tempStart].isErrorFace == 1) // 1번 Face가 오류 Face
					{
						//printf("Case7 Error Face 발생\n");
						fIndex01 = Fs[tempStart].v1;	//face index 값 넣기 1
						fIndex02 = Fs[tempStart].v2;	//face index 값 넣기 2
						fIndex03 = Fs[tempStart].v3;	//face index 값 넣기 3
						nearPoint0 = findNearIndex(Fs, Vs, tempStart);   //해당 face의 가까운 복셀 꼭짓점 찾기

						newVs[fIndex01].P = nearPoint0; // 가까운 꼭짓점을 넣기 1
						newVs[fIndex02].P = nearPoint0; // 가까운 꼭짓점을 넣기 2
						newVs[fIndex03].P = nearPoint0; // 가까운 꼭짓점을 넣기 3

						Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
						newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
						newFaceNum--;							// face 개수 한개 제거
						nowErrorCase[7]++;
						nowErrorCaseTemp[7] = 1;

						for (int j = 0; j < faceNum; j++)		// face 1번 값으로 2,3번 값 바꾸기
						{
							if (newFs[j].v1 == fIndex02 || newFs[j].v1 == fIndex03)
							{
								newFs[j].v1 = fIndex01;
							}
							if (newFs[j].v2 == fIndex02 || newFs[j].v2 == fIndex03)
							{
								newFs[j].v2 = fIndex01;
							}
							if (newFs[j].v3 == fIndex02 || newFs[j].v3 == fIndex03)
							{
								newFs[j].v3 = fIndex01;
							}
						}
					}

					//2번째 face (tempStart + 1 , fIndex11,12,13 , nearPoint1)
					if (Fs[tempStart + 1].isErrorFace == 1) // 2번 Face가 오류 Face
					{
						//printf("Case7 Error Face 발생\n");
						fIndex11 = Fs[tempStart + 1].v1;	//face index 값 넣기 1
						fIndex12 = Fs[tempStart + 1].v2;	//face index 값 넣기 2
						fIndex13 = Fs[tempStart + 1].v3;	//face index 값 넣기 3
						nearPoint1 = findNearIndex(Fs, Vs, tempStart + 1);   //해당 face의 가까운 복셀 꼭짓점 찾기

						newVs[fIndex11].P = nearPoint1; // 가까운 꼭짓점을 넣기 1
						newVs[fIndex12].P = nearPoint1; // 가까운 꼭짓점을 넣기 2
						newVs[fIndex13].P = nearPoint1; // 가까운 꼭짓점을 넣기 3

						Fs[tempStart + 1].use = 0;					// 이 face list에 넣지 않게 표시
						newFs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
						newFaceNum--;								// face 개수 한개 제거
						if (nowErrorCaseTemp[7] == 0)
						{
							nowErrorCaseTemp[7] = 1;
							nowErrorCase[7]++;
						}						

						for (int j = 0; j < faceNum; j++)			// face 1번 값으로 2,3번 값 바꾸기
						{
							if (newFs[j].v1 == fIndex12 || newFs[j].v1 == fIndex13)
							{
								newFs[j].v1 = fIndex11;
							}
							if (newFs[j].v2 == fIndex12 || newFs[j].v2 == fIndex13)
							{
								newFs[j].v2 = fIndex11;
							}
							if (newFs[j].v3 == fIndex12 || newFs[j].v3 == fIndex13)
							{
								newFs[j].v3 = fIndex11;
							}
						}
					}

					//3번째 face (tempStart + 2 , findex21,22,23 , nearPoint2)
					if (Fs[tempStart + 2].isErrorFace == 1) // 3번 Face가 오류 Face
					{
						//printf("Case7 Error Face 발생\n");
						fIndex21 = Fs[tempStart + 2].v1;	//face index 값 넣기 1
						fIndex22 = Fs[tempStart + 2].v2;	//face index 값 넣기 2
						fIndex23 = Fs[tempStart + 2].v3;	//face index 값 넣기 3
						nearPoint2 = findNearIndex(Fs, Vs, tempStart + 2);   //해당 face의 가까운 복셀 꼭짓점 찾기

						newVs[fIndex21].P = nearPoint2; // 가까운 꼭짓점을 넣기 1
						newVs[fIndex22].P = nearPoint2; // 가까운 꼭짓점을 넣기 2
						newVs[fIndex23].P = nearPoint2; // 가까운 꼭짓점을 넣기 3

						Fs[tempStart + 2].use = 0;					// 이 face list에 넣지 않게 표시
						newFs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
						newFaceNum--;								// face 개수 한개 제거

						if (nowErrorCaseTemp[7] == 0)
						{
							nowErrorCaseTemp[7] = 1;
							nowErrorCase[7]++;
						}

						for (int j = 0; j < faceNum; j++)			// face 1번 값으로 2,3번 값 바꾸기
						{
							if (newFs[j].v1 == fIndex22 || newFs[j].v1 == fIndex23)
							{
								newFs[j].v1 = fIndex21;
							}
							if (newFs[j].v2 == fIndex22 || newFs[j].v2 == fIndex23)
							{
								newFs[j].v2 = fIndex21;
							}
							if (newFs[j].v3 == fIndex22 || newFs[j].v3 == fIndex23)
							{
								newFs[j].v3 = fIndex21;
							}
						}
					}

					//4번째 face (tempStart + 3 , findex31,32,33 , nearPoint3)
					if (Fs[tempStart + 3].isErrorFace == 1) // 4번 Face가 오류 Face
					{
						//printf("Case7 Error Face 발생\n");
						fIndex31 = Fs[tempStart + 3].v1;	//face index 값 넣기 1
						fIndex32 = Fs[tempStart + 3].v2;	//face index 값 넣기 2
						fIndex33 = Fs[tempStart + 3].v3;	//face index 값 넣기 3
						nearPoint3 = findNearIndex(Fs, Vs, tempStart + 3);   //해당 face의 가까운 복셀 꼭짓점 찾기

						newVs[fIndex31].P = nearPoint3; // 가까운 꼭짓점을 넣기 1
						newVs[fIndex32].P = nearPoint3; // 가까운 꼭짓점을 넣기 2
						newVs[fIndex33].P = nearPoint3; // 가까운 꼭짓점을 넣기 3

						Fs[tempStart + 3].use = 0;					// 이 face list에 넣지 않게 표시
						newFs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
						newFaceNum--;								// face 개수 한개 제거

						if (nowErrorCaseTemp[7] == 0)
						{
							nowErrorCaseTemp[7] = 1;
							nowErrorCase[7]++;
						}

						for (int j = 0; j < faceNum; j++)			// face 1번 값으로 2,3번 값 바꾸기
						{
							if (newFs[j].v1 == fIndex32 || newFs[j].v1 == fIndex33)
							{
								newFs[j].v1 = fIndex31;
							}
							if (newFs[j].v2 == fIndex32 || newFs[j].v2 == fIndex33)
							{
								newFs[j].v2 = fIndex31;
							}
							if (newFs[j].v3 == fIndex32 || newFs[j].v3 == fIndex33)
							{
								newFs[j].v3 = fIndex31;
							}
						}
					}
				}
				else
				{
					printf("!!Case7 face %d개 수정필요\n", Vo[i].vFaceNum);
				}
			}
			//Case7 제거 완료

			//Case 1, 3, 7, 10, 12 완료

			//인접 삼각면 2개 형태 Case2, 11 ,13

			//Case2
			
			if (Vo[i].voxelCase == 2)//평행사변형
			{
				Point nearPoint0, nearPoint1, nearPoint2, nearPoint3;
				int fIndex01 = 0, fIndex02 = 0, fIndex03 = 0;
				int fIndex11 = 0, fIndex12 = 0, fIndex13 = 0;
				int fIndex21 = 0, fIndex22 = 0, fIndex23 = 0;
				int fIndex31 = 0, fIndex32 = 0, fIndex33 = 0;
				nowErrorCaseTemp[2] = 0;

				//printf("%d index 복셀 Case2\n", Vo[i].voxelIndex);
				//한개씩 처리 두번
				if (Vo[i].vErrorFaceNum > 0 && Vo[i].vFaceNum == 2)
				{
					tempStart = Vo[i].startIndex;
					//1번 Face가 오류
					if (Fs[tempStart].isErrorFace == 1)
					{
						fIndex01 = Fs[tempStart].v1;	//face index 값 넣기 1
						fIndex02 = Fs[tempStart].v2;	//face index 값 넣기 2
						fIndex03 = Fs[tempStart].v3;	//face index 값 넣기 3

						//01-02 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex02].P, Vo, i)))
						{
							//printf("Case2 ErrorFace 01-02번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[2] == 0)
							{
								nowErrorCase[2]++;
								nowErrorCaseTemp[2] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex02)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex02)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex02)
									newFs[j].v3 = fIndex01;
							}
						}
						//01-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case2 ErrorFace 01-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[2] == 0)
							{
								nowErrorCase[2]++;
								nowErrorCaseTemp[2] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex01;
							}
						}
						//02-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex02].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case2 ErrorFace 02-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex02].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex02].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[2] == 0)
							{
								nowErrorCase[2]++;
								nowErrorCaseTemp[2] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex02;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex02;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex02;
							}
						}
						else
						{
							printf("!!Case2 ErrorFace 1번 face 오류 제거 과정 중 오류 발생\n");
						}

					}
					//2번 Face가 오류
					if (Fs[tempStart + 1].isErrorFace == 1)
					{
						fIndex11 = Fs[tempStart + 1].v1;	//face index 값 넣기 1
						fIndex12 = Fs[tempStart + 1].v2;	//face index 값 넣기 2
						fIndex13 = Fs[tempStart + 1].v3;	//face index 값 넣기 3

						//11-12 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex12].P, Vo, i)))
						{
							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[2] == 0)
							{
								nowErrorCase[2]++;
								nowErrorCaseTemp[2] = 1;
							}
							
							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex12)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex12)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex12)
									newFs[j].v3 = fIndex11;
							}
						}
						//11-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case2 ErrorFace 11-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[2] == 0)
							{
								nowErrorCase[2]++;
								nowErrorCaseTemp[2] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex11;
							}
						}
						//12-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex12].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case2 ErrorFace 12-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex12].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex12].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[2] == 0)
							{
								nowErrorCase[2]++;
								nowErrorCaseTemp[2] = 1;
							}
							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex12;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex12;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex12;
							}
						}
						else
						{
							printf("!!Case2 ErrorFace 2번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
				}
				else
				{
					printf("!!Case2 face %d개 수정필요\n", Vo[i].vFaceNum);
				}
			}
			//Case2 제거 완료
			
			//Case11
			
			if (Vo[i].voxelCase == 11)//평행사변형
			{
				//Case11용
				Point nearPoint0, nearPoint1, nearPoint2, nearPoint3;
				int NearIndex01 = 0, NearIndex02 = 0, OutIndex01 = 0, Case11Val = 1;
				int fIndexNear01 = 0, fIndexNear02 = 0, fIndexNear03 = 0;
				int fIndexNear11 = 0, fIndexNear12 = 0, fIndexNear13 = 0;
				int fIndexOut01 = 0, fIndexOut02 = 0, fIndexOut03 = 0;
				nowErrorCaseTemp[11] = 0;

				if (Vo[i].vFaceNum == 3)
				{
					//printf("%d index 복셀 Case11\n", Vo[i].voxelIndex);
					Case11Val = 1;					// 초기화
					tempStart = Vo[i].startIndex;	//복셀의 face 파일 시작 index 값 넣기

					//printf("%lf %lf %lf\n", newVs[Fs[tempStart].v1].P.x, newVs[Fs[tempStart].v1].P.y, newVs[Fs[tempStart].v1].P.z);

					//먼저 3개의 face 중 붙어있는 2개의 face가 무슨 Face인지 부터 알아내자
					//nearIndex 1,2는 인접한 face, OutIndex01 따로 있는 face

					//tempStart, tempStart + 1 Face가 평행사변형을 이룬다면
					if (IsNearFace(tempStart, tempStart + 1, Fs) == 1)
					{
						NearIndex01 = tempStart;
						NearIndex02 = tempStart + 1;
						OutIndex01 = tempStart + 2;
					}
					//tempStart+1, tempStart + 2 Face가 평행사변형을 이룬다면
					else if (IsNearFace(tempStart + 1, tempStart + 2, Fs) == 1)
					{
						NearIndex01 = tempStart + 1;
						NearIndex02 = tempStart + 2;
						OutIndex01 = tempStart;
					}
					//tempStart, tempStart + 2 Face가 평행사변형을 이룬다면
					else if (IsNearFace(tempStart, tempStart + 2, Fs) == 1)
					{
						NearIndex01 = tempStart;
						NearIndex02 = tempStart + 2;
						OutIndex01 = tempStart + 1;
					}
					else
					{
						Case11Val = 0; // 알고리즘 미적용
						printf("!!Case11 에러 인접한 Face 없음");
					}
					if (IsNearFace(tempStart, tempStart + 1, Fs) == 1 && IsNearFace(tempStart + 1, tempStart + 2, Fs) == 1 && IsNearFace(tempStart, tempStart + 2, Fs) == 1)
					{
						Case11Val = 0; // 알고리즘 미적용
						printf("!!Case11 에러 Face 3개 모두 인접함");
					}

					fIndexOut01 = Fs[OutIndex01].v1;	//OutIndex01 face index 값 넣기 1
					fIndexOut02 = Fs[OutIndex01].v2;	//OutIndex01 face index 값 넣기 2
					fIndexOut03 = Fs[OutIndex01].v3;	//OutIndex01 face index 값 넣기 3

					fIndexNear01 = Fs[NearIndex01].v1;	//nearIndex1 face index 값 넣기 1
					fIndexNear02 = Fs[NearIndex01].v2;	//nearIndex1 face index 값 넣기 2
					fIndexNear03 = Fs[NearIndex01].v3;	//nearIndex1face index 값 넣기 3

					fIndexNear11 = Fs[NearIndex02].v1;	//nearIndex2 face index 값 넣기 1
					fIndexNear12 = Fs[NearIndex02].v2;	//nearIndex2 face index 값 넣기 2
					fIndexNear13 = Fs[NearIndex02].v3;	//nearIndex2 face index 값 넣기 3
					//여기까지 해서 tempStart ~ tempStart+2가 neaerIndex1,2, OutIndex01에 들어간다.

					//skinny triangle 제거 적용
					//Case11 skinny triangle이 1개이상이고 기본 Case 형태이다.
					if (Vo[i].vErrorFaceNum > 0 && Case11Val != 0)
					{
						// 혼자 떨어져 있는 OutIndex01 Face가 skinny triangle
						if (Fs[OutIndex01].isErrorFace == 1)
						{
							//printf("Case11 ErrorFace, OutIndex01가 skinny triangle\n");
							//가장 가까운 복셀 꼭짓점 찾기
							nearPoint0 = findNearIndex(Fs, Vs, OutIndex01);
							//3점의 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndexOut01].P = nearPoint0;
							newVs[fIndexOut02].P = nearPoint0;
							newVs[fIndexOut03].P = nearPoint0;
							//printf("%lf, %lf, %lf\n", nearPoint0.x, nearPoint0.y, nearPoint0.z);
							Fs[OutIndex01].use = 0;				// OutIndex01을 face list에 넣지 않게 표시
							newFs[OutIndex01].use = 0;			// OutIndex01을 face list에 넣지 않게 표시
							newFaceNum--;						// face 개수 한개 제거
							nowErrorCase[11]++;
							nowErrorCaseTemp[11] = 1;

							for (int j = 0; j < faceNum; j++)	// fIndexOut02,03 값을 fIndexOut01 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndexOut02 || newFs[j].v1 == fIndexOut03)
								{
									newFs[j].v1 = fIndexOut01;
								}
								if (newFs[j].v2 == fIndexOut02 || newFs[j].v2 == fIndexOut03)
								{
									newFs[j].v2 = fIndexOut01;
								}
								if (newFs[j].v3 == fIndexOut02 || newFs[j].v3 == fIndexOut03)
								{
									newFs[j].v3 = fIndexOut01;
								}
							}
						}
						// 인접한 Face 들 중 NearIndex01 face가 skinny triangle 이라면
						if (Fs[NearIndex01].isErrorFace == 1)
						{
							//NearIndex01 fIndexNear01-02가 짧은 거리
							if (IsSamePoint(getCloseCorner(Vs[fIndexNear01].P, Vo, i), getCloseCorner(Vs[fIndexNear02].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex01 01-02번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear01].P = getCloseCorner(Vs[fIndexNear01].P, Vo, i);
								newVs[fIndexNear02].P = getCloseCorner(Vs[fIndexNear02].P, Vo, i);

								Fs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[11] == 0)
								{
									nowErrorCase[11]++;
									nowErrorCaseTemp[11] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear02번 값을 fIndexNear01번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear02)
										newFs[j].v1 = fIndexNear01;
									if (newFs[j].v2 == fIndexNear02)
										newFs[j].v2 = fIndexNear01;
									if (newFs[j].v3 == fIndexNear02)
										newFs[j].v3 = fIndexNear01;
								}
							}
							//NearIndex01 fIndexNear01-03가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear01].P, Vo, i), getCloseCorner(Vs[fIndexNear03].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex01 01-03번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear01].P = getCloseCorner(Vs[fIndexNear01].P, Vo, i);
								newVs[fIndexNear03].P = getCloseCorner(Vs[fIndexNear03].P, Vo, i);

								Fs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[11] == 0)
								{
									nowErrorCase[11]++;
									nowErrorCaseTemp[11] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear03번 값을 fIndexNear01번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear03)
										newFs[j].v1 = fIndexNear01;
									if (newFs[j].v2 == fIndexNear03)
										newFs[j].v2 = fIndexNear01;
									if (newFs[j].v3 == fIndexNear03)
										newFs[j].v3 = fIndexNear01;
								}
							}
							//NearIndex01 fIndexNear02-03가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear02].P, Vo, i), getCloseCorner(Vs[fIndexNear03].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex01 02-03번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear02].P = getCloseCorner(Vs[fIndexNear02].P, Vo, i);
								newVs[fIndexNear03].P = getCloseCorner(Vs[fIndexNear03].P, Vo, i);

								Fs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[11] == 0)
								{
									nowErrorCase[11]++;
									nowErrorCaseTemp[11] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear03번 값을 fIndexNear02번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear03)
										newFs[j].v1 = fIndexNear02;
									if (newFs[j].v2 == fIndexNear03)
										newFs[j].v2 = fIndexNear02;
									if (newFs[j].v3 == fIndexNear03)
										newFs[j].v3 = fIndexNear02;
								}
							}
							else
							{
								printf("!!Case11 NearIndex01 face 제거 오류\n");
							}
						}
						// 인접한 Face 들 중 NearIndex02 face가 skinny triangle 이라면
						if (Fs[NearIndex02].isErrorFace == 1)
						{
							//NearIndex02 fIndexNear11-12가 짧은 거리
							if (IsSamePoint(getCloseCorner(Vs[fIndexNear11].P, Vo, i), getCloseCorner(Vs[fIndexNear12].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex02 11-12번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear11].P = getCloseCorner(Vs[fIndexNear11].P, Vo, i);
								newVs[fIndexNear12].P = getCloseCorner(Vs[fIndexNear12].P, Vo, i);

								Fs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[11] == 0)
								{
									nowErrorCase[11]++;
									nowErrorCaseTemp[11] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear12번 값을 fIndexNear11번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear12)
										newFs[j].v1 = fIndexNear11;
									if (newFs[j].v2 == fIndexNear12)
										newFs[j].v2 = fIndexNear11;
									if (newFs[j].v3 == fIndexNear12)
										newFs[j].v3 = fIndexNear11;
								}
							}
							//NearIndex02 fIndexNear11-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear11].P, Vo, i), getCloseCorner(Vs[fIndexNear13].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex02 11-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear11].P = getCloseCorner(Vs[fIndexNear11].P, Vo, i);
								newVs[fIndexNear13].P = getCloseCorner(Vs[fIndexNear13].P, Vo, i);

								Fs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[11] == 0)
								{
									nowErrorCase[11]++;
									nowErrorCaseTemp[11] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear13번 값을 fIndexNear11번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear13)
										newFs[j].v1 = fIndexNear11;
									if (newFs[j].v2 == fIndexNear13)
										newFs[j].v2 = fIndexNear11;
									if (newFs[j].v3 == fIndexNear13)
										newFs[j].v3 = fIndexNear11;
								}
							}
							//NearIndex02 fIndexNear12-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear12].P, Vo, i), getCloseCorner(Vs[fIndexNear13].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex02 12-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear12].P = getCloseCorner(Vs[fIndexNear12].P, Vo, i);
								newVs[fIndexNear13].P = getCloseCorner(Vs[fIndexNear13].P, Vo, i);

								Fs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[11] == 0)
								{
									nowErrorCase[11]++;
									nowErrorCaseTemp[11] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear13번 값을 fIndexNear12번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear13)
										newFs[j].v1 = fIndexNear12;
									if (newFs[j].v2 == fIndexNear13)
										newFs[j].v2 = fIndexNear12;
									if (newFs[j].v3 == fIndexNear13)
										newFs[j].v3 = fIndexNear12;
								}
							}
							else
							{
								printf("!!Case11 NearIndex02 face 제거 오류\n");
							}
						}
					}
					else
					{
						printf("!!Case11 총 face 개수가 3개 일때 에러 + 기본형태와 다름\n");
					}
				}
				else //Case11에서 face가 3개가 아닐 경우
				{
					//if (Vo[i].vFaceNum != 5)
						printf("!!Case11 face %d개 수정필요\n", Vo[i].vFaceNum);
					//Case11일 경우 face 5개가 나올 수 있다.
				}
			}
			//Case11 제거 완료
			
			//Case13
			
			if (Vo[i].voxelCase == 13)//평행사변형 2개
			{
				//Case13용
				Point nearPoint0, nearPoint1, nearPoint2, nearPoint3;
				int NearIndexA01 = 0, NearIndexA02 = 0, NearIndexB01 = 0, NearIndexB02 = 0, Case13Val = 1;
				int fIndexNearA01 = 0, fIndexNearA02 = 0, fIndexNearA03 = 0;
				int fIndexNearA11 = 0, fIndexNearA12 = 0, fIndexNearA13 = 0;
				int fIndexNearB01 = 0, fIndexNearB02 = 0, fIndexNearB03 = 0;
				int fIndexNearB11 = 0, fIndexNearB12 = 0, fIndexNearB13 = 0;
				nowErrorCaseTemp[13] = 0;

				if (Vo[i].vFaceNum == 4)
				{
					tempStart = Vo[i].startIndex;	//복셀의 face 파일 시작 index 값 넣기
					Case13Val = 1;					//초기화

					//먼저 4개의 face 중 붙어있는 2쌍의 face가 무엇인지 부터 알아내자
					//NearIndexA01, NearIndexA02 인접한 2개의 face, NearIndexB01, NearIndexB02 인접한 2개의 face

					//tempStart와 tempStart+1이 인접하면 당연히 tempStart+2, tempStart+3도 인접해야함
					if (IsNearFace(tempStart, tempStart + 1, Fs) == 1)
					{
						if (IsNearFace(tempStart + 2, tempStart + 3, Fs) == 1)
						{
							NearIndexA01 = tempStart;
							NearIndexA02 = tempStart + 1;
							NearIndexB01 = tempStart + 2;
							NearIndexB02 = tempStart + 3;
						}
						else
						{
							Case13Val = 0;
							printf("!!Case13 temp 2-3 인접 발생안함 오류\n");
						}
					}
					//tempStart와 tempStart+2이 인접하면 당연히 tempStart+1, tempStart+3도 인접해야함
					else if (IsNearFace(tempStart, tempStart + 2, Fs) == 1)
					{
						if (IsNearFace(tempStart + 1, tempStart + 3, Fs) == 1)
						{
							NearIndexA01 = tempStart;
							NearIndexA02 = tempStart + 2;
							NearIndexB01 = tempStart + 1;
							NearIndexB02 = tempStart + 3;
						}
						else
						{
							Case13Val = 0;
							printf("!!Case13 temp 1-3 인접 발생안함 오류\n");
						}
					}
					//tempStart와 tempStart+3이 인접하면 당연히 tempStart+1, tempStart+2도 인접해야함
					else if (IsNearFace(tempStart, tempStart + 3, Fs) == 1)
					{
						if (IsNearFace(tempStart + 1, tempStart + 2, Fs) == 1)
						{
							NearIndexA01 = tempStart;
							NearIndexA02 = tempStart + 3;
							NearIndexB01 = tempStart + 1;
							NearIndexB02 = tempStart + 2;
						}
						else
						{
							Case13Val = 0;
							printf("!!Case13 temp 1-2 인접 발생안함 오류\n");
						}
					}
					else
					{
						Case13Val = 0;
						printf("!!Case13 temp0과 인접하는 face 없음 오류\n");
					}

					//해당 Index넣기
					fIndexNearA01 = Fs[NearIndexA01].v1;	//NearIndexA01 face index 값 넣기 1
					fIndexNearA02 = Fs[NearIndexA01].v2;	//NearIndexA01 face index 값 넣기 2
					fIndexNearA03 = Fs[NearIndexA01].v3;	//NearIndexA01 face index 값 넣기 3

					fIndexNearA11 = Fs[NearIndexA02].v1;	//NearIndexA02 face index 값 넣기 1
					fIndexNearA12 = Fs[NearIndexA02].v2;	//NearIndexA02 face index 값 넣기 2
					fIndexNearA13 = Fs[NearIndexA02].v3;	//NearIndexA02 face index 값 넣기 3

					fIndexNearB01 = Fs[NearIndexB01].v1;	//NearIndexB01 face index 값 넣기 1
					fIndexNearB02 = Fs[NearIndexB01].v2;	//NearIndexB01 face index 값 넣기 2
					fIndexNearB03 = Fs[NearIndexB01].v3;	//NearIndexB01 face index 값 넣기 3

					fIndexNearB11 = Fs[NearIndexB02].v1;	//NearIndexB02 face index 값 넣기 1
					fIndexNearB12 = Fs[NearIndexB02].v2;	//NearIndexB02 face index 값 넣기 2
					fIndexNearB13 = Fs[NearIndexB02].v3;	//NearIndexB02 face index 값 넣기 3
					//여기까지 Case13 인접한 Index 구분 완료


					//skinny triangle 제거 적용
					//Case13 skinny triangle이 1개이고 기본 Case 형태이다.
					if (Vo[i].vErrorFaceNum > 0 && Case13Val != 0)
					{
						//NearIndexA01이 skinny triangle
						if (Fs[NearIndexA01].isErrorFace == 1)
						{
							//NearIndexA01 fIndexNearA01-02가 짧은 거리
							if (IsSamePoint(getCloseCorner(Vs[fIndexNearA01].P, Vo, i), getCloseCorner(Vs[fIndexNearA02].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexA01 01-02번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearA01].P = getCloseCorner(Vs[fIndexNearA01].P, Vo, i);
								newVs[fIndexNearA02].P = getCloseCorner(Vs[fIndexNearA02].P, Vo, i);

								Fs[NearIndexA01].use = 0;			//NearIndexA01을 face list에 넣지 않기
								newFs[NearIndexA01].use = 0;		//NearIndexA01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearA02번 값을 fIndexNearA01번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearA02)
										newFs[j].v1 = fIndexNearA01;
									if (newFs[j].v2 == fIndexNearA02)
										newFs[j].v2 = fIndexNearA01;
									if (newFs[j].v3 == fIndexNearA02)
										newFs[j].v3 = fIndexNearA01;
								}
							}
							//NearIndexA01 fIndexNearA01-03가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNearA01].P, Vo, i), getCloseCorner(Vs[fIndexNearA03].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexA01 01-03번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearA01].P = getCloseCorner(Vs[fIndexNearA01].P, Vo, i);
								newVs[fIndexNearA03].P = getCloseCorner(Vs[fIndexNearA03].P, Vo, i);

								Fs[NearIndexA01].use = 0;			//NearIndexA01을 face list에 넣지 않기
								newFs[NearIndexA01].use = 0;		//NearIndexA01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearA03번 값을 fIndexNearA01번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearA03)
										newFs[j].v1 = fIndexNearA01;
									if (newFs[j].v2 == fIndexNearA03)
										newFs[j].v2 = fIndexNearA01;
									if (newFs[j].v3 == fIndexNearA03)
										newFs[j].v3 = fIndexNearA01;
								}
							}
							//NearIndexA01 fIndexNearA02-03가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNearA02].P, Vo, i), getCloseCorner(Vs[fIndexNearA03].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexA01 02-03번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearA02].P = getCloseCorner(Vs[fIndexNearA02].P, Vo, i);
								newVs[fIndexNearA03].P = getCloseCorner(Vs[fIndexNearA03].P, Vo, i);

								Fs[NearIndexA01].use = 0;			//NearIndexA01을 face list에 넣지 않기
								newFs[NearIndexA01].use = 0;			//NearIndexA01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearA03번 값을 fIndexNearA02번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearA03)
										newFs[j].v1 = fIndexNearA02;
									if (newFs[j].v2 == fIndexNearA03)
										newFs[j].v2 = fIndexNearA02;
									if (newFs[j].v3 == fIndexNearA03)
										newFs[j].v3 = fIndexNearA02;
								}
							}
							else
							{
								printf("!!Case13 skinny triangle NearIndexA01 face 제거 오류\n");
							}
						}
						//NearIndexA02이 skinny triangle
						if (Fs[NearIndexA02].isErrorFace == 1)
						{
							//NearIndexA02 fIndexNearA11-12가 짧은 거리
							if (IsSamePoint(getCloseCorner(Vs[fIndexNearA11].P, Vo, i), getCloseCorner(Vs[fIndexNearA12].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexA02 11-12번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearA11].P = getCloseCorner(Vs[fIndexNearA11].P, Vo, i);
								newVs[fIndexNearA12].P = getCloseCorner(Vs[fIndexNearA12].P, Vo, i);

								Fs[NearIndexA02].use = 0;			//NearIndexA02을 face list에 넣지 않기
								newFs[NearIndexA02].use = 0;		//NearIndexA02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearA12번 값을 fIndexNearA11번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearA12)
										newFs[j].v1 = fIndexNearA11;
									if (newFs[j].v2 == fIndexNearA12)
										newFs[j].v2 = fIndexNearA11;
									if (newFs[j].v3 == fIndexNearA12)
										newFs[j].v3 = fIndexNearA11;
								}
							}
							//NearIndexA02 fIndexNearA11-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNearA11].P, Vo, i), getCloseCorner(Vs[fIndexNearA13].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexA02 11-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearA11].P = getCloseCorner(Vs[fIndexNearA11].P, Vo, i);
								newVs[fIndexNearA13].P = getCloseCorner(Vs[fIndexNearA13].P, Vo, i);

								Fs[NearIndexA02].use = 0;			//NearIndexA02을 face list에 넣지 않기
								newFs[NearIndexA02].use = 0;		//NearIndexA02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearA13번 값을 fIndexNearA11번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearA13)
										newFs[j].v1 = fIndexNearA11;
									if (newFs[j].v2 == fIndexNearA13)
										newFs[j].v2 = fIndexNearA11;
									if (newFs[j].v3 == fIndexNearA13)
										newFs[j].v3 = fIndexNearA11;
								}
							}
							//NearIndexA02 fIndexNearA12-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNearA12].P, Vo, i), getCloseCorner(Vs[fIndexNearA13].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexA02 12-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearA12].P = getCloseCorner(Vs[fIndexNearA12].P, Vo, i);
								newVs[fIndexNearA13].P = getCloseCorner(Vs[fIndexNearA13].P, Vo, i);

								Fs[NearIndexA02].use = 0;			//NearIndexA02을 face list에 넣지 않기
								newFs[NearIndexA02].use = 0;			//NearIndexA02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearA13번 값을 fIndexNearA12번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearA13)
										newFs[j].v1 = fIndexNearA12;
									if (newFs[j].v2 == fIndexNearA13)
										newFs[j].v2 = fIndexNearA12;
									if (newFs[j].v3 == fIndexNearA13)
										newFs[j].v3 = fIndexNearA12;
								}
							}
							else
							{
								printf("!!Case13 skinny triangle NearIndexA02 face 제거 오류\n");
							}
						}
						//NearIndexB01이 skinny triangle
						if (Fs[NearIndexB01].isErrorFace == 1)
						{
							//NearIndexB01 fIndexNearB01-02가 짧은 거리
							if (IsSamePoint(getCloseCorner(Vs[fIndexNearB01].P, Vo, i), getCloseCorner(Vs[fIndexNearB02].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexB01 01-02번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearB01].P = getCloseCorner(Vs[fIndexNearB01].P, Vo, i);
								newVs[fIndexNearB02].P = getCloseCorner(Vs[fIndexNearB02].P, Vo, i);

								Fs[NearIndexB01].use = 0;			//NearIndexB01을 face list에 넣지 않기
								newFs[NearIndexB01].use = 0;		//NearIndexB01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearB02번 값을 fIndexNearB01번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearB02)
										newFs[j].v1 = fIndexNearB01;
									if (newFs[j].v2 == fIndexNearB02)
										newFs[j].v2 = fIndexNearB01;
									if (newFs[j].v3 == fIndexNearB02)
										newFs[j].v3 = fIndexNearB01;
								}
							}
							//NearIndexB01 fIndexNearB01-03가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNearB01].P, Vo, i), getCloseCorner(Vs[fIndexNearB03].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexB01 01-03번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearB01].P = getCloseCorner(Vs[fIndexNearB01].P, Vo, i);
								newVs[fIndexNearB03].P = getCloseCorner(Vs[fIndexNearB03].P, Vo, i);

								Fs[NearIndexB01].use = 0;			//NearIndexB01을 face list에 넣지 않기
								newFs[NearIndexB01].use = 0;		//NearIndexB01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearB03번 값을 fIndexNearB01번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearB03)
										newFs[j].v1 = fIndexNearB01;
									if (newFs[j].v2 == fIndexNearB03)
										newFs[j].v2 = fIndexNearB01;
									if (newFs[j].v3 == fIndexNearB03)
										newFs[j].v3 = fIndexNearB01;
								}
							}
							//NearIndexB01 fIndexNearB02-03가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNearB02].P, Vo, i), getCloseCorner(Vs[fIndexNearB03].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexB01 02-03번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearB02].P = getCloseCorner(Vs[fIndexNearB02].P, Vo, i);
								newVs[fIndexNearB03].P = getCloseCorner(Vs[fIndexNearB03].P, Vo, i);

								Fs[NearIndexB01].use = 0;			//NearIndexB01을 face list에 넣지 않기
								newFs[NearIndexB01].use = 0;			//NearIndexB01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearB03번 값을 fIndexNearB02번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearB03)
										newFs[j].v1 = fIndexNearB02;
									if (newFs[j].v2 == fIndexNearB03)
										newFs[j].v2 = fIndexNearB02;
									if (newFs[j].v3 == fIndexNearB03)
										newFs[j].v3 = fIndexNearB02;
								}
							}
							else
							{
								printf("!!Case13 skinny triangle NearIndexB01 face 제거 오류\n");
							}
						}
						//NearIndexB02이 skinny triangle
						if (Fs[NearIndexB02].isErrorFace == 1)
						{
							//NearIndexB02 fIndexNearB11-12가 짧은 거리
							if (IsSamePoint(getCloseCorner(Vs[fIndexNearB11].P, Vo, i), getCloseCorner(Vs[fIndexNearB12].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexB02 11-12번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearB11].P = getCloseCorner(Vs[fIndexNearB11].P, Vo, i);
								newVs[fIndexNearB12].P = getCloseCorner(Vs[fIndexNearB12].P, Vo, i);

								Fs[NearIndexB02].use = 0;			//NearIndexB02을 face list에 넣지 않기
								newFs[NearIndexB02].use = 0;		//NearIndexB02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearB12번 값을 fIndexNearB11번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearB12)
										newFs[j].v1 = fIndexNearB11;
									if (newFs[j].v2 == fIndexNearB12)
										newFs[j].v2 = fIndexNearB11;
									if (newFs[j].v3 == fIndexNearB12)
										newFs[j].v3 = fIndexNearB11;
								}
							}
							//NearIndexB02 fIndexNearB11-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNearB11].P, Vo, i), getCloseCorner(Vs[fIndexNearB13].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexB02 11-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearB11].P = getCloseCorner(Vs[fIndexNearB11].P, Vo, i);
								newVs[fIndexNearB13].P = getCloseCorner(Vs[fIndexNearB13].P, Vo, i);

								Fs[NearIndexB02].use = 0;			//NearIndexB02을 face list에 넣지 않기
								newFs[NearIndexB02].use = 0;		//NearIndexB02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearB13번 값을 fIndexNearB11번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearB13)
										newFs[j].v1 = fIndexNearB11;
									if (newFs[j].v2 == fIndexNearB13)
										newFs[j].v2 = fIndexNearB11;
									if (newFs[j].v3 == fIndexNearB13)
										newFs[j].v3 = fIndexNearB11;
								}
							}
							//NearIndexB02 fIndexNearB12-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNearB12].P, Vo, i), getCloseCorner(Vs[fIndexNearB13].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndexB02 12-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNearB12].P = getCloseCorner(Vs[fIndexNearB12].P, Vo, i);
								newVs[fIndexNearB13].P = getCloseCorner(Vs[fIndexNearB13].P, Vo, i);

								Fs[NearIndexB02].use = 0;			//NearIndexB02을 face list에 넣지 않기
								newFs[NearIndexB02].use = 0;			//NearIndexB02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[13] == 0)
								{
									nowErrorCase[13]++;
									nowErrorCaseTemp[13] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNearB13번 값을 fIndexNearB12번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNearB13)
										newFs[j].v1 = fIndexNearB12;
									if (newFs[j].v2 == fIndexNearB13)
										newFs[j].v2 = fIndexNearB12;
									if (newFs[j].v3 == fIndexNearB13)
										newFs[j].v3 = fIndexNearB12;
								}
							}
							else
							{
								printf("!!Case13 skinny triangle NearIndexB02 face 제거 오류\n");
							}
						}
					}
					else
					{
						printf("!!Case13 총 face 개수가 4개 일때 에러 + 기본형태와 다름\n");
					}

				}
				else
				{
					printf("!!Case13 face %d개 수정필요\n", Vo[i].vFaceNum);
				}
			}
			//Case13 제거 완료
			
			//인접 삼각면 3개 형태 Case4, 6

			//Case4
			
			if (Vo[i].voxelCase == 4)//삼각형 3개 묶음
			{
				Point nearPoint0;
				int fIndex01 = 0, fIndex02 = 0, fIndex03 = 0;
				int fIndex11 = 0, fIndex12 = 0, fIndex13 = 0;
				int fIndex21 = 0, fIndex22 = 0, fIndex23 = 0;
				nowErrorCaseTemp[4] = 0;

				//한개씩 처리 세번
				if (Vo[i].vErrorFaceNum > 0 && Vo[i].vFaceNum == 3)
				{
					tempStart = Vo[i].startIndex;
					//1번 Face가 오류
					if (Fs[tempStart].isErrorFace == 1)
					{
						fIndex01 = Fs[tempStart].v1;	//face index 값 넣기 1
						fIndex02 = Fs[tempStart].v2;	//face index 값 넣기 2
						fIndex03 = Fs[tempStart].v3;	//face index 값 넣기 3

						//01-02 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex02].P, Vo, i)))
						{
							//printf("Case4 ErrorFace 01-02번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[4] == 0)
							{
								nowErrorCase[4]++;
								nowErrorCaseTemp[4] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex02)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex02)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex02)
									newFs[j].v3 = fIndex01;
							}
						}
						//01-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case4 ErrorFace 01-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[4] == 0)
							{
								nowErrorCase[4]++;
								nowErrorCaseTemp[4] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex01;
							}
						}
						//02-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex02].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case4 ErrorFace 02-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex02].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex02].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[4] == 0)
							{
								nowErrorCase[4]++;
								nowErrorCaseTemp[4] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex02;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex02;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex02;
							}
						}
						else
						{
							printf("!!Case4 ErrorFace 1번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//2번 Face가 오류
					if (Fs[tempStart + 1].isErrorFace == 1)
					{
						fIndex11 = Fs[tempStart + 1].v1;	//face index 값 넣기 1
						fIndex12 = Fs[tempStart + 1].v2;	//face index 값 넣기 2
						fIndex13 = Fs[tempStart + 1].v3;	//face index 값 넣기 3

						//11-12 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex12].P, Vo, i)))
						{
							//printf("Case4 ErrorFace 11-12번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[4] == 0)
							{
								nowErrorCase[4]++;
								nowErrorCaseTemp[4] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex12)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex12)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex12)
									newFs[j].v3 = fIndex11;
							}
						}
						//11-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case4 ErrorFace 11-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[4] == 0)
							{
								nowErrorCase[4]++;
								nowErrorCaseTemp[4] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex11;
							}
						}
						//12-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex12].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case4 ErrorFace 12-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex12].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex12].P, Vo, i);


							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[4] == 0)
							{
								nowErrorCase[4]++;
								nowErrorCaseTemp[4] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex12;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex12;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex12;
							}
						}
						else
						{
							printf("!!Case4 ErrorFace 2번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//3번 Face가 오류
					if (Fs[tempStart + 2].isErrorFace == 1)
					{
						fIndex21 = Fs[tempStart + 2].v1;	//face index 값 넣기 1
						fIndex22 = Fs[tempStart + 2].v2;	//face index 값 넣기 2
						fIndex23 = Fs[tempStart + 2].v3;	//face index 값 넣기 3

						//21-22 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex21].P, Vo, i), getCloseCorner(Vs[fIndex22].P, Vo, i)))
						{
							//printf("Case4 ErrorFace 21-22번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex21].P = getCloseCorner(Vs[fIndex21].P, Vo, i);
							newVs[fIndex22].P = getCloseCorner(Vs[fIndex21].P, Vo, i);

							Fs[tempStart + 2].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[4] == 0)
							{
								nowErrorCase[4]++;
								nowErrorCaseTemp[4] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex22)
									newFs[j].v1 = fIndex21;
								if (newFs[j].v2 == fIndex22)
									newFs[j].v2 = fIndex21;
								if (newFs[j].v3 == fIndex22)
									newFs[j].v3 = fIndex21;
							}
						}
						//21-23 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex21].P, Vo, i), getCloseCorner(Vs[fIndex23].P, Vo, i)))
						{
							//printf("Case4 ErrorFace 21-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex21].P = getCloseCorner(Vs[fIndex21].P, Vo, i);
							newVs[fIndex23].P = getCloseCorner(Vs[fIndex21].P, Vo, i);

							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[4] == 0)
							{
								nowErrorCase[4]++;
								nowErrorCaseTemp[4] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex23)
									newFs[j].v1 = fIndex21;
								if (newFs[j].v2 == fIndex23)
									newFs[j].v2 = fIndex21;
								if (newFs[j].v3 == fIndex23)
									newFs[j].v3 = fIndex21;
							}
						}
						//22-23 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex22].P, Vo, i), getCloseCorner(Vs[fIndex23].P, Vo, i)))
						{
							//printf("Case4 ErrorFace 22-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex22].P = getCloseCorner(Vs[fIndex22].P, Vo, i);
							newVs[fIndex23].P = getCloseCorner(Vs[fIndex22].P, Vo, i);


							Fs[tempStart + 2].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[4] == 0)
							{
								nowErrorCase[4]++;
								nowErrorCaseTemp[4] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex23)
									newFs[j].v1 = fIndex22;
								if (newFs[j].v2 == fIndex23)
									newFs[j].v2 = fIndex22;
								if (newFs[j].v3 == fIndex23)
									newFs[j].v3 = fIndex22;
							}
						}
						else
						{
							printf("!!Case4 ErrorFace 3번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
				}
				else
				{
					printf("!!Case4 face %d개 수정필요\n", Vo[i].vFaceNum);
				}


			}
			//Case4 제거 완료
			
			//Case6
			
			if (Vo[i].voxelCase == 6)
			{
				Point nearPoint0;
				int NearIndex01 = 0, NearIndex02 = 0, NearIndex03 = 0, OutIndex01 = 0, Case6Val = 1;
				int fIndexNear01 = 0, fIndexNear02 = 0, fIndexNear03 = 0;
				int fIndexNear11 = 0, fIndexNear12 = 0, fIndexNear13 = 0;
				int fIndexNear21 = 0, fIndexNear22 = 0, fIndexNear23 = 0;
				int fIndexOut01 = 0, fIndexOut02 = 0, fIndexOut03 = 0;
				nowErrorCaseTemp[6] = 0;

				if (Vo[i].vFaceNum == 4)
				{
					//printf("%d index 복셀 Case6\n", Vo[i].voxelIndex);
					Case6Val = 1;					// 초기화
					tempStart = Vo[i].startIndex;	//복셀의 face 파일 시작 index 값 넣기

					//printf("%lf %lf %lf\n", newVs[Fs[tempStart].v1].P.x, newVs[Fs[tempStart].v1].P.y, newVs[Fs[tempStart].v1].P.z);

					//먼저 4개의 face 중 붙어있는 3개의 face가 무슨 Face인지 부터 알아내자
					//nearIndex 1,2,3은 인접한 face, OutIndex01가 따로 있는 face

					//만약 tempStart와 tesmpStart + 1이 인접한다면
					if (IsNearFace(tempStart, tempStart + 1, Fs) == 1)
					{
						//tempStart와 tempStart +2가 인접 또는 tempStart + 1과 tempStart + 2가 인접 == tempStart + 3 이 떨어져 있음
						if ((IsNearFace(tempStart, tempStart + 2, Fs) == 1) || (IsNearFace(tempStart + 1, tempStart + 2, Fs) == 1))
						{
							NearIndex01 = tempStart;
							NearIndex02 = tempStart + 1;
							NearIndex03 = tempStart + 2;
							OutIndex01 = tempStart + 3;
						}
						//tempStart와 tempStart +3가 인접 또는 tempStart + 1과 tempStart + 3가 인접 == tempStart + 2 가 떨어져 있음
						else if ((IsNearFace(tempStart, tempStart + 3, Fs) == 1) || (IsNearFace(tempStart + 1, tempStart + 3, Fs) == 1))
						{
							NearIndex01 = tempStart;
							NearIndex02 = tempStart + 1;
							NearIndex03 = tempStart + 3;
							OutIndex01 = tempStart + 2;
						}
						//tempStart와 tempStart+1만 인접 Case6의 기본 형태가 아님
						else
						{
							Case6Val = 0;
							printf("!!Case6 tempStart와 tempStart+1만 인접하는 오류\n");
						}
					}
					//만약 tempStart와 tesmpStart + 1이 인접하지않는다. (인접하지않아도 붙은 3개의 양 끝일 수도 있다.)
					else if (IsNearFace(tempStart, tempStart + 1, Fs) == 0)
					{
						//tempStart는 tempStart + 2와 tempStart + 3 둘 다 인접하지 않는다. == tempStart 가 떨어져 있음
						if ((IsNearFace(tempStart, tempStart + 2, Fs) == 0) && (IsNearFace(tempStart, tempStart + 3, Fs) == 0))
						{
							NearIndex01 = tempStart + 1;
							NearIndex02 = tempStart + 2;
							NearIndex03 = tempStart + 3;
							OutIndex01 = tempStart;
						}
						//tempStart + 1는 tempStart + 2와 tempStart + 3 둘 다 인접하지 않는다. == tempStart + 1 가 떨어져 있음
						else if ((IsNearFace(tempStart + 1, tempStart + 2, Fs) == 0) && (IsNearFace(tempStart + 1, tempStart + 3, Fs) == 0))
						{
							NearIndex01 = tempStart;
							NearIndex02 = tempStart + 2;
							NearIndex03 = tempStart + 3;
							OutIndex01 = tempStart + 1;
						}
						//tempStart와 tempStart + 1 이 인접한 3개의 face 중 양 끝을 담당할때
						//tempStart + 2 가 그 안쪽일때 == tempStart와 tempStart + 2가 인접하고, tempStart + 1과 tempStart +2가 인접 == tempStart + 3 이 떨어져 있음
						else if ((IsNearFace(tempStart, tempStart + 2, Fs) == 1) && (IsNearFace(tempStart + 1, tempStart + 2, Fs) == 1))
						{
							NearIndex01 = tempStart;
							NearIndex02 = tempStart + 2;
							NearIndex03 = tempStart + 1;
							OutIndex01 = tempStart + 3;
						}
						//tempStart + 3 가 그 안쪽일때 == tempStart와 tempStart + 3가 인접하고, tempStart + 1과 tempStart +3가 인접 == tempStart + 2 가 떨어져 있음
						else if ((IsNearFace(tempStart, tempStart + 3, Fs) == 1) && (IsNearFace(tempStart + 3, tempStart + 2, Fs) == 1))
						{
							NearIndex01 = tempStart;
							NearIndex02 = tempStart + 3;
							NearIndex03 = tempStart + 1;
							OutIndex01 = tempStart + 2;
						}
						//인접해야하는 개수가 모자람, Case6의 기본 형태가 아님
						else
						{
							Case6Val = 0;
							printf("!!Case6 인접해야하는 face 개수가 모자라는 오류\n");
						}
					}
					//한점만 같은 경우거나 다른 오류
					else
					{
						Case6Val = 0;
						printf("!!Case6 인접 처리시 한점만 같거나 다른 예기치 못한 오류\n");
					}

					fIndexOut01 = Fs[OutIndex01].v1;	//OutIndex01 face index 값 넣기 1
					fIndexOut02 = Fs[OutIndex01].v2;	//OutIndex01 face index 값 넣기 2
					fIndexOut03 = Fs[OutIndex01].v3;	//OutIndex01 face index 값 넣기 3

					fIndexNear01 = Fs[NearIndex01].v1;	//nearIndex1 face index 값 넣기 1
					fIndexNear02 = Fs[NearIndex01].v2;	//nearIndex1 face index 값 넣기 2
					fIndexNear03 = Fs[NearIndex01].v3;	//nearIndex1face index 값 넣기 3

					fIndexNear11 = Fs[NearIndex02].v1;	//nearIndex2 face index 값 넣기 1
					fIndexNear12 = Fs[NearIndex02].v2;	//nearIndex2 face index 값 넣기 2
					fIndexNear13 = Fs[NearIndex02].v3;	//nearIndex2 face index 값 넣기 3

					fIndexNear21 = Fs[NearIndex03].v1;	//nearIndex2 face index 값 넣기 1
					fIndexNear22 = Fs[NearIndex03].v2;	//nearIndex2 face index 값 넣기 2
					fIndexNear23 = Fs[NearIndex03].v3;	//nearIndex2 face index 값 넣기 3
					//여기까지 해서 tempStart ~ tempStart+3가 neaerIndex01,02,03, OutIndex01에 들어간다.

					//SkinnyTriangle 제거 시작
					//Case6 skinny triangle이 1개 이상이고 기본 Case 형태이다.
					if (Vo[i].vErrorFaceNum > 0 && Case6Val != 0)
					{
						// 혼자 떨어져 있는 OutIndex01 Face가 skinny triangle
						if (Fs[OutIndex01].isErrorFace == 1)
						{
							//printf("Case11 ErrorFace, OutIndex01가 skinny triangle\n");
							//가장 가까운 복셀 꼭짓점 찾기
							nearPoint0 = findNearIndex(Fs, Vs, OutIndex01);
							//3점의 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndexOut01].P = nearPoint0;
							newVs[fIndexOut02].P = nearPoint0;
							newVs[fIndexOut03].P = nearPoint0;
							//printf("%lf, %lf, %lf\n", nearPoint0.x, nearPoint0.y, nearPoint0.z);
							Fs[OutIndex01].use = 0;				// OutIndex01을 face list에 넣지 않게 표시
							newFs[OutIndex01].use = 0;			// OutIndex01을 face list에 넣지 않게 표시
							newFaceNum--;						// face 개수 한개 제거
							if (nowErrorCaseTemp[6] == 0)
							{
								nowErrorCase[6]++;
								nowErrorCaseTemp[6] = 1;
							}							

							for (int j = 0; j < faceNum; j++)	// fIndexOut02,03 값을 fIndexOut01 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndexOut02 || newFs[j].v1 == fIndexOut03)
								{
									newFs[j].v1 = fIndexOut01;
								}
								if (newFs[j].v2 == fIndexOut02 || newFs[j].v2 == fIndexOut03)
								{
									newFs[j].v2 = fIndexOut01;
								}
								if (newFs[j].v3 == fIndexOut02 || newFs[j].v3 == fIndexOut03)
								{
									newFs[j].v3 = fIndexOut01;
								}
							}
						}
						// 인접한 Face 들 중 NearIndex01 face가 skinny triangle 이라면
						if (Fs[NearIndex01].isErrorFace == 1)
						{
							//NearIndex01 fIndexNear01-02가 짧은 거리
							if (IsSamePoint(getCloseCorner(Vs[fIndexNear01].P, Vo, i), getCloseCorner(Vs[fIndexNear02].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex01 01-02번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear01].P = getCloseCorner(Vs[fIndexNear01].P, Vo, i);
								newVs[fIndexNear02].P = getCloseCorner(Vs[fIndexNear02].P, Vo, i);

								Fs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[6] == 0)
								{
									nowErrorCase[6]++;
									nowErrorCaseTemp[6] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear02번 값을 fIndexNear01번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear02)
										newFs[j].v1 = fIndexNear01;
									if (newFs[j].v2 == fIndexNear02)
										newFs[j].v2 = fIndexNear01;
									if (newFs[j].v3 == fIndexNear02)
										newFs[j].v3 = fIndexNear01;
								}
							}
							//NearIndex01 fIndexNear01-03가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear01].P, Vo, i), getCloseCorner(Vs[fIndexNear03].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex01 01-03번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear01].P = getCloseCorner(Vs[fIndexNear01].P, Vo, i);
								newVs[fIndexNear03].P = getCloseCorner(Vs[fIndexNear03].P, Vo, i);

								Fs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[6] == 0)
								{
									nowErrorCase[6]++;
									nowErrorCaseTemp[6] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear03번 값을 fIndexNear01번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear03)
										newFs[j].v1 = fIndexNear01;
									if (newFs[j].v2 == fIndexNear03)
										newFs[j].v2 = fIndexNear01;
									if (newFs[j].v3 == fIndexNear03)
										newFs[j].v3 = fIndexNear01;
								}
							}
							//NearIndex01 fIndexNear02-03가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear02].P, Vo, i), getCloseCorner(Vs[fIndexNear03].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex01 02-03번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear02].P = getCloseCorner(Vs[fIndexNear02].P, Vo, i);
								newVs[fIndexNear03].P = getCloseCorner(Vs[fIndexNear03].P, Vo, i);

								Fs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFs[NearIndex01].use = 0;			//NearIndex01을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[6] == 0)
								{
									nowErrorCase[6]++;
									nowErrorCaseTemp[6] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear03번 값을 fIndexNear02번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear03)
										newFs[j].v1 = fIndexNear02;
									if (newFs[j].v2 == fIndexNear03)
										newFs[j].v2 = fIndexNear02;
									if (newFs[j].v3 == fIndexNear03)
										newFs[j].v3 = fIndexNear02;
								}
							}
							else
							{
								printf("!!Case6 NearIndex01 face 제거 오류\n");
							}
						}
						// 인접한 Face 들 중 NearIndex02 face가 skinny triangle 이라면
						if (Fs[NearIndex02].isErrorFace == 1)
						{
							//NearIndex02 fIndexNear11-12가 짧은 거리
							if (IsSamePoint(getCloseCorner(Vs[fIndexNear11].P, Vo, i), getCloseCorner(Vs[fIndexNear12].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex02 11-12번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear11].P = getCloseCorner(Vs[fIndexNear11].P, Vo, i);
								newVs[fIndexNear12].P = getCloseCorner(Vs[fIndexNear12].P, Vo, i);

								Fs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[6] == 0)
								{
									nowErrorCase[6]++;
									nowErrorCaseTemp[6] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear12번 값을 fIndexNear11번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear12)
										newFs[j].v1 = fIndexNear11;
									if (newFs[j].v2 == fIndexNear12)
										newFs[j].v2 = fIndexNear11;
									if (newFs[j].v3 == fIndexNear12)
										newFs[j].v3 = fIndexNear11;
								}
							}
							//NearIndex02 fIndexNear11-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear11].P, Vo, i), getCloseCorner(Vs[fIndexNear13].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex02 11-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear11].P = getCloseCorner(Vs[fIndexNear11].P, Vo, i);
								newVs[fIndexNear13].P = getCloseCorner(Vs[fIndexNear13].P, Vo, i);

								Fs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[6] == 0)
								{
									nowErrorCase[6]++;
									nowErrorCaseTemp[6] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear13번 값을 fIndexNear11번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear13)
										newFs[j].v1 = fIndexNear11;
									if (newFs[j].v2 == fIndexNear13)
										newFs[j].v2 = fIndexNear11;
									if (newFs[j].v3 == fIndexNear13)
										newFs[j].v3 = fIndexNear11;
								}
							}
							//NearIndex02 fIndexNear12-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear12].P, Vo, i), getCloseCorner(Vs[fIndexNear13].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex02 12-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear12].P = getCloseCorner(Vs[fIndexNear12].P, Vo, i);
								newVs[fIndexNear13].P = getCloseCorner(Vs[fIndexNear13].P, Vo, i);

								Fs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFs[NearIndex02].use = 0;			//NearIndex02을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[6] == 0)
								{
									nowErrorCase[6]++;
									nowErrorCaseTemp[6] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear13번 값을 fIndexNear12번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear13)
										newFs[j].v1 = fIndexNear12;
									if (newFs[j].v2 == fIndexNear13)
										newFs[j].v2 = fIndexNear12;
									if (newFs[j].v3 == fIndexNear13)
										newFs[j].v3 = fIndexNear12;
								}
							}
							else
							{
								printf("!!Case 11 NearIndex02 face 제거 오류\n");
							}
						}
						// 인접한 Face 들 중 NearIndex03 face가 skinny triangle 이라면
						if (Fs[NearIndex03].isErrorFace == 1)
						{
							//NearIndex03 fIndexNear21-22가 짧은 거리
							if (IsSamePoint(getCloseCorner(Vs[fIndexNear21].P, Vo, i), getCloseCorner(Vs[fIndexNear22].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex03 21-22번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear21].P = getCloseCorner(Vs[fIndexNear21].P, Vo, i);
								newVs[fIndexNear22].P = getCloseCorner(Vs[fIndexNear22].P, Vo, i);

								Fs[NearIndex03].use = 0;			//NearIndex03을 face list에 넣지 않기
								newFs[NearIndex03].use = 0;			//NearIndex03을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[6] == 0)
								{
									nowErrorCase[6]++;
									nowErrorCaseTemp[6] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear22번 값을 fIndexNear21번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear22)
										newFs[j].v1 = fIndexNear21;
									if (newFs[j].v2 == fIndexNear22)
										newFs[j].v2 = fIndexNear21;
									if (newFs[j].v3 == fIndexNear22)
										newFs[j].v3 = fIndexNear21;
								}
							}
							//NearIndex03 fIndexNear21-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear21].P, Vo, i), getCloseCorner(Vs[fIndexNear23].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex03 11-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear21].P = getCloseCorner(Vs[fIndexNear21].P, Vo, i);
								newVs[fIndexNear23].P = getCloseCorner(Vs[fIndexNear23].P, Vo, i);

								Fs[NearIndex03].use = 0;			//NearIndex03을 face list에 넣지 않기
								newFs[NearIndex03].use = 0;			//NearIndex03을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[6] == 0)
								{
									nowErrorCase[6]++;
									nowErrorCaseTemp[6] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear23번 값을 fIndexNear21번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear23)
										newFs[j].v1 = fIndexNear21;
									if (newFs[j].v2 == fIndexNear23)
										newFs[j].v2 = fIndexNear21;
									if (newFs[j].v3 == fIndexNear23)
										newFs[j].v3 = fIndexNear21;
								}
							}
							//NearIndex03 fIndexNear22-13가 짧은 거리
							else if (IsSamePoint(getCloseCorner(Vs[fIndexNear22].P, Vo, i), getCloseCorner(Vs[fIndexNear23].P, Vo, i)))
							{
								//printf("Case11 ErrorFace, NearIndex03 12-13번 index edge가 짧다\n");

								//가장 짧은 edge에 가까운 복셀의 꼭짓점으로 이동한다.
								newVs[fIndexNear22].P = getCloseCorner(Vs[fIndexNear22].P, Vo, i);
								newVs[fIndexNear23].P = getCloseCorner(Vs[fIndexNear23].P, Vo, i);

								Fs[NearIndex03].use = 0;			//NearIndex03을 face list에 넣지 않기
								newFs[NearIndex03].use = 0;			//NearIndex03을 face list에 넣지 않기
								newFaceNum--;
								if (nowErrorCaseTemp[6] == 0)
								{
									nowErrorCase[6]++;
									nowErrorCaseTemp[6] = 1;
								}

								for (int j = 0; j < faceNum; j++)	// face list에서 fIndexNear23번 값을 fIndexNear22번 값으로 바꾸기
								{
									if (newFs[j].v1 == fIndexNear23)
										newFs[j].v1 = fIndexNear22;
									if (newFs[j].v2 == fIndexNear23)
										newFs[j].v2 = fIndexNear22;
									if (newFs[j].v3 == fIndexNear23)
										newFs[j].v3 = fIndexNear22;
								}
							}
							else
							{
								printf("!!Case 11 NearIndex03 face 제거 오류\n");
							}
						}
					}
					else
					{
						printf("!!Case6 총 face 개수가 4개 일때 에러 + 기본형태와 다름\n");
					}
				}
				else
				{
					printf("!!Case6 face %d개 수정필요\n", Vo[i].vFaceNum);
					//Case6일 경우 face ?개가 나올 수 있다.
				}
			}
			//Case6 제거 완료
			

			//인접 삼각면 4개 형태 Case8, 9 ,14

			
			//Case8
			
			if (Vo[i].voxelCase == 8)
			{
				Point nearPoint0;
				int fIndex01 = 0, fIndex02 = 0, fIndex03 = 0;
				int fIndex11 = 0, fIndex12 = 0, fIndex13 = 0;
				int fIndex21 = 0, fIndex22 = 0, fIndex23 = 0;
				int fIndex31 = 0, fIndex32 = 0, fIndex33 = 0;
				nowErrorCaseTemp[8] = 0;
				
				//한개씩 처리 4번
				if (Vo[i].vErrorFaceNum > 0 && Vo[i].vFaceNum == 4)
				{
					//printf("%d index 복셀 Case8\n", Vo[i].voxelIndex);

					tempStart = Vo[i].startIndex;	//복셀의 face 파일 시작 index 값 넣기
					//printf("%lf %lf %lf\n", newVs[Fs[tempStart].v1].P.x, newVs[Fs[tempStart].v1].P.y, newVs[Fs[tempStart].v1].P.z);

					//1번 Face가 오류
					if (Fs[tempStart].isErrorFace == 1)
					{
						fIndex01 = Fs[tempStart].v1;	//face index 값 넣기 1
						fIndex02 = Fs[tempStart].v2;	//face index 값 넣기 2
						fIndex03 = Fs[tempStart].v3;	//face index 값 넣기 3

						//01-02 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex02].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 01-02번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex02)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex02)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex02)
									newFs[j].v3 = fIndex01;
							}
						}
						//01-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 01-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex01;
							}
						}
						//02-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex02].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 02-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex02].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex02].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex02;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex02;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex02;
							}
						}
						else
						{
							printf("!!Case8 ErrorFace 1번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//2번 Face가 오류
					if (Fs[tempStart + 1].isErrorFace == 1)
					{
						fIndex11 = Fs[tempStart + 1].v1;	//face index 값 넣기 1
						fIndex12 = Fs[tempStart + 1].v2;	//face index 값 넣기 2
						fIndex13 = Fs[tempStart + 1].v3;	//face index 값 넣기 3

						//11-12 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex12].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 11-12번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex12)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex12)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex12)
									newFs[j].v3 = fIndex11;
							}
						}
						//11-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 11-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex11;
							}
						}
						//12-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex12].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 12-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex12].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex12].P, Vo, i);


							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex12;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex12;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex12;
							}
						}
						else
						{
							printf("!!Case8 ErrorFace 2번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//3번 Face가 오류
					if (Fs[tempStart + 2].isErrorFace == 1)
					{
						fIndex21 = Fs[tempStart + 2].v1;	//face index 값 넣기 1
						fIndex22 = Fs[tempStart + 2].v2;	//face index 값 넣기 2
						fIndex23 = Fs[tempStart + 2].v3;	//face index 값 넣기 3

						//21-22 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex21].P, Vo, i), getCloseCorner(Vs[fIndex22].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 21-22번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex21].P = getCloseCorner(Vs[fIndex21].P, Vo, i);
							newVs[fIndex22].P = getCloseCorner(Vs[fIndex21].P, Vo, i);

							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex22)
									newFs[j].v1 = fIndex21;
								if (newFs[j].v2 == fIndex22)
									newFs[j].v2 = fIndex21;
								if (newFs[j].v3 == fIndex22)
									newFs[j].v3 = fIndex21;
							}
						}
						//21-23 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex21].P, Vo, i), getCloseCorner(Vs[fIndex23].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 21-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex21].P = getCloseCorner(Vs[fIndex21].P, Vo, i);
							newVs[fIndex23].P = getCloseCorner(Vs[fIndex21].P, Vo, i);

							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex23)
									newFs[j].v1 = fIndex21;
								if (newFs[j].v2 == fIndex23)
									newFs[j].v2 = fIndex21;
								if (newFs[j].v3 == fIndex23)
									newFs[j].v3 = fIndex21;
							}
						}
						//22-23 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex22].P, Vo, i), getCloseCorner(Vs[fIndex23].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 22-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex22].P = getCloseCorner(Vs[fIndex22].P, Vo, i);
							newVs[fIndex23].P = getCloseCorner(Vs[fIndex22].P, Vo, i);


							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex23)
									newFs[j].v1 = fIndex22;
								if (newFs[j].v2 == fIndex23)
									newFs[j].v2 = fIndex22;
								if (newFs[j].v3 == fIndex23)
									newFs[j].v3 = fIndex22;
							}
						}
						else
						{
							printf("!!Case8 ErrorFace 3번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//4번 Face가 오류
					if (Fs[tempStart + 3].isErrorFace == 1)
					{
						fIndex31 = Fs[tempStart + 3].v1;	//face index 값 넣기 1
						fIndex32 = Fs[tempStart + 3].v2;	//face index 값 넣기 2
						fIndex33 = Fs[tempStart + 3].v3;	//face index 값 넣기 3

						//31-32 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex31].P, Vo, i), getCloseCorner(Vs[fIndex32].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 21-22번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex31].P = getCloseCorner(Vs[fIndex31].P, Vo, i);
							newVs[fIndex32].P = getCloseCorner(Vs[fIndex31].P, Vo, i);

							Fs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 3].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex32)
									newFs[j].v1 = fIndex31;
								if (newFs[j].v2 == fIndex32)
									newFs[j].v2 = fIndex31;
								if (newFs[j].v3 == fIndex32)
									newFs[j].v3 = fIndex31;
							}
						}
						//31-33 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex31].P, Vo, i), getCloseCorner(Vs[fIndex33].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 21-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex31].P = getCloseCorner(Vs[fIndex31].P, Vo, i);
							newVs[fIndex33].P = getCloseCorner(Vs[fIndex31].P, Vo, i);

							Fs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 3].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex33)
									newFs[j].v1 = fIndex31;
								if (newFs[j].v2 == fIndex33)
									newFs[j].v2 = fIndex31;
								if (newFs[j].v3 == fIndex33)
									newFs[j].v3 = fIndex31;
							}
						}
						//32-33 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex32].P, Vo, i), getCloseCorner(Vs[fIndex33].P, Vo, i)))
						{
							//printf("Case8 ErrorFace 22-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex32].P = getCloseCorner(Vs[fIndex32].P, Vo, i);
							newVs[fIndex33].P = getCloseCorner(Vs[fIndex32].P, Vo, i);


							Fs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 3].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[8] == 0)
							{
								nowErrorCase[8]++;
								nowErrorCaseTemp[8] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex33)
									newFs[j].v1 = fIndex32;
								if (newFs[j].v2 == fIndex33)
									newFs[j].v2 = fIndex32;
								if (newFs[j].v3 == fIndex33)
									newFs[j].v3 = fIndex32;
							}
						}
						else
						{
							printf("!!Case8 ErrorFace 4번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
				}
				else
				{
					printf("!!Case8 face %d개 수정필요\n", Vo[i].vFaceNum);
				}
			}
			//Case8 제거 완료
			
			
			//Case9
			
			if (Vo[i].voxelCase == 9)
			{
				Point nearPoint0;
				int fIndex01 = 0, fIndex02 = 0, fIndex03 = 0;
				int fIndex11 = 0, fIndex12 = 0, fIndex13 = 0;
				int fIndex21 = 0, fIndex22 = 0, fIndex23 = 0;
				int fIndex31 = 0, fIndex32 = 0, fIndex33 = 0;
				nowErrorCaseTemp[9] = 0;

				//한개씩 처리 4번
				if (Vo[i].vErrorFaceNum > 0 && Vo[i].vFaceNum == 4)
				{
					//printf("%d index 복셀 Case9\n", Vo[i].voxelIndex);

					tempStart = Vo[i].startIndex;	//복셀의 face 파일 시작 index 값 넣기
					//printf("%lf %lf %lf\n", newVs[Fs[tempStart].v1].P.x, newVs[Fs[tempStart].v1].P.y, newVs[Fs[tempStart].v1].P.z);

					//1번 Face가 오류
					if (Fs[tempStart].isErrorFace == 1)
					{
						fIndex01 = Fs[tempStart].v1;	//face index 값 넣기 1
						fIndex02 = Fs[tempStart].v2;	//face index 값 넣기 2
						fIndex03 = Fs[tempStart].v3;	//face index 값 넣기 3

						//01-02 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex02].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 01-02번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex02)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex02)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex02)
									newFs[j].v3 = fIndex01;
							}
						}
						//01-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 01-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex01;
							}
						}
						//02-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex02].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 02-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex02].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex02].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex02;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex02;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex02;
							}
						}
						else
						{
							printf("!!Case9 ErrorFace 1번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//2번 Face가 오류
					if (Fs[tempStart + 1].isErrorFace == 1)
					{
						fIndex11 = Fs[tempStart + 1].v1;	//face index 값 넣기 1
						fIndex12 = Fs[tempStart + 1].v2;	//face index 값 넣기 2
						fIndex13 = Fs[tempStart + 1].v3;	//face index 값 넣기 3

						//11-12 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex12].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 11-12번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex12)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex12)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex12)
									newFs[j].v3 = fIndex11;
							}
						}
						//11-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 11-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex11;
							}
						}
						//12-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex12].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 12-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex12].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex12].P, Vo, i);


							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex12;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex12;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex12;
							}
						}
						else
						{
							printf("!!Case9 ErrorFace 2번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//3번 Face가 오류
					if (Fs[tempStart + 2].isErrorFace == 1)
					{
						fIndex21 = Fs[tempStart + 2].v1;	//face index 값 넣기 1
						fIndex22 = Fs[tempStart + 2].v2;	//face index 값 넣기 2
						fIndex23 = Fs[tempStart + 2].v3;	//face index 값 넣기 3

						//21-22 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex21].P, Vo, i), getCloseCorner(Vs[fIndex22].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 21-22번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex21].P = getCloseCorner(Vs[fIndex21].P, Vo, i);
							newVs[fIndex22].P = getCloseCorner(Vs[fIndex21].P, Vo, i);

							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex22)
									newFs[j].v1 = fIndex21;
								if (newFs[j].v2 == fIndex22)
									newFs[j].v2 = fIndex21;
								if (newFs[j].v3 == fIndex22)
									newFs[j].v3 = fIndex21;
							}
						}
						//21-23 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex21].P, Vo, i), getCloseCorner(Vs[fIndex23].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 21-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex21].P = getCloseCorner(Vs[fIndex21].P, Vo, i);
							newVs[fIndex23].P = getCloseCorner(Vs[fIndex21].P, Vo, i);

							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex23)
									newFs[j].v1 = fIndex21;
								if (newFs[j].v2 == fIndex23)
									newFs[j].v2 = fIndex21;
								if (newFs[j].v3 == fIndex23)
									newFs[j].v3 = fIndex21;
							}
						}
						//22-23 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex22].P, Vo, i), getCloseCorner(Vs[fIndex23].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 22-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex22].P = getCloseCorner(Vs[fIndex22].P, Vo, i);
							newVs[fIndex23].P = getCloseCorner(Vs[fIndex22].P, Vo, i);


							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex23)
									newFs[j].v1 = fIndex22;
								if (newFs[j].v2 == fIndex23)
									newFs[j].v2 = fIndex22;
								if (newFs[j].v3 == fIndex23)
									newFs[j].v3 = fIndex22;
							}
						}
						else
						{
							printf("!!Case9 ErrorFace 3번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//4번 Face가 오류
					if (Fs[tempStart + 3].isErrorFace == 1)
					{
						fIndex31 = Fs[tempStart + 3].v1;	//face index 값 넣기 1
						fIndex32 = Fs[tempStart + 3].v2;	//face index 값 넣기 2
						fIndex33 = Fs[tempStart + 3].v3;	//face index 값 넣기 3

						//31-32 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex31].P, Vo, i), getCloseCorner(Vs[fIndex32].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 21-22번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex31].P = getCloseCorner(Vs[fIndex31].P, Vo, i);
							newVs[fIndex32].P = getCloseCorner(Vs[fIndex31].P, Vo, i);

							Fs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 3].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex32)
									newFs[j].v1 = fIndex31;
								if (newFs[j].v2 == fIndex32)
									newFs[j].v2 = fIndex31;
								if (newFs[j].v3 == fIndex32)
									newFs[j].v3 = fIndex31;
							}
						}
						//31-33 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex31].P, Vo, i), getCloseCorner(Vs[fIndex33].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 21-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex31].P = getCloseCorner(Vs[fIndex31].P, Vo, i);
							newVs[fIndex33].P = getCloseCorner(Vs[fIndex31].P, Vo, i);

							Fs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 3].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex33)
									newFs[j].v1 = fIndex31;
								if (newFs[j].v2 == fIndex33)
									newFs[j].v2 = fIndex31;
								if (newFs[j].v3 == fIndex33)
									newFs[j].v3 = fIndex31;
							}
						}
						//32-33 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex32].P, Vo, i), getCloseCorner(Vs[fIndex33].P, Vo, i)))
						{
							//printf("Case9 ErrorFace 22-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex32].P = getCloseCorner(Vs[fIndex32].P, Vo, i);
							newVs[fIndex33].P = getCloseCorner(Vs[fIndex32].P, Vo, i);


							Fs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 3].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[9] == 0)
							{
								nowErrorCase[9]++;
								nowErrorCaseTemp[9] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex33)
									newFs[j].v1 = fIndex32;
								if (newFs[j].v2 == fIndex33)
									newFs[j].v2 = fIndex32;
								if (newFs[j].v3 == fIndex33)
									newFs[j].v3 = fIndex32;
							}
						}
						else
						{
							printf("!!Case9 ErrorFace 4번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
				}
				else
				{
					printf("!!Case9 face %d개 수정필요\n", Vo[i].vFaceNum);
				}
			}
			//Case9 제거 완료
			
			
			//Case14

			if (Vo[i].voxelCase == 14)
			{
				Point nearPoint0;
				int fIndex01 = 0, fIndex02 = 0, fIndex03 = 0;
				int fIndex11 = 0, fIndex12 = 0, fIndex13 = 0;
				int fIndex21 = 0, fIndex22 = 0, fIndex23 = 0;
				int fIndex31 = 0, fIndex32 = 0, fIndex33 = 0;
				nowErrorCaseTemp[14] = 0;

				//한개씩 처리 4번
				if (Vo[i].vErrorFaceNum > 0 && Vo[i].vFaceNum == 4)
				{
					//printf("%d index 복셀 Case14\n", Vo[i].voxelIndex);

					tempStart = Vo[i].startIndex;	//복셀의 face 파일 시작 index 값 넣기
					//printf("%lf %lf %lf\n", newVs[Fs[tempStart].v1].P.x, newVs[Fs[tempStart].v1].P.y, newVs[Fs[tempStart].v1].P.z);

					//1번 Face가 오류
					if (Fs[tempStart].isErrorFace == 1)
					{
						fIndex01 = Fs[tempStart].v1;	//face index 값 넣기 1
						fIndex02 = Fs[tempStart].v2;	//face index 값 넣기 2
						fIndex03 = Fs[tempStart].v3;	//face index 값 넣기 3

						//01-02 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex02].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 01-02번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex02)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex02)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex02)
									newFs[j].v3 = fIndex01;
							}
						}
						//01-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex01].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 01-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex01].P = getCloseCorner(Vs[fIndex01].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex01].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex01;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex01;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex01;
							}
						}
						//02-03 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex02].P, Vo, i), getCloseCorner(Vs[fIndex03].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 02-03번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex02].P = getCloseCorner(Vs[fIndex02].P, Vo, i);
							newVs[fIndex03].P = getCloseCorner(Vs[fIndex02].P, Vo, i);

							Fs[tempStart].use = 0;					// 이 face list에 넣지 않게 표시
							newFs[tempStart].use = 0;				// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex03)
									newFs[j].v1 = fIndex02;
								if (newFs[j].v2 == fIndex03)
									newFs[j].v2 = fIndex02;
								if (newFs[j].v3 == fIndex03)
									newFs[j].v3 = fIndex02;
							}
						}
						else
						{
							printf("!!Case14 ErrorFace 1번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//2번 Face가 오류
					if (Fs[tempStart + 1].isErrorFace == 1)
					{
						fIndex11 = Fs[tempStart + 1].v1;	//face index 값 넣기 1
						fIndex12 = Fs[tempStart + 1].v2;	//face index 값 넣기 2
						fIndex13 = Fs[tempStart + 1].v3;	//face index 값 넣기 3

						//11-12 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex12].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 11-12번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex12)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex12)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex12)
									newFs[j].v3 = fIndex11;
							}
						}
						//11-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex11].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 11-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex11].P = getCloseCorner(Vs[fIndex11].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex11].P, Vo, i);

							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex11;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex11;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex11;
							}
						}
						//12-13 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex12].P, Vo, i), getCloseCorner(Vs[fIndex13].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 12-13번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex12].P = getCloseCorner(Vs[fIndex12].P, Vo, i);
							newVs[fIndex13].P = getCloseCorner(Vs[fIndex12].P, Vo, i);


							Fs[tempStart + 1].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 1].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex13)
									newFs[j].v1 = fIndex12;
								if (newFs[j].v2 == fIndex13)
									newFs[j].v2 = fIndex12;
								if (newFs[j].v3 == fIndex13)
									newFs[j].v3 = fIndex12;
							}
						}
						else
						{
							printf("!!Case14 ErrorFace 2번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//3번 Face가 오류
					if (Fs[tempStart + 2].isErrorFace == 1)
					{
						fIndex21 = Fs[tempStart + 2].v1;	//face index 값 넣기 1
						fIndex22 = Fs[tempStart + 2].v2;	//face index 값 넣기 2
						fIndex23 = Fs[tempStart + 2].v3;	//face index 값 넣기 3

						//21-22 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex21].P, Vo, i), getCloseCorner(Vs[fIndex22].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 21-22번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex21].P = getCloseCorner(Vs[fIndex21].P, Vo, i);
							newVs[fIndex22].P = getCloseCorner(Vs[fIndex21].P, Vo, i);

							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex22)
									newFs[j].v1 = fIndex21;
								if (newFs[j].v2 == fIndex22)
									newFs[j].v2 = fIndex21;
								if (newFs[j].v3 == fIndex22)
									newFs[j].v3 = fIndex21;
							}
						}
						//21-23 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex21].P, Vo, i), getCloseCorner(Vs[fIndex23].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 21-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex21].P = getCloseCorner(Vs[fIndex21].P, Vo, i);
							newVs[fIndex23].P = getCloseCorner(Vs[fIndex21].P, Vo, i);

							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex23)
									newFs[j].v1 = fIndex21;
								if (newFs[j].v2 == fIndex23)
									newFs[j].v2 = fIndex21;
								if (newFs[j].v3 == fIndex23)
									newFs[j].v3 = fIndex21;
							}
						}
						//22-23 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex22].P, Vo, i), getCloseCorner(Vs[fIndex23].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 22-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex22].P = getCloseCorner(Vs[fIndex22].P, Vo, i);
							newVs[fIndex23].P = getCloseCorner(Vs[fIndex22].P, Vo, i);


							Fs[tempStart + 2].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 2].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex23)
									newFs[j].v1 = fIndex22;
								if (newFs[j].v2 == fIndex23)
									newFs[j].v2 = fIndex22;
								if (newFs[j].v3 == fIndex23)
									newFs[j].v3 = fIndex22;
							}
						}
						else
						{
							printf("!!Case14 ErrorFace 3번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
					//4번 Face가 오류
					if (Fs[tempStart + 3].isErrorFace == 1)
					{
						fIndex31 = Fs[tempStart + 3].v1;	//face index 값 넣기 1
						fIndex32 = Fs[tempStart + 3].v2;	//face index 값 넣기 2
						fIndex33 = Fs[tempStart + 3].v3;	//face index 값 넣기 3

						//31-32 짧은 거리
						if (IsSamePoint(getCloseCorner(Vs[fIndex31].P, Vo, i), getCloseCorner(Vs[fIndex32].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 21-22번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex31].P = getCloseCorner(Vs[fIndex31].P, Vo, i);
							newVs[fIndex32].P = getCloseCorner(Vs[fIndex31].P, Vo, i);

							Fs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 3].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 2번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex32)
									newFs[j].v1 = fIndex31;
								if (newFs[j].v2 == fIndex32)
									newFs[j].v2 = fIndex31;
								if (newFs[j].v3 == fIndex32)
									newFs[j].v3 = fIndex31;
							}
						}
						//31-33 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex31].P, Vo, i), getCloseCorner(Vs[fIndex33].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 21-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex31].P = getCloseCorner(Vs[fIndex31].P, Vo, i);
							newVs[fIndex33].P = getCloseCorner(Vs[fIndex31].P, Vo, i);

							Fs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 3].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 1번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex33)
									newFs[j].v1 = fIndex31;
								if (newFs[j].v2 == fIndex33)
									newFs[j].v2 = fIndex31;
								if (newFs[j].v3 == fIndex33)
									newFs[j].v3 = fIndex31;
							}
						}
						//32-33 짧은 거리
						else if (IsSamePoint(getCloseCorner(Vs[fIndex32].P, Vo, i), getCloseCorner(Vs[fIndex33].P, Vo, i)))
						{
							//printf("Case14 ErrorFace 22-23번 index edge가 짧다\n");

							//가장 짧은 edge 값에 위치를 가장 가까운 꼭짓점으로 이동한다.
							newVs[fIndex32].P = getCloseCorner(Vs[fIndex32].P, Vo, i);
							newVs[fIndex33].P = getCloseCorner(Vs[fIndex32].P, Vo, i);


							Fs[tempStart + 3].use = 0;				// 이 face list에 넣지 않게 표시
							newFs[tempStart + 3].use = 0;			// 이 face list에 넣지 않게 표시
							newFaceNum--;							// face 개수 한개 제거
							if (nowErrorCaseTemp[14] == 0)
							{
								nowErrorCase[14]++;
								nowErrorCaseTemp[14] = 1;
							}

							for (int j = 0; j < faceNum; j++)		// face list에서 3번 값을 2번 값으로 바꾸기
							{
								if (newFs[j].v1 == fIndex33)
									newFs[j].v1 = fIndex32;
								if (newFs[j].v2 == fIndex33)
									newFs[j].v2 = fIndex32;
								if (newFs[j].v3 == fIndex33)
									newFs[j].v3 = fIndex32;
							}
						}
						else
						{
							printf("!!Case14 ErrorFace 4번 face 오류 제거 과정 중 오류 발생\n");
						}
					}
				}
				else
				{
					printf("!!Case14 face %d개 수정필요\n", Vo[i].vFaceNum);
				}
			}
			//Case14 제거 완료
			

			//남은건 Case 5이지만 기본 Case 5는 Error Face가 생길 수 없는 형태
		}
	}


	//p1.x =  Vs

	printf("참조선2\n");

	int plusErrorFace = 0;			//세점 중 같은 점이 있어서 추가로 제거되는 Face
	int AlgoErrorFace = 0;			//알고리즘 코드로 인해 제거되는 Face

	for (i = 0; i < faceNum; i++)
	{
		if (newFs[i].use != 0)
		{
			if (newFs[i].v1 == newFs[i].v2 || newFs[i].v1 == newFs[i].v3 || newFs[i].v2 == newFs[i].v3) // face가 되지않는 경우
			{
				plusErrorFace++;
			}
		}
	}
	printf("newFaceNum : %d\nplusErrorFace : %d\n", newFaceNum, plusErrorFace);
	AlgoErrorFace = faceNum - newFaceNum;
	newFaceNum -= plusErrorFace;		//알고리즘으로 인해 제거된 Face 개수에서 같은 점으로 인해 제거된 Face도 빼기

	

	fprintf(fpNewVOFF, "OFF\n\n");
	fprintf(fpNewVOFF, "%d %d 0\n", vertexNum, newFaceNum);
	for (i = 0; i < vertexNum; i++)
	{
		fprintf(fpNewVOFF, "%lf %lf %lf\n", newVs[i].P.x, newVs[i].P.y, newVs[i].P.z);
	}
	for (i = 0; i < faceNum; i++)
	{
		if (newFs[i].use != 0)
		{
			if (newFs[i].v1 != newFs[i].v2 && newFs[i].v1 != newFs[i].v3 && newFs[i].v2 != newFs[i].v3) // face가 되지않는 경우 제거(선 또는 점)
			{
				fprintf(fpNewVOFF, "3 %d %d %d\n", newFs[i].v1, newFs[i].v2, newFs[i].v3);
			}
		}
	}

	delete[] Vo;
	delete[] Vs;
	delete[] Fs;

	fprintf(fpErrorCase, "각 Case 별 Skinny triangle을 포함하는 Voxel의 수\n");
	for (i = 1; i <= 14; i++)
	{
		printf("%6d", i);
		fprintf(fpErrorCase, "%8d", i);
	}
	printf("\n");
	fprintf(fpErrorCase, "\n");
	for (i = 1; i <= 14; i++)
	{
		printf("%6d", errorCase[i]);
		fprintf(fpErrorCase, "%8d", errorCase[i]);
		AllErrorCase += errorCase[i];
	}
	printf("\n");
	fprintf(fpErrorCase, "\n");
	for (i = 1; i <= 14; i++)
	{
		printf("%6d", nowErrorCase[i]);
		fprintf(fpErrorCase, "%8d", nowErrorCase[i]);
		AllnowErrorCase += nowErrorCase[i];
	}

	fprintf(fpErrorCase, "\n제거 가능 Case (1, 3, 7, 10, 12), (2, 11, 13), (4 ,6), (8,9,14)\n");
	fprintf(fpErrorCase, "(복셀)전체 Error Case : %d개\n", AllErrorCase);
	fprintf(fpErrorCase, "(복셀)실제 제거 Error Case : %d개\n", AllnowErrorCase);
	fprintf(fpErrorCase, "(복셀)현재 제거 비율 %lf\n", (double)AllnowErrorCase / valVoxelNum);
	fprintf(fpErrorCase, "(삼각면)알고리즘에 의해 제거된 Face : %d개\n", AlgoErrorFace);
	fprintf(fpErrorCase, "(삼각면)추가로 제거된 Face : %d개\n", plusErrorFace);
	fprintf(fpErrorCase, "(삼각면)전체 Face %d개 에서 총 %d개 제거되서 face는 총 %d개 이다.\n", faceNum, AlgoErrorFace + plusErrorFace, newFaceNum);
	


	fclose(fpErrorCase);
	fclose(fpVOFF);
	fclose(fpEdgePassCheckFile1);
	//fclose(fpErrorFace);
	fclose(fpVoxelCase);
	fclose(fpNewVOFF);

	return 0;
}

// Face에 가까운 꼭짓점 위치 구하기 (Case 1용)
Point findNearIndex(faceSet Fs[], vertexSet Vs[], int n)
{
	Point nearPoint, p1, p2, p3;
	nearPoint.x = 0;
	nearPoint.y = 0;
	nearPoint.z = 0;

	p1 = Vs[Fs[n].v1].P;
	p2 = Vs[Fs[n].v2].P;
	p3 = Vs[Fs[n].v3].P;

	if (p1.x == p2.x || p1.x == p3.x)
	{
		nearPoint.x = p1.x;
	}
	else if (p2.x == p3.x)
	{
		nearPoint.x = p2.x;
	}
	else
	{
		printf("findNearIndex X 에러\n");
	}

	if (p1.y == p2.y || p1.y == p3.y)
	{
		nearPoint.y = p1.y;
	}
	else if (p2.y == p3.y)
	{
		nearPoint.y = p2.y;
	}
	else
	{
		printf("findNearIndex Y 에러\n");
	}

	if (p1.z == p2.z || p1.z == p3.z)
	{
		nearPoint.z = p1.z;
	}
	else if (p2.z == p3.z)
	{
		nearPoint.z = p2.z;
	}
	else
	{
		printf("findNearIndex Z 에러\n");
	}

	return nearPoint;
}

/*
//점이 복셀의 양끝 쪽짓점이 아니면 edge의 중간 값에 넣기 (미사용)
void setCenterNotCorner(Point p1, Point* newp1, Voxel* Vo, int i)
{
	if (p1.x != (Vo[i].b * stepSize) && p1.x != ((Vo[i].b * stepSize) + stepSize))	//x값이 복셀의 양끝 쪽짓점이 아니면 중간 값 넣기
	{
		(*newp1).x = (Vo[i].b * stepSize) + half_stepSize;
		//printf("체인지x ");
	}
	if (p1.y != (Vo[i].c * stepSize) && p1.y != ((Vo[i].c * stepSize) + stepSize))	//y값이 복셀의 양끝 쪽짓점이 아니면 중간 값 넣기
	{
		(*newp1).y = (Vo[i].c * stepSize) + half_stepSize;
		//printf("체인지y ");
	}
	if (p1.z != (Vo[i].h * stepSize) && p1.z != ((Vo[i].h * stepSize) + stepSize))	//z값이 복셀의 양끝 쪽짓점이 아니면 중간 값 넣기
	{
		(*newp1).z = (Vo[i].h * stepSize) + half_stepSize;
		//printf("체인지z ");
	}
	//printf("\n");
}
*/
//점에서 가장 가까운 코너를 찾은뒤 해당 값으로 바꾸기
void setCloseCorner(Point p1, Point* newp1, Voxel Vo[], int i)
{
	double xBound = 0, yBound = 0, zBound = 0;
	xBound = (Vo[i].b * stepSize) + half_stepSize;
	yBound = (Vo[i].c * stepSize) + half_stepSize;
	zBound = (Vo[i].h * stepSize) + half_stepSize;

	//x축 가까운 값 찾기
	if (p1.x >= xBound)
		(*newp1).x = (Vo[i].b * stepSize) + stepSize;
	else
		(*newp1).x = (Vo[i].b * stepSize);
	//y축 가까운 값 찾기
	if (p1.y >= yBound)
		(*newp1).y = (Vo[i].c * stepSize) + stepSize;
	else
		(*newp1).y = (Vo[i].c * stepSize);
	//z축 가까운 값 찾기
	if (p1.z >= zBound)
		(*newp1).z = (Vo[i].h * stepSize) + stepSize;
	else
		(*newp1).z = (Vo[i].h * stepSize);
}

//점에서 가장 가까운 코너 찾기
Point getCloseCorner(Point p1, Voxel Vo[], int i)
{
	Point result;
	double xBound = 0, yBound = 0, zBound = 0;
	xBound = (Vo[i].b * stepSize) + half_stepSize;
	yBound = (Vo[i].c * stepSize) + half_stepSize;
	zBound = (Vo[i].h * stepSize) + half_stepSize;

	//x축 가까운 값 찾기
	if (p1.x >= xBound)
		result.x = (Vo[i].b * stepSize) + stepSize;
	else
		result.x = (Vo[i].b * stepSize);
	//y축 가까운 값 찾기
	if (p1.y >= yBound)
		result.y = (Vo[i].c * stepSize) + stepSize;
	else
		result.y = (Vo[i].c * stepSize);
	//z축 가까운 값 찾기
	if (p1.z >= zBound)
		result.z = (Vo[i].h * stepSize) + stepSize;
	else
		result.z = (Vo[i].h * stepSize);

	return result;
}

//두 점이 같은 위치 인가?
int IsSamePoint(Point p1, Point p2)
{
	if (p1.x == p2.x)
		if (p1.y == p2.y)
			if (p1.z == p2.z)
				return 1;

	return 0;
}

//두 Face가 인접한 Face인가? (한 edge만 공유)
int IsNearFace(int A, int B, faceSet Fs[])
{
	int inA1, inA2, inA3, inB1, inB2, inB3;

	inA1 = Fs[A].v1;
	inA2 = Fs[A].v2;
	inA3 = Fs[A].v3;

	inB1 = Fs[B].v1;
	inB2 = Fs[B].v2;
	inB3 = Fs[B].v3;

	if (inA1 == inB1 || inA1 == inB2 || inA1 == inB3) // A1이 같은 값 있음
	{
		if (inA2 == inB1 || inA2 == inB2 || inA2 == inB3) // A1, A2가 같은 값 있음
		{
			if (inA3 == inB1 || inA3 == inB2 || inA3 == inB3) // A1, A2, A3 같은 값 있음
			{
				printf("!!두 face 인접 비교시 같은 Face로 나오는 오류 발생\n");
				return 3;
			}
			else // A1, A2만 같은 값 있음
			{
				return 1;
			}
		}
		else if (inA3 == inB1 || inA3 == inB2 || inA3 == inB3) //A1, A3 같은 값 있음
		{
			return 1;
		}
		else //A1만 같은 값 있음 = 한 점만 공유
		{
			printf("!!두 face 인접 비교시 한점만 같은 위치 발생\n");
			return 2;
		}
	}
	else // A1이 같은 값에 없음
	{
		if (inA2 == inB1 || inA2 == inB2 || inA2 == inB3) // A2가 같은 값 있음
		{
			if (inA3 == inB1 || inA3 == inB2 || inA3 == inB3) // A2, A3 같은 값 있음
			{
				return 1;
			}
			else // A2만 같은 값 있음
			{
				printf("!!두 face 인접 비교시 한점만 같은 위치 발생\n");
				return 2;
			}
		}
		else if (inA3 == inB1 || inA3 == inB2 || inA3 == inB3) //A3 만 같은 값 있음
		{
			printf("!!두 face 인접 비교시 한점만 같은 위치 발생\n");
			return 2;
		}
		else //같은 값이 없음 인접하지 않음
		{
			return 0;
		}
	}
}


//현재 문제 사항(확실)
//1. 기본 Case와 다른 face 개수가 나옴
//Case 10 face 4개 나옴(기본은 2개)
//Case 12 face 5개 나옴(기본은 3개)
//Case 11 face 5개 나옴(기본은 3개)
//현재 if 문으로 출력제외 시켜놓음