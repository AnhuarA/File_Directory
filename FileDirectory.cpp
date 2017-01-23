#include "stdafx.h"
#include "FileDirectory.h"
#include <iostream>

using namespace std;

FileDirectory::FileDirectory() { // : to initialize all entries in the fileDirectory and FAT16 to be 0; i.e.safe values.
	for (int i = 0; i < 256; i++) {
		FAT16[i] = 0;
	}
	for(int i = 0; i < 4; i++) {
		for (int j = 0; j < 32; j++) {
			fileDirectory[i][j] = 0;
		}
	}
}

bool FileDirectory::create(char   filename[], int numberBytes) {
	int i, j;
	int clustersNeeded, availableClusters;
	availableClusters = 0;

	clustersNeeded = numberBytes / 4;
	if (numberBytes % 4 != 0)
		clustersNeeded = clustersNeeded + 1;

//(1)	to check if there is an unused entry in the File Directory;  (i.e.the first character of the file name in the File Directory is zero).Return false if not true.
	for (i = 0; i < 4; i++) {
		if (fileDirectory[i][0] == '\0') {//Unused entry found
//(2)	to check if there are  enough unused clusters to store the file with the numberBytes.Return false if not true.
			for (j = 2; j < 256; j++) {//count amount of available clusters
				if (FAT16[j] == 0 || FAT16[j] == 1)
					availableClusters++;
			}
			if (availableClusters < clustersNeeded) {
				cout << "\nNot enough empty clusters" << endl;
				return false;//Not enought empty clusters
			}
			else return true;
		}//end of 
	}
	if (i == 4)
		return false; //There are no unused entries
}

bool FileDirectory::deleteFile(char   filename[]) {
//(0)	to check if the file to be deleted; filename[]; is in the Directory.If not; return false.
	int i, j, k;
	unsigned short int firstClusterAddress, nextClusterAddress, clusterAddress;
	unsigned short int storeClusterLocation[256];
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
			if (fileDirectory[i][j] != filename[j])
				break;
		}
		if (j == 8) {// found a file 
//(1)	to change the first character of the file name in the File Directory to be zero;
			fileDirectory[i][0] = 0;
//(2)	to change all entries of the clusters of this file in the FAT to 1; i.e.; unused.
			firstClusterAddress = fileDirectory[i][26] + (fileDirectory[i][27] << 8);
			clusterAddress = firstClusterAddress;
			for (k = 0; k < 256 && clusterAddress < 0xFFF8; k++) 
			{
				storeClusterLocation[k] = FAT16[clusterAddress];
				clusterAddress = FAT16[clusterAddress];
			}
		
			for (int M = 0; M < k; M++) {
				FAT16[storeClusterLocation[M]] = 1;
			}
			return true;// file deleted
		}//end of j loop
	}//end of i loop
	if (i == 4)//file not found
		return false;
	
}

bool FileDirectory::read(char   filename[]) {
//purpose: to read  of data from data[] array from the file with the specified file name.
//(0)	to check if the file to be read; filename[]; is in the Directory.If not; return false.
	int i, j, k, p; 
	unsigned short int firstClusterAddress, nextClusterAddress, clusterAddress;
	unsigned short int storeClusterLocation[256];
	unsigned char fileData[1024];

	for(i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
			if (fileDirectory[i][j] != filename[j])
				break;	
		}
		if (j == 8) {// found a file in the directory matching the input file name
//(1)	use the file name to get the file information from the File Directory; including date; time; number of bytes and the first cluster address;
//(2)	use the first cluster address to get all the cluster addresses of this file from the FAT - 16;
			firstClusterAddress = fileDirectory[i][26] + (fileDirectory[i][27] << 8);
			clusterAddress = firstClusterAddress;
			for (k = 0; k < 256 && clusterAddress < 0xFFF8; k++) 
			{
				storeClusterLocation[k] = clusterAddress;
				clusterAddress = FAT16[clusterAddress];
			}
//(3)	use all the cluster addresses to read the data from the disk / flash memory.
			p = 0;
			for (int M = 0; M < k; M++) {
				for (int L = 0; L < 4; L++) {
					fileData[p] = data[storeClusterLocation[M] * 4 + L];
					p = p + 1;
				}
			}
		}//end of j loop
	}//end of i loop
	if (i == 4)
		return false;
	else return true;
}

void FileDirectory::write(const char   filename[], const char fileExt[], int numberBytes, char dataInput[], int year, int month, int day, int hour, int minute, int second) {
//	purpose: to write numberBytes bytes of data from data[] array into the file with the specified file name
//(0)	to look for the first unused entry(0 or 1) in the FAT - 16; and use it as the First Cluster Address.
	unsigned short int firstClusterAddress;
	unsigned short int savedClusters, numClusters;


	unsigned short int nextClusterAddress, currentClusterAddress;
	int j;

	for (int i = 2; i < 256; i++) {
		if (FAT16[i] == 0 || FAT16[i] == 1) {
			firstClusterAddress = i;
			break;
		}
	}

//(1)	to look for the next unused entry(0 or 1) in the FAT - 16; and use it as the Next Cluster Address; and write its value into the FAT - 16.
	savedClusters = 1;
	currentClusterAddress = firstClusterAddress;
	numClusters = numberBytes / 4;
	if (numberBytes % 4 != 0)
		numClusters = numClusters + 1;

	while (savedClusters < numClusters) {
		for (int i = currentClusterAddress + 1; i < 256; i++) {
			if (FAT16[i] == 0 || FAT16[i] == 1) {
			    nextClusterAddress = i;
			    FAT16[currentClusterAddress] = nextClusterAddress;
				currentClusterAddress = nextClusterAddress;
				savedClusters++;
			}
			if (savedClusters == numClusters)
				break;
		}

	}
	//At end of file, point to last cluster to signify EOF
	FAT16[currentClusterAddress] = 255;
//(2)	Repeat Step 2 until all clusters are found and the FAT - 16 is updated.
//(3)	to write / update the file name; extension; date; time; file length and first cluster address into the first unused entry in the File Directory;
	unsigned int empty;
	for (int i = 0; i < 4; i++) {
		if (fileDirectory[i][0] == 0) {
			empty = i;
			break;
		}
	}
	//write data
	unsigned short int clusterAddress;//tempAddress holder
	unsigned short int storeClusterLocation[256];//array to hold cluster addresses
	int k;
	clusterAddress = firstClusterAddress;
	for (k = 0; k < numClusters; k++)
	{
		storeClusterLocation[k] = clusterAddress;

		clusterAddress = FAT16[clusterAddress];
	}


	for (int M = 0 ; M < k ; M++) {
				for (int L = 0; L < 4; L++) {
					data[storeClusterLocation[M] * 4 + L] = dataInput[(M * 4) + L ];
				}
			}
		
	//Write file name into the ith entry of the file directory
	for (j = 0; j < 8; j++) {
		fileDirectory[empty][j] = filename[j];
	}
	for (j= 8; j < 11; j++) {
		fileDirectory[empty][j] = fileExt[j - 8];
	}
	//write date into directory[25:24]
	fileDirectory[empty][25] = (year - 1980 << 1) + ((month >> 3) & 0x01);
	fileDirectory[empty][24] = (month << 5 & 0xE0)+ day;
	//write time into directory[23:22]
	fileDirectory[empty][23] = (hour << 3 ) + (minute >>3);
	fileDirectory[empty][22] = ((minute << 5) & 0xE0) + (second >> 1);
	//write first sector address into directory[27:26]
	fileDirectory[empty][27] = firstClusterAddress >> 8;
	fileDirectory[empty][26] = firstClusterAddress;
	//write file length into directory[31:28]
	fileDirectory[empty][31] = numberBytes >> 24;
	fileDirectory[empty][30] = numberBytes >> 16;
	fileDirectory[empty][29] = numberBytes >> 8;
	fileDirectory[empty][28] = numberBytes;

	
}

void FileDirectory::printClusters(char filename[]) {
//purpose: to print all the clusters of a file.
	int i, j, k;
	unsigned short int firstClusterAddress, nextClusterAddress, clusterAddress;
	unsigned short int storeClusterLocation[256];
//(1)	to check if the file to be printed; filename[]; is in the Directory.If not; return false.
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
			if (fileDirectory[i][j] != filename[j])
				break;
		}
		if (j == 8) {
//(2)	use the file name to get the file information from the File Directory; including the first cluster address;
			firstClusterAddress = fileDirectory[i][26] + (fileDirectory[i][27] << 8);
//(3)	use the first cluster address to get all cluster addresses from the FAT - 16;
			clusterAddress = firstClusterAddress;
			
			cout << "\Printing cluster locations for ";
			for (int j = 0; j < 8; j++) {
				if (fileDirectory[i][j] != 0)
					cout << fileDirectory[i][j];
			}
			cout << '.';

			for (int j = 8; j < 11; j++) {
				cout << fileDirectory[i][j];
			}
			cout << endl;
			cout << clusterAddress;
			for (k = 0; k < 256 && clusterAddress < 255; k++)
			{
				storeClusterLocation[k] = FAT16[clusterAddress];
				clusterAddress = FAT16[clusterAddress];
				cout << "\n" << clusterAddress;
			}

			cout << endl;

		}
	}




}

void FileDirectory::printDirectory() {

	unsigned short int date, time, firstClusterAddress,fileLength, decodedYear;

	cout << "\nFile Name\t" << "Date Created\t" << "Time Created\t" << "File Size" << endl;
	cout << "---------------------------------------------------------" << endl;
	//prints all the  files of the directory.
	//(1)	If the file name is valid, print file name, '.', and file extension
	for (int i = 0; i < 4; i++) {
		if (fileDirectory[i][0] != '\0') {
			for (int j = 0; j < 8; j++) {
				if(fileDirectory[i][j] != 0)
					cout << fileDirectory[i][j];
			}
			cout << '.';

			for (int j = 8; j < 11; j++) {
				cout << fileDirectory[i][j];
			}
			cout << "\t";
		//(2) Print date
			date = (fileDirectory[i][25] << 8) + fileDirectory[i][24];
			decodedYear = ((date & 0xFE00) >> 9);
		
			cout << ((date & 0x01E0) >> 5);	//output month
			cout << '/' << (date & 0x001F);	//output day
			cout << '/' << 1980 + decodedYear; //output year
		//(3) Print time
			time = (fileDirectory[i][23] << 8) + fileDirectory[i][22];
			cout <<"\t" << ((time & 0xF800) >> 11);//hour
			cout << ':' << ((time & 0x07E0) >> 5);//minute
			cout << ':' << ((time & 0x001F) << 1);//second 
			//second
		//(4) Print file length
			fileLength = (fileDirectory[i][31] << 24) + (fileDirectory[i][30] << 16) + (fileDirectory[i][29] << 8) +fileDirectory[i][28];
			cout << "\t" << fileLength << endl;

		
			}
		
	}
}//end of function

void FileDirectory::printData(char filename[]) {
// prints the data of a file.
	int i, j, k;
	unsigned short int firstClusterAddress, nextClusterAddress, clusterAddress;
	unsigned short int storeClusterLocation[256];

	//Look for input file
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
			if (fileDirectory[i][j] != filename[j])
				break;
		}
		if (j == 8) {
//(1)	use the file name to get the file information from the File Directory; including the first cluster address;
			firstClusterAddress = fileDirectory[i][26] + (fileDirectory[i][27] << 8);
//(2)	use the first cluster address to get all cluster addresses from the FAT - 16;
			clusterAddress = firstClusterAddress;
			for (k = 0; k < 256 && clusterAddress < 255; k++)
			{
				storeClusterLocation[k] = FAT16[clusterAddress];
				clusterAddress = FAT16[clusterAddress];
			}
//(3)	use cluster address to read the data of the file.Use the file length to print these data in hexadecimal format.
			for (int M = 0; M < k; M++) {
				for (int L = 0; L < 4; L++) {
					cout << data[(storeClusterLocation[M] * 4) + L] << " ";
					if (M % 4 == 0)
						cout << endl;
					
				}
			}
		}
	}
}

