bool matchStr(const char* str1, const char* str2) {
    if (!str1 || !str2)
        return false;

    char c1, c2;
    do {
        c1 = *str1++;
        c2 = *str2++;
        if (c1 != c2) return false;
    } while (c1 && c2);

    return true;
}
