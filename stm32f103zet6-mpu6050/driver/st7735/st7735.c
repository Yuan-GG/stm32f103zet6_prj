#include <stdio.h>
#include "stm32f10x.h"
#include "main.h"
#include "lcd_spi.h"
#include "st7735.h"

#define CS_PORT     GPIOA
#define CS_PIN      GPIO_Pin_4
#define DC_PORT     GPIOA
#define DC_PIN      GPIO_Pin_6
#define RES_PORT    GPIOC
#define RES_PIN     GPIO_Pin_4
#define BLK_PORT    GPIOB
#define BLK_PIN     GPIO_Pin_1

// 4k�Դ�
#define GRAM_BUFFER_SIZE 4096

// ST7735 Commands
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_GAMSET  0x26
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define CMD_DELAY      0xFF
#define CMD_EOF        0xFF

static const uint8_t init_cmd_list[] =
{
    0x11,  0,
    CMD_DELAY, 12,
    0xB1,  3,  0x01, 0x2C, 0x2D,
    0xB2,  3,  0x01, 0x2C, 0x2D,
    0xB3,  6,  0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
    0xB4,  1,  0x07,
    0xC0,  3,  0xA2, 0x02, 0x84,
    0xC1,  1,  0xC5,
    0xC2,  2,  0x0A, 0x00,
    0xC3,  2,  0x8A, 0x2A,
    0xC4,  2,  0x8A, 0xEE,
    0xC5,  1,  0x0E,
    0x36,  1,  0xC8,
    0xE0,  16, 0x0F, 0x1A, 0x0F, 0x18, 0x2F, 0x28, 0x20, 0x22, 0x1F, 0x1B, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
    0xE1,  16, 0x0F, 0x1B, 0x0F, 0x17, 0x33, 0x2C, 0x29, 0x2E, 0x30, 0x30, 0x39, 0x3F, 0x00, 0x07, 0x03, 0x10,
    0x2A,  4,  0x00, 0x00, 0x00, 0x7F,
    0x2B,  4,  0x00, 0x00, 0x00, 0x9F,
    0xF6,  1,  0x00,
    0x3A,  1,  0x05,
    0x29,  0,

    CMD_DELAY, CMD_EOF,
};
static volatile bool spi_async_done;
static uint8_t gram_buff[GRAM_BUFFER_SIZE];

static void SPI_On_Async_Finish(void)
{
    spi_async_done = true;
}

static void ST7735_Select(void) {
    GPIO_ResetBits(CS_PORT, CS_PIN);
}

static void ST7735_Unselect(void) {
    GPIO_SetBits(CS_PORT, CS_PIN);
}

/**
 * @brief  ST7735��λ-оƬ�ֲ�����*Reset Timing*�����ҵ�
 * @param  None
 * @retval None
 */
static void ST7735_Reset(void) {
    GPIO_ResetBits(RES_PORT, RES_PIN);
    delay(2);
    GPIO_SetBits(RES_PORT, RES_PIN);
    delay(150);
}

static void ST7735_Bl_On(void) {
    GPIO_SetBits(BLK_PORT, BLK_PIN);
}

static void ST7735_Write_Cmd(uint8_t cmd)
{
    GPIO_ResetBits(DC_PORT, DC_PIN);
    LCD_SPI_Write_Sync(&cmd, 1);
}

static void ST7735_Write_Data(uint8_t* data, size_t size)
{
    GPIO_SetBits(DC_PORT, DC_PIN);
    spi_async_done = false;
    LCD_SPI_Write_Async(data, size);
    while (!spi_async_done);
}

static void ST7735_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    // CS-��ʱ��ͼ��֪�͵�ƽ��Ч
    GPIO_InitStructure.GPIO_Pin = CS_PIN;
    GPIO_Init(CS_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(CS_PORT, CS_PIN, Bit_SET);

    // DC-����/����-ͨ��0��ʾ����
    GPIO_InitStructure.GPIO_Pin = DC_PIN;
    GPIO_Init(DC_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(DC_PORT, DC_PIN, Bit_RESET);

    // RES-��λ-0-�����λ(0)�����������1���ͻ���������ź�
    GPIO_InitStructure.GPIO_Pin = RES_PIN;
    GPIO_Init(RES_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(RES_PORT, RES_PIN, Bit_RESET);

    // BLK-����-����ν
    GPIO_InitStructure.GPIO_Pin = BLK_PIN;
    GPIO_Init(BLK_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(BLK_PORT, BLK_PIN, Bit_RESET);
}

static void ST7735_Exec_Cmds(const uint8_t* cmd_list) {
    while (1) {
        uint8_t cmd = *cmd_list++;
        uint8_t num = *cmd_list++;
        if (cmd == CMD_DELAY) {
            if (num == CMD_EOF)
                break;
            else
                delay(num * 10);
        }
        else {
            ST7735_Write_Cmd(cmd);
            if (num > 0) {
                ST7735_Write_Data((uint8_t*)cmd_list, num);
            }
            cmd_list += num;
        }
    }
}



static void ST7735_Set_Window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    // column address set
    ST7735_Write_Cmd(ST7735_CASET);
    uint8_t data[] = { 0x00, x0 + ST7735_XSTART, 0x00, x1 + ST7735_XSTART };
    ST7735_Write_Data(data, sizeof(data));

    // row address set
    ST7735_Write_Cmd(ST7735_RASET);
    data[1] = y0 + ST7735_YSTART;
    data[3] = y1 + ST7735_YSTART;
    ST7735_Write_Data(data, sizeof(data));

    // write to RAM
    ST7735_Write_Cmd(ST7735_RAMWR);
}

/**
 * @brief  ST7735��ʼ��
 * @param  None
 * @retval None
 */
void ST7735_Init(void) {
    LCD_SPI_Init();
    LCD_SPI_Send_Finish_Register(SPI_On_Async_Finish);
    ST7735_GPIO_Init();

    ST7735_Reset();

    ST7735_Select();
    ST7735_Exec_Cmds(init_cmd_list);
    ST7735_Unselect();

    ST7735_Bl_On();
}

void ST7735_Fill_Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT)
        return;
    if (x + w - 1 >= ST7735_WIDTH)
        w = ST7735_WIDTH - x;
    if (y + h - 1 >= ST7735_HEIGHT)
        h = ST7735_HEIGHT - y;

    ST7735_Select();
    ST7735_Set_Window(x, y, x + w - 1, y + h - 1);

    // ��λ��ǰ
    uint8_t pixel[2] = { color >> 8, color & 0xFF };
    uint32_t pixelNumsInGRamBuf = GRAM_BUFFER_SIZE / 2;
    // ׼������д��
    for (uint32_t i = 0; i < w * h; i += pixelNumsInGRamBuf)
    {
        uint32_t size = w * h - i;
        if (size > pixelNumsInGRamBuf)
            size = pixelNumsInGRamBuf;
        // ���ñ�����ɫ(���赥��)
        if (i == 0)
        {
            uint8_t* pbuff = gram_buff;
            for (uint32_t j = 0; j < size; j++)
            {
                *pbuff++ = pixel[0];
                *pbuff++ = pixel[1];
            }
        }
        ST7735_Write_Data(gram_buff, size * 2);
    }

    ST7735_Unselect();
}

void ST7735_Fill_Screen(uint16_t color) {
    ST7735_Fill_Rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

void ST7735_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT)
        return;

    uint8_t data[] = { color >> 8, color & 0xFF };

    ST7735_Select();
    ST7735_Set_Window(x, y, x + 1, y + 1);
    ST7735_Write_Data(data, sizeof(data));
    ST7735_Unselect();
}

/**
 * @brief  ��ʾһ���ַ�
 * @param  x, y: �����
 * @param  ch: �ַ�
 * @param  font: ����
 * @param  color: ��ɫ
 * @param  bgcolor: ������ɫ
 * @retval None
 */
void ST7735_Write_Char(uint16_t x, uint16_t y, char ch, st_fonts_t* font, uint16_t color, uint16_t bgcolor)
{
    ST7735_Select();

    ST7735_Set_Window(x, y, x + font->width - 1, y + font->height - 1);

    // ����ȡ��
    uint32_t bytes_per_line = (font->width + 7) / 8;

    uint8_t* pbuff = gram_buff;
    for (uint32_t y = 0; y < font->height; y++)
    {
        const uint8_t* pcode = font->data + ch * font->height * bytes_per_line + y * bytes_per_line;
        for (uint32_t x = 0; x < font->width; x++)
        {
            uint8_t b = pcode[x / 8];
            if ((b << (x & 0x7)) & 0x80)
            {
                *pbuff++ = color >> 8;
                *pbuff++ = color & 0xFF;
            }
            else
            {
                *pbuff++ = bgcolor >> 8;
                *pbuff++ = bgcolor & 0xFF;
            }
        }
    }

    ST7735_Write_Data(gram_buff, pbuff - gram_buff);

    ST7735_Unselect();
}

void ST7735_Write_Font(uint16_t x, uint16_t y, st_fonts_t* font, uint32_t index, uint16_t color, uint16_t bgcolor)
{
    ST7735_Write_Char(x, y, index, font, color, bgcolor);
}

void ST7735_Write_Fonts(uint16_t x, uint16_t y, st_fonts_t* font, uint32_t index, uint32_t count, uint16_t color, uint16_t bgcolor)
{
    for (uint32_t i = index; i < count && i < font->count; i++)
    {
        if (x + font->width >= ST7735_WIDTH)
        {
            x = 0;
            y += font->height;
            if (y + font->height >= ST7735_HEIGHT)
            {
                break;
            }
        }
        ST7735_Write_Font(x, y, font, i, color, bgcolor);
        x += font->width;
    }
}
/**
 * @brief  ��ʾһ���ַ���
 * @param  x, y: �����
 * @param  str: �ַ���
 * @param  font: ����
 * @param  color: ��ɫ
 * @param  bgcolor: ������ɫ
 * @retval None
 */
void ST7735_Write_String(uint16_t x, uint16_t y, const char* str, st_fonts_t* font, uint16_t color, uint16_t bgcolor)
{
    while (*str)
    {
        // �ȴ������з�
        if (*str == '\n') {
            x = 0;
            y += font->height;
            str++;
            continue;
        }

        // �ո���ǰ�жϲ���������
        if (*str == ' ') {
            if (x + font->width >= ST7735_WIDTH) {
                x = 0;
                y += font->height;
                if (y + font->height >= ST7735_HEIGHT)
                    break;
            }
            x += font->width;
            str++;
            continue;
        }

        // �ǿո�/�����ַ���������
        if (x + font->width >= ST7735_WIDTH) {
            x = 0;
            y += font->height;
            if (y + font->height >= ST7735_HEIGHT)
                break;
        }

        ST7735_Write_Char(x, y, *str, font, color, bgcolor);
        x += font->width;
        str++;
    }

}

void ST7735_Draw_Image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* data)
{
    if ((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT))
        return;
    if ((x + w - 1) >= ST7735_WIDTH)
        return;
    if ((y + h - 1) >= ST7735_HEIGHT)
        return;

    ST7735_Select();
    ST7735_Set_Window(x, y, x + w - 1, y + h - 1);
    ST7735_Write_Data(data, w * h * 2);
    ST7735_Unselect();
}
