/*
 * max7219_digits.c
 *
 *  The MIT License.
 *  Created on: 01.02.2019
 *		Author: Mateusz Salamon
 *		www.msalamon.pl
 *		mateusz@msalamon.pl
 *
 *      Website: https://msalamon.pl/nigdy-wiecej-multipleksowania-na-gpio!-max7219-w-akcji-cz-1/
 *      GitHub:  https://github.com/lamik/MAX7219_digits_STM32_HAL
 */


#include "main.h"
#include "spi.h"

#include <string.h>
#include "max7219_digits.h"

#ifndef SPI_CS_HARDWARE_CONTROL
#include "gpio.h"
#endif
SPI_HandleTypeDef *max7219_spi;
uint8_t Max7219SpiBuffer[MAX7219_DEVICES * 2];

const uint8_t Max7219_Digits[] = {
		0x00,	// Clear
		0x7E,	// 0
		0x30,	// 1
		0x6D,	// 2
		0x79,	// 3
		0x33,	// 4
		0x5B,	// 5
		0x5F,	// 6
		0x70,	// 7
		0x7F,	// 8
		0x7B,	// 9
		0x77,	// A
		0x1F,	// b
		0x0D,	// c
		0x4E,	// C
		0x3D,	// d
		0x6F,	// e
		0x4F,	// E
		0x47,	// F
		0x1E, 	// h
		0x37,	// H
		0x06,	// I
		0x38,	// J
		0x0E,	// L
		0x15, 	// n
		0x1D,	// o
		0x67,	// P
		0x05,	// r
				// S - same as 5
		0x1C,	// u
		0x3E,	// U
		0x3B,	// y
				// Z - same as 2
		0x63,	// *
};

MAX7219_STATUS MAX7219_SendToDevice(uint8_t DeviceNumber, uint8_t Register, uint8_t Data)
{
	uint8_t Offset = DeviceNumber * 2;

	memset(Max7219SpiBuffer, 0x00, (MAX7219_DEVICES * 2));
	Max7219SpiBuffer[(MAX7219_DEVICES * 2)-Offset-2] = Register;
	Max7219SpiBuffer[(MAX7219_DEVICES * 2)-Offset-1] = Data;

#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(MAX7219_CS_GPIO_Port, MAX7219_CS_Pin, GPIO_PIN_RESET);
#endif

	if(HAL_OK != HAL_SPI_Transmit(max7219_spi, Max7219SpiBuffer, (MAX7219_DEVICES * 2), 10))
	{
		return MAX7219_ERROR;
	}

#ifndef SPI_CS_HARDWARE_CONTROL
	HAL_GPIO_WritePin(MAX7219_CS_GPIO_Port, MAX7219_CS_Pin, GPIO_PIN_SET);
#endif

	return MAX7219_OK;
}

MAX7219_STATUS MAX7219_SetDecodeMode(uint8_t DeviceNumber, MAX7219_DecodeMode DecodeMode)
{
	if(DeviceNumber >= MAX7219_DEVICES || DecodeMode > 4)
	{
		return MAX7219_ERROR;
	}

	if(MAX7219_OK != MAX7219_SendToDevice(DeviceNumber, MAX7219_DECODE_MODE_REGISTER, DecodeMode))
	{
		return MAX7219_ERROR;
	}

	return MAX7219_OK;
}

MAX7219_STATUS MAX7219_SetIntensity(uint8_t DeviceNumber, uint8_t Intensity)
{
	if(DeviceNumber >= MAX7219_DEVICES || Intensity > 16)
	{
		return MAX7219_ERROR;
	}

	if(MAX7219_OK != MAX7219_SendToDevice(DeviceNumber, MAX7219_INTENSITY_REGISTER, Intensity))
	{
		return MAX7219_ERROR;
	}

	return MAX7219_OK;
}

MAX7219_STATUS MAX7219_SetScanLimit(uint8_t DeviceNumber, MAX7219_ScanLimit Limit)
{
	if(DeviceNumber >= MAX7219_DEVICES || Limit > 8)
	{
		return MAX7219_ERROR;
	}

	if(MAX7219_OK != MAX7219_SendToDevice(DeviceNumber, MAX7219_SCAN_LIMIT_REGISTER, Limit))
	{
		return MAX7219_ERROR;
	}

	return MAX7219_OK;
}

MAX7219_STATUS MAX7219_Shutdown(uint8_t DeviceNumber, MAX7219_ShutdownMode Shutdown)
{
	if(DeviceNumber >= MAX7219_DEVICES)
	{
		return MAX7219_ERROR;
	}

	if(MAX7219_OK != MAX7219_SendToDevice(DeviceNumber, MAX7219_SHUTDOWN_REGISTER, Shutdown?1:0))
	{
		return MAX7219_ERROR;
	}

	return MAX7219_OK;
}

MAX7219_STATUS MAX7219_SetDisplayTest(uint8_t DeviceNumber, MAX7219_TestMode Enable)
{
	if(DeviceNumber >= MAX7219_DEVICES)
	{
		return MAX7219_ERROR;
	}

	if(MAX7219_OK != MAX7219_SendToDevice(DeviceNumber, MAX7219_DISPLAY_TEST_REGISTER, Enable?1:0))
	{
		return MAX7219_ERROR;
	}

	return MAX7219_OK;
}

MAX7219_STATUS MAX7219_SetDigit(uint8_t DeviceNumber, uint8_t Digit, uint8_t Value, uint8_t Dot)
{
	uint8_t ValueToSend;
	if(DeviceNumber >= MAX7219_DEVICES)
	{
		return MAX7219_ERROR;
	}

	if(Digit > 7 || Digit < 0)
	{
		return MAX7219_OUT_OF_RANGE;
	}

	switch (Value)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			ValueToSend = Max7219_Digits[Value+1];
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			ValueToSend = Max7219_Digits[Value-47];
			break;
		case ' ':
			ValueToSend = Max7219_Digits[0];
			break;
		case 'a':
		case 'A':
			ValueToSend = Max7219_Digits[11];
			break;
		case 'b':
		case 'B':
			ValueToSend = Max7219_Digits[12];
			break;
		case 'c':
			ValueToSend = Max7219_Digits[13];
			break;
		case 'C':
			ValueToSend = Max7219_Digits[14];
			break;
		case 'd':
		case 'D':
			ValueToSend = Max7219_Digits[15];
			break;
		case 'e':
			ValueToSend = Max7219_Digits[16];
			break;
		case 'E':
			ValueToSend = Max7219_Digits[17];
			break;
		case 'F':
		case 'f':
			ValueToSend = Max7219_Digits[18];
			break;
		case 'h':
			ValueToSend = Max7219_Digits[19];
			break;
		case 'H':
			ValueToSend = Max7219_Digits[20];
			break;
		case 'i':
		case 'I':
			ValueToSend = Max7219_Digits[21];
			break;
		case 'j':
		case 'J':
			ValueToSend = Max7219_Digits[22];
			break;
		case 'l':
		case 'L':
			ValueToSend = Max7219_Digits[23];
			break;
		case 'n':
		case 'N':
			ValueToSend = Max7219_Digits[24];
			break;
		case 'o':
		case 'O':
			ValueToSend = Max7219_Digits[25];
			break;
		case 'p':
		case 'P':
			ValueToSend = Max7219_Digits[26];
			break;
		case 'r':
		case 'R':
			ValueToSend = Max7219_Digits[27];
			break;
		case 's':
		case 'S':
			ValueToSend = Max7219_Digits[6];
			break;
		case 'u':
			ValueToSend = Max7219_Digits[28];
			break;
		case 'U':
			ValueToSend = Max7219_Digits[29];
			break;
		case 'y':
		case 'Y':
			ValueToSend = Max7219_Digits[30];
			break;
		case 'z':
		case 'Z':
			ValueToSend = Max7219_Digits[3];
			break;
		case '*':
			ValueToSend = Max7219_Digits[31];
			break;
		default:
			return MAX7219_UNSUPPORTED_CHAR;
			break;
	}

	if(MAX7219_OK != MAX7219_SendToDevice(DeviceNumber, (MAX7219_DIGIT0_REGISTER + Digit), ValueToSend|(Dot?0x80:0x00)))
	{
		return MAX7219_ERROR;
	}

	return MAX7219_OK;
}

MAX7219_STATUS MAX7219_ClearAll(void)
{
	uint8_t i, j;
	for(i = 0; i < MAX7219_DEVICES; i++)
	{
		for(j = 0; j < 8; j++)
		{
			if(MAX7219_OK != MAX7219_SetDigit(i, j, ' ', 0)) return MAX7219_ERROR;
		}
	}
	return MAX7219_OK;
}

MAX7219_STATUS MAX7219_PutStringHelper(char *String, uint8_t Adjust, int Start)
{
	uint8_t StringLength;
	int Offset;
	uint8_t Device;
	uint8_t Dot = 0;
	uint8_t Commas = 0;
	char tmp;
	uint8_t i;

	StringLength = strlen(String);

	for(i = 0; i < StringLength; i++)
	{
		if(String[i] == '.' || String[i] == ',')
		{
			Commas++;
		}
	}

	if(Adjust == 0) // Right
	{
		Offset = (MAX7219_DEVICES * 8) - StringLength + Commas;
		Device = Offset / 8;
		Offset %= 8;
	}
	else if(Adjust == 1) // Left
	{
		Offset = 0;
		Device = 0;
	}
	else if(Adjust == 2) // Center
	{
		Offset = ((MAX7219_DEVICES * 8) - StringLength + Commas) / 2;
		Device = Offset / 8;
		Offset %= 8;
	}
	else if(Adjust == 3) // Center
	{
		Offset = Start;

		if(Offset < 0)
		{
			Device = 0;
		}
		else if(Offset > (MAX7219_DEVICES * 7))
		{
			Device = MAX7219_DEVICES - 1;
		}
		else
		{
			Device = Offset / 8;
			Offset %= 8;
		}

	}
	else
	{
		return MAX7219_ERROR;
	}

	while((tmp = *String))
	{
		if(*(String+1) == '.' || *(String+1) == ',')
		{
			Dot = 1;
			String++;
		}
		else
		{
			Dot = 0;
		}

		if(MAX7219_ERROR == MAX7219_SetDigit(Device, (7-(Offset++)), tmp, Dot)) return MAX7219_ERROR;

		if(Offset > 7)
		{
			Device++;
			Offset = 0;
		}
		String++;
	}

	return MAX7219_OK;
}

MAX7219_STATUS MAX7219_PutString(int Start, char *String)
{
	return MAX7219_PutStringHelper(String, 3, Start);
}

MAX7219_STATUS MAX7219_PutStringRightAdjust(char *String)
{
	return MAX7219_PutStringHelper(String, 0, 0);
}

MAX7219_STATUS MAX7219_PutStringLeftAdjust(char *String)
{
	return MAX7219_PutStringHelper(String, 1, 0);
}

MAX7219_STATUS MAX7219_PutStringCenterAdjust(char *String)
{
	return MAX7219_PutStringHelper(String, 2, 0);
}

MAX7219_STATUS MAX7219_Init(SPI_HandleTypeDef *hspi)
{
	uint8_t i;
	max7219_spi = hspi;

	for(i = 0; i < MAX7219_DEVICES; i++)
	{
		if(MAX7219_OK != MAX7219_SetDecodeMode(i, NoDecode)) return MAX7219_ERROR;
		if(MAX7219_OK != MAX7219_SetIntensity(i, 1)) return MAX7219_ERROR;
		if(MAX7219_OK != MAX7219_SetScanLimit(i, ScanDigit0_7)) return MAX7219_ERROR;
		if(MAX7219_OK != MAX7219_SetDisplayTest(i, TestOff)) return MAX7219_ERROR;
		if(MAX7219_OK != MAX7219_Shutdown(i, NormalOperation)) return MAX7219_ERROR;
		if(MAX7219_OK != MAX7219_ClearAll()) return MAX7219_ERROR;
	}

	return MAX7219_OK;
}
