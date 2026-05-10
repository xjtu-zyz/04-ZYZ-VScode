int sum(int a[], int n)
{
    int i;
    int s;
    i = 0;
    s = 0;
    while (i < n)
    {
        s = s + a[i];
        i = i + 1;
    }
    return s;
}
