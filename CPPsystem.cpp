// CPPsystem.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FileDirectory.h"
#include <iostream>

using namespace std;

void initData(char[], int);

int main()
{
	char f1Data[40], f2Data[200], f3Data[300], f4Data[500]; //initialize data arrays
	initData(f1Data, 40);
	initData(f2Data, 200);
	initData(f3Data, 300);
	initData(f4Data, 500);

	FileDirectory f1;	//Creates a new file directory object

	//1.	create and write a file; file1;  of 40 bytes;

	if (f1.create("file1", 40)) {
		f1.write("file1", "txt", 40, f1Data, 2017, 01, 21, 14, 52, 43);
		cout << "file1 written.\n\n";
	}
	//2.	create and write a file; file2;  of 200 bytes;
	if (f1.create("file2", 200)) {
		f1.write("file2", "txt", 200, f2Data, 2017, 01, 20, 8, 30, 14);
		cout << "file2 written.\n\n";
	}
	//3.	create and write a file; file3;  of 300 bytes;
	if (f1.create("file3", 300)) {
		f1.write("file3", "doc", 300, f3Data, 2016, 12, 13, 17, 52, 43);
		cout << "file3 written.\n\n";
	}
	//4.	create and write a file; file4;  of 500 bytes.
	f1.create("file4", 500);

	//5.	delete file2;
	if(f1.deleteFile("file2"))
		cout << "file2 deleted.\n\n";

	//6.	create and write a file; file4;  of 500 bytes.
	if (f1.create("file4", 500)) {
		f1.write("file4", "cpp", 500, f4Data, 2017, 01, 17, 13, 12, 13);
		cout << "file4 written.\n\n";
	}

	//Print directory
	f1.printDirectory();
	//Print clusters for each file
	f1.printClusters("file1");
	f1.printClusters("file3");
	f1.printClusters("file4");
	//Print data to check that data is being written
	f1.printData("file1");
	system("pause");
    return 0;
}

void initData(char input[], int bytes){
	for (int i = 0; i < bytes; i++) {
		input[i] = i;
	}
}