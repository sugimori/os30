int strcmp(char *s1, char *s2)
{
    while(*s1 != 0 || *s2 != 0) {
        if(*s1 > *s2) return 1;
        if(*s1 < *s2) return -1;
        s1++;
        s2++;
    }
    return 0;
}

int strncmp(char *s1, char *s2, int n)
{
    int i;
    for(i=0;i<n;i++) {
        if(s1[i] > s2[i]) return 1;
        if(s1[i] < s2[i]) return -1;
        if(s1[i] == 0 && s2[i] == 0) return 0;
    }
    return 0;
}

int strlen(char *s)
{
    int i = 0;
    while(*s != 0) {
        i++;
        s++;
    }
    return i;
}