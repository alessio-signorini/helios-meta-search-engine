#include <stdio.h>
#include <stdlib.h>																								// per la malloc
#include <string.h>																								// per manipolare stringhe
#include <fcntl.h>																								// per fcntl
#include <sys/socket.h>

#include "macro.h"





// strrpl ---------------------------------------------------------------------
// Return  a pointer to a new string equal to the given STR string but where is
// replaced CUT with PASTE
// ----------------------------------------------------------------------------
char *strrpl(char *str, char *cut, char *paste) {
	char paste_length, cut_length;
	short cut_index, newstr_length, tail_length;
	char *cut_ptr;
	char *newstr;

	cut_ptr = strstr(str, cut);
	if (cut_ptr==NULL) return strdup(str);
	paste_length = strlen(paste);
	cut_length = strlen(cut);
	cut_index = cut_ptr - str;
	tail_length = strlen(cut_ptr) - strlen(cut);
	newstr_length = strlen(str) - strlen(cut) + strlen(paste) + 1;
	
	newstr = malloc(newstr_length); bzero(newstr, newstr_length);
	strncpy(newstr, str, cut_index);
	strcat(newstr, paste);
	strcat(newstr, cut_ptr + cut_length);
	
	#ifdef STRING_REPLACE_DEBUG
	printf("Input String : %s",str);
	printf("Output String: %s",newstr);
	#endif
	
	return newstr;
}
// ----------------------------------------------------------------------------





// cmalloc --------------------------------------------------------------------
// Allocate SIZE bytes of memory and reset it
// ----------------------------------------------------------------------------
void *cmalloc(int size) {
	void *t;
	t = malloc(size);																								// allocate memory
	bzero(t, size);																								// reset it
	return t;																										// return the pointer to it
}
// ----------------------------------------------------------------------------






// strndup --------------------------------------------------------------------
// Return  a  pointer  to a new string composed by first LEN chars of the given
// STR string.
// ----------------------------------------------------------------------------
char *strndup(char *str, int len) {
	char *tmp;
	
	tmp = cmalloc(len+1);
	strncpy(tmp,str,len);
	return tmp;
}
// ----------------------------------------------------------------------------






// itoa -----------------------------------------------------------------------
// Return a pointer to a new string containing the given N number
// ----------------------------------------------------------------------------
char *itoa(int n) {
	char *tmp;
	char i=1;
	int x=n;
	
	while (x/=10) i++;																							// calc the number's digit
	tmp = malloc(i+1);
	sprintf(tmp,"%d",n);
	return tmp;
}
// ----------------------------------------------------------------------------






// ----------------------------------------------------------------------------
char *strstr2(char *str, char *sub1, char *sub2) {
	char *t1, *t2;
	t1 = strstr(str, sub1);
	t2 = strstr(str, sub2);
	if (t2<t1) return t2;
	return t1;
}
// ----------------------------------------------------------------------------






// ----------------------------------------------------------------------------
char *join2(char *str1, char *str2) {
	char *tmp;
	short len=strlen(str1)+strlen(str2)+1;
	
	tmp = cmalloc(len);
	strcpy(tmp,str1); strcat(tmp,str2);
	return tmp;
}
// ----------------------------------------------------------------------------





// ----------------------------------------------------------------------------
char *join3(char *str1, char *str2, char *str3) {
	char *tmp;
	short len=strlen(str1)+strlen(str2)+strlen(str3)+1;
	
	tmp = cmalloc(len);
	strcpy(tmp,str1); strcat(tmp,str2); strcat(tmp,str3);
	return tmp;
}
// ----------------------------------------------------------------------------






// strsep2 --------------------------------------------------------------------
// Quicker clone of standard function STRSEP (1/2 of the time)
// ----------------------------------------------------------------------------
char *strsep2(char **s, char c) {
	char *t = *s, *k;

	if (*s==NULL) return NULL;
	k = strchr(*s, c);
	if (k==NULL) *s=NULL;
		else k[0]='\0', *s = k+1;
	return t;
}
// ----------------------------------------------------------------------------






// strAttach ------------------------------------------------------------------
// If DST is empty simply associate *dst with a pointer to the SRC string. Else
// create  a new string composed by the given strings and associate it with the
// DST pointer, freeing previous allocated memory.
// ----------------------------------------------------------------------------
int strAttach(char **dst, char *src) {
	short len;
	char *t;
	
	if (*dst==NULL) {
		*dst = strdup(src);
		return 0;
	}
	len = strlen(*dst) + strlen(src) + 1;
	t = cmalloc(len);
	strcpy(t,*dst); strcat(t, src);
	free(*dst);
	*dst = t;
	return 1;
}
// ----------------------------------------------------------------------------






// chrrpl ---------------------------------------------------------------------
// Substitute all occurrency of char C with the char D in the given STR string
// ----------------------------------------------------------------------------
char chrrpl(char *str, char c, char d) {
	if (c==d) return 0;
	
	str = strchr(str, c);
	if (str==NULL) return 0;
	
	while (str!=NULL) {
		str[0]=d;
		str = strchr(str, c);
	}
	return 1;
}
// ----------------------------------------------------------------------------






// allocateString -------------------------------------------------------------
// Allocate the given string into memory and return a pointer to it
// ----------------------------------------------------------------------------
char *allocateString(char *str) {
	char *k;
	
	k = malloc(strlen(str)+1);
	strcpy(k,str);
	return k;
}
// ----------------------------------------------------------------------------






/*
// ----------------------------------------------------------------------------
char *createName(int n) {
	char *tmp, *num;
	char name[] = "buffer", ext[] = ".htm";
	
	num = itoa(n);
	tmp = malloc(strlen(name)+strlen(num)+strlen(ext)+1);
	bzero(tmp,sizeof(tmp));
	strcpy(tmp,name); strcat(tmp,num); strcat(tmp,ext);
	free(num);
	return tmp;
}
// ----------------------------------------------------------------------------







// ----------------------------------------------------------------------------
void printSetInfo(int c, short socket[], short engines, fd_set *rset) {
	short i;
	
	printf("c=%d",c);
	for (i=0; i<engines; i++)
		printf(", s%d=%d",i,FD_ISSET(socket[i],rset));
	printf("\n");
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
int maxfnd(int a[]) {
	int i, n, max=a[0];
	
	n = sizeof(a) / sizeof(a[0]);
	for (i=1; i<n; i++)
		if (a[i]>max) max=a[i];
	return max;
}
// ----------------------------------------------------------------------------
*/
