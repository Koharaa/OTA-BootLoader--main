#include "at24c02.h"
#include "i2c.h"
#include "delay.h"
#include "main.h"
#include "string.h"

//返回值约定：0 表示成功，1 表示失败
uint8_t AT24C02_WriteByte(uint8_t Addr, uint8_t Data)
{
    uint8_t txData[2];

    txData[0] = Addr;
    txData[1] = Data;

    if (My_I2C_SendBytes(AT24C02_I2C_PORT,
                          AT24C02_WADDR,
                          txData,
                          2) != 0)
    {
        return 1;
    }

    // 等待 EEPROM 内部写周期完成
    Delay(6);

    return 0;
}

uint8_t AT24C02_WritePage(uint8_t Addr, uint8_t *writeData)
{
    uint8_t txData[AT24C02_PAGE_SIZE + 1];
    uint8_t i;

    if (writeData == 0)
    {
        return 1;
    }

    txData[0] = Addr;

    for (i = 0; i < AT24C02_PAGE_SIZE; i++)
    {
        txData[i + 1] = writeData[i];
    }

    if (My_I2C_SendBytes(AT24C02_I2C_PORT,
                          AT24C02_WADDR,
                          txData,
                          AT24C02_PAGE_SIZE + 1) != 0)
    {
        return 1;
    }

    // 等待 EEPROM 内部写周期完成
    Delay(6);

    return 0;
}

uint8_t AT24C02_ReadData(uint8_t Addr, uint8_t *readData, uint16_t datalen)
{
    if (readData == 0)
    {
        return 1;
    }

    if (datalen == 0)
    {
        return 0;
    }

    // 先发送待读取的存储地址
    if (My_I2C_SendBytes(AT24C02_I2C_PORT,
                          AT24C02_WADDR,
                          &Addr,
                          1) != 0)
    {
        return 1;
    }

    // 使用读地址读取数据
    if (My_I2C_ReceiveBytes(AT24C02_I2C_PORT,
                             AT24C02_RADDR,readData,datalen) != 0)
    {
        return 1;//读取失败，返回1
    }

    return 0;
}
//void AT24C02_ReadOTAInfo(void)
//{
//    memset(&OTA_info, 0, OTA_INFOCB_SIZE);
//    AT24C02_ReadData(0, (uint8_t *)&OTA_info, OTA_INFOCB_SIZE);
//}
//void AT24C02_WriteOTAInfo(void)
//{
//    size_t i=0;
//    uint8_t *wptr;
//    wptr=(uint8_t *)&OTA_info;
//    for(i=0;i<OTA_INFOCB_SIZE;i++)
//    {
//     AT24C02_WritePage(i*16, wptr+i*16);
//     Delay(5);
//    }   
//}
void AT24C02_WriteOTAInfo(void)
{
    const uint8_t *wptr = (const uint8_t *)&OTA_info;
    uint8_t i;
    uint8_t page_buf[AT24C02_PAGE_SIZE];

    /* OTA_INFOCB_SIZE 必须是 8 的倍数，不是的话补到 8 的倍数 */
    for (i = 0; i < (OTA_INFOCB_SIZE + AT24C02_PAGE_SIZE - 1) / AT24C02_PAGE_SIZE; i++)
    {
        memset(page_buf, 0xFF, AT24C02_PAGE_SIZE);
        memcpy(page_buf, wptr + i * AT24C02_PAGE_SIZE,
               (i * AT24C02_PAGE_SIZE + AT24C02_PAGE_SIZE <= OTA_INFOCB_SIZE)
                   ? AT24C02_PAGE_SIZE
                   : (OTA_INFOCB_SIZE - i * AT24C02_PAGE_SIZE));

        AT24C02_WritePage(OTA_INFO_EEPROM_ADDR + i * AT24C02_PAGE_SIZE, page_buf);
        Delay(10);
    }
}
int AT24C02_ReadOTAInfo(void)
{
    // 直接读进全局变量 OTA_info
    if (AT24C02_ReadData(OTA_INFO_EEPROM_ADDR, (uint8_t *)&OTA_info, OTA_INFOCB_SIZE) != 0)
    {
        // 只有 I2C 读取失败（没应答、被拒收）才清零，避免用到半截数据
        memset(&OTA_info, 0, OTA_INFOCB_SIZE);
        return -1;
    }

    // EEPROM 从没写过时，读出来全是 0xFF，版本号字符串没有结束符，打印会乱
    // 这种情况只把版本号清空，其他字段保持原样
    if ((uint8_t)OTA_info.OTA_version[0] == 0xFF)
    {
        memset(OTA_info.OTA_version, 0, sizeof(OTA_info.OTA_version));
    }

    return 0;
}



