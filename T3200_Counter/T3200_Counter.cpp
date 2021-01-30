// T3200_Counter.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <iostream>
#include "ftd2xx.h"
#include <windows.h>
#include <stdio.h>
#include <string>


typedef unsigned long  uint32;

uint32 deserialize_uint32(char* buffer)
{
	uint32 value = 0;

	value |= buffer[0] << 24;
	value |= buffer[1] << 16;
	value |= buffer[2] << 8;
	value |= buffer[3];
	return value;

}

int main()
{
	//Variables declaration 
	FT_HANDLE ftHandle;
	FT_STATUS ftStatus;
	DWORD numDevs;
	DWORD Flags;
	DWORD ID;
	DWORD Type;
	DWORD LocId;
	PVOID SerialNumber[16];
	PVOID Description[64];
	DWORD BytesWritten;// needed to calibration
	DWORD BytesReceived;// needed to calibration
	DWORD TxBytes;
	DWORD RxBytes = 8;
	DWORD EventDWord;

	//Registers data below: 
	byte cmd_res[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };//Reset and start of calibration
	
	byte cmd_ctrl [] = { 0x04, 0x23, 0x00, 0x00, 0x00 };//Write control value in CTRL register, time interval, A<->B, 1s range
	byte cmd_en [] = { 0x03, 0x04, 0x00, 0x00, 0x00 };// Write of EN register BRAKE 
	byte cmd_rd_s[] = { 0xF2, 0x01, 0x00, 0x00, 0x00 };//Read of S register
	byte cmd_wr_s [] = { 0x02, 0xee, 0x04, 0x00, 0x00 };// Write of S register set START A, STOP B, slope rise (both), internal clock ON
	byte cmd_wr_dac  [] = { 0x05, 0x8f, 0x8f, 0x00, 0x00 }; //Write data for DAC  threshold 0.5 V to START/STOP.

	byte cmd_meas [] = { 0x01, 0x01, 0x00, 0x00, 0x00 };//Start of measurement
	byte cmd_rd_meas_no [] = { 0xF1, 0x01, 0x00, 0x00, 0x00 };//Read current number of executed measurements 
	byte cmd_rd_f_data [] = { 0xF0, 0x00, 0x00, 0x00, 0x00 };//Read data
	
	
	//Additional variables
	bool calibrated = false;
	uint32_t numBytesRead = 0;
	char RxBuffer[4 * 1024];
	uint32_t dataR = 0;
	uint32_t data = 0;


	//Code below:
	printf("T3200 APPLICATION\n");
	printf("Created by Mateusz Gorniak and Pawel Slapaczynski\n");
	printf("WYDZIAL ELEKTRONIKI WAT\n\n");
	printf("------------------------------------------------------------------------------\n");
	Sleep(1000);
	ftStatus = FT_CreateDeviceInfoList(&numDevs);
	if (ftStatus == FT_OK) {
		printf("Number of devices is %d\n", numDevs);
	}
	printf("------------------------------------------------------------------------------\n");
	Sleep(1000);
	ftStatus = FT_GetDeviceInfoDetail(0, &Flags, &Type, &ID, &LocId, SerialNumber, Description, &ftHandle);
	if (ftStatus == FT_OK) {
		printf("Infos about connected devices\n");
		Sleep(200);
		printf("Dev 0:\n");
		Sleep(200);
		printf(" Flags=0x%x\n", Flags);
		Sleep(200);
		printf(" Type=0x%x\n", Type);
		Sleep(200);
		printf(" ID=0x%x\n", ID);
		Sleep(200);
		printf(" LocId=0x%x\n", LocId);
		Sleep(200);
		printf(" SerialNumber=%s\n", SerialNumber);
		Sleep(200);
		printf(" Description=%s\n", Description);
		Sleep(200);
		printf(" ftHandle=0x%x\n", ftHandle);
		Sleep(200);
		printf("------------------------------------------------------------------------------\n");
	}
	Sleep(1000);

	ftStatus = FT_OpenEx((PVOID)"FTVW9H5E", FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
	if (ftStatus == FT_OK) {
		// FT_ResetDevice OK
		printf("FT_Device OK\n");
		printf("T3200u Counter was opened by SerialNumber: FTVW9H5E \n");
	}
	else {
		// FT_ResetDevice failed
		printf("FT_Device failed\n");
	}
	printf("------------------------------------------------------------------------------\n");

	if (ftStatus != FT_OK) printf("ftStatus not ok %d\n", ftStatus); //check for error
	else
	{ /*
		printf("Reseting Device ....\n\n");
		ftStatus = FT_ResetDevice(ftHandle);
		if (ftStatus == FT_OK) {
			// FT_ResetDevice OK
			printf("FT_ResetDevice OK\n");
		}
		else {
			// FT_ResetDevice failed
			printf("FT_ResetDevice failed\n");
		}
		printf("------------------------------------------------------------------------------\n");
		Sleep(1000);
		*/

		FT_Write(ftHandle, cmd_ctrl, sizeof(cmd_ctrl), &BytesWritten);
		FT_Write(ftHandle, cmd_en, sizeof(cmd_en), &BytesWritten);
	    FT_Write(ftHandle, cmd_wr_s, sizeof(cmd_wr_s), &BytesWritten);
		FT_Write(ftHandle, cmd_wr_dac, sizeof(cmd_wr_dac), &BytesWritten);

		printf("Wait on calibration\n");
		ftStatus = FT_Write(ftHandle, cmd_res, 5, &BytesWritten);
		if (ftStatus == FT_OK) {
			// FT_Write OK
			printf("FT_Write RESET REGISTER OK\n");
		}
		else {
			// FT_Write Failed
			printf("FT_Write RESET REGISTER Failed\n");
		}
		
		Sleep(5000);
		
		do
		{
			ftStatus = FT_Write(ftHandle, cmd_rd_f_data, sizeof(cmd_rd_f_data), &BytesWritten);
			FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
			if (RxBytes > 0) {
				ftStatus = FT_Read(ftHandle, RxBuffer, RxBytes, &BytesReceived);
				if (ftStatus == FT_OK) {
					// FT_Read OK
					//data =(byte)RxBuffer;
					data = deserialize_uint32(RxBuffer);
					std::cout << data;
					//std::cout << data << std::endl;
					//std::cout << dataR << std::endl;
					printf("*");
					//Sleep(50);
				}
				else {
					// FT_Read Failed
					printf("Read Failed");
				}
			}
			if ((data & 2048) == 2048) {calibrated = true; }
		} while (!calibrated);
		
		printf("\n\n");
		printf("Calibration done\n");
		printf("------------------------------------------------------------------------------\n");

		double Offset = 0.000000000000000; // declaration variable with big precission 
		//double Meas = 0.000000000000000;
		UINT64 Count = 0;
		uint32 A = 0;
		uint32 B = 0;

		cmd_rd_f_data[1] = 0x02;
		FT_Write(ftHandle, cmd_rd_f_data, 5, &BytesWritten);
		
		FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
		do{
			FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
			std::cout << RxBytes;
		} while (RxBytes < 8);
		printf("\n");

		ftStatus = FT_Read(ftHandle, RxBuffer, 8, &BytesReceived);
		if (ftStatus == FT_OK) {
			// FT_Read OK
			std::cout << "RxBuffer = " << RxBuffer << "\n";
			data = deserialize_uint32(RxBuffer);
			std::cout << "DATA = " << data << "\n";
		}
		else {
			// FT_Read Failed
			printf("Read Failed");
		}
		//Count = data >> 20;

		A = data & 0x3FF;
		std::cout << "A = " << A<< "\n";
		B = (data >> 10) & 0x3FF;
		std::cout << "B = " << B << "\n";
		Count = 0;
		//Count = (data << 12) | Count;

		std::cout  << "Count = " << Count << "\n";

		Offset = 4 * ((double)Count + (((double)A - (double)B) / 1024));
		std::cout<<"Offset = "<<Offset<< " ns\n";
		

	}
	ftStatus = FT_Close(ftHandle);
	printf("Press Key To End Program\n");

	return EXIT_SUCCESS;
}