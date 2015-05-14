#ifndef ASC_TABLE_H
#define ASC_TABLE_H

#define ASC_SIZE 16

void ASC_GetAscFont(const unsigned char a_ucAsc, unsigned char *a_aucFont);
void ASC_DrawAscString(int a_row, int a_column, int fontColSize,const unsigned char *a_aucString);

#endif