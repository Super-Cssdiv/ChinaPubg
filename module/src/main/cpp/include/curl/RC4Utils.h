#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <stdio.h>
#include "strnormalize.h"

#define SIZE 256


char *strupr(char *str) {
    char *orign = str;
    for (; *str!='\0'; str++)
        *str = toupper(*str);
    return orign;
}

char *binToHex(char *data) {
    std::ostringstream strHex;
    int size = strlen(data);
    strHex << std::hex << std::setfill('0');
    for (int i = 0; i < size; ++i) {
        strHex << std::setw(2) << static_cast < unsigned int >(data[i]);
    }
    std::string str = strHex.str();
    return strupr(const_cast<char *>(str.c_str()));
}

char *hexToBin(const std::string & hex) {
    static std::vector < unsigned char >dest;
    unsigned int len = hex.size();
    dest.reserve( hex.size() / 2);
    for (int i = 0; i < len; i += 2) {
        unsigned int element;
        std::istringstream strHex(hex.substr(i, 2));
        strHex >> std::hex >> element;
        dest.push_back(static_cast <unsigned char>(element));
    }
    return reinterpret_cast<char *>(dest.data());
}

// 把字节码转为十六进制码，一个字节两个十六进制，内部为字符串分配空间
char* ByteToHex(const unsigned char* vByte, const int vLen) {
    if (!vByte)
        return NULL;
    char* tmp = new char[vLen * 2]; // 一个字节两个十六进制码，最后要多一个'/0'
    int tmp2;
    for (int i = 0; i < vLen; i++) {
        tmp2 = (int)(vByte[i]) / 16;
        tmp[i * 2] = (char)(tmp2 + ((tmp2 > 9) ? 'A' - 10 : '0'));
        tmp2 = (int)(vByte[i]) % 16;
        tmp[i * 2 + 1] = (char)(tmp2 + ((tmp2 > 9) ? 'A' - 10 : '0'));
    }
    tmp[vLen * 2 + 1] = { 0 };
    return tmp;
}

// 把十六进制字符串，转为字节码，每两个十六进制字符作为一个字节
unsigned char* HexToByte(const char* szHex){
    if (!szHex)
        return NULL;
    int iLen = strlen(szHex);
    if (iLen <= 0 || 0 != iLen % 2)
        return NULL;
    unsigned char* pbBuf = new unsigned char[iLen / 2];  // 数据缓冲区
    int tmp1, tmp2;
    for (int i = 0; i < iLen / 2; i++){
        tmp1 = (int)szHex[i * 2] - (((int)szHex[i * 2] >= 'a') ? 'a' - 10 : '0');
        if (tmp1 >= 16)
            return NULL;
        tmp2 = (int)szHex[i * 2 + 1] - (((int)szHex[i * 2 + 1] >= 'a') ? 'a' - 10 : '0');
        if (tmp2 >= 16)
            return NULL;
        pbBuf[i] = (tmp1 * 16 + tmp2);
    }

    return pbBuf;
}

/*
char* rc4_encryption(char* data, char* key) {
	data = RC4Base(data, key, strlen(data));
	char* mw = binToHex(data);
	return mw;
}


char* rc4_decrypt(const char* d, const char* key) {
	char* mw = hexToBin(d);
	char *data = RC4Base(mw, key, strlen(mw));
	return data;
}

*/
char __to_hex(int i){
    if (i>=0 && i<=9)
        return i+'0';
    else
        return i-10+'a';
}
int __un_hex(char c){
    if (c>='0' && c<='9')
        return c-'0';
    else
        return c-'a'+10;
}

// str是原来的字符串，len是字符串的长度，hex_str是转化后的字符串，且长度是len的两倍
void byte_2_hex(char str[], int len, char hex_str[]) {
    int i, t;
    for (i = 0; i < len; i++) {
        t = str[i] >> 4;
        hex_str[2*i] = __to_hex(t);
        t = str[i] & 0xf;
        hex_str[2*i+1] = __to_hex(t);
    }
}

// hex_str是十六进制的字符串，str是转化后的字符串，len是str的长度，且是hex_str长度的一半
void hex_2_byte(char hex_str[], char str[], int len) {
    int i, t;
    for (i = 0; i < len; i++) {
        t = __un_hex(hex_str[2*i]);
        str[i] = t << 4;
        t = __un_hex(hex_str[2*i+1]);
        str[i] += t;
    }
}



// 初始化密钥
void _init_key(char key[], int key_len, char key_state[SIZE]) {
    int i, j;
    char tmp;

    for (i = 0; i < SIZE; i++)
        key_state[i] = i;
    for (i = j = 0; i < SIZE; i++) {
        j = (j + key_state[i] + key[i%key_len]) % SIZE;
        tmp = key_state[i];
        key_state[i] = key_state[j];
        key_state[j] = tmp;
    }
}

// data是原文，stream是密文，假设两者长度相等
void _rc4_streamming(char key[], int key_len, char data[], int data_len, char stream[]) {
    int i, j, k, xor_index;
    char tmp;

    char *ks = (char*)malloc(SIZE);
    _init_key(key, key_len, ks);

    for (i = j = k = 0; k < data_len; k++) {
        i = (i + 1) % SIZE;
        j = (j + ks[i]) % SIZE;
        tmp = ks[i];
        ks[i] = ks[j];
        ks[j] = tmp;
        xor_index = (ks[i] + ks[j]) % SIZE;
        stream[k] = data[k] ^ ks[xor_index];
    }

    free(ks);
    ks = NULL;
}


// 加密，输入原字节，输出十六进制（返回的指针用完需要free）
string rc4_encryption(char data[], char key[]) {
    int data_len = strlen(data);
    int hex_data_len = data_len << 1;
    char *hex_data = (char*)malloc(hex_data_len);
    char *stream = (char*)malloc(data_len);
    _rc4_streamming(key, strlen(key), data, data_len, stream);
    byte_2_hex(stream, data_len, hex_data);
    free(stream);
    stream = NULL;
    return string(hex_data);
}

// 解密，输入十六进制，输出原字节（返回的指针用完需要free）
string rc4_decrypt(char hex_data[], char key[]) {
    str_normalize_init();
    int hex_data_len = strlen(hex_data);
    int data_len = hex_data_len >> 1;
    char *data = (char*)malloc(data_len);
    char *stream = (char*)malloc(data_len);
    hex_2_byte(hex_data, data, data_len);
    _rc4_streamming(key, strlen(key), data, data_len, stream);
    unsigned int utf8buffer_len = data_len * 3 + 1;
    char *utf8buffer  = (char *) malloc(utf8buffer_len);
    memset(utf8buffer , 0, utf8buffer_len);
    gbk_to_utf8(stream, data_len, utf8buffer, &utf8buffer_len);
    free(data);
    free(stream);
    stream = NULL;
    data = NULL;
    return string(utf8buffer);
}

char *strstrstr(const char *str, const char *front, const char *rear,bool isStatic) {
    static char StaticStr[512];
    if (!str)
        return NULL;

    /*
    const char *one = strstr(str,front);
    if(one == NULL)return NULL;
    int onelen = strlen(front);
    one += onelen;
    const char *two = strstr(one,rear);
    if(two == NULL)return NULL;
    onelen = two - one;
    char *newstr;
    if (isStatic)newstr = sstr;
    else {newstr = (char *)malloc(onelen + 1);if(newstr == NULL)return NULL;}
    strncpy(newstr, one, onelen);
    newstr[onelen] = '\0';
    return newstr;
*/

    const char *one;
    const char *two;
    const char *f;
    const char *r;

    if (front) //如果第一个查找参数为空则跳过查找
    {
        do {
            if (*str == '\0') //如果被查找参数结束则返回NULL
                return NULL;
            one = str;           //给第一个寄存器赋值为被查找参数
            str++;               //被查找参数向前推进
            f = front;           //给寄存器赋值为第一个查找参数
            while (*one == *f) //如果字符匹配则继续匹配下一个
            {
                if (*f == '\0') //如果第一个查找参数结束并且被查找字符串结束则查找失败
                    return NULL;
                one++; //继续匹配下一个
                f++;
            }
        } while (*f); //查找结束
        //此处one为截取结果头部指针
        str = one; //被查找参数移动到第一个查找参数后面
    } else
        one = str; //如果第一个查找参数为空则跳过查找

    if (rear == NULL || *rear == '\0')
        while (*str)
            str++;
    else {
        do {
            if (*str == '\0')
                return NULL;
            two = str;
            str++;
            r = rear;
            while (*two == *r) {
                if (*r == '\0')
                    break;
                two++;
                r++;
            }
        } while (*r);
        str--;
    }

    two = str;

    //two = strstr(one,rear);
    int onelen = two - one;
    char *newstr;
    if (isStatic)
        newstr = StaticStr;
    else if ((newstr = (char *) malloc(onelen + 1)) == NULL)
        return NULL;
    strncpy(newstr, one, onelen);
    newstr[onelen] = '\0';
    return newstr;
}
