void lowerString(char* orig, char* ret, int strSize){
    int i=0;
    while (*(orig+i) && i<strSize-1){
        *(ret+i) = *(orig+i);
        if (*(orig+i) >= 'A' && *(orig+i) <= 'Z')
            *(ret+i) += 0x20;
        i++;
    }
    *(ret+i) = 0;
}
