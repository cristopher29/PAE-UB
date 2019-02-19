#ifndef UART_H_
#define UART_H_

//extern uint32_t current_time;
//extern byte received_data;
//extern byte read_data_UART;

void init_UART(void);
void wheelMode(void);
struct RxReturn RxPacket(void);
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[16]);
void sentit_dades_Rx(void);
void sentit_dades_Tx(void);
bool time_out(int time);
void TxUAC2(byte bTxdData);

void EUSCIA2_IRQHandler(void);
void TA1_0_IRQHandler(void);

#endif /* UART_H_ */
