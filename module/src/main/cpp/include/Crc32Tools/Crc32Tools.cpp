#include <Crc32Tools.h>
#include <cstdio>
#include <malloc.h>
#include <cstring>
#include "strings.h"
#include <string>
#include <bits/fcntl.h>
#include <asm/fcntl.h>
#include <asm/mman.h>
#include <sys/mman.h>
#include <unistd.h>
#include <zconf.h>
#include <fstream>
#include <fcntl.h>
#include <Structs.h>
#include <cstdio>
#include <dirent.h>
#include <Logger.h>

#define Limit            0xEDB88320
#define Num0                            0
#define num1                            0x1
#define num2                            0x2
#define Num8                            0x8
#define Num255                          0xFF
#define Num256                          0x100
#define Num16777215                     0xFFFFFF
#define dwPolynomial                    0xEDB88320
#define Num2147483647                   0x7FFFFFFF
#define NumNegative1                    0xFFFFFFFF
#define NumNegative2                    0xFFFFFFFE
#define NumNegative256                  0xFFFFFF00

#define random(a, b) (rand()%(b-a)+a)

int CRC32Edit(unsigned char *byt, int bytLen, int lOffset, int lcrc, int &retlCRC) {

    int CRCdata[256];
    bytLen--;
    int i, x, crc;
    int crcTable[256];
    for (i = 0; i <= 255; i++) {
        for (crc = i, x = 0; x < 8; x++) {
            if (crc & 1)
                crc = (((crc & 0xFFFFFFFE) / 2) & 0x7FFFFFFF) ^ Limit;
            else
                crc = (crc & 0xFFFFFFFE) / 2;
        }
        crcTable[i] = crc;
        CRCdata[i] = crc;
    }
    if (bytLen < 0)return 0;
    unsigned int crcResult = NumNegative1;
    if (lOffset < 0 || lOffset > bytLen)
        lOffset = bytLen + 1;

    for (i = 0; i <= lOffset - 1; i++) {
        crcResult = (crcResult >> 8 ^ crcTable[((crcResult & 255) ^ byt[i]) & 255]);
    }
    int crcFront = ~crcResult;

    int k = bytLen - lOffset - 3;

    int cr1, j;
    if (k > 0) {
        unsigned char *backbyt;
        backbyt = byt + lOffset + 4;
        cr1 = ~lcrc;
        for (j = bytLen; j > bytLen - k; j--) {
            for (i = 0; i <= 255; i++) {
                if ((cr1 & 0xff000000) == (CRCdata[i] & 0xff000000))
                    break;
            }
            cr1 = cr1 ^ CRCdata[i];
            cr1 = cr1 << 8;
            cr1 = (i ^ byt[j]) | cr1;
        }
    } else
        cr1 = ~lcrc;

    for (j = 0; j <= 3; j++) {
        for (i = 0; i <= 255; i++) {
            if ((cr1 & 0xff000000) == (CRCdata[i] & 0xff000000))
                break;
        }
        cr1 = cr1 ^ CRCdata[i];
        cr1 = cr1 << 8;
        cr1 = i | cr1;
    }
    retlCRC = cr1 ^ crcResult;

    return crcFront;
}


int editor(const char *file, int lcrc) {
    //打开文件
    FILE *fp = fopen(file, "r");
    if (fp) {
        //到文件尾部
        fseek(fp, 0L, SEEK_END);
        //读大小
        int size = ftell(fp);
        //回到文件头部
        rewind(fp);
        //初始化char*
        auto *data = (unsigned char *) malloc(size * 2);
        //读内容
        fread(data, size, 1, fp);
        //关闭文件
        fclose(fp);
        //计算CRC32覆盖值
        int crcout;
        CRC32Edit(data, size, -1, lcrc, crcout);//0xD27E4835
        //追加4字节
        memcpy(data + size, &crcout, 4);
        //打开文件
        FILE *fp_w = fopen(file, "w+");
        //写入内容
        fwrite(data, size + 4, 1, fp_w);
        //关闭文件
        fclose(fp_w);
    }
    return 1;
}

uint32_t crc32_for_byte(uint32_t r) {
    for (int j = 0; j < 8; ++j) {
        r = (r & 1 ? 0 : (uint32_t) 0xEDB88320L) ^ r >> 1;
    }
    return r ^ (uint32_t) 0xFF000000L;
}

static uint32_t crc32(uint8_t *buf, int len) {
    int i, j;
    uint32_t crc, mask;

    crc = 0xFFFFFFFF;
    for (i = 0; i < len; i++) {
        crc = crc ^ (uint32_t) buf[i];
        for (j = 7; j >= 0; j--) {    // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

uint32_t crc32(const char* s) {
    uint32_t crc = 0xffffffff;
    size_t i = 0;
    while (s[i] != '\0'){
        uint8_t byte = s[i];
        crc = crc ^ byte;
        for (uint8_t j = 8; j > 0; --j)
        {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }

        i++;
    }
    return crc ^ 0xffffffff;
}


char* genRandomString(int len) {
    int flag, i;
    char *string;
    srand((unsigned) time(nullptr));
    if ((string = (char *) malloc(len)) == NULL) {
        return NULL;
    }
    for (i = 0; i < len - 1; i++) {
        flag = rand() % 3;
        switch (flag) {
            case 0:
                string[i] = 'A' + rand() % 26;
                break;
            case 1:
                string[i] = 'a' + rand() % 26;
                break;
            case 2:
                string[i] = '0' + rand() % 10;
                break;
            default:
                string[i] = 'x';
                break;
        }
    }
    string[len - 1] = '\0';
    return string;

}



int allEditor(const char *path) {
    DIR *dir;
    char pathname[255];        //目录的全名，=当前目录名+子目录名
    if ((dir = opendir(path)) == nullptr)    //无法打开则跳过
    {
        //LOGE("open %s failed!", path);
        return -1;
    }

    struct dirent *stdir;

    while (true) {
        if ((stdir = readdir(dir)) == nullptr) break;  //遍历完一整个文件夹就停止循环
        if (stdir->d_type == 8)                            //文件则输出
        {
            char tmp[128];
            sprintf(tmp, "%s/%s", path, stdir->d_name);

            //生成随机字符串
            char* str = genRandomString(random(32,1024));
            uint32_t lcrc =  crc32((char*)str);
            //根据字符串生成crc32进行修改
            editor(tmp, lcrc);
            //LOGE("name:%s crc32:lcrc:%x str:%s", tmp,lcrc,str);
            //LOGE("name: %25s/%s", path, stdir->d_name);

        } else //if(stdir->d_type == 4)					//文件夹则递归进行下一轮，打开文件夹
        {
            sprintf(pathname, "%s%s", path, stdir->d_name);        //获得目录全名（当前目录名 + 子目录名）
            allEditor(pathname);
        }
    }
    closedir(dir);        //关闭目录
    return 1;
}

