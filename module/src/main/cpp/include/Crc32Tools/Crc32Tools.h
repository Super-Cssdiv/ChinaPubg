#pragma once

int CRC32Edit(unsigned char *byt, int bytLen, int lOffset, int lcrc, int &retlCRC);

int editor(const char *file, int lcrc);

int allEditor(const char *path);