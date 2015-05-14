#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "osini.h"

void __mg_rewind (FILE *fp)
{
		rewind (fp);
}

FILE* __mg_tmpfile (void)
{
		return fopen ("/tmp/mg-etc-tmp", "w+");
}

int __mg_close_tmpfile (FILE *tmp_fp)
{
		return fclose (tmp_fp);
}

static struct FIXSTR {
		unsigned char bitmap[LEN_BITMAP];
		int offset[NR_HEAP];
		char* heap[NR_HEAP + 1];
		pthread_mutex_t lock;
} FixStrHeap;

/****************** Module functions *****************************************/
int InitFixStr (void)
{
		int i, j, offset;
		unsigned char* bitmap;

		// allocate memory.
		if (!(FixStrHeap.heap [0] = malloc (MAX_LEN_FIXSTR * 8 * NR_HEAP))) return FALSE;

		for (i = 1; i <= NR_HEAP; i++) {
				FixStrHeap.heap[i] = FixStrHeap.heap[0] + MAX_LEN_FIXSTR * 8 * i;
		}

		// reset bitmap
		bitmap = FixStrHeap.bitmap;
		offset = 0;
		for (i = 0; i < NR_HEAP; i++) {
				for (j = 0; j < (1<<i); j ++)
						bitmap[j] |= 0xFF;

				bitmap += 1<<i;

				FixStrHeap.offset[i] = offset;

				offset += 1<<i;
		}

		pthread_mutex_init (&FixStrHeap.lock, NULL);
		return TRUE;
}

void TerminateFixStr (void)
{
		pthread_mutex_destroy (&FixStrHeap.lock);
		free (FixStrHeap.heap[0]);
}

static char zero_string [] = {'\0'};

char*  FixStrAlloc (int len)
{
		unsigned int ulen = (unsigned int)len;
		int i, j, btlen, bufflen;
		char* heap;
		unsigned char* bitmap;

		if (len < 0)
				return NULL;

		if (len == 0)
				return zero_string;

		if (len >= MAX_LEN_FIXSTR)
				return (char*)malloc (len + 1);

		// determine which heap will use.
		i = 0;
		while (ulen) {
				ulen = ulen >> 1;
				i++;
		}

		// if 2 > len >= 1, then i = 1.
		// if 4 > len >= 2, then i = 2.
		// if 8 > len >= 4, then i = 3;
		// ...
		// if 512 > len >= 256, then i = 9;
		// ...
		// if 2K > len >= 1K, then i = 11;
		if (i == 1) i = 2;
		bufflen = 1 << i;

		i = NR_HEAP + 1 - i;
		// i is the heap index;
		// if i == 7; then bufflen = 4
		// if i == 6; then bufflen = 8
		// ..
		// if i == 0; then bufflen = 512

		pthread_mutex_lock (&FixStrHeap.lock);

		heap = FixStrHeap.heap[i];
		bitmap = FixStrHeap.bitmap + FixStrHeap.offset[i];
		btlen = 1 << i;

		for (i = 0; i < btlen; i++) {
				for(j = 0; j < 8; j++) {
						if (*bitmap & (0x80 >> j)) {
								*bitmap &= (~(0x80 >> j));
								pthread_mutex_unlock (&FixStrHeap.lock);
#if 0
								printf ("FixStrAlloc, len: %d, heap: %p.\n", len, heap);
#endif
								return heap;
						}

						heap += bufflen;
				}

				bitmap++;
		}

		pthread_mutex_unlock (&FixStrHeap.lock);
		return (char*)malloc (len + 1);
}


void  FreeFixStr (char* str)
{
		char* heap;
		unsigned char* bitmap;
		int i;
		int bufflen;
		int stroff;

		if (str [0] == '\0')
				return;

		if (str >= FixStrHeap.heap [NR_HEAP] || str < FixStrHeap.heap [0]) {
				free (str);
				return;
		}

		for (i = 1; i <= NR_HEAP; i++) {
				if (str < FixStrHeap.heap[i])
						break;
		}
		i--;
		// i is the heap index;

		// if i == 7; then bufflen = 4
		// if i == 6; then bufflen = 8
		// ..
		// if i == 0; then bufflen = 512
		bufflen = 1 << (NR_HEAP + 1 - i);

		pthread_mutex_lock (&FixStrHeap.lock);

		heap = FixStrHeap.heap[i];
		bitmap = FixStrHeap.bitmap + FixStrHeap.offset[i];

		// locate the bitmap and reset the bit.
		stroff = 0;
		while (str != heap) {
				heap += bufflen;
				stroff ++;
		}

#if 0
		printf ("FreeFixStr, len: %d, str: %p: heap: %p.\n", strlen (str), str, heap);
#endif

		bitmap = bitmap + (stroff>>3);
		*bitmap |= (0x80 >> (stroff%8));

		pthread_mutex_unlock (&FixStrHeap.lock);
}

char*  FixStrDup (const char* str)
{
		char* buff;

		if (str == NULL)
				return NULL;

		buff = FixStrAlloc (strlen (str));

		if (buff) {
				strcpy (buff, str);
		}

		return buff;
}

FILE* __mg_tmpfile (void);
int __mg_close_tmpfile (FILE *tmp_fp);
void __mg_rewind (FILE *fp);

/****************************** ETC file support ******************************/

static char* get_section_name (char *section_line)
{
		char* current;
		char* name;

		if (!section_line)
				return NULL;

		current = section_line;

		while (*current == ' ' ||  *current == '\t') current++;

		if (*current == ';' || *current == '#')
				return NULL;

		if (*current++ == '[')
				while (*current == ' ' ||  *current == '\t') current ++;
		else
				return NULL;

		name = current;
		while (*current != ']' && *current != '\n' &&
						*current != ';' && *current != '#' && *current != '\0')
				current++;
		*current = '\0';
		while (*current == ' ' || *current == '\t') {
				*current = '\0';
				current--;
		}

		return name;
}

static int get_key_value (char *key_line, char **mykey, char **myvalue)
{
		char* current;
		char* tail;
		char* value;

		if (!key_line)
				return -1;

		current = key_line;

		while (*current == ' ' ||  *current == '\t') current++;

		if (*current == ';' || *current == '#')
				return -1;

		if (*current == '[')
				return 1;

		if (*current == '\n' || *current == '\0')
				return -1;

		tail = current;
		while (*tail != '=' && *tail != '\n' &&
						*tail != ';' && *tail != '#' && *tail != '\0')
				tail++;

		value = tail + 1;
		if (*tail != '=')
				*value = '\0';

		*tail-- = '\0';
		while (*tail == ' ' || *tail == '\t') {
				*tail = '\0';
				tail--;
		}

		tail = value;
		while (*tail != '\n' && *tail != '\0') tail++;
		*tail = '\0';

		if (mykey)
				*mykey = current;
		if (myvalue)
				*myvalue = value;

		return 0;
}


/* This function locate the specified section in the etc file. */
static int etc_LocateSection(FILE* fp, const char* pSection, FILE* bak_fp)
{
		char szBuff[ETC_MAXLINE + 1];
		char *name;

		while (TRUE) {
				if (!fgets(szBuff, ETC_MAXLINE, fp)) {
						if (feof (fp))
								return ETC_SECTIONNOTFOUND;
						else
								return ETC_FILEIOFAILED;
				}
				else if (bak_fp && fputs (szBuff, bak_fp) == EOF)
						return ETC_FILEIOFAILED;

				name = get_section_name (szBuff);
				if (!name)
						continue;

				if (strcmp (name, pSection) == 0)
						return ETC_OK;
		}

		return ETC_SECTIONNOTFOUND;
}

/* This function locate the specified key in the etc file. */
static int etc_LocateKeyValue(FILE* fp, const char* pKey,
				int bCurSection, char* pValue, int iLen,
				FILE* bak_fp, char* nextSection)
{
		char szBuff[ETC_MAXLINE + 1 + 1];
		char* current;
		char* value;
		int ret;

		while (TRUE) {
				int bufflen;

				if (!fgets(szBuff, ETC_MAXLINE, fp))
						return ETC_FILEIOFAILED;
				bufflen = strlen (szBuff);
				if (szBuff [bufflen - 1] == '\n')
						szBuff [bufflen - 1] = '\0';

				ret = get_key_value (szBuff, &current, &value);
				if (ret < 0)
						continue;
				else if (ret > 0) {
						fseek (fp, -bufflen, SEEK_CUR);
						return ETC_KEYNOTFOUND;
				}

				if (strcmp (current, pKey) == 0) {
						if (pValue)
								strncpy (pValue, value, iLen);

						return ETC_OK;
				}
				else if (bak_fp && *current != '\0') {
						fprintf (bak_fp, "%s=%s\n", current, value);
				}
		}

		return ETC_KEYNOTFOUND;
}

static PETCSECTION etc_NewSection (ETC_S* petc)
{
		PETCSECTION psect;

		if (petc->section_nr == petc->sect_nr_alloc) {
				/* add 5 sections each time we realloc */
				petc->sect_nr_alloc += NR_SECTS_INC_ALLOC;
				petc->sections = realloc (petc->sections,
								sizeof (ETCSECTION)*petc->sect_nr_alloc);
		}
		psect = petc->sections + petc->section_nr;

		psect->name = NULL;

		petc->section_nr ++;

		return psect;
}

static void etc_NewKeyValue (PETCSECTION psect,
				const char* key, const char* value)
{
		if (psect->key_nr == psect->key_nr_alloc) {
				psect->key_nr_alloc += NR_KEYS_INC_ALLOC;
				psect->keys = realloc (psect->keys,
								sizeof (char*) * psect->key_nr_alloc);
				psect->values = realloc (psect->values,
								sizeof (char*) * psect->key_nr_alloc);
		}

		psect->keys [psect->key_nr] = FixStrDup (key);
		psect->values [psect->key_nr] = FixStrDup (value);
		psect->key_nr ++;
}

static int etc_ReadSection (FILE* fp, PETCSECTION psect)
{
		char szBuff[ETC_MAXLINE + 1 + 1];
		char* sect_name;

		psect->name = NULL;
		psect->key_nr = 0;
		psect->keys = NULL;
		psect->values = NULL;

		while (TRUE) {
				int bufflen;

				if (!fgets(szBuff, ETC_MAXLINE, fp)) {
						if (feof (fp)) {
								if (psect->name)
										break;
								else
										return ETC_SECTIONNOTFOUND;
						}
						else
								return ETC_FILEIOFAILED;
				}

				bufflen = strlen (szBuff);
				if (szBuff [bufflen - 1] == '\n')
						szBuff [bufflen - 1] = '\0';

				if (!psect->name) { /* read section name */
						sect_name = get_section_name (szBuff);
						if (!sect_name)
								continue;

						psect->name = FixStrDup (sect_name);
						psect->key_nr = 0;
						psect->key_nr_alloc = NR_KEYS_INIT_ALLOC;
						psect->keys = malloc (sizeof (char*) * NR_KEYS_INIT_ALLOC);
						psect->values = malloc (sizeof (char*) * NR_KEYS_INIT_ALLOC);
				}
				else { /* read key and value */
						int ret;
						char *key, *value;

						ret = get_key_value (szBuff, &key, &value);
						if (ret < 0)
								continue;
						else if (ret > 0) {  /* another section begins */
								fseek (fp, -bufflen, SEEK_CUR);
								break;
						}

						etc_NewKeyValue (psect, key, value);
				}
		}

		return ETC_OK;
}

ETC_HANDLE  LoadEtcFile (const char * pEtcFile)
{
		FILE* fp = NULL;
		ETC_S *petc;

		if (pEtcFile && !(fp = fopen (pEtcFile, "r")))
				return 0;

		petc = (ETC_S*) malloc (sizeof(ETC_S));
		petc->section_nr = 0;
		petc->sect_nr_alloc = NR_SECTS_INIT_ALLOC;

		/* we allocate 15 sections first */
		petc->sections =
				(PETCSECTION) malloc (sizeof(ETCSECTION)*NR_SECTS_INIT_ALLOC);

		if (pEtcFile == NULL) { /* return an empty etc object */
				return (ETC_HANDLE) petc;
		}

		while (etc_ReadSection (fp, petc->sections + petc->section_nr) == ETC_OK) {
				petc->section_nr ++;
				if (petc->section_nr == petc->sect_nr_alloc) {
						/* add 5 sections each time we realloc */
						petc->sect_nr_alloc += NR_SECTS_INC_ALLOC;
						petc->sections = realloc (petc->sections,
										sizeof(ETCSECTION)*petc->sect_nr_alloc);
				}
		}

		fclose (fp);
		return (ETC_HANDLE)petc;
}

int  UnloadEtcFile (ETC_HANDLE hEtc)
{
		int i;
		ETC_S *petc = (ETC_S*) hEtc;

		if (!petc)
				return -1;

		for (i=0; i<petc->section_nr; i++) {
				PETCSECTION psect = petc->sections + i;
				int j;

				if (!psect->name)
						continue;

				for (j=0; j<psect->key_nr; j++) {
						FreeFixStr (psect->keys [j]);
						FreeFixStr (psect->values [j]);
				}
				free (psect->keys);
				free (psect->values);
				FreeFixStr (psect->name);
		}

		free (petc->sections);
		free (petc);

		return 0;
}

static int etc_GetSectionValue (PETCSECTION psect, const char* pKey,
				char* pValue, int iLen)
{
		int i;

		for (i=0; i<psect->key_nr; i++) {
				if (strcmp (psect->keys [i], pKey) == 0) {
						break;
				}
		}

		if (iLen > 0) { /* get value */
				if (i >= psect->key_nr)
						return ETC_KEYNOTFOUND;

				strncpy (pValue, psect->values [i], iLen);
		}
		else { /* set value */
				if (psect->key_nr_alloc <= 0)
						return ETC_READONLYOBJ;

				if (i >= psect->key_nr) {
						etc_NewKeyValue (psect, pKey, pValue);
				}
				else {
						FreeFixStr (psect->values [i]);
						psect->values [i] = FixStrDup (pValue);
				}
		}

		return ETC_OK;
}

int  GetValueFromEtc (ETC_HANDLE hEtc, const char* pSection,
				const char* pKey, char* pValue, int iLen)
{
		int i, empty_section = -1;
		ETC_S *petc = (ETC_S*) hEtc;
		PETCSECTION psect = NULL;

		if (!petc || !pValue)
				return -1;

		for (i=0; i<petc->section_nr; i++) {
				psect = petc->sections + i;
				if (!psect->name) {
						empty_section = i;
						continue;
				}

				if (strcmp (psect->name, pSection) == 0) {
						break;
				}
		}

		if (i >= petc->section_nr) {
				if (iLen > 0)
						return ETC_SECTIONNOTFOUND;
				else {
						if (petc->sect_nr_alloc <= 0)
								return ETC_READONLYOBJ;

						if (empty_section >= 0)
								psect = petc->sections + empty_section;
						else {
								psect = etc_NewSection (petc);
						}

						if (psect->name == NULL) {
								psect->key_nr = 0;
								psect->name = FixStrDup (pSection);
								psect->key_nr_alloc = NR_KEYS_INIT_ALLOC;
								psect->keys = malloc (sizeof (char* ) * NR_KEYS_INIT_ALLOC);
								psect->values = malloc (sizeof (char* ) * NR_KEYS_INIT_ALLOC);
						}
				}
		}

		return etc_GetSectionValue (psect, pKey, pValue, iLen);
}

int  GetIntValueFromEtc (ETC_HANDLE hEtc, const char* pSection,
				const char* pKey, int* value)
{
		int ret;
		char szBuff [51];

		ret = GetValueFromEtc (hEtc, pSection, pKey, szBuff, 50);
		if (ret < 0) {
				return ret;
		}

		*value = strtol (szBuff, NULL, 0);
		if ((*value == LONG_MIN || *value == LONG_MAX) && errno == ERANGE)
				return ETC_INTCONV;

		return ETC_OK;
}

/* Function: GetValueFromEtcFile(const char* pEtcFile, const char* pSection,
 *                               const char* pKey, char* pValue, int iLen);
 * Parameter:
 *     pEtcFile: etc file path name.
 *     pSection: Section name.
 *     pKey:     Key name.
 *     pValue:   The buffer will store the value of the key.
 *     iLen:     The max length of value string.
 * Return:
 *     int               meaning
 *     ETC_FILENOTFOUND           The etc file not found.
 *     ETC_SECTIONNOTFOUND        The section is not found.
 *     ETC_EKYNOTFOUND        The Key is not found.
 *     ETC_OK            OK.
 */
int  GetValueFromEtcFile(const char* pEtcFile, const char* pSection,
				const char* pKey, char* pValue, int iLen)
{
		FILE* fp;
		char tempSection [ETC_MAXLINE + 2];

		if (!(fp = fopen(pEtcFile, "r")))
				return ETC_FILENOTFOUND;

		if (pSection)
				if (etc_LocateSection (fp, pSection, NULL) != ETC_OK) {
						fclose (fp);
						return ETC_SECTIONNOTFOUND;
				}

		if (etc_LocateKeyValue (fp, pKey, pSection != NULL,
								pValue, iLen, NULL, tempSection) != ETC_OK) {
				fclose (fp);
				return ETC_KEYNOTFOUND;
		}

		fclose (fp);
		return ETC_OK;
}

/* Function: GetIntValueFromEtcFile(const char* pEtcFile, const char* pSection,
 *                               const char* pKey);
 * Parameter:
 *     pEtcFile: etc file path name.
 *     pSection: Section name.
 *     pKey:     Key name.
 * Return:
 *     int                      meaning
 *     ETC_FILENOTFOUND             The etc file not found.
 *     ETC_SECTIONNOTFOUND          The section is not found.
 *     ETC_EKYNOTFOUND              The Key is not found.
 *     ETC_OK                       OK.
 */
int  GetIntValueFromEtcFile (const char* pEtcFile, const char* pSection,
				const char* pKey, int* value)
{
		int ret;
		char szBuff [51];

		ret = GetValueFromEtcFile (pEtcFile, pSection, pKey, szBuff, 50);
		if (ret < 0)
				return ret;

		*value = strtol (szBuff, NULL, 0);
		if ((*value == LONG_MIN || *value == LONG_MAX) && errno == ERANGE)
				return ETC_INTCONV;

		return ETC_OK;
}

static int etc_CopyAndLocate (FILE* etc_fp, FILE* tmp_fp,
				const char* pSection, const char* pKey, char* tempSection)
{
		if (pSection && etc_LocateSection (etc_fp, pSection, tmp_fp) != ETC_OK)
				return ETC_SECTIONNOTFOUND;

		if (etc_LocateKeyValue (etc_fp, pKey, pSection != NULL,
								NULL, 0, tmp_fp, tempSection) != ETC_OK)
				return ETC_KEYNOTFOUND;

		return ETC_OK;
}

static int etc_FileCopy (FILE* sf, FILE* df)
{
		char line [ETC_MAXLINE + 1];

		while (fgets (line, ETC_MAXLINE + 1, sf) != NULL)
				if (fputs (line, df) == EOF) {
						return ETC_FILEIOFAILED;
				}

		return ETC_OK;
}

/* Function: SetValueToEtcFile(const char* pEtcFile, const char* pSection,
 *                               const char* pKey, char* pValue);
 * Parameter:
 *     pEtcFile: etc file path name.
 *     pSection: Section name.
 *     pKey:     Key name.
 *     pValue:   Value.
 * Return:
 *     int                      meaning
 *     ETC_FILENOTFOUND         The etc file not found.
 *     ETC_TMPFILEFAILED        Create tmp file failure.
 *     ETC_OK                   OK.
 */
int  SetValueToEtcFile (const char* pEtcFile, const char* pSection,
				const char* pKey, char* pValue)
{
		FILE* etc_fp;
		FILE* tmp_fp;
		int rc;
		char tempSection [ETC_MAXLINE + 2] = {0};

		if ((tmp_fp = __mg_tmpfile ()) == NULL)
				return ETC_TMPFILEFAILED;

		if (!(etc_fp = fopen (pEtcFile, "r+"))) {
				__mg_close_tmpfile (tmp_fp);

				if (!(etc_fp = fopen (pEtcFile, "w"))) {
						return ETC_FILEIOFAILED;
				}
				fprintf (etc_fp, "[%s]\n", pSection);
				fprintf (etc_fp, "%s=%s\n", pKey, pValue);
				fclose (etc_fp);
				return ETC_OK;
		}

		switch (etc_CopyAndLocate (etc_fp, tmp_fp, pSection, pKey, tempSection)) {
				case ETC_SECTIONNOTFOUND:
						fprintf (tmp_fp, "\n[%s]\n", pSection);
						fprintf (tmp_fp, "%s=%s\n", pKey, pValue);
						break;

				case ETC_KEYNOTFOUND:
						fprintf (tmp_fp, "%s=%s\n", pKey, pValue);
						fprintf (tmp_fp, "%s\n", tempSection);
						break;

				default:
						fprintf (tmp_fp, "%s=%s\n", pKey, pValue);
						break;
		}

		if ((rc = etc_FileCopy (etc_fp, tmp_fp)) != ETC_OK)
				goto error;

		// replace etc content with tmp file content
		// truncate etc content first
		fclose (etc_fp);
		if (!(etc_fp = fopen (pEtcFile, "w"))) {
				__mg_close_tmpfile (tmp_fp);
				return ETC_FILEIOFAILED;
		}

		__mg_rewind (tmp_fp);
		rc = etc_FileCopy (tmp_fp, etc_fp);

error:
		fclose (etc_fp);
		__mg_close_tmpfile (tmp_fp);
		return rc;
}

static int etc_WriteSection (FILE* fp, PETCSECTION psect, int bSectName)
{
		int i;

		if (psect->name == NULL)
				return ETC_OK;

		if (bSectName)
				fprintf (fp, "[%s]\n", psect->name);

		for (i = 0; i < psect->key_nr; i++) {
				fprintf (fp, "%s=%s\n", psect->keys [i], psect->values [i]);
		}

		if (fprintf (fp, "\n") != 1)
				return ETC_FILEIOFAILED;

		return ETC_OK;
}

int  SaveEtcToFile (ETC_HANDLE hEtc, const char* file_name)
{
		int i;
		FILE* fp;
		ETC_S *petc = (ETC_S*) hEtc;

		if (petc == NULL)
				return ETC_INVALIDOBJ;

		if (!(fp = fopen (file_name, "w"))) {
				return ETC_FILEIOFAILED;
		}

		for (i = 0; i < petc->section_nr; i++) {
				if (etc_WriteSection (fp, petc->sections + i, TRUE)) {
						fclose (fp);
						return ETC_FILEIOFAILED;
				}
		}

		fclose (fp);
		return ETC_OK;
}

ETC_HANDLE  FindSectionInEtc (ETC_HANDLE hEtc,
				const char* pSection, int bCreateNew)
{
		int i, empty_section = -1;
		ETC_S *petc = (ETC_S*) hEtc;
		ETCSECTION* psect = NULL;

		if (petc == NULL || pSection == NULL)
				return 0;

		for (i = 0; i < petc->section_nr; i++) {
				if (petc->sections [i].name == NULL) {
						empty_section = i;
				}
				else if (strcmp (petc->sections [i].name, pSection) == 0)
						return (ETC_HANDLE) (petc->sections + i);
		}

		/* not found */
		if (bCreateNew) {
				if (petc->sect_nr_alloc <= 0)
						return ETC_READONLYOBJ;

				if (empty_section >= 0) {
						psect = petc->sections + empty_section;
				}
				else {
						psect = etc_NewSection (petc);
				}

				if (psect->name == NULL) {
						psect->key_nr = 0;
						psect->name = FixStrDup (pSection);
						psect->key_nr_alloc = NR_KEYS_INIT_ALLOC;
						psect->keys = malloc (sizeof (char*) * NR_KEYS_INIT_ALLOC);
						psect->values = malloc (sizeof (char*) * NR_KEYS_INIT_ALLOC);
				}
		}

		return (ETC_HANDLE)psect;
}

int  GetValueFromEtcSec (ETC_HANDLE hSect,
				const char* pKey, char* pValue, int iLen)
{
		PETCSECTION psect = (PETCSECTION)hSect;

		if (psect == NULL)
				return ETC_INVALIDOBJ;

		return etc_GetSectionValue (psect, pKey, pValue, iLen);
}

int  GetIntValueFromEtcSec (ETC_HANDLE hSect, const char* pKey, int* pValue)
{
		PETCSECTION psect = (PETCSECTION)hSect;
		int ret;
		char szBuff [51];

		if (psect == NULL)
				return ETC_INVALIDOBJ;

		ret = etc_GetSectionValue (psect, pKey, szBuff, 50);
		if (ret < 0) {
				return ret;
		}

		*pValue = strtol (szBuff, NULL, 0);
		if ((*pValue == LONG_MIN || *pValue == LONG_MAX) && errno == ERANGE)
				return ETC_INTCONV;

		return ETC_OK;
}

int  SetValueToEtcSec (ETC_HANDLE hSect, const char* pKey, char* pValue)
{
		PETCSECTION psect = (PETCSECTION)hSect;

		if (psect == NULL)
				return ETC_INVALIDOBJ;

		return etc_GetSectionValue (psect, pKey, pValue, -1);
}

int  RemoveSectionInEtc (ETC_HANDLE hEtc, const char* pSection)
{
		int i;
		PETCSECTION psect;

		if (hEtc == 0)
				return ETC_INVALIDOBJ;

		if (((ETC_S*)hEtc)->sect_nr_alloc == 0)
				return ETC_READONLYOBJ;

		psect = (PETCSECTION) FindSectionInEtc (hEtc, pSection, FALSE);
		if (psect == NULL)
				return ETC_SECTIONNOTFOUND;

		for (i = 0; i < psect->key_nr; i ++) {
				FreeFixStr (psect->keys [i]);
				FreeFixStr (psect->values [i]);
		}
		free (psect->keys);
		free (psect->values);
		FreeFixStr (psect->name);

		psect->key_nr = 0;
		psect->name = NULL;
		psect->keys = NULL;
		psect->values = NULL;

		return ETC_OK;
}

/* This function locate the specified section in the etc file. */
static int etc_LocateSection_2 (FILE* fp, const char* pSection, FILE* bak_fp)
{
		char szBuff[ETC_MAXLINE + 1];
		char line[ETC_MAXLINE + 1];
		char *name;

		while (TRUE) {
				if (!fgets(line, ETC_MAXLINE, fp)) {
						if (feof (fp))
								return ETC_SECTIONNOTFOUND;
						else
								return ETC_FILEIOFAILED;
				}

				strcpy (szBuff, line);
				name = get_section_name (szBuff);

				if (name && strcmp (name, pSection) == 0)
						return ETC_OK;
				else if (bak_fp && fputs (line, bak_fp) == EOF)
						return ETC_FILEIOFAILED;
		}

		return ETC_SECTIONNOTFOUND;
}

static int etc_FindNextSection (FILE* fp)
{
		char szBuff[ETC_MAXLINE + 1];
		char *name;

		while (TRUE) {
				int len;
				if (!fgets(szBuff, ETC_MAXLINE, fp)) {
						if (feof (fp))
								return ETC_OK;
						else
								return ETC_FILEIOFAILED;
				}

				len = strlen(szBuff);
				name = get_section_name (szBuff);
				if (name) {
						fseek (fp, -len, SEEK_CUR);
						break;
				}
		}

		return ETC_OK;
}

static int etc_ReviseSectionInEtcFile (const char* pEtcFile, PETCSECTION psect,
				int bRemove)
{
		FILE* etc_fp;
		FILE* tmp_fp;
		int rc;

		if ((tmp_fp = __mg_tmpfile ()) == NULL)
				return ETC_TMPFILEFAILED;

		/* open or create a etc file */
		if (!(etc_fp = fopen (pEtcFile, "r+"))) {
				__mg_close_tmpfile (tmp_fp);

				if (bRemove) {
						rc = ETC_SECTIONNOTFOUND;
				}
				else {
						if (!(etc_fp = fopen (pEtcFile, "w"))) {
								return ETC_FILEIOFAILED;
						}

						rc = etc_WriteSection (etc_fp, psect, TRUE);
						fclose (etc_fp);
				}

				return rc;
		}

		rc = etc_LocateSection_2 (etc_fp, psect->name, tmp_fp);

		if (rc == ETC_SECTIONNOTFOUND) {
				__mg_close_tmpfile (tmp_fp);
				if (!bRemove) {
						etc_WriteSection (etc_fp, psect, TRUE);
						fclose (etc_fp);
						return ETC_OK;
				}
				else
						return ETC_SECTIONNOTFOUND;
		}
		else if (rc == ETC_FILEIOFAILED) {
				__mg_close_tmpfile (tmp_fp);
				fclose (etc_fp);
				return ETC_FILEIOFAILED;
		}

		if(!bRemove) {
				etc_WriteSection (tmp_fp, psect, TRUE);
		}

		etc_FindNextSection (etc_fp);

		if ((rc = etc_FileCopy (etc_fp, tmp_fp)) != ETC_OK)
				goto error;

		// replace etc content with tmp file content
		// truncate etc content first
		fclose (etc_fp);

		if (!(etc_fp = fopen (pEtcFile, "w"))) {
				__mg_close_tmpfile (tmp_fp);
				return ETC_FILEIOFAILED;
		}

		__mg_rewind (tmp_fp);
		rc = etc_FileCopy (tmp_fp, etc_fp);

error:
		fclose (etc_fp);
		__mg_close_tmpfile (tmp_fp);
		return rc;
}

int  RemoveSectionInEtcFile (const char* pEtcFile, const char *pSection)
{
		ETCSECTION sect;

		sect.name = (char *)pSection;
		sect.key_nr = 0;

		return etc_ReviseSectionInEtcFile (pEtcFile, &sect, TRUE);
}

int  SaveSectionToEtcFile (const char* pEtcFile, PETCSECTION psect)
{
		return etc_ReviseSectionInEtcFile (pEtcFile, psect, FALSE);
}
