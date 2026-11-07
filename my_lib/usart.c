#include "usart.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "delay.h"
//串口printf函数 最后while循环 TC标志位
UCB_RXPtr USART_Rx_Ptr;// 接收管理句柄  调用时必须使用该句柄
uint8_t USART1_Rx_Buffer[RX_BUF_SIZE];// 接收缓冲区
uint8_t USART1_Tx_Buffer[TX_BUF_SIZE];// 发送缓冲区

UCB_RXPtr USART3_Rx_Ptr;// 接收管理句柄  调用时必须使用该句柄
uint8_t USART3_Rx_Buffer[RX3_BUF_SIZE];// 接收缓冲区
uint8_t USART3_Tx_Buffer[TX3_BUF_SIZE];// 发送缓冲区
void usart_init(void)
{
	//1.初始化usart1引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//2.初始化usart1
RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
USART_InitTypeDef USART_InitStructure;
USART_InitStructure.USART_BaudRate = 115200;
USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据位
USART_InitStructure.USART_StopBits = USART_StopBits_1;//1位停止位
USART_InitStructure.USART_Parity = USART_Parity_No;//无校验位
USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//发送和接收模式
USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
USART_Init(USART1, &USART_InitStructure);
USART_Cmd(USART1, ENABLE);

//3.使能usart1中断
//设置中断优先级
NVIC_InitTypeDef NVIC_InitStructure;
NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//子优先级
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);

Usart1Rx_PtrInit();
DMA_init();
//4.初始化dma
/* 清除可能已经存在的 IDLE 标志 */
USART_GetFlagStatus(USART1, USART_FLAG_IDLE);// 读 SR 寄存器
USART_ReceiveData(USART1);// 读 DR 寄存器 硬件清零
USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);// 置位 CR3 寄存器的 DMAR 位，打开串口的 DMA 接收请求
DMA_Cmd(DMA1_Channel5, ENABLE);// 置位通道 5 CCR 寄存器的 EN 位，启动，DMA_init() 只是配置了源地址（DR）、目的地址
USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);// 置位 CR1 寄存器的 IDLEIE 位，打开空闲中断一个字节没收到数据触发，“总长度 − CNDTR"就能算出这一帧收了多少字节
}
//字节到达 → DR 寄存器 → 串口发 DMA 请求 → DMA 搬到 USART1_Rx_Buffer[i]，CNDTR 减 1
 //   ……一帧数据收完……
//线路空闲 → IDLE 中断 → USART1_IRQHandler 读 SR/DR 清标志，用 CNDTR 算出长度，记入接收队列

void usart3_init(void)
{
	//1.初始化usart3引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//2.初始化usart3
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
 USART_InitTypeDef USART_InitStructure;
 USART_InitStructure.USART_BaudRate = 115200;
 USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据位
 USART_InitStructure.USART_StopBits = USART_StopBits_1;//1位停止位
 USART_InitStructure.USART_Parity = USART_Parity_No;//无校验位
 USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//发送和接收模式
 USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
 USART_Init(USART3, &USART_InitStructure);
 USART_Cmd(USART3, ENABLE);
 


//3.使能usart3中断
//设置中断优先级
 NVIC_InitTypeDef NVIC_InitStructure;
 NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级
 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//子优先级
 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 NVIC_Init(&NVIC_InitStructure);
Usart3Rx_PtrInit();
 //4.初始化dma
/* 清除可能已经存在的 IDLE 标志 */
USART_GetFlagStatus(USART3, USART_FLAG_IDLE);// 读 SR 寄存器
USART_ReceiveData(USART3);// 读 DR 寄存器 硬件清零
USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);// 置位 CR3 寄存器的 DMAR 位，打开串口的 DMA 接收请求
DMA_Cmd(DMA1_Channel3, ENABLE);// 置位通道 3 CCR 寄存器的 EN 位，启动，DMA_init() 只是配置了源地址（DR）、目的地址
USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);// 置位 CR1 寄存器的 IDLEIE 位，打开空闲中断一个字节没收到数据触发，“总长度 − CNDTR"就能算出这一帧收了多少字节
}

/*串口硬件把字节放进 DR 寄存器，同时置 RXNE 标志（"接收寄存器非空"）。
因为开启了串口的 DMA 接收请求，RXNE 不会触发中断，而是向 DMA 发出一个请求信号。
DMA 收到请求后，从 DR 读出这个字节，写到 CMAR 指向的内存地址。读 DR 这个动作同时会自动清掉 RXNE 标志。
内存地址自动加 1，计数器 CNDTR 自动减 1，然后等待下一个请求。*/

//初始化dma1通道5 串口Rx
void DMA_init(void)
{
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
DMA_InitTypeDef DMA_InitStructure;
DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;//设置外设地址为 USART1 的数据寄存器 DR。串口收到的每个字节都会放在 DR 里，DMA 就从这里取数据。
DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设端每次传输 1 个字节（8 位）
DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不自增
DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART1_Rx_Buffer;//接受缓冲区的数组地址，搬过来的数据存到这个数组里。
DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存端每次也写 1 个字节，和外设端保持一致，数组是 uint8_t 类型
DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址自增。每存完一个字节，地址加 1，下一个字节存到数组的下一个位置，否则所有数据都会覆盖在 USART_Rx_Buffer[0] 上。
DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//串口接收，所以是外设到内存
DMA_InitStructure.DMA_BufferSize = RX_BUF_SIZE;//大小就是接收地址缓冲区长度
DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//普通模式，不循环传输
DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//关闭内存到内存模式。这里是外设触发的传输，串口每收到一个字节才发一次请求，DMA 才搬一次。如果开启 M2M，DMA 会不等请求连续搬运，只在内存之间拷贝数据时使用。
DMA_InitStructure.DMA_Priority = DMA_Priority_High;//高优先级，多个 DMA 通道同时请求时，优先级高的先处理
DMA_Cmd(DMA1_Channel5, DISABLE);
DMA_Init(DMA1_Channel5, &DMA_InitStructure);

DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;//设置外设地址为 USART3 的数据寄存器 DR。串口收到的每个字节都会放在 DR 里，DMA 就从这里取数据。
DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设端每次传输 1 个字节（8 位）
DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不自增
DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART3_Rx_Buffer;//接受缓冲区的数组地址，搬过来的数据存到这个数组里。
DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存端每次也写 1 个字节，和外设端保持一致，数组是 uint8_t 类型
DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址自增。每存完一个字节，地址加 1，下一个字节存到数组的下一个位置，否则所有数据都会覆盖在 USART_Rx_Buffer[0] 上。
DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//串口接收，所以是外设到内存
DMA_InitStructure.DMA_BufferSize = RX3_BUF_SIZE;//大小就是接收地址缓冲区长度
DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//普通模式，不循环传输
DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//关闭内存到内存模式。这里是外设触发的传输，串口每收到一个字节才发一次请求，DMA 才搬一次。如果开启 M2M，DMA 会不等请求连续搬运，只在内存之间拷贝数据时使用。
DMA_InitStructure.DMA_Priority = DMA_Priority_High;//高优先级，多个 DMA 通道同时请求时，优先级高的先处理
DMA_Cmd(DMA1_Channel3, DISABLE);
DMA_Init(DMA1_Channel3, &DMA_InitStructure);
}
//初始化usart1接收管理句柄

 void Usart1Rx_PtrInit(void) //串口管理结构体
{

 USART_Rx_Ptr.URxDataIn = &USART_Rx_Ptr.URxData[0];// se指针对结构体数组接收数据入队指针，定义起始在接收缓冲区第一个位置
  USART_Rx_Ptr.URxDataOut = &USART_Rx_Ptr.URxData[0];// se指针对结构体数组接收数据出队指针，定义起始在接收缓冲区第一个位置 相当于用两个指针来指向一个传入的缓冲区数据
  USART_Rx_Ptr.URxDataEnd = &USART_Rx_Ptr.URxData[USART_RX_QUEUE_SIZE];// 接收数据结束指针
  USART_Rx_Ptr.URxDataIn->start = USART1_Rx_Buffer;// 接收数据入队指针指向接收缓冲区的地址
  USART_Rx_Ptr.URXCount = 0;// 接收数据量初始化为0
 }
 
//初始化usart3接收管理句柄
 void Usart3Rx_PtrInit(void) //串口管理结构体
{

  USART3_Rx_Ptr.URxDataIn = &USART3_Rx_Ptr.URxData[0];// se指针对结构体数组接收数据入队指针，定义起始在接收缓冲区第一个位置
  USART3_Rx_Ptr.URxDataOut = &USART3_Rx_Ptr.URxData[0];// se指针对结构体数组接收数据出队指针，定义起始在接收缓冲区第一个位置 相当于用两个指针来指向一个传入的缓冲区数据
  USART3_Rx_Ptr.URxDataEnd = &USART3_Rx_Ptr.URxData[USART_RX_QUEUE_SIZE];// 接收数据结束指针
  USART3_Rx_Ptr.URxDataIn->start = USART3_Rx_Buffer;// 接收数据入队指针指向接收缓冲区的地址
  USART3_Rx_Ptr.URXCount = 0;// 接收数据量初始化为0
 }

// @简介：通过串口格式化打印字符串
// 
// @参数 USARTx：串口名称，如USART1, USART2, USART3 ...
// @参数 Format：字符串的格式
// @参数 ...   ：可变参数
//va_start 就是拿到这段内存的起始地址，vsprintf 内部的 va_arg 则按格式符给出的类型逐个往后取：%d 取 4 字节，%f 取 8 字节（float 传入可变参数时会被自动提升为 double）。

//这也说明了为什么格式符必须和实际参数的类型一致。比如用 %d 去打印一个 float，va_arg 会按错误的长度去读内存，后面所有参数都会跟着错位，打印出来就是乱码。
void My_USART_Printf(USART_TypeDef *USARTx, const char *Format, ...)//va_list 是用来遍历可变参数的类型。在 ARM 编译器里，它本质上是一个指向参数存放位置的指针。
{
	char format_buffer[128];
	va_list argptr;
	
	__va_start(argptr, Format);//让 argptr 指向 Format 后面的第一个可变参数。编译器靠最后一个固定参数（Format）来确定 ... 从哪里开始，所以这里要传 Format。
	
	vsprintf(format_buffer, Format, argptr);//vsprintf 和 sprintf 作用相同，区别在于它不接收 ...，而是接收一个已经准备好的 va_list
	
	__va_end(argptr);//结束对可变参数的访问，和 va_start 成对使用
	
	My_USART_SendString(USARTx, format_buffer);//把拼好的字符串交给你自己写的发送函数，一般是循环等待 TXE 标志，然后逐字节写数据寄存器，直到遇到 '\0' 为止。
}
//
// @简介：使用串口发送一个字节的数据
// 
// @参数 USARTx：串口名称，如USART1, USART2, USART3 ...
// @参数 Data  : 要发送的数据
//
void My_USART_SendByte(USART_TypeDef *USARTx, const uint8_t Data)
{
	My_USART_SendBytes(USARTx, &Data, 1);
}

//
// @简介：使用串口发送多个字节的数据
// 
// @参数 USARTx：串口名称，如USART1, USART2, USART3 ...
// @参数 pData : 要发送的数据（数组）
// @参数 Size  ：要发送数据的数量，单位是字节
//
__weak void My_USART_SendBytes(USART_TypeDef *USARTx, const uint8_t *pData, uint16_t Size)
{
	if(Size == 0) return;
	
	for(uint16_t i=0; i < Size; i++)
	{
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
		
		USART_SendData(USARTx, pData[i]);
	}
	
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
}

//
// @简介：通过串口发送一个字符
// 
// @参数 USARTx：串口名称，如USART1, USART2, USART3 ...
// @参数 C     ：要发送的字符
//
void My_USART_SendChar(USART_TypeDef *USARTx, const char C)
{
	My_USART_SendBytes(USARTx, (const uint8_t *)&C, 1);
}

//
// @简介：通过串口发送字符串
// 
// @参数 USARTx：串口名称，如USART1, USART2, USART3 ...
// @参数 Str   ：要发送的字符串
//
void My_USART_SendString(USART_TypeDef *USARTx, const char *Str)
{
	My_USART_SendBytes(USARTx, (const uint8_t *)Str, strlen(Str));
}




//
// @简介：通过串口读取一字节的数据
// 
// @参数 USARTx  ：串口名称，如USART1, USART2, USART3 ...
// 
// @返回值：读取到的字节
//
uint8_t My_USART_ReceiveByte(USART_TypeDef *USARTx)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET);
	
	return USART_ReceiveData(USARTx);
}

//
// @简介：通过串口读取多个字节的数据
// 
// @参数 USARTx  ：串口名称，如USART1, USART2, USART3 ...
// @参数 pDataOut：输出参数，读取到的数据将输出到此数组当中
// @参数 Size    ：需要读取的字节数量
// @参数 Timeout ：超时时间，单位是毫秒，负数表示无限长。如果超时时间内没有读取完成则返回。
// 
// @返回值：实际读取到的数据数量
//
__weak uint16_t My_USART_ReceiveBytes(USART_TypeDef *USARTx, uint8_t *pDataOut, uint16_t Size, int Timeout)
{
	uint32_t expireTime;
	
	Delay_Init();
	
	if(Timeout >= 0)
	{
		expireTime = GetTick() + Timeout; // 计算过期时间，过期时间 = 当前时间+Timeout
	}
	
	uint16_t i = 0;
	
	do
	{
		if(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == SET)
		{
			pDataOut[i++] = USART_ReceiveData(USARTx);
			
			if(i==Size) break;
		}
	}
	while(Timeout < 0 || GetTick() < expireTime); // 判断是否超时
	
	return i;
}


//
// @简介：通过串口读取一行字符串
// 
// @参数 USARTx       ：串口名称，如USART1, USART2, USART3 ...
// @参数 pStrOut      ：输出参数，读取到的数据将输出到此数组当中
// @参数 MaxLength    ：字符串的最大长度
// @参数 LineSeperator：行分隔符 LINE_SEPERATOR_CR   - 回车 \r
//                               LINE_SEPERATOR_LF   - 换行 \n
//                               LINE_SEPERATOR_CRLF - 回车+换行 \r\n
// @参数 Timeout      ：超时时间，单位是毫秒，负数表示无限长。如果超时时间内没有读取完成则返回
// 
// @返回值：0 - 成功读到一行字符串
//         -1 - 超时（Timeout内未读到一行完整的字符串）
//         -2 - 超过字符串的最大长度（字符串的最大长度用MaxLength参数设置）
//
int My_USART_ReceiveLine(USART_TypeDef *USARTx, char *pStrOut, uint16_t MaxLength, uint16_t LineSeperator, int Timeout)
{
	// 如果最大长度都不足以装下行分隔符
	// 就直接返回失败
	if(MaxLength < 2 || ((LineSeperator == LINE_SEPERATOR_CRLF) && (MaxLength < 1)))
	{
		return -2;
	}
	
	int ret = -1;
	uint32_t expireTime;
	
	Delay_Init(); // 要用到单片机当前时间，所以初始化延迟函数
	
	if(Timeout >= 0)
	{
		expireTime = GetTick() + Timeout; // 计算过期时间，过期时间 = 当前时间+Timeout
	}
	
	uint16_t i = 0;
	
	do
	{
		if(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == SET)
		{
			char c = (char)USART_ReceiveData(USARTx);
			pStrOut[i++] = c;
			
			if(LineSeperator == LINE_SEPERATOR_CR && c == '\r') // \r
			{
				ret = 0;
				break;
			}
			else if(LineSeperator == LINE_SEPERATOR_LF && c == '\n') // \n
			{
				ret = 0;
				break;
			}
			else if(i >= 2 && pStrOut[i-2] == '\r' && c == '\n') // \r\n
			{
				ret = 0;
				break;
			}
			
			if(i == MaxLength) // 超过最大长度
			{
				ret = -2;
				break;
			}
		}
	}
	while(Timeout < 0 || GetTick() < expireTime); // 判断是否超时
	
	// 在字符串末尾增加'\0'
	if(i == MaxLength)
	{
		pStrOut[i-1] = '\0';
	}
	else
	{
		pStrOut[i] = '\0';
	}
	
	return ret;
}
