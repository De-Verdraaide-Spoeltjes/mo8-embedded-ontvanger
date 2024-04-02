#include "rsa_key_transmitter.h"

#include <stdbool.h>
#include "xstatus.h"
#include "xuartps.h"
#include "xtime_l.h"

#include "defines.h"
#include "generate_rsa_keys.h"

#define UART_DEVICE_ID XPAR_PS7_UART_1_DEVICE_ID

XUartPs uart;		/* Instance of the UART Device */

// Function declarations
bool UartSendAvailable();
void UartSendBytes(u8 *data, int length);
bool UartRecvAvailable();
void UartRecvBytes(u8 *data, int length);

XStatus initKeyTransmitter() {

	// Initialize UART
	XUartPs_Config *Config;
	int status;

	Config = XUartPs_LookupConfig(UART_DEVICE_ID);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	status = XUartPs_CfgInitialize(&uart, Config, Config->BaseAddress);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set UART baud rate
	XUartPs_SetBaudRate(&uart, 9600);

	xil_printf("UART initialized\n\r");

	return XST_SUCCESS;
}

// TODO: Add timeouts to UART

void runKeyTransmitter(rsaData *RSAData) {
	// Check if there is UART data to read
	if (!UartRecvAvailable()) {
		return;
	}

	u8 data, resp;
	// Read command
	UartRecvBytes(&data, 1);

	// Check if command is correct
	if (data != 0x01) {
		#ifdef DEBUG
			xil_printf("Error: Received unknown command\n\r");
		#endif
		return;
	}

	// Wait for room to send data
	while (!UartSendAvailable());

	// Acknowledge command
	resp = 0x01;
	UartSendBytes(&resp, 1);

	#ifdef DEBUG
		xil_printf("Transmitting RSA key\n\r");
	#endif

	// Calculate a seed based on the runtime of the program
	XTime currentTime;
	XTime_GetTime(&currentTime);

	// Generate RSA keys
	generateRSAKeys(RSAData, currentTime);

	// Wait for room to send data
	while (!UartSendAvailable());

	// Send RSA key command
	data = 0x02;
	UartSendBytes(&data, 1);

	// Wait for response
	while (!UartRecvAvailable());

	// Receive response
	UartRecvBytes(&resp, 1);

	// Check if response is correct
	if (resp != 0x02) {
		#ifdef DEBUG
			xil_printf("Error: Received incorrect response\n\r");
		#endif
		return;
	}

	// Wait for room to send data
	while (!UartSendAvailable());

	// Send RSA key data
	u8 keyData[8];
	*((uint64_t *) keyData) = RSAData->publicKey;
	UartSendBytes(keyData, 8);

	// Wait for response
	while (!UartRecvAvailable());

	// Receive response
	UartRecvBytes(&resp, 1);

	// Check if response is correct
	if (resp != 0x03) {
		#ifdef DEBUG
			xil_printf("Error: Received incorrect response\n\r");
		#endif
		return;
	}

	// Wait for room to send data
	while (!UartSendAvailable());

	// Send RSA modulus data
	u8 modulusData[4];
	*((uint32_t *) modulusData) = RSAData->modulus;
	UartSendBytes(modulusData, 4);

	// Wait for response
	while (!UartRecvAvailable());

	// Receive response
	UartRecvBytes(&resp, 1);

	// Check if response is correct
	if (resp != 0x04) {
		#ifdef DEBUG
			xil_printf("Error: Received incorrect response\n\r");
		#endif
		return;
	}

	#ifdef DEBUG
		xil_printf("RSA key transmitted\n\r");
	#endif
}

// Check if UART is ready to send data
bool UartSendAvailable() {
	u32 status = XUartPs_ReadReg(uart.Config.BaseAddress, XUARTPS_SR_OFFSET);
	return (status & XUARTPS_SR_TXFULL) == 0;
}

// Send data via UART
void UartSendBytes(u8 *data, int length) {
	int bytesSent = 0;
	while (bytesSent < length) {
		bytesSent += XUartPs_Send(&uart, data + length - bytesSent - 1, 1);
	}
}

// Check if UART is ready to receive data
bool UartRecvAvailable() {
	u32 status = XUartPs_ReadReg(uart.Config.BaseAddress, XUARTPS_SR_OFFSET);
	return (status & XUARTPS_SR_RXEMPTY) == 0;
}

// Receive data from UART
void UartRecvBytes(u8 *data, int length) {
	int bytesReceived = 0;
	while (bytesReceived < length) {
		bytesReceived += XUartPs_Recv(&uart, data + length - bytesReceived -1, 1);
	}
}
